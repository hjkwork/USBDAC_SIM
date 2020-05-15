#include "pch.h"
#include "testdllarr.h"
#include "pch.h"
#include <stdlib.h>
#include <process.h>


// DLL internal state variables:

static __int32 *outIntBuf;
static __int32      *inData;
static int			chNum;
static bool			haveInit;
//��Ҫ��ʼ������
static UCHAR sizeOfInt32;
static int									MaxPktSize;

//ͬ�����
//ֻ�л�ȡ���ݺ�in�̲߳�����ӣ������֮ǰ�����ݲ��ܱ䣬
static HANDLE			dataInEvent;
//�����һ���ڵ��ʱ����pull��
static int				num;//����

//��������
//void queueBufStart(__int32 *allChIn1, int ch, __int32 *out);
void testIn(void *);
void testFormat(void *);
void testOut(void *);

void initQueueBuf(int ch)
{
	haveInit = false;
	chNum = ch;
	inData = NULL;
	
	








	
	//ͬ�����
	num = 0;
	dataInEvent = CreateEvent(NULL, false, false, NULL);


	_beginthread(testIn, 0, NULL);
	

	haveInit = true;
}
//������������ͨ����
void queueBufStart(__int32 *allChIn1, __int32 *out)
{
	/*if (!haveInit)
	{
		initQueueBuf(ch, outBytes);
		out[0] = 123456;
	}*/

	/*else
	{*/
		//��ȡint�͵������ַ����ȡ��������
		inData = allChIn1;
		//��ȡ�����¼�����
		SetEvent(dataInEvent);
		outIntBuf = out;

		//out[0] = 654321;
	/*}*/

}

//����Ϊ�����õ���һ�β�������,����ΪchNum��__int32������
void testIn(void *)
{
	while (1)
	{
		WaitForSingleObject(dataInEvent, INFINITE);
		for (int i = 0; i < chNum; i++)
		{
			inData[i] += 3000;
		}
		memcpy(outIntBuf, inData, chNum * 4);
	}
}
