// usbdac_sim.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "usbdac_sim.h"

#include "sine_wave.h"


#include "InterfaceOfUSB.h"
#include "HjkDataDefine.h"
#include "HjkUSBInterface.h"
#include "HjkUSBDevice.h"
#include "HjkUSBXfer.h"

#define interfaceofusb 0
using namespace std;
//����ӿ���
//���建��
static UCHAR			*inQueBuf;
static UCHAR			*outQueBuf;
//����ڵ�
static QUEUE_NODE		inNode[IN_RQUEUE_LEN];
static QUEUE_NODE		midNode;
static QUEUE_NODE		outNode[OUT_RQUEUE_LEN];
static QUEUE_NODE		*temNode;
//static InterfaceOfUSB interOfUSB(4, 2);//4��2Ŀǰ���ã�Դ����Ĭ��Ϊ4��2��03.23��
//static InterfaceOfUSB interOfUSB(&inNode[0], &midNode,0,0,0,0);
static InterfaceOfUSB interOfUSB;
static HjkUSBInterface <QUEUE_NODE_A, QUEUE_NODE_B,IN_RQUEUE_LEN, OUT_RQUEUE_LEN> hjkInterf;
static HANDLE inMutex[IN_RQUEUE_LEN];
static HANDLE outMutex[OUT_RQUEUE_LEN];
static HANDLE *temMutex;
static HANDLE hjkMutex;
static OVERLAPPED hjkInOvLap[IN_RQUEUE_LEN];
static OVERLAPPED hjkOutOvLap[OUT_RQUEUE_LEN];
static HjkDataFormat hjkdataf(0);
static UINT inQueIndex;
//����1
static RingBufCPP<QUEUE_NODE_A, IN_RQUEUE_LEN> inQue;
static RingBufCPP<QUEUE_NODE_B, OUT_RQUEUE_LEN> outQue;
static QUEUE_NODE_A *nodea;
static QUEUE_NODE_B  *nodeb;
static QUEUE_NODE_A *nodeabuf;
static QUEUE_NODE_B *nodebbuf;
//����2 interfaceofusb 
static RingBufCPP<QUEUE_NODE, IN_RQUEUE_LEN> inRQue;
static RingBufCPP<QUEUE_NODE, OUT_RQUEUE_LEN> outRQue;
static QUEUE_NODE *nodeIn;
static QUEUE_NODE *nodeOut;
static QUEUE_NODE *nodeInbuf;
static QUEUE_NODE *nodeOutbuf;
//hjkusb
HjkUSBDevice hjkUdev;
HjkUSBXfer usbxfer;
//static bool  first;
void testIn(void *);
void testFormat(void *);
void testOut(void *);
unsigned __stdcall testThreadDatain(void *);
unsigned __stdcall  testXferDataToUSB(void *);
int _tmain(int argc, _TCHAR* argv[])
{
	/*******************�ַ������ò���2020.03.***********/
	/*
	char wavBuf[512];
	stringbuf sbuf;
	sbuf.pubsetbuf(wavBuf,512);
	streamsize i = sbuf.in_avail();
	char sen []={65,66,67,68};
	sbuf.sputn(sen,10);
	sbuf.str();

	char wBuf[32];//���εĻ�������
	
	wave(wBuf,sizeof(wBuf));
	cout<<sbuf.str();
	*/
	//�첽������ʼ��
	//boolContr = false;
	
	//hjkOvLap.hEvent = CreateEvent(NULL, false, false, NULL);
	//hjkOvLap.hEvent = CreateMutex(NULL, false, NULL);
	//hjkOvLap.hEvent = CreateSemaphore(NULL, 1, 10, NULL);
	inQueIndex = 0;
	//���Ƶ�һ�����������������first���Ƴ�ʼֵ��������Խ����г�ʼ������Ϊ0
	//first = true;
	hjkMutex = CreateMutex(NULL, false, NULL);
	//1.��ʼ������,���泤��Ϊʵ���ֽڳ���
	for (int i = 0; i < IN_RQUEUE_LEN; i++)
	{
		inMutex[i] = CreateMutex(NULL, false, NULL);
		hjkInOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);
		inNode[i].datalen = 4 * sizeof(dacData);
		inNode[i].data = (UCHAR *)malloc(inNode[i].datalen );

		//����2
		inRQue.add(inNode[i]);
		
		
	}
	
	//inQueBuf =(UCHAR *) malloc(IN_RQUEUE_LEN*inNode.datalen);
	//inNode.data = inQueBuf;//��ʼ��ַ
	//thread 
	
	//2.��ʼ���ӿ�
	//ע�ⳤ�ȵĶ���Ҫ׼ȷ
	midNode.datalen = inNode[0].datalen * 5;
	midNode.data = (UCHAR *)malloc(midNode.datalen * sizeof(dacData));

	nodeabuf = new QUEUE_NODE_A;
	nodebbuf = new QUEUE_NODE_B;
	nodeInbuf = new QUEUE_NODE;
	nodeOutbuf = new QUEUE_NODE;


	

	//3.��ʼ��USB�豸
	
	//HjkUSBDevice hUdevice;
	//��ʼ�����
	for(int i = 0; i <OUT_RQUEUE_LEN;i++)
	{
		outMutex[i] = CreateMutex(NULL, false, NULL);
		outNode[i].datalen = inNode[i].datalen * 5;
		outNode[i].data = (UCHAR *)malloc(outNode[i].datalen );
		hjkOutOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);
		//����2
		
		outRQue.add(outNode[i]);
	}
	//outQueBuf = (UCHAR *)malloc(OUT_RQUEUE_LEN*outNode.datalen);
	//��ʼ�����趨���ݴ�����
	//hjkdataf.waveDataFormat(&inNode, &midNode, 0);
	
	//4.�������ݷ����߳�	
	/*
	_beginthreadex(
		NULL,					//��ȫ��NULL�����ܱ��̳�
		0,						//Ĭ�϶�ջ��С	
		testThreadDatain,		//������ڵ�ַ
		NULL,					//�����б�
		0,						//��ʼ��״̬��0 for running or CREATE_SUSPENDED for suspended
		NULL					//Points to a 32-bit variable that receives the thread identifier. Might be NULL, in which case it is not used.
	);*/
	//5.�����ӿڵ����������̡߳������̺߳���������߳�

	_beginthread(testIn, 0, NULL);
	_beginthread(testFormat, 0, NULL);
	_beginthread(testOut, 0,NULL);
	//6.����USB�����߳�
	//_beginthread(testXferDataToUSB, 0, NULL);

	while (1);
	return 0;
}

