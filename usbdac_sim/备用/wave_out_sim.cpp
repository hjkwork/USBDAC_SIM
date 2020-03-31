//wave_out_sim.cpp
//����һ�����η������������󣬿�ͨ����һ�����ν��б任��ͬʱ���1~n���������ݡ����ò�ͬ����ģ�⣬���ķ�ֵ��Χ��0��1����
//Ϊ����ƣ���һ�����Ǻ����������ݣ��������������ݸ���n��
#include "stdafx.h"
#include "usbdac_sim.h"

using namespace std;
//����һ����������Ҫ�Ĳ����ṹ��
struct waveSet
{
	double scale;//��ֵ��������
	double x;//������ֵ��
	double step;//������Ĳ���ֵ
	double frequency;//���Ƶ��
	stringbuf *buf;//������ָ��
	int bufsize;//��������С


};

void waveSine(void *wavSet)
{
	
	waveSet *wset = (waveSet *)wavSet;
	double x = wset->x;
	double scale = wset->scale;
	double step = wset->step;
	double freq=wset->frequency;
	stringbuf *sbuf=wset->buf;
	int sizeofInt = sizeof(int);
	int inttem;
	basic_string<char> res;
	while(1)
	{
		inttem = (int)sin(x)*scale;//������ҪתΪint������
	//	res =(char *)&inttem;
		sbuf->sputn((char *)&inttem,sizeofInt);
		x+=step;
		res = sbuf->str();
		Sleep(1000/freq);//�����Ǹ�bug��Sleep��ҪDWORD
	}
	
}

int wave(char *cbuf,int bufsize)
{
	double x=0;
	double step =0.1;



	waveSet wSet;
	wSet.scale=WAV_SCALE;
	wSet.x=0;
	wSet.step=0.1;
	wSet.frequency=1000;
	//char cbuf[32];
	//wSet.buf=cbuf;

	int waveThreadID = WAVE_THREAD_ID;

	
	stringbuf sbuf;
	sbuf.str(cbuf);
	//sbuf.pubsetbuf(cbuf,bufsize);//���������鴫��stringbuf
	wSet.buf=&sbuf;//�趨waveSet��stringbuf
	//streamsize i = sbuf.in_avail();
	//char sen []={65,66,67,68};
	//sbuf.sputn(sen,10);
	//sbuf.str();
	_beginthread(waveSine,0,&wSet);
	while(1);
	return 0;
}