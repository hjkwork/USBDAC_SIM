#include "pch.h"
#include "HjkDataFormat.h"
#include <stdlib.h>
#include <stdio.h>





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
		transpositionInt(oddGroup, tem);
		transpositionInt(evenGroup, tem + (inNode->datalen / 2));		
	}

	//outDataFormat_64CH(qnodeChInt32 * in, qnodeChInt32 * out, UCHAR bitWide)
#ifdef DAC_24BIT
	//  24λ���ֽ���װ��ȡ��24λ,С�˸�ʽ�У���һ��64��32λ����(0~63)ȥ����24~31��56~63��16��32λ���ݼ���.
	size_x = (inNode->datalen - 16) * 4;
	//�������index�����ܳ���midNode��datalen
	if(index + size_x <= (midNode->datalen * 4))
	{

		//��������д���indexλ�ÿ�ʼ��midNode��
		memcpy(midNode->data + (index / 4), tem, 24 * 4);
		memcpy(midNode->data + (index / 4) + 24, tem + 32, 24 * 4);
	}
		
	else
	{
		//printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}
	
#else

	//�������index�����ܳ���midNode��datalen
	if (index + size_x <= (midNode->datalen * 4))
		//��������д���indexλ�ÿ�ʼ��midNode��
		memcpy(midNode->data + (index / 4), tem, size_x);
	else
	{
		//printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}
	
#endif // DAC_24BIT
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

UINT HjkDataFormat::waveDataFormat(qnodeChInt32* inNode, UCHAR outbuf[CHARS_PER_DAC24_GROUP])
{
	

	UINT size_x = inNode->datalen * 4;
	__int32* tem = inNode->data;
	if (isFrist)
	{
		oddGroup = (__int32*)malloc((inNode->datalen / 2) * sizeof(__int32));//��Ϊ0
		evenGroup = (__int32*)malloc((inNode->datalen / 2) * sizeof(__int32));//��Ϊ0
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
		transpositionInt(oddGroup, tem);
		transpositionInt(evenGroup, tem + (inNode->datalen / 2));
	}

	//outDataFormat_64CH(qnodeChInt32 * in, qnodeChInt32 * out, UCHAR bitWide)
#ifdef DAC_24BIT
	//  24λ���ֽ���װ��ȡ��24λ,С�˸�ʽ�У���һ��64��32λ����(0~63)ȥ����24~31��56~63��16��32λ���ݼ���.
	size_x = (inNode->datalen - 16) * 4;
	//�������index�����ܳ���midNode��datalen
	

		//��������д���indexλ�ÿ�ʼ��midNode��
		memcpy(outbuf, tem, 24 * 4);
		memcpy(outbuf + 24*4, tem + 32, 24 * 4);
	

#else
	memcpy(midNode->data , tem, size_x);
#endif // DAC_24BIT
	 ;

	return size_x;
}

UINT HjkDataFormat::waveDataFormat(__int32* inBuf, UCHAR* outbuf, UINT inLen, UCHAR bitwide)
{
	UINT size_x = inLen*(bitwide/8);
	//�˴���������ͨ�������ܶ�̬�ı�
	if (isFrist)
	{
		oddGroup = (__int32*)malloc((inLen / 2) * sizeof(__int32));//��Ϊ0
		evenGroup = (__int32*)malloc((inLen / 2) * sizeof(__int32));//��Ϊ0
		//outbuf = (UCHAR*)malloc(size_x);
		isFrist = false;
	}	
	//�����ݴ��벢����ż����
		for (UCHAR i = 0; i < inLen; i++)
		{
			if (i % 2 == 0)
				oddGroup[i / 2] = inBuf[i];
			else
				evenGroup[i / 2] = inBuf[i];
		}

		//ת��
		transpositionInt(oddGroup, inBuf);
		transpositionInt(evenGroup, inBuf + (inLen / 2));
		switch(bitwide)
		{
		case DAC_24BIT:
			//��������д���indexλ�ÿ�ʼ��midNode��
			memcpy(outbuf, inBuf, size_x/2);
			memcpy(outbuf + 24 * 4, inBuf + 32, size_x / 2);
			break;
		case DAC_32BIT:
			memcpy(outbuf, inBuf, size_x);			
			break;
		default:
			memcpy(outbuf, inBuf, size_x);
			size_x = inLen * 4;
			break;

		}
			
	return size_x;
}

//ת�÷���
	//�����ֱ�Ϊ�������顢������顢��Чλ����DACλ��,������Ϊ24bit��
	//����Ϊ�̶���32λת�ã�
UINT HjkDataFormat::transpositionInt(__int32 *in, __int32 *out)
{
	//unsigned __int32 padBit = 0x01 << (bitWide - 1);//��Чλ�����λȡ1
	unsigned __int32 padBit = 0x01 << 31;//��Чλ�����λȡ1
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
void HjkDataFormat::outDataFormat_64CH(qnodeChInt32* in, qnodeChInt32* out, UCHAR bitWide)
{
	if (in->datalen == 64)
	{
		switch (bitWide)
		{
		case 16:
			printf("have not develop,exit\n");
			exit(1);

			break;
		case 24:
			//  24λ���ֽ���װ��ȡ��24λ,С�˸�ʽ�У���һ��64��32λ����(0~63)ȥ����24~31��56~63��16��32λ���ݼ���.
			memcpy(out->data, in->data, 24 * 4);
			memcpy(out->data + 24, in->data + 32, 24 * 4);		
			break;
		case 32:
			break;
		default:break;
		}
		
	}
	else
	{
		printf("Channel counts must be 64\n");
		exit(1);
	}
		

	
}