static void  testIn(void *)
{
	int i = 0;
	dacData dda = 0;
#ifdef interfaceofusb
	while (1)
	{
		


		//	WaitForSingleObject(inMutex[index], INFINITE);//��Ҫ�ȴ����������߲�������
	//��ȡͷ�ڵ��ַ
		nodeIn = (QUEUE_NODE *)inRQue.getHead();
		WaitForSingleObject(hjkMutex, INFINITE);
		for (i = 0; i < nodeIn->datalen; i += (sizeof(dacData)))
		{
			
			memcpy(nodeIn->data + i, &dda, sizeof(dacData));
			//printf("in data is %d\n", *(dacData*)(nodeIn->data + i));

			dda++;
		}
		ReleaseMutex(hjkMutex);
		Sleep(50);
		//���
		inRQue.add(*nodeIn);
		
	}	
#else
	
	while (1)
	{		
		

			//	WaitForSingleObject(inMutex[index], INFINITE);//��Ҫ�ȴ����������߲�������
			//��ȡͷ�ڵ��ַ
			nodea = (QUEUE_NODE_A *) inQue.getHead();
			WaitForSingleObject(hjkMutex, INFINITE);
			for (i = 0; i < nodea->datalen; i += (sizeof(dacData)))
			{
				//uc = (UCHAR *)&dda;
				memcpy(nodea->data + i, &dda, sizeof(dacData));
			//	printf("in data is %d\n", *(dacData*)(nodea->data + i));

				dda++;
			}
			ReleaseMutex(hjkMutex);
			Sleep(50);
			//���
			inQue.add(*nodea);

	}
#endif
}
static void  testFormat(void *)
{
	int index = 0;
	UINT x = 0;

#ifdef interfaceofusb
	while (1)
	{
		if (!inRQue.isEmpty())
		{
			//��ȡ��ͷ�Ľڵ�
			nodeOut = outRQue.getHead();			
			if (index < nodeOut->datalen)
			{
				//WaitForSingleObject(inQ2MidNodeSem, TIMEOUT_MS);//�ź���Ϊ0ʱ�޷�����ִ��
				inRQue.pull(nodeInbuf);//����ͷ�����ݴ���һ������
				//��������
				//����inNode��ֱ��tempMidNode��
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
					printf("add nodeOut overwrite!\n");
				}		
				
			}

		}
		else
		{
			//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������
		}
	}
