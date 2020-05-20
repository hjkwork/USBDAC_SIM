#pragma once
#ifndef __HJKDATAFORMAT_
#define __HJKDATAFORMAT_
#include "HjkDataDefine.h"

#define CHANLES_NUM 64
#define DAC_BITS 32
#define DAC_24BIT 24
#define	DAC_32BIT 32
#define CHARS_PER_DAC24_GROUP 192
using namespace std;

class HjkDataFormat
{
private:
	QUEUE_NODE * inNode;
	QUEUE_NODE * midNode;
	UINT index;
	UINT method;
	__int32* mutiChsBuf;
	__int32 *oddGroup;
	__int32 *evenGroup;
	bool isFrist;
	
public:
	HjkDataFormat();
	HjkDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	
	HjkDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index);
	//~HjkDataFormat();
	//index 是midNode的有效字节索引，其值为字节数，即记录下一个写入位置,返回midNode的
	//处理inNode中的数据，将处理后的数据放入midNode的data中从index开始的内存中，返回处理后的数据大小.
	//注意：此处插入的数据不能超出midNode的范围！！（需要加强）
	 UINT waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	 //
	 UINT waveDataFormat(qnodeChInt32 * inNode, qnodeChInt32 * midNode, UINT index);
	 UINT waveDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index);
	 UINT waveDataFormat(qnodeChInt32* inNode, UCHAR outbuf[CHARS_PER_DAC24_GROUP]);
	 //inLen为整型长度。输入的bitwide只能是32、24和16；
	 //返回输出大小
	 UINT waveDataFormat(__int32* inBuf, UCHAR *outbuf, UINT inLen, UCHAR bitwide);
	 
	 
	
private:
	UINT transpositionInt(__int32 *in, __int32 *out);
	/*
	 dac数据输出前，需要按照dac的位数格式化输出数据，一是注意dac数据格式的大小端问题，
	 二是，24位dac需要忽略8位数据.
	 */
	void outDataFormat_64CH(qnodeChInt32 * in, qnodeChInt32* out, UCHAR bitWide);
};

#endif // !