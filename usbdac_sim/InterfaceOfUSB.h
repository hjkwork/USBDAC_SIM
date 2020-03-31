#pragma once
#ifndef __INTERFACEOFUSB_H_
#define __INTERFACEOFUSB_H_
#include "usbdac_sim.h"
#include "HjkDataDefine.h"
#include "HjkDataFormat.h"
#include "RingBufCPP.h"

/*
InterfaceOfUSB�ӿ�ʵ��˫·�������(ѭ������)�������ⲿ���ݵ��ӿڵĶ���rToInterfaceQue,�ͽӿڵ�USB�Ķ���rToUSBQue
�ⲿ��n��bytes��ɵ�������Ϊһ���ڵ㣬ֻҪrToInterfaceQue���в�������addһ���ڵ���룬
����ɾ����ɵĽڵ�Ԫ����add���˴�ע��DEBUG����waveDataFormat���������rToInterfaceQue
��ȡ����ͷ�ڵ㣬��������һ����ʱ����ڵ㣬���ڵ���ʱ������rToUSBQue�����У���������ɾ����ɵĽڵ�Ԫ�أ�
��ʱ������ڵ��Сһ��ΪMaxPktSize*PPX����USB�����ݰ����ֵ*ÿ�η��͵����ݰ�����
����ע�⣺MaxPktSize*PPXҪ�ܱ�rToInterfaceQue�Ľڵ�len����������PPX����Ϊ�ýڵ�len�������׹���
*/
#define TIMEOUT_MS INFINITE
using namespace std;


class InterfaceOfUSB
{
public:
	//�������
	RingBufCPP<QUEUE_NODE, IN_RQUEUE_LEN>		inRQueue;
	//һ������ΪQUEUE_LEN��ѭ�����У������еĽڵ�Ԫ��ΪQUEUE_NODE����
	RingBufCPP<QUEUE_NODE, OUT_RQUEUE_LEN>		outRQueue;
private:
	 //���建��
	UCHAR										*inQueBuf;
	UCHAR										*outQueBuf;

	 //�����м�ڵ������
	UINT										index;
	HANDLE										inQ2MidNodeMutex;
	HANDLE										midNode2OutQMuetx;
	//�����ź����������м�ڵ��ȡ��д��ʱ��������в�����ͻ
	HANDLE										inQ2MidNodeSem;
	HANDLE										midNode2OutQSem;

	static int									MaxPktSize;
	static int									PPX;
	static int									QueueSize;
	static int									TimeOut;
	static int									dacDataSize;
	//����˵�
	//static QUEUE_NODE							*inNode;
	//��ʱ������ڵ�
	 QUEUE_NODE							*tempInNode;
	//һ����ʱ��������нڵ㣬��������һ�����ݲ��Ϸ���
	 QUEUE_NODE							*tempMidNode;
	//һ����ʱ��������нڵ㣬��������һ�����ݷ���
	// QUEUE_NODE							*tempOutNode;
	

public:
	//InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * outNode);
	InterfaceOfUSB();
	
	InterfaceOfUSB(
		QUEUE_NODE		*inNode,//�ⲿ���ݵ����
		QUEUE_NODE		*midNode,
		const UINT		inNodeLen,//�ⲿ���ݵĳ���
		const UINT		outNodeLen,
		const UINT		inQueLen,
		const UINT		outQueLen
		);
	
	
	~InterfaceOfUSB(); 
	//�����߳�������������inNode��rToInterfaceQue���У�ֻҪ������в�Ϊ�վ���ӽڵ�,
	 bool interfaceForIn(QUEUE_NODE inNode,HANDLE *mutex);
	//�ӿ��߳������������rToInterfaceQue����ͷ�ڵ㵽tempInNode���������ݡ�����tempMidNode��rToUSBQue����
	 bool interfaceStart();
	 bool interfaceStart(HjkDataFormat *hjkDataFormat, HANDLE *inMutex,HANDLE *outMutex);
	//USB�߳������������rToUSBQue����ͷ�ڵ��ַ��
	 bool interfaceForUSBOut(QUEUE_NODE outNode);
	//USB�߳������������rToUSBQue����ͷ�ڵ㡣
	//static bool interfaceForUSBOut( CCyUSBEndPoint *EndPt, OVERLAPPED inOvLap);
	
	
private:
	//��������ڵ�,һ��ΪnLen��bytes
	UINT setToInterfaceNode(const UINT nLen);
	//��������ڵ㣬һ��ΪnLen������ڵ��С
	UINT setToUSBNode(const UINT nLen);
	//�����������
	UINT setInRQueue();
	//�����������
	UINT setOutRQueue();
	//������ʱ����ڵ�
	UINT setUSBNodeBuf();
	//����������
	 QUEUE_NODE* waveDataFormat();
	//index ��midNode����Ч�ֽ���������ֵΪ�ֽ���������¼��һ��д��λ��,����midNode��
	//����inNode�е����ݣ������������ݷ���midNode��data�д�index��ʼ���ڴ��У����ش��������ݴ�С.
	//ע�⣺�˴���������ݲ��ܳ���midNode�ķ�Χ��������Ҫ��ǿ��
	 UINT waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	//�������Ĳ������ݷ��������  
	void formatDataToRqueue(QUEUE_NODE *USBNodeBuf, QUEUE_NODE *formatDataNode);
	
};

#endif