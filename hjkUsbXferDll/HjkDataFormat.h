#pragma once
#ifndef __HJKDATAFORMAT_
#define __HJKDATAFORMAT_
#include "HjkDataDefine.h"

#define CHANLES_NUM 64
#define DAC_BITS 32
#define DAC_24BIT 24
using namespace std;

class HjkDataFormat
{
private:
	QUEUE_NODE * inNode;
	QUEUE_NODE * midNode;
	UINT index;
	UINT method;
	__int32 *oddGroup;
	__int32 *evenGroup;
	bool isFrist;
	
public:
	HjkDataFormat();
	HjkDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	
	HjkDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index);
	//~HjkDataFormat();
	//index ��midNode����Ч�ֽ���������ֵΪ�ֽ���������¼��һ��д��λ��,����midNode��
	//����inNode�е����ݣ������������ݷ���midNode��data�д�index��ʼ���ڴ��У����ش��������ݴ�С.
	//ע�⣺�˴���������ݲ��ܳ���midNode�ķ�Χ��������Ҫ��ǿ��
	 UINT waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	 //
	 UINT waveDataFormat(qnodeChInt32 * inNode, qnodeChInt32 * midNode, UINT index);
	 UINT waveDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index);
	 /*
	 dac�������ǰ����Ҫ����dac��λ����ʽ��������ݣ�һ��ע��dac���ݸ�ʽ�Ĵ�С�����⣬
	 ���ǣ�24λdac��Ҫ����8λ����.
	 */
	 
	
private:
	UINT transpositionInt(__int32 *in, __int32 *out);
	void outDataFormat_64CH(qnodeChInt32 * in, qnodeChInt32* out, UCHAR bitWide);
};

#endif // !