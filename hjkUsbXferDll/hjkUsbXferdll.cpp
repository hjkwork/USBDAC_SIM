#include "pch.h"
#include <stdlib.h>
#include <process.h>
#include <iostream>
#include <time.h>
#include "hjkUsbXferdll.h"
#include "HjkDataDefine.h"
#include "RingBufCPP.h"
#include "hjkDataFormat.h"
#include "HjkUSBDevice.h"
// DLL internal state variables:
static qnodeChInt32								inNode[IN_RQUEUE_LEN];
static qnodeChInt32								outNode[OUT_RQUEUE_LEN];
static RingBufCPP<qnodeChInt32, IN_RQUEUE_LEN>	inRQue;
static RingBufCPP<qnodeChInt32, OUT_RQUEUE_LEN>	outRQue;
static qnodeChInt32								*nodeIn;
static qnodeChInt32								*nodeOut;
static qnodeChInt32								*nodeInbuf;
static qnodeChInt32								*nodeOutbuf;
static __int32									*outIntBuf;
static __int32									*inData;
static int										chNum;
static bool										haveInit;
//��Ҫ��ʼ������
static UCHAR									sizeOfInt32;
static int										MaxPktSize;
static long										xferUSBDataSize;
//��Ҫ�����ʼ��������
static HjkDataFormat							hjkdataf;
//ͬ�����
//ֻ�л�ȡ���ݺ�in�̲߳�����ӣ������֮ǰ�����ݲ��ܱ䣬
static HANDLE									dataInEvent;
static HANDLE									dataInMutex;
//�����һ���ڵ��ʱ����pull��
static int										num;//����
//usb���
static string									hjkUSBdevices;
static HjkUSBDevice								hjkUSB;
static CCyBulkEndPoint							*hjkBulkOutEndpt;


