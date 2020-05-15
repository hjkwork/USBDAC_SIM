#include "pch.h"
#include "HjkDataFormat.h"
#include <stdlib.h>





HjkDataFormat::HjkDataFormat()
{
	isFrist = true;
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
		//printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}

	return size_x;
}

UINT HjkDataFormat::waveDataFormat(qnodeChInt32 * inNode, qnodeChInt32 * midNode, UINT index)
{
	//����inNode�е�����
	//תΪ��ΪDACʹ�õ����ݣ�
	//��������ݵĳ���Ϊsize_x,����tem��

	UINT size_x = inNode->datalen *4;
	__int32 *tem = inNode->data;
	if (isFrist)
	{
		oddGroup = (__int32 *)malloc((inNode->datalen / 2) * sizeof(__int32));//��Ϊ0
		evenGroup = (__int32 *)malloc((inNode->datalen / 2) * sizeof(__int32));//��Ϊ0
		isFrist = false;
	}
	
	{
		//�������
		//�����ݴ��벢����ż����
		for (UCHAR i = 0; i < inNode->datalen; i++)
		{
			if (i % 2 == 0)
				oddGroup[i / 2] = inNode->data[i];
			else
				evenGroup[i / 2] = inNode->data[i];
		}

		//ת��
		transpositionInt(oddGroup, tem, DAC_BITS);
		transpositionInt(evenGroup, tem + (inNode->datalen / 2), DAC_BITS);
	}


	//�������index�����ܳ���midNode��datalen
	if (index + size_x <= (midNode->datalen*4) )
		//��������д���indexλ�ÿ�ʼ��midNode��
		memcpy(midNode->data + (index/4), tem, size_x);
	else
	{
		//printf("SERROR:out of the len of midNode's data.\n");
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
		//printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}

	return size_x;
}
//ת�÷���
	//�����ֱ�Ϊ�������顢������顢��Чλ����DACλ��,������Ϊ24bit��
	//����Ϊ�̶���32λת�ã�
UINT HjkDataFormat::transpositionInt(__int32 *in, __int32 *out, char bitWide)
{
	unsigned __int32 padBit = 0x01 << (bitWide - 1);//��Чλ�����λȡ1
	for (int i = 0; i < 32; i++)//32λ�޷������͵�ת���������32*4�ֽڡ�
	{
		out[i] = 0;
		//��i���������������32��������iλ���綼�������λ����ĳλ����������������ƶ�0-31λ�����Ľ������out_1 = data_1_bit&data_2_biit&...data_32_bit.���У�bit��ĳλ��ָ��
		for (int j = 0; j < 32; j++)
		{
			out[i] |= (((in[j] << i) & padBit) >> j);

		}
	}
	return 0;

}