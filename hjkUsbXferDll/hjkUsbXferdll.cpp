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
static PUCHAR									*outBuf= new PUCHAR[OUT_RQUEUE_LEN];
static qnodeChInt32								*nodeIn;
static qnodeChInt32								*nodeOut;
static qnodeChInt32								*outQueNodeOut;
static qnodeChInt32								*inQueNodeOut;
static qnodeChInt32								*nodeOutbuf;
static __int32									*outIntBuf;
static __int32* dataOfOutQueNodeOut;
static __int32									*dataOfInQueNodeOut;
static __int32* pDataIn;
static __int32* dataInBuf;
static int										chNum;
static bool										haveInit;
static bool										isInFirst;
static bool										isOutFirst;
static bool										isXfer;

static UCHAR* dacDataBuf;
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
static HANDLE									outbufMutex = CreateMutex(NULL, false, NULL);
//�����һ���ڵ��ʱ����pull��
static	UINT										inQIndex;//����
static	UINT										outQIndex;
//usb���
static string									hjkUSBdevices;
static HjkUSBDevice								hjkUSB;
static CCyBulkEndPoint							*hjkBulkOutEndpt;


static int xferIntNum;
static long ppx;

static PUCHAR* outBufs;
static UINT outBufsLen;
static UINT	outBufsIndex;
static CCyIsoPktInfo** isoPktInfos;
static PUCHAR* contexts;
static OVERLAPPED* inOvLap;