static int xferIntNum;
//static OVERLAPPED								hjkOutOvLap[OUT_RQUEUE_LEN];
//static UCHAR									hjkOutContxt[OUT_RQUEUE_LEN];
//��������
//void queueBufStart(__int32 *allChIn1, int ch, __int32 *out);
void testIn(void *);
void testFormat(void *);
void testOut(void *);
void abortXfer(int pending);
/*
usb�豸��ʼ������
����usb�豸������豸��С��1�����˳�
�ȴ�ѡ��Ҫʹ�õ��豸
ѡ�������豸�����ã��˳�
���豸����
���ö˿ںʹ���֡��Ϣ
��ʼ����ɷ���
*/
long  initHjkUSBXfer(string usbdevices, int ppx)
{
	
	string endinfos;
	long len ;
	int slecd = -1;
	int n = hjkUSB.getHjkUSBDevice(&usbdevices);
	if (n > 0)
	{
#ifdef CMD
		cout << usbdevices;
		while (slecd < 0 || slecd >= n)
		{

			cout << "Slect one  hjkUSB device to xfer." << endl;
			cin >> slecd;
		}
#else 
		slecd = 0;
#endif
		if (hjkUSB.USBDevice->Open(slecd))
		{
			//��ȡ��˿ڣ�����������ֽ���
			hjkBulkOutEndpt = hjkUSB.USBDevice->BulkOutEndPt;

			if (hjkBulkOutEndpt == NULL)
			{
				cout << "The USB bulk end point is null, program exit." << endl;
				return 0;
			}
			//ÿ�δ�����ֽ���Ϊxferlen��������4m bytes��
			len = hjkBulkOutEndpt->MaxPktSize*ppx;
			if (len > 0x400000)
			{
				len = 0x400000;
			}
			hjkBulkOutEndpt->SetXferSize(len);
			return len;
		}
		else
		{
			cout << "USB device is not a hjkUSB device, program exit." << endl;
			return 0;
		}

	}
	else
	{
		cout << "USB device is 0, program exit,please check the USB device or driver."<<endl;
		return 0;
	}
	//

}
void initQueueBuf(int ch, int outBytes)
{
	haveInit = false;
	chNum = ch;
	inData = NULL;
	sizeOfInt32 = sizeof(__int32);
	//����USB���ø�ֵ
	
	xferUSBDataSize=initHjkUSBXfer(hjkUSBdevices, chNum);
	if (xferUSBDataSize == 0)
	{
		cout << "USB device init failed, progarma exit. " << endl;
		exit(1);
	}
	//hjkOutOvLap.hEvent = CreateEvent(NULL, false, false, L"hjk_bulk_out");

	//MaxPktSize = outBytes;
	//�������ݴ�����
	//hjkdataf = HjkDataFormat(ch, 0);

	//2.��ʼ���ڵ㻺��
	//*inNode = new qnodeChInt32;
	nodeInbuf = new qnodeChInt32;
	nodeOutbuf = new qnodeChInt32;


	//1.��ʼ������ڵ�
	for (int i = 0; i < IN_RQUEUE_LEN; i++)
	{
		//datalenһ��Ϊͨ����
		inNode[i].datalen = chNum ;
		inNode[i].data = (__int32 *)malloc(inNode[i].datalen*sizeOfInt32);
		
		//���
		inRQue.add(inNode[i]);
		
	}
	//��ն���
	for (int i = 0; i < IN_RQUEUE_LEN; i++)
		inRQue.pull(nodeInbuf);
	
	
	


	
	//outIntBuf = (__int32 *)malloc(MaxPktSize);

	


	//��ʼ�����
	for (int i = 0; i < OUT_RQUEUE_LEN; i++)
	{
		outNode[i].datalen = xferUSBDataSize /sizeOfInt32;
		outNode[i].data = (__int32 *)malloc(xferUSBDataSize);
		//usb�첽���
		outNode[i].ovLap.hEvent = CreateEvent(NULL, false, false, NULL);
		
		outRQue.add(outNode[i]);
	
	}
	for (int i = 0; i < OUT_RQUEUE_LEN; i++)	
		outRQue.pull(nodeOutbuf);

	
	//ͬ�����
	num = 0;
	dataInEvent = CreateEvent(NULL, false, false, NULL);
	dataInMutex = CreateMutex(NULL, false, NULL);
	//debug
	xferIntNum = 0;
	

	//_beginthread(testIn, 0, NULL);
	_beginthread(testFormat, 0, NULL);
	_beginthread(testOut, 0, NULL);

	haveInit = true;
}
//������������ͨ����
//allChIn1������ĵ�ַ��ch�ǳ��뵽32λint���ݵĸ�����һ���ڵ��int��������
//out�������ַ��outBytes�������һ���ڵ㣩���ֽ���
 void queueBufStart(__int32 *allChIn1,__int32 *out, int ch, int outBytes)
{
	 if (!haveInit)
	 {
		initQueueBuf(ch, outBytes);
		out[0] = 123456;//��������
	 }
		
	else
	{ 
		//��ȡint�͵������ַ����ȡ��������
		 WaitForSingleObject(dataInMutex, INFINITE);
		 inData = allChIn1;
		 nodeIn = (qnodeChInt32 *)inRQue.getHead();
		 memcpy(nodeIn->data, inData, nodeIn->datalen * sizeOfInt32);
		 if (!inRQue.add(*nodeIn, false))
		 {
			 cout << "Data in false." << endl;
			 //�ɲ�ͣ�����ֱ����ʱ����
		 }
		//��ȡ�����¼�����
		//SetEvent(dataInEvent);
		outIntBuf=out  ;
		ReleaseMutex(dataInMutex);
		
		//out[0] = 654321;
	}
	
}