#else
	while (1)
	{
		//���봦�����������ͷ�ڵ�����
		//interOfUSB.interfaceStart(&hjkdataf,&inMutex[0],&outMutex[0]);
		//hjkInterf.interfaceStart(&hjkdataf, &inMutex[0], &outMutex[0]);
		/*********test***********/
		if (!inQue.isEmpty())
		{
			//��ȡ��ͷ�Ľڵ�
			nodeb = outQue.getHead();
			//outRQueue.pull(tempMidNode);
			if (index < nodeb->datalen)
			{
				//WaitForSingleObject(inQ2MidNodeSem, TIMEOUT_MS);//�ź���Ϊ0ʱ�޷�����ִ��
				inQue.pull(nodeabuf);//����ͷ�����ݴ���һ������
				//��������
				//����inNode��ֱ��tempMidNode��
				index += hjkdataf.waveDataFormat(nodeabuf, nodeb, index);
				//for (int i = 0; i < nodea->datalen; i += (sizeof(dacData)))
				//	printf("out data is %d\n", *(dacData*)(nodeb->data + index+i));
				 

			}

			else ////��tempMidNode�е�������ʱ�������������
			{
				//��������
				index = 0;

				if (!outQue.isFull())
				{
					//д��ʱ����

					outQue.add(*nodeb, false);
					
					//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//������к��ź���+1
				}
				else
				{
					outQue.add(*nodeb, true);
					//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//������к��ź���+1
					printf("add tempMidNode overwrite!\n");
				}
			}

		}
		else
		{
			//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������
		}

	}
#endif
}
static void  testOut(void *)
{
	
	int i = 0;
#ifdef interfaceofusb
	
		while (1)
		{
			if (!outRQue.isEmpty())//���зǿվ�pull
			{
				if (outRQue.pull(nodeOutbuf))
				{
					for (i = 0; i < nodeOutbuf->datalen; i += sizeof(dacData))
							printf("nodeOutbuf data is %d\n", *(dacData *)(nodeOutbuf->data + i));
					if(hjkUdev.USBDevice->DeviceCount() > 0)
					usbxfer.xferDataToUSBEp(hjkUdev.USBDevice->BulkOutEndPt, nodeOutbuf->data, nodeOutbuf->datalen, hjkOutOvLap[outRQue.numElements()]);
						
				}
			}
		}
	
#else
	while (1)
	{
		/*for (i = 0; i < OUT_RQUEUE_LEN; i++)
		{
			interOfUSB.interfaceForUSBOut(outNode[i]);
		}

		*/
		if (!outQue.isEmpty())//���зǿվ�pull
		{
			outQue.pull(nodebbuf);
			for (i = 0; i < nodebbuf->datalen; i += sizeof(dacData))
				printf("out data is %d\n",*(dacData *)(nodebbuf->data+i));
		}
		
		
	}
#endif
}
unsigned __stdcall testThreadDatain(void *)
{
	dacData dda = 0;
	UINT i = 0;
	//int index = 0;
	UCHAR *uc;

		
	while (1)
	{
		//SetEvent(hjkOvLap.hEvent);
		//�ظ����������������
		
			if (inQueIndex < IN_RQUEUE_LEN)
			{
				
			//	WaitForSingleObject(inMutex[index], INFINITE);//��Ҫ�ȴ����������߲�������
				
				for (i = 0; i < inNode[inQueIndex].datalen; i += (sizeof(dacData)))
				{
					//uc = (UCHAR *)&dda;
					memcpy(inNode[inQueIndex].data + i, &dda, sizeof(dacData));
					printf("in data is %d\n", *(dacData*)(inNode[inQueIndex].data + i));
					dda++;
				}
				//	inNode.data += inNode.datalen;
				Sleep(1);
				//temNode = &inNode[inQueIndex];
				//temMutex = &inMutex[index];
				
				SetEvent(hjkInOvLap[inQueIndex].hEvent);//�¼�����:����һ���ڵ�
				//memset((dacData*)inNode.data, 0xef, inNode.datalen*sizeof(dacData));//���̶�������

				
				WaitForSingleObject(inMutex[inQueIndex],INFINITE);
				inQueIndex++;
	
		//		ReleaseMutex(inMutex[index]);//д���ɶ�
			}
			else inQueIndex = 0;
		
	}
	
	return 1;
}
unsigned __stdcall  testXferDataToUSB(void *)
{
	while (1)
	{
		
		//memset(&outNode, 0x12, outNode.datalen * sizeof(dacData));
		
	}
	return 1;
}

//����Ϊһ��2ά���飬chΪͨ������nΪÿͨ��������
static void dacDataArrIn(__int32 ** arr2D, int ch, int n)
{
	//ѭ����ȡÿ�β�������ͨ��������
	for (int i = 0; i < n;i++)
	{

	}
}
//����Ϊ�����õ���һ�β�������
static void dacDataArrIn(__int32 * allChIn1, int ch)
{
	int i = 0;
	dacData dda = 0;
	while (1)
	{



		
	//��ȡͷ�ڵ��ַ
		nodeIn = (QUEUE_NODE *)inRQue.getHead();
		WaitForSingleObject(hjkMutex, INFINITE);
		for (i = 0; i < nodeIn->datalen; i += (sizeof(dacData)))
		{

			memcpy(nodeIn->data + i, &dda, sizeof(dacData));
			//printf("in data is %d\n", *(dacData*)(nodeIn->data + i));

			dda++;
		}
		ReleaseMutex(hjkMutex);
		Sleep(50);
		//���
		inRQue.add(*nodeIn);

	}
}