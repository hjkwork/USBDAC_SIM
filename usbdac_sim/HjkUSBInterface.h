#pragma once
#ifndef __HJKUSBINTERFACE_H_
#define __HJKUSBINTERFACE_H_
#include "stdafx.h"
#include "RingBufCPP.h"

#include "usbdac_sim.h"
#include "HjkDataDefine.h"
#include "HjkDataFormat.h"


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
template <typename inType, typename outType, UINT inQueElems, UINT outQueElems>

class HjkUSBInterface
{
public:
	//�������
	 RingBufCPP<inType, inQueElems>		inRQueue;
	//һ������ΪQUEUE_LEN��ѭ�����У������еĽڵ�Ԫ��ΪQUEUE_NODE����
	 RingBufCPP<outType, outQueElems>		outRQueue;
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
	inType							*tempInNode;
	//һ����ʱ��������нڵ㣬��������һ�����ݲ��Ϸ���
	outType							*tempMidNode;
	//һ����ʱ��������нڵ㣬��������һ�����ݷ���
	// QUEUE_NODE							*tempOutNode;


public:
	//InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * outNode);
	HjkUSBInterface(QUEUE_NODE * inNode, QUEUE_NODE * midNode, const UINT inNodeLen, const UINT outNodeLen, const UINT inQueLen, const UINT outQueLen) :
	tempInNode(inNode), tempMidNode(midNode)
{
	index = 0;
}

//InterfaceOfUSB::InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * outNode) :
	HjkUSBInterface() :
	index(0),
	inQ2MidNodeSem(CreateSemaphore(NULL, 0, inRQueue.getMaxEles(), NULL)),
	midNode2OutQSem(CreateSemaphore(NULL, 0, outRQueue.getMaxEles(), NULL))

{
	tempInNode = new inType;
	tempMidNode = new outType;
	//tempInNode->datalen = 4 * sizeof(dacData);
	//tempInNode->data = (UCHAR *)malloc(tempInNode->datalen * sizeof(dacData));
	//tempMidNode->datalen = tempInNode->datalen * 5;
	//tempMidNode->data = (UCHAR *)malloc(tempMidNode->datalen * sizeof(dacData));
	//setInRQueue();
	//setOutRQueue();
}

~HjkUSBInterface()
{
	CloseHandle(inQ2MidNodeSem);
	CloseHandle(midNode2OutQSem);
}
bool interfaceForIn(QUEUE_NODE inNode, HANDLE *mutex)
{
	if (!inRQueue.isFull())//add�����������жϣ���������2 falseʱ���������᷵�ش���
	{

		if (inRQueue.add(inNode, false))
		{
			ReleaseSemaphore(inQ2MidNodeSem, 1, NULL);//��Ӻ��ź�����1
			return true;
		}

		else//����ִ�е��Ĳ���
		{
			printf("interfaceForIn rToInterfaceQue.add() failed.\n");
			return false;
		}
	}
	else
	{
		if (inRQueue.add(inNode, true))
		{
			ReleaseSemaphore(inQ2MidNodeSem, 1, NULL);//��Ӻ��ź�����1
			printf("add inNode overwrite!\n");
			return true;
		}

		else
		{
			printf("interfaceForIn rToInterfaceQue.add() overwrite failed.\n");
			return false;
		}

	}
}
bool interfaceStart(HjkDataFormat *hjkDataFormat, HANDLE *inMutex, HANDLE *outMutex)
{

	//��ͷ���ݴ���ʱ��Ҫ��֤����ʱ���зǿգ��Ҷ�ͷ�ڵ�û����д�����
	if (!inRQueue.isEmpty())
	{
		//��ȡ��ͷ�Ľڵ�
		tempMidNode = outRQueue.getHead();
		//outRQueue.pull(tempMidNode);
		if (index < tempMidNode->datalen)
		{
			//WaitForSingleObject(inQ2MidNodeSem, TIMEOUT_MS);//�ź���Ϊ0ʱ�޷�����ִ��
			inRQueue.pull(tempInNode);//����ͷ�����ݴ���һ������
			//��������
			//����inNode��ֱ��tempMidNode��
			index += hjkDataFormat->waveDataFormat(tempInNode, tempMidNode, index);

		}

		else ////��tempMidNode�е�������ʱ�������������
		{
			//��������
			index = 0;

			if (!outRQueue.isFull())
			{
				//д��ʱ����

				outRQueue.add(*tempMidNode, false);
				//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//������к��ź���+1
			}
			else
			{
				outRQueue.add(*tempMidNode, true);
				//ReleaseSemaphore(midNode2OutQSem, 1, NULL);//������к��ź���+1
				printf("add tempMidNode overwrite!\n");
			}
		}

	}
	else
	{
		//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������
	}
	return true;
}
bool interfaceForUSBOut(QUEUE_NODE outNode)
{
	//rToUSBQue.pull(tempOutNode);//����ͷ�����ݴ���һ������

	if (!outRQueue.isEmpty())//pull�����������жϣ������п�ʱ������false
	{
		WaitForSingleObject(midNode2OutQSem, TIMEOUT_MS);//�ź�������0�������ͷ���û�н������޷�����
		if (outRQueue.pull(&outNode))//����ڵ��ַ
		{
			return true;
		}
		else
		{
			printf("interfaceForUSBOut rToUSBQue.pull() failed.\n");
			return false;
		}
	}
	else
	{
		printf("rToUSBQue is empty.\n");
		return true;
	}

}

	





private:
	//��������ڵ�,һ��ΪnLen��bytes
	UINT setToInterfaceNode(const UINT nLen);
	//��������ڵ㣬һ��ΪnLen������ڵ��С
	UINT setToUSBNode(const UINT nLen);
	//�����������
	UINT setInRQueue()
	{
		//inRQueue.setMaxEles(qLen);
		//inRQueue = malloc(sizeof(QUEUE_NODE)*inRQueue.getMaxEles());
		inQueBuf = (UCHAR *)malloc(inRQueue.getMaxEles()*tempInNode->datalen);
		return sizeof(*inQueBuf);
	}
	//�����������
	UINT setOutRQueue()
	{
		//outRQueue.setMaxEles(qLen);
		outQueBuf = (UCHAR *)malloc(outRQueue.getMaxEles()*tempMidNode->datalen);

		return sizeof(*outQueBuf);
	}
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