//����Ϊ�����õ���һ�β�������,����ΪchNum��__int32������
 void testIn(void *)
{
	while(1)
	{

		if (chNum > 0)
		{
			//WaitForSingleObject(dataInEvent, INFINITE);
			//inRQue.pull(nodeIn);
			WaitForSingleObject(dataInMutex, INFINITE);
			nodeIn = (qnodeChInt32 *)inRQue.getHead();
			memcpy(nodeIn->data,inData,nodeIn->datalen * sizeOfInt32);
			
			//add�ڶ�������Ϊtrue������������ݶ�ʧ����
			if (!inRQue.add(*nodeIn, false))
			{
				cout << "Data in false." << endl;
			}
			ReleaseMutex(dataInMutex);
			/*	
			if (num < IN_RQUEUE_LEN)
				num++;
			else num = IN_RQUEUE_LEN;
			*/
			/*
			if (inRQue.add(*nodeIn,true))
			{
				if(num < IN_RQUEUE_LEN)
					num++;
				else num = IN_RQUEUE_LEN;
				
			}
			else
			{
				//printf("add head fail");
			}
			*/
		}
	}
}
 void  testFormat(void *)
{
	UINT index = 0;
	UINT x = 0;


	while (1)
	{
		if (!inRQue.isEmpty() )
		{
			//��ȡ��ͷ�Ľڵ㣬ע��nodeOut���ܵĸı�����Ĳ���ȫ
			if(index == 0)nodeOut = outRQue.getHead();
			if (index < nodeOut->datalen*sizeOfInt32)
			{
				inRQue.pull(nodeInbuf);//����ͷ�����ݴ���һ������

				//��������
				//����inNode��ֱ��tempMidNode��,
				index += hjkdataf.waveDataFormat(nodeInbuf, nodeOut, index);
				//for (int i = 0; i < nodeOut->datalen; i += (sizeof(dacData)))
				//	printf("out data is %d\n", *(dacData*)(nodeOut->data + index+i));


			}
			else if(index >= nodeOut->datalen*sizeOfInt32)////��tempMidNode�е�������ʱ�������������
			{
				//��������
				index = 0;

				if (!outRQue.isFull())
				{
					//д��ʱ����
					outRQue.add(*nodeOut, false);
					
					//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//������к��ź���+1
				}
				else
				{
					outRQue.add(*nodeOut, true);
					cout << "Data out false." << endl;
					//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//������к��ź���+1
				//	printf("add nodeOut overwrite!\n");
				}

			}

		}
		else
		{
			//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������
		}
	}
}
 void  testOut(void *)
 {

	 int i = 0;
	 while (1)
	 {
		 if (!outRQue.isEmpty())//���зǿվ�pull
		 {
			 if (outRQue.pull(nodeOutbuf))
			 {
				 //usb�첽����
				 nodeOutbuf->context = hjkBulkOutEndpt->BeginDataXfer(
					 (UCHAR *)nodeOutbuf->data, xferUSBDataSize, &nodeOutbuf->ovLap);
				 if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
				 {
					 //Display(String::Concat("Xfer request rejected. NTSTATUS = ", EndPt->NtStatus.ToString("x")));
				 //	AbortXferLoop(i + 1, buffers, isoPktInfos, contexts, inOvLap);
					 return;
				 }

				 if (!hjkBulkOutEndpt->WaitForXfer(&nodeOutbuf->ovLap, XFER_TIME_OUT))
				 {
					 hjkBulkOutEndpt->Abort();
					 if (hjkBulkOutEndpt->LastError == ERROR_IO_PENDING)
						 WaitForSingleObject(nodeOutbuf->ovLap.hEvent, 2000);
				 }

				 if (hjkBulkOutEndpt->FinishDataXfer(
					 (UCHAR *)nodeOutbuf->data, xferUSBDataSize, &nodeOutbuf->ovLap, nodeOutbuf->context))
				 {
					 //debug
					 /*
					 xferIntNum += xferUSBDataSize / 4;

					 if (xferIntNum % 1048576 == 0)
					 {
						 cout << "xferIntNum = " << xferIntNum << ",time is " << time(NULL) << endl;
					 }
					 */
				 }


				 //memcpy(outIntBuf, nodeOutbuf->data, nodeOutbuf->datalen*sizeOfInt32);

				 //for (i = 0; i < nodeOutbuf->datalen; i+=sizeof(__int32))
				 //	printf("nodeOutbuf data is %d\n", *(dacData *)(nodeOutbuf->data + i));
			 //	if (hjkUdev.USBDevice->DeviceCount() > 0)
				 //	usbxfer.xferDataToUSBEp(hjkUdev.USBDevice->BulkOutEndPt, nodeOutbuf->data, nodeOutbuf->datalen, hjkOutOvLap[outRQue.numElements()]);

			 }
		 }
	 }
	 //������ 

	 abortXfer(OUT_RQUEUE_LEN);
 }


 void abortXfer(int pending)
 {
	// xferUSBDataSize = hjkBulkOutEndpt->MaxPktSize * chNum;
	 hjkBulkOutEndpt->Abort();
	 for (int i = 0; i < OUT_RQUEUE_LEN; i++)
	 {
		 if (i < pending)
		 {
			 hjkBulkOutEndpt->WaitForXfer(&outNode[i].ovLap, XFER_TIME_OUT);
			 hjkBulkOutEndpt->FinishDataXfer(
				 (UCHAR *)outNode[i].data, xferUSBDataSize, &outNode[i].ovLap, outNode[i].context);
		 }
		 CloseHandle(outNode[i].ovLap.hEvent);
	 }
	 delete[] outNode;

 }
//����Ϊһ�β�������ͨ�����Ҳ��ý����µĶ���
 void dacDataIn(void *)
{
	while (1) 
	{

	
		if (chNum > 0)
		{
			//nodeIn = (qnodeChInt32 *)inRQue.getHead();
			nodeIn->datalen = chNum;
			nodeIn->data = inData;
			inRQue.add(*nodeIn);

		}
	}
}