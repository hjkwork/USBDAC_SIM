#ifndef  __BUF_QUEUE_H__
#define __BUF_QUEUE_H__
//#include <pthread.h>
/*********8��������֡��������� 2020.3.21******************/
//�ڲ�������ʱ���л�����ź����Ĳ�������ʵ�ֲ���
//�ο���https://blog.csdn.net/u012459903/article/details/103959872
/************���в����������ɿ�����������*********/
//���̵�ַ��https://github.com/xant/libhl
//��https://github.com/craflin/LockFreeQueue


#include "usbdac_sim.h"

#define MAX_SEM_COUNT 10
#define THREADCOUNT 12
#define TIMEOUT_MS  5000
#define TIMEOUT_ZERO  0L

#define LOOP_ADD(i,r) ((i)+1)>=(r)?0:(i+1)
#define SIGLEFRAME_LEN (1024*125) //������н��Ĵ�С����һ���������ֽ���

typedef struct _QUEUE_NODE_
{
	HANDLE mutex;
	int datalen;
	UCHAR data[SIGLEFRAME_LEN];
} QUEUE_NODE;
//BufQueue��ʵ�ֵ��Ƕ��̶߳�д�������У����������maxNodes���ڵ㣬��ʵ�ֳ��ӡ����
class BufQueue
{
private:
	HANDLE mindex_mutex;
	int maxNodes;
	QUEUE_NODE *que;
	int mInindex;
	int mOutindex;
	int mEffectives;
	HANDLE mSem;

public :
	BufQueue(int frames);
	BufQueue(dacData *databuf);
	~BufQueue();
	/*  ���ӣ�ɾ��ջ��Ԫ�ء�
	*\\������
	*\\���أ�
	*/
	int pop(QUEUE_NODE *node);
	/*��ӣ����β����Ԫ�ء�
	*/
	int push(QUEUE_NODE *node);

	int getbuffer();
	int releasebuffer();
private:
	void addInindex();
	void addOutindex();
	//
	int IncreaseEffectives();
	int reduceEffectives();
};

#endif