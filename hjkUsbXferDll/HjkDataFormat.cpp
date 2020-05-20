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
	
	//处理inNode中的数据
	//转为可为DAC使用的数据，
	//处理后数据的长度为size_x,存在tem中

	UINT size_x = inNode->datalen;
	UCHAR *tem = inNode->data;

	//处理过程


	//处理后检测index，不能超过midNode的datalen
	if(index+size_x <= midNode->datalen )
	//将处理结果写入从index位置开始的midNode中
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
	//处理inNode中的数据
	//转为可为DAC使用的数据，
	//处理后数据的长度为size_x,存在tem中

	UINT size_x = inNode->datalen *4;
	__int32 *tem = inNode->data;
	if (isFrist)
	{
		oddGroup = (__int32 *)malloc((inNode->datalen / 2) * sizeof(__int32));//设为0
		evenGroup = (__int32 *)malloc((inNode->datalen / 2) * sizeof(__int32));//设为0
		isFrist = false;
	}
	
	{
		//处理过程
		//将数据传入并分奇偶两组
		for (UCHAR i = 0; i < inNode->datalen; i++)
		{
			if (i % 2 == 0)
				oddGroup[i / 2] = inNode->data[i];
			else
				evenGroup[i / 2] = inNode->data[i];
		}

		//转置
		transpositionInt(oddGroup, tem);
		transpositionInt(evenGroup, tem + (inNode->datalen / 2));		
	}

	//outDataFormat_64CH(qnodeChInt32 * in, qnodeChInt32 * out, UCHAR bitWide)
#ifdef DAC_24BIT
	//  24位按字节填装，取高24位,小端格式中，将一组64个32位数据(0~63)去掉第24~31和56~63共16个32位数据即可.
	size_x = (inNode->datalen - 16) * 4;
	//处理后检测index，不能超过midNode的datalen
	if(index + size_x <= (midNode->datalen * 4))
	{

		//将处理结果写入从index位置开始的midNode中
		memcpy(midNode->data + (index / 4), tem, 24 * 4);
		memcpy(midNode->data + (index / 4) + 24, tem + 32, 24 * 4);
	}
		
	else
	{
		//printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}
	
#else

	//处理后检测index，不能超过midNode的datalen
	if (index + size_x <= (midNode->datalen * 4))
		//将处理结果写入从index位置开始的midNode中
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

	//处理过程


	//处理后检测index，不能超过midNode的datalen
	if (index + size_x <= midNode->datalen)
		//将处理结果写入从index位置开始的midNode中
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
		oddGroup = (__int32*)malloc((inNode->datalen / 2) * sizeof(__int32));//设为0
		evenGroup = (__int32*)malloc((inNode->datalen / 2) * sizeof(__int32));//设为0
		isFrist = false;
	}

	{
		//处理过程
		//将数据传入并分奇偶两组
		for (UCHAR i = 0; i < inNode->datalen; i++)
		{
			if (i % 2 == 0)
				oddGroup[i / 2] = inNode->data[i];
			else
				evenGroup[i / 2] = inNode->data[i];
		}

		//转置
		transpositionInt(oddGroup, tem);
		transpositionInt(evenGroup, tem + (inNode->datalen / 2));
	}

	//outDataFormat_64CH(qnodeChInt32 * in, qnodeChInt32 * out, UCHAR bitWide)
#ifdef DAC_24BIT
	//  24位按字节填装，取高24位,小端格式中，将一组64个32位数据(0~63)去掉第24~31和56~63共16个32位数据即可.
	size_x = (inNode->datalen - 16) * 4;
	//处理后检测index，不能超过midNode的datalen
	

		//将处理结果写入从index位置开始的midNode中
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
	//此处导致输入通道数不能动态改变
	if (isFrist)
	{
		oddGroup = (__int32*)malloc((inLen / 2) * sizeof(__int32));//设为0
		evenGroup = (__int32*)malloc((inLen / 2) * sizeof(__int32));//设为0
		//outbuf = (UCHAR*)malloc(size_x);
		isFrist = false;
	}	
	//将数据传入并分奇偶两组
		for (UCHAR i = 0; i < inLen; i++)
		{
			if (i % 2 == 0)
				oddGroup[i / 2] = inBuf[i];
			else
				evenGroup[i / 2] = inBuf[i];
		}

		//转置
		transpositionInt(oddGroup, inBuf);
		transpositionInt(evenGroup, inBuf + (inLen / 2));
		switch(bitwide)
		{
		case DAC_24BIT:
			//将处理结果写入从index位置开始的midNode中
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

//转置方法
	//参数分别为输入数组、输出数组、有效位数（DAC位数,本工程为24bit）
	//函数为固定的32位转置，
UINT HjkDataFormat::transpositionInt(__int32 *in, __int32 *out)
{
	//unsigned __int32 padBit = 0x01 << (bitWide - 1);//有效位的最高位取1
	unsigned __int32 padBit = 0x01 << 31;//有效位的最高位取1
	for (int i = 0; i < 32; i++)//32位无符号整型的转置输出，即32*4字节。
	{
		out[i] = 0;
		//第i个整型数的输出是32个数左移i位后（如都移至最高位）与某位相与后，再依次向右移动0-31位并相或的结果。即out_1 = data_1_bit&data_2_biit&...data_32_bit.其中，bit是某位代指。
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
			//  24位按字节填装，取高24位,小端格式中，将一组64个32位数据(0~63)去掉第24~31和56~63共16个32位数据即可.
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