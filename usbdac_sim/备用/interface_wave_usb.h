#ifndef __INTERFACE_WAVE_USB_H_

#define __INTERFACE_WAVE_USB_H_

#include "usbdac_sim.h"

#define MIN_BUF_SIZE 8//��8����������Ҫ�����ݣ���8���ֽ�
#define MID_BUF_SIZE 16
#define MAX_BUF_SIZE 32
//#define TOUSB_BUF_DATA_LEN MIN_BUF_SIZE//����һ���ڵ�����ݸ���
#define DAC_RESOLUTION 24//dac�ķֱ���


#define QUEUE_LEN 12 //���г���
#define PACKETS_PER_XFER 30  //ÿ�δ���İ���
#define TIMEOUT 2000 //��ʱ��ms��
#define BULK_OUT_ADDR 0x04
#define MAX_CHANNLE_COUNT 64
#define TRANSPOTSION_COUNT 32
#define DAC_24BIT 24
#define DAC_32BIT 32
#define CHANNEL_COUNT_24 24
#define CHANNEL_COUNT_32 32
#define MAX_QUEUE_SZ  12
#define QUEUE_NODE_LEN  8
typedef UINT dacData;
struct bufSource
{
	dacData *dac_data_buf;
	int buf_len;
	int next_pos;
	int read_pos;
	
};
typedef struct _QUEUE_NODE_
{
	UINT datalen;
	UCHAR *data;
} QUEUE_NODE;

void addSample(bufSource *bufs,dacData dataSample);
#endif


 