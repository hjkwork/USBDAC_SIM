#pragma once
#ifndef __HJKDATAFORMAT_
#define __HJKDATAFORMAT_
#include "HjkDataDefine.h"

using namespace std;

class HjkDataFormat
{
private:
	QUEUE_NODE * inNode;
	QUEUE_NODE * midNode;
	UINT index;
	UINT method;
public:
	HjkDataFormat(UINT method);
	HjkDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	HjkDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index);
	//~HjkDataFormat();
	//index ��midNode����Ч�ֽ���������ֵΪ�ֽ���������¼��һ��д��λ��,����midNode��
	//����inNode�е����ݣ������������ݷ���midNode��data�д�index��ʼ���ڴ��У����ش��������ݴ�С.
	//ע�⣺�˴���������ݲ��ܳ���midNode�ķ�Χ��������Ҫ��ǿ��
	 UINT waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	 UINT waveDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index);

};

#endif // !