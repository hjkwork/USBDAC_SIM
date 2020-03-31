#include "stdafx.h"
#include "HjkDataFormat.h"



HjkDataFormat::HjkDataFormat(UINT method):
	method(method)
{
}

UINT HjkDataFormat::waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index)
{
	
	//����inNode�е�����
	//תΪ��ΪDACʹ�õ����ݣ�
	//��������ݵĳ���Ϊsize_x,����tem��

	UINT size_x = inNode->datalen;
	UCHAR *tem = inNode->data;

	//�������


	//�������index�����ܳ���midNode��datalen
	if(index+size_x <= midNode->datalen )
	//��������д���indexλ�ÿ�ʼ��midNode��
	(UCHAR*)memcpy(midNode->data + index, tem, size_x);
	else
	{
		printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}

	return size_x;
}

UINT HjkDataFormat::waveDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index)
{
	UINT size_x = inNode->datalen;
	UCHAR *tem = inNode->data;

	//�������


	//�������index�����ܳ���midNode��datalen
	if (index + size_x <= midNode->datalen)
		//��������д���indexλ�ÿ�ʼ��midNode��
		(UCHAR*)memcpy(midNode->data + index, tem, size_x);
	else
	{
		printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}

	return size_x;
}