//ͨ������
static __int32* mutiChsBuf;
static __int32* oddSetBuf;
static __int32* evenSetBuf;
//�������
int debugindex = 0;
int t1, t2;
//static OVERLAPPED								hjkOutOvLap[OUT_RQUEUE_LEN];
//static UCHAR									hjkOutContxt[OUT_RQUEUE_LEN];
//��������
//void queueBufStart(__int32 *allChIn1, int ch, __int32 *out);
void testIn(void *);
void testFormat(void *);
void testOut(void *);
void toDacData(void*);
void dacDataToOutQue(UCHAR* dacDataBuf);
void dacDataToOutQueT(void*);
bool xferOutBulkdataAsy(CCyBulkEndPoint* endPt, PUCHAR bufer, long xferSize, UCHAR* context, OVERLAPPED ovLap);
bool xferBuf(PUCHAR buf);
void debugThead(void*);
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
long  initHjkUSBXfer(string usbdevices, int chs)
{
	
	string endinfos;
	long len ;
	long ppx;
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
			//������ö˵�
			hjkBulkOutEndpt->Abort();
			hjkBulkOutEndpt->Reset();
			//ÿ�δ�����ֽ���Ϊxferlen��������4m bytes��
			ppx = chs * 3;
			len = hjkBulkOutEndpt->MaxPktSize*ppx;
			//len = 192 * 16;
			if (len > 0x400000)
			{
				len = 4177920;
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
void initQueueBuf(int ch, int queueSize)
{
	haveInit = false;
	chNum = ch;

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
	

	inQueNodeOut = new qnodeChInt32;
	inQueNodeOut->datalen = chNum;
	dataOfInQueNodeOut = (__int32*)malloc(inQueNodeOut->datalen * sizeOfInt32);
	inQueNodeOut->data = dataOfInQueNodeOut;

	outQueNodeOut = new qnodeChInt32;
	outQueNodeOut->datalen = xferUSBDataSize / sizeOfInt32;
	dataOfOutQueNodeOut = (__int32*)malloc(outQueNodeOut->datalen * sizeOfInt32);
	outQueNodeOut->data = dataOfOutQueNodeOut;


	nodeOutbuf = new qnodeChInt32;


	//1.��ʼ������ڵ�
	for (inQIndex = 0; inQIndex < IN_RQUEUE_LEN; inQIndex++)
	{
		//datalenһ��Ϊͨ����
		inNode[inQIndex].datalen = chNum ;
		inNode[inQIndex].data = (__int32 *)malloc(inNode[inQIndex].datalen*sizeOfInt32);
		inNode[inQIndex].mutex = CreateMutex(NULL, false, NULL);		
	}
	inQIndex = 0;

	dataInBuf = new __int32[chNum];
	//��ʼ�����
	outBufsLen = queueSize;
	outBufsIndex = 0;
	outBufs = new PUCHAR[outBufsLen];
	isoPktInfos = new CCyIsoPktInfo * [outBufsLen];
	contexts = new PUCHAR[outBufsLen];
	inOvLap = new OVERLAPPED[outBufsLen];
	
	/*mutiChsBuf = new __int32[chNum];
	oddSetBuf = new __int32[chNum /2];
	evenSetBuf = new __int32[chNum /2];*/


	//_beginthread(testIn, 0, NULL);
	//_beginthread(testFormat, 0, NULL);
	//_beginthread(dacDataToOutQueT, 0, NULL);
	//_beginthread(testOut, 0, NULL);
	_beginthread(toDacData, 0, NULL);
	
	//_beginthread(debugThead, 0, NULL);
	haveInit = true;
	isInFirst = true;
	isOutFirst = true;
	isXfer = true;
}
//������������ͨ����
//allChIn1������ĵ�ַ��ch�ǳ��뵽32λint���ݵĸ�����һ���ڵ��int��������
//out�������ַ��outBytes�������һ���ڵ㣩���ֽ���
 void queueBufStart(__int32 *allChIn1,__int32 *out, int ch, int outQueSize)
{
	 pDataIn = allChIn1;
	 if (!haveInit)
	 {
		initQueueBuf(ch, outQueSize);
		out[0] = 123456;//��������
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
			/*
			WaitForSingleObject(dataInMutex, INFINITE);
			nodeIn = (qnodeChInt32 *)inRQue.getHead();
			memcpy(nodeIn->data,inData,nodeIn->datalen * sizeOfInt32);
			
			//add�ڶ�������Ϊtrue������������ݶ�ʧ����
			if (!inRQue.add(*nodeIn, false))
			{
				cout << "Data in false." << endl;
			}
			ReleaseMutex(dataInMutex);
			*/
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
		if (!isInFirst)
		{
			while (inRQue.isEmpty())
			{
				//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������	
			}
			inRQue.pull(inQueNodeOut);
			//��Ϊpull������ֻ�Ǹı���noInbuf��dataָ���ֵ����û�н�ʵ�����ݿ���һ�ݣ�����Ҫ�����ݸ��ơ�
			//�ڸ���������ǰ����Ҫ����nodeInbuf->data�е����ݡ�Ȼ�󽫽ڵ��е����ݷ��뻺��ڵ�������
			//WaitForSingleObject(inQueNodeOut->mutex, INFINITE);
			memcpy(dataOfInQueNodeOut, inQueNodeOut->data, inQueNodeOut->datalen * sizeOfInt32);
			inQueNodeOut->data = dataOfInQueNodeOut;
			//ReleaseMutex(inQueNodeOut->mutex);

			if (isOutFirst)
			{
				index += hjkdataf.waveDataFormat
				(inQueNodeOut->data, (UCHAR*)outNode[outQIndex].data + index, inQueNodeOut->datalen, DAC_24BIT);
				if (index == outNode[outQIndex].datalen * sizeOfInt32)
				{
					outNode[outQIndex].context = hjkBulkOutEndpt->BeginDataXfer(
					(PUCHAR)outNode[outQIndex].data, xferUSBDataSize, &outNode[outQIndex].ovLap);
					if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
					{
						printf("Xfer request rejected.\n");
						return;
					}
					/*if (!outRQue.add(outNode[outQIndex], false))
						printf("Out queue first add node false.\n");*/
					index = 0;
					outQIndex++;
					if (outQIndex == OUT_RQUEUE_LEN)
					{
						outQIndex = 0;
						isOutFirst = false;
					}
				}
			}
			else
			{
				
				if(index == 0)
					xferOutBulkdataAsy(hjkBulkOutEndpt, (PUCHAR)outNode[outQIndex].data, xferUSBDataSize,
						outNode[outQIndex].context, outNode[outQIndex].ovLap);

			/*	while (outRQue.isFull());*/
			//	WaitForSingleObject(outNode[outQIndex].mutex, INFINITE);
				index += hjkdataf.waveDataFormat
				(inQueNodeOut->data, (UCHAR*)outNode[outQIndex].data + index, inQueNodeOut->datalen, DAC_24BIT);
				if (debugindex == 0)
					t1 = GetTickCount();
				debugindex++;
				if (debugindex == 100000)
				{
					t2 = GetTickCount();
					printf("Average across time is %d ms per waveDataFormat.\n", (t2 - t1) / 1000);
					debugindex = 0;
				}
				if (index == outNode[outQIndex].datalen * sizeOfInt32)
				{
					index = 0;
					
					outNode[outQIndex].context = hjkBulkOutEndpt->BeginDataXfer(
					(PUCHAR)outNode[outQIndex].data, xferUSBDataSize, &outNode[outQIndex].ovLap);
					if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
					{
						printf("Xfer request rejected.\n");
						return;
					}

				/*	if (!outRQue.add(outNode[outQIndex], false))
						printf("Out queue add node false.\n");*/
				//	ReleaseMutex(outNode[outQIndex].mutex);
					outQIndex++;
					if (outQIndex == OUT_RQUEUE_LEN)
					{
						outQIndex = 0;
						isOutFirst = false;
					}
				}

			}
		}
	}
		
}

 void  testOut(void *)
 {

	 int i = 0;	
	// HANDLE oMutex = CreateMutex(NULL, false, NULL);

	 while (1)
	 {
		 //if (isOutFirst && outRQue.isFull())
		 //{
			// for (outQIndex = 0; outQIndex < OUT_RQUEUE_LEN; outQIndex++)
			// {
			//	 outNode[outQIndex].context = hjkBulkOutEndpt->BeginDataXfer(
			//		 (PUCHAR)outNode[outQIndex].data, xferUSBDataSize, &outNode[outQIndex].ovLap);
			//	 if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
			//	 {
			//		 //Display(String::Concat("Xfer request rejected. NTSTATUS = ", EndPt->NtStatus.ToString("x")));
			//	 //	AbortXferLoop(i + 1, buffers, isoPktInfos, contexts, inOvLap);
			//		 printf("Xfer request rejected.\n");
			//		 return ;
			//	 }
			// }
			// outQIndex = 0;

		 //}
		 //
		// if (!isOutFirst)
		// {
			 while (outRQue.isEmpty());
			 //���зǿվ�pull
			 if (outRQue.pull(outQueNodeOut))
			 {
				//  WaitForSingleObject(outQueNodeOut->mutex, INFINITE);
				// xferOutBulkdataAsy(hjkBulkOutEndpt, (PUCHAR)outQueNodeOut->data, xferUSBDataSize,
				//	 outQueNodeOut->context, outQueNodeOut->ovLap);
				 //outQueNodeOut->context = hjkBulkOutEndpt->BeginDataXfer(
				 //(PUCHAR)outQueNodeOut->data, xferUSBDataSize, &outQueNodeOut->ovLap);
				 //if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
				 //{
					// printf("Xfer request rejected.\n");
					// return;
				 //}
				//  ReleaseMutex(outQueNodeOut->mutex);			 
			 }
			 else
				 printf("Out queue pull node false.\n");
		 //}
		 

	 }
		//������ 
		 abortXfer(OUT_RQUEUE_LEN);	 
 }
 void dacDataToOutQue(UCHAR* dacDataBuf)
 {
	 while (outRQue.isFull())
	 {
		 //�ȴ���������п���ռ�
	 }
	 nodeOut = outRQue.getHead();	
	 WaitForSingleObject(nodeOut->mutex, INFINITE);
	 memcpy(nodeOut->data, dacDataBuf, xferUSBDataSize);	
	 if(!outRQue.add(*nodeOut, false)) 
		 printf("In queue add node false.\n");
	 ReleaseMutex(nodeOut->mutex);

 }
 void dacDataToOutQueT(void *)
 {
	 while (1)
	 {
		 //��ʼ������ڵ�����
		 if (isOutFirst)
		 {
			 memcpy(outNode[outQIndex].data, dacDataBuf, outNode[inQIndex].datalen * sizeOfInt32);
			 outQIndex++;
		 }
		 else 
		 {
			 while (outRQue.isFull())
			 {
				 //�ȴ���������п���ռ�
			 }

			 nodeOut = outRQue.getHead();
			 WaitForSingleObject(nodeOut->mutex, INFINITE);
			 // WaitForSingleObject(nodeOut->mutex, INFINITE);
			 WaitForSingleObject(outbufMutex, INFINITE);
			 memcpy(nodeOut->data, dacDataBuf, xferUSBDataSize);
			 ReleaseMutex(outbufMutex);

			 if (!outRQue.add(*nodeOut, false))
				 printf("In queue add node false.\n");
			 ReleaseMutex(nodeOut->mutex);
		 }
		
			
		 
		 
		// ReleaseMutex(nodeOut->mutex);
			

	 }
	 

 }
 //usb bulk�첽���
 bool xferOutBulkdataAsy(CCyBulkEndPoint *endPt, PUCHAR bufer, long xferSize, UCHAR * context,OVERLAPPED ovLap)
 {
	
	
	 if (!endPt->WaitForXfer(&ovLap, XFER_TIME_OUT))
	 {
		 endPt->Abort();
		 if (endPt->LastError == ERROR_IO_PENDING)
			 WaitForSingleObject(ovLap.hEvent, 2000);
		 printf("WaitForXfer error.\n");
	 }

	 if (!endPt->FinishDataXfer(bufer, xferSize, &ovLap, context))
	 {
		 printf("FinishDataXfer error.\n");
		 return false;
	 }
	 //context = endPt->BeginDataXfer(
		// bufer, xferSize, &ovLap);
	 //if (endPt->NtStatus || endPt->UsbdStatus) // BeginDataXfer failed
	 //{
		// //Display(String::Concat("Xfer request rejected. NTSTATUS = ", EndPt->NtStatus.ToString("x")));
	 ////	AbortXferLoop(i + 1, buffers, isoPktInfos, contexts, inOvLap);
		// return false;
	 //}

	 return true;
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

 /*����������е����ݴ������뻺��*/

 void  toDacData(void*)
 {
	 UINT index = 0;
	 __int32* pTstIn;
	 //��������
	 for (outBufsIndex = 0; outBufsIndex < outBufsLen; outBufsIndex++)
	 {
		 outBufs[outBufsIndex] = new UCHAR[xferUSBDataSize];
		 isoPktInfos[outBufsIndex] = new CCyIsoPktInfo[ppx];
		 inOvLap[outBufsIndex].hEvent = CreateEvent(NULL, false, false, NULL);
		 index = 0;

		 while (index < xferUSBDataSize)//����һ�η��͵�����
		 {
			 //��������У��������ݵ����ݲ��ܱ䡣ͬ������,����ͬ���������������
			 //pTstIn = pDataIn;
			 memcpy(dataInBuf, pDataIn, chNum * sizeOfInt32);
			 index += hjkdataf.waveDataFormat(dataInBuf, outBufs[outBufsIndex] + index,chNum , DAC_24BIT);
			 //��������Ƿ�ı�
			//

		 }
	 }
	 //��һ�η���
	 for (outBufsIndex = 0; outBufsIndex < outBufsLen; outBufsIndex++)
	 {
		 contexts[outBufsIndex] = hjkBulkOutEndpt->
			 BeginDataXfer(outBufs[outBufsIndex], xferUSBDataSize, &inOvLap[outBufsIndex]);
		 if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
		 {
			 printf("Xfer request rejected.\n");
			 return;
		 }
	 }

	 outBufsIndex = 0;
	 while (isXfer)
	 {
		 long rLen = xferUSBDataSize;	// Reset this each time through because
			// FinishDataXfer may modify it

		 if (!hjkBulkOutEndpt->WaitForXfer(&inOvLap[outBufsIndex], XFER_TIME_OUT))
		 {
			 hjkBulkOutEndpt->Abort();
			 if (hjkBulkOutEndpt->LastError == ERROR_IO_PENDING)
				 WaitForSingleObject(inOvLap[outBufsIndex].hEvent, 2000);
			 printf("WaitForXfer error.\n");
		 }


		 if (hjkBulkOutEndpt->Attributes == 1) // ISOC Endpoint
		 {
			 /*
			  if (EndPt->FinishDataXfer(buffers[q], rLen, &inOvLap[q], contexts[q], isoPktInfos[q]))
			  {
				  CCyIsoPktInfo *pkts = isoPktInfos[q];

			  }
			  */
		 }
		 else // BULK Endpoint
		 {

			 if (!(hjkBulkOutEndpt->FinishDataXfer(outBufs[outBufsIndex], rLen, &inOvLap[outBufsIndex], contexts[outBufsIndex])))
				 return ;
		 }
		 // Re-submit this queue element to keep the queue full
		 index = 0;
		 while (index < xferUSBDataSize)//����һ�η��͵�����
		 {
			 // ��������У��������ݵ����ݲ��ܱ䡣ͬ������, ����ͬ���������������
			 memcpy(dataInBuf, pDataIn, chNum * sizeOfInt32);
			 index += hjkdataf.waveDataFormat(dataInBuf, outBufs[outBufsIndex] + index, chNum, DAC_24BIT);
		 }


		contexts[outBufsIndex] = hjkBulkOutEndpt->
			BeginDataXfer(outBufs[outBufsIndex], xferUSBDataSize, &inOvLap[outBufsIndex]);
		if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
		{
			printf("Xfer request rejected.\n");
			return;
		}

		outBufsIndex++;
		if (outBufsIndex == outBufsLen) //
			 outBufsIndex = 0;
	 }
	 abortXfer(outBufsLen);
 }

 bool xferBuf(PUCHAR buf)
 {
	 if (hjkBulkOutEndpt->XferData(buf, xferUSBDataSize))
		 return true;
 }
 void debugThead(void*)
 {
	 nodeOutbuf->data = (int*)malloc(xferUSBDataSize);
	 nodeOutbuf->datalen = xferUSBDataSize / sizeOfInt32;	 
	 nodeOutbuf->ovLap.hEvent = CreateEvent(NULL, false, false, NULL);
	 for (int i = 0; i < nodeOutbuf->datalen; i++)
	 {
		 if (i % 24 == 0)
			 nodeOutbuf->data[i] = 0;
		 else  nodeOutbuf->data[i] = 0xffffffff;
	 }
	 while(1)
	 xferOutBulkdataAsy(hjkBulkOutEndpt, (PUCHAR)nodeOutbuf->data, xferUSBDataSize,
		 nodeOutbuf->context, nodeOutbuf->ovLap);
 }