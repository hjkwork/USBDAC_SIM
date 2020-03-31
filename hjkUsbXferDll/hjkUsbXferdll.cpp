#include "pch.h"
#include <stdlib.h>
#include <process.h>
#include "hjkUsbXferdll.h"
#include "HjkDataDefine.h"
#include "RingBufCPP.h"
#include "hjkDataFormat.h"
// DLL internal state variables:
static qnodeChInt32		inNode[IN_RQUEUE_LEN];
static qnodeChInt32		outNode[OUT_RQUEUE_LEN];
static RingBufCPP<qnodeChInt32, IN_RQUEUE_LEN> inRQue;
static RingBufCPP<qnodeChInt32, OUT_RQUEUE_LEN> outRQue;
static qnodeChInt32 *nodeIn;
static qnodeChInt32 *nodeOut;
static qnodeChInt32 *nodeInbuf;
static qnodeChInt32 *nodeOutbuf;
static __int32 *outIntBuf;
static __int32      *inData;
static int			chNum;
static bool			haveInit;
//��Ҫ��ʼ������
static UCHAR sizeOfInt32;
static int									MaxPktSize;
static HjkDataFormat						hjkdataf(0);
//ͬ�����
//ֻ�л�ȡ���ݺ�in�̲߳�����ӣ������֮ǰ�����ݲ��ܱ䣬
static HANDLE			dataInEvent;
//�����һ���ڵ��ʱ����pull��
static int				num;//����

//��������
void queueBufStart(__int32 *allChIn1, int ch, __int32 *out);
void testIn(void *);
void testFormat(void *);
void testOut(void *);

void initQueueBuf(int ch, int outBytes)
{
	haveInit = false;
	chNum = ch;
	inData = NULL;
	sizeOfInt32 = sizeof(__int32);
	//����USB���ø�ֵ
	MaxPktSize = outBytes;
	//1.��ʼ������
	for (int i = 0; i < IN_RQUEUE_LEN; i++)
	{
		//datalenһ��Ϊͨ����
		inNode[i].datalen = chNum ;
		inNode[i].data = (__int32 *)malloc(inNode[i].datalen*sizeOfInt32);
		
		//���
		inRQue.add(inNode[i]);
	}



	//2.��ʼ���ڵ㻺��	
	nodeInbuf = new qnodeChInt32;
	nodeOutbuf = new qnodeChInt32;
	//outIntBuf = (__int32 *)malloc(MaxPktSize);

	


	//��ʼ�����
	for (int i = 0; i < OUT_RQUEUE_LEN; i++)
	{
		outNode[i].datalen = MaxPktSize/sizeOfInt32;
		outNode[i].data = (__int32 *)malloc(MaxPktSize);
		
		outRQue.add(outNode[i]);
	}
	//ͬ�����
	num = 0;
	dataInEvent = CreateEvent(NULL, false, false, NULL);
	

	_beginthread(testIn, 0, NULL);
	_beginthread(testFormat, 0, NULL);
	_beginthread(testOut, 0, NULL);

	haveInit = true;
}
//������������ͨ����
 void queueBufStart(__int32 *allChIn1,__int32 *out, int ch, int outBytes)
{
	 if (!haveInit)
	 {
		initQueueBuf(ch, outBytes);
		out[0] = 123456;
	 }
		
	else
	{ 
		//��ȡint�͵������ַ����ȡ��������
		inData = allChIn1;
		//��ȡ�����¼�����
		SetEvent(dataInEvent);
		outIntBuf = out;
		
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
			WaitForSingleObject(dataInEvent, INFINITE);
			nodeIn = (qnodeChInt32 *)inRQue.getHead();
			memcpy(nodeIn->data,inData,nodeIn->datalen * sizeOfInt32);
			
			//add�ڶ�������Ϊtrue������������ݶ�ʧ����
			inRQue.add(*nodeIn, true);
			if (num < IN_RQUEUE_LEN)
				num++;
			else num = IN_RQUEUE_LEN;
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
		if (!inRQue.isEmpty() && num== IN_RQUEUE_LEN)
		{
			//��ȡ��ͷ�Ľڵ㣬ע��nodeOut���ܵĸı�����Ĳ���ȫ
			nodeOut = outRQue.getHead();
			if (index < nodeOut->datalen*sizeOfInt32)
			{
				inRQue.pull(nodeInbuf);//����ͷ�����ݴ���һ������

				//��������
				//����inNode��ֱ��tempMidNode��,
				index += hjkdataf.waveDataFormat(nodeInbuf, nodeOut, index);
				//for (int i = 0; i < nodeOut->datalen; i += (sizeof(dacData)))
				//	printf("out data is %d\n", *(dacData*)(nodeOut->data + index+i));


			}

			else ////��tempMidNode�е�������ʱ�������������
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
		if (!outRQue.isEmpty() && num == IN_RQUEUE_LEN)//���зǿվ�pull
		{
			if (outRQue.pull(nodeOutbuf))
			{
				memcpy(outIntBuf, nodeOutbuf->data, nodeOutbuf->datalen*sizeOfInt32);

				//for (i = 0; i < nodeOutbuf->datalen; i+=sizeof(__int32))
				//	printf("nodeOutbuf data is %d\n", *(dacData *)(nodeOutbuf->data + i));
			//	if (hjkUdev.USBDevice->DeviceCount() > 0)
				//	usbxfer.xferDataToUSBEp(hjkUdev.USBDevice->BulkOutEndPt, nodeOutbuf->data, nodeOutbuf->datalen, hjkOutOvLap[outRQue.numElements()]);

			}
		}
	}
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