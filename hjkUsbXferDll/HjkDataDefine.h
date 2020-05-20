#pragma once
#ifndef __HJKDATADEFINE_
#define __HJKDATADEFINE_

using namespace std;

typedef unsigned int UINT ;
typedef unsigned char UCHAR;
typedef UINT dacData;
//����������ݵĻ���ṹ
struct bufSource
{
	dacData *dac_data_buf;
	int buf_len;
	int next_pos;
	int read_pos;

};
//������ݵĽڵ�ṹ
typedef struct _QUEUE_NODE_
{
	UINT datalen;
	UCHAR *data;
} QUEUE_NODE;
//һ���ڵ���Դ���ch*n��32λint������
typedef struct _QUEUE_NODE_INT32_ARR_
{
	__int32 **data;
	int		ch;//ͨ����
	int     n;//ÿͨ��һ�β�����	
} qnodeInt32s;
typedef struct NODE_INT32_ARR_
{
	UINT		datalen;//ͨ����
	__int32		*data;
	OVERLAPPED	ovLap;
	UCHAR		*context;
	HANDLE		mutex;
} qnodeChInt32;
typedef struct _QUEUE_NODE_A_
{
	UINT datalen=16;
	UCHAR data[16];
} QUEUE_NODE_A;
typedef struct _QUEUE_NODE_B_
{
	UINT datalen = 80;
	UCHAR data[80];
} QUEUE_NODE_B;
class HjkQueueElement
{
public:
	UINT datalen;
	dacData *data;
	HjkQueueElement(UINT datalen):
		datalen(datalen)
	{

	}
	~HjkQueueElement();
};
class HjkDataDefine
{

};
#endif
