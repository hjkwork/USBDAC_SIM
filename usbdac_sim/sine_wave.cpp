
#include "stdafx.h"
#include "sine_wave.h"

//#define DAC_RESOLUTION 24




dacData sampleSine(double t, int level, double freq,double phase){
		
	//dacData *data;
	//int res= level * sin(2 * DOUBLE_PI * freq * t + phase);
        return level * sin(2 * DOUBLE_PI * freq * t + phase);
    };
/*

*/

void  waveOut(bufSource *bufs,double freq)
{
	
	int level = pow(2.0,DAC_RESOLUTION);
	bool stop=false;//��������

	double x= 0;//��ʼʱ��
	while(!stop)
	{
		addSample(bufs, sampleSine(x,level,freq,0));
		Sleep(1000/freq);
	}

}