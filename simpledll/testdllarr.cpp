#include "pch.h"
#include "testdllarr.h"

void testBuf(int *in, int *out,int len)
{
	/*
	for (int i = 0; i <  len; i++)
	{
		out[i] = in[i] + 10;
	}
	*/
	memset(out, 0xff, len * 4);
}
//ת�÷���
	//�����ֱ�Ϊ�������顢������顢��Чλ����DACλ��,������Ϊ24bit��
	//����Ϊ�̶���32λת�ã����ڸĽ�
int transpositionInt(unsigned int in[32], unsigned int out[32], char bitWide)
{
	unsigned int padBit = 0x01 << (bitWide - 1);//��Чλ�����λȡ1
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