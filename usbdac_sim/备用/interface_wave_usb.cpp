/*�������ݺ�usb�Ľӿ�
����USB��DAC��Ӳ���豸�������ղ��η����������ݣ���������͸�USB�豸���ӿ���2��������in_buf�����У���out_buf����������СΪn*len��lenΪһ������һ�εĻ��峤�ȣ���lenΪDAC_BIT���������������������´���
	1�����ղ��η�������n�����ݷ���in_buf�У���in_buf����ʱ���Ƴ���һ�����ݣ������µ�һ�����ݣ�ֱ����������ѭ��ִ�У�֪���յ�ֹͣ����
	2����x������ת�ú����out_buf.
**/

#include "stdafx.h"
#include <queue>
#include "RingBufCPP.h"
#include "interface_wave_usb.h"

using namespace std;


static dacData *inbuf;
static dacData *outbuf;
const static int inbuf_size = MIN_BUF_SIZE;//�����С��ÿ�����ݴ�С��dacData����
const static int outbuf_size = MIN_BUF_SIZE;


void interfaceToUSB() {}
bufSource *setBufsource(bufSource *bufs, int buf_len)
{
	bufs->dac_data_buf = (dacData *)calloc(buf_len, sizeof(dacData));
	if (bufs->dac_data_buf == NULL)
	{
		printf("interface_wave_usb exception,inbuf is null.\n");
		exit(1);
	}

	return bufs;

}
/*
dacData *setOutbuf(int outbuf_size)
{
	dacData *outbuf;
	outbuf = (dacData *)calloc(outbuf_size,sizeof(dacData));
	if(outbuf_size==NULL)
	{
		printf("interface_wave_usb exception,outbuf_size is null.\n");
		exit(1);
	}

	return outbuf_size;

}
*/
//��buf���������
void addSample(bufSource *bufs, dacData dataSample)
{
	//dacData *databuf=bufs->dac_data_buf;
	//���������д������
	if (bufs->next_pos < bufs->buf_len)//д������ʱ������
	{
		//databuf+=bufs->next_pos*sizeof(dacData);
		//*databuf = data;
		bufs->dac_data_buf[bufs->next_pos] = dataSample;
		bufs->next_pos++;

	}
	else bufs->next_pos = 0;
}
//�����������������
void dataFromat(bufSource *inbufs, bufSource *outbufs)
{
	outbufs = inbufs;
}
/*
������������Ϊ����Ԫ�أ�push����У�
ֻҪ���в������Ϳ���push��
����������ִ��pop��Ԫ������̶�����������������usb����ʱ�ж�ʧ����
������в�Ϊ�գ�pop����Ԫ�ص�usb�豸�������Ͷ�ͷԪ�غ�ɾ����ͷ
������Ϊ�գ�usb���͹̶���������Ԫ�����ݡ�

*/
void bufToUSBLoop(std::queue<dacData> bufQueue, bufSource *bufs, dacData *tembuf, const int bufsize, bool ready)
{
	//��������
	while (bufQueue.size()!=MAX_QUEUE_SZ)
	{
		bufQueue.push(bufs->dac_data_buf[bufsize]);

	}
	for(int i = 0; i < bufQueue.size(); i++)
	{
		if(!bufQueue.empty())
		xferQueueData(bufQueue.front());
		else xferQueueData(tempbuf);
	}
	//������ʱ�����Ӳ�����
	if (bufQueue.size() == MAX_QUEUE_SZ) {}
	
	{
	
	}

}


static int PPX = PACKETS_PER_XFER;//ÿ�δ��ݵİ���
static int QueueSize = QUEUE_SIZE;
static int TimeOut = TIMEOUT;
static UCHAR eptAddr = BULK_OUT_ADDR;

static UCHAR *outBuffer;
static UCHAR *outContext;

/*��ʼ��һ���˵�*/
void initUSBEndPt()
{
	static CCyUSBEndPoint		*EndPt;
	long len = EndPt->MaxPktSize*PPX;//ÿ�δ���PPX�����ݰ�
	EndPt->SetXferSize(len);
	// Allocate the arrays needed for queueing
	PUCHAR			*buffers = new PUCHAR[QueueSize];
	CCyIsoPktInfo	**isoPktInfos = new CCyIsoPktInfo*[QueueSize];
	PUCHAR			*contexts = new PUCHAR[QueueSize];
	OVERLAPPED		inOvLap[MAX_QUEUE_SZ];
	int i = 0;
	// Allocate all the buffers for the queues
	for (i = 0; i < QueueSize; i++)
	{
		buffers[i] = new UCHAR[len];
		isoPktInfos[i] = new CCyIsoPktInfo[PPX];
		inOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);

		memset(buffers[i], 0xEF, len);
	}
	//inOvLap.hEvent = CreateEvent(NULL, false, false, L"USB_OUT");
}

/*
���������ݻ���뻺����У����������queue_size��buffer��ɵ��ڴ�����

*/
//���Ͷ����е�����
static void xferQueueData(queue<UCHAR> *xferQueue,CCyUSBEndPoint *EndPt,int queueindex, PUCHAR *buffers, long len, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED inOvLap [])
{
	//OVERLAPPED outOvLab, inOvLab;
	
	
	
	
	contexts[queueindex] = EndPt->BeginDataXfer(buffers[queueindex], len, &inOvLap[queueindex]);
	if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
	{
		printf("Xfer request rejected. NTSTATUS = %lld", EndPt->NtStatus);
		AbortXferLoop(EndPt, queueindex + 1, buffers, isoPktInfos, contexts, inOvLap);
		return;
	}
	


	
}
static bool					bStreaming;
static void AbortXferLoop(CCyUSBEndPoint *EndPt,int pending, PUCHAR *buffers, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED inOvLap [])
{
	//EndPt->Abort(); - This is disabled to make sure that while application is doing IO and user unplug the device, this function hang the app.
	long len = EndPt->MaxPktSize * PPX;
	EndPt->Abort();

	for (int j = 0; j < QueueSize; j++)
	{
		if (j < pending)
		{
			EndPt->WaitForXfer(&inOvLap[j], TimeOut);
			/*{
				EndPt->Abort();
				if (EndPt->LastError == ERROR_IO_PENDING)
					WaitForSingleObject(inOvLap[j].hEvent,2000);
			}*/
			EndPt->FinishDataXfer(buffers[j], len, &inOvLap[j], contexts[j]);
		}

		CloseHandle(inOvLap[j].hEvent);

		delete[] buffers[j];
		delete[] isoPktInfos[j];
	}

	delete[] buffers;
	delete[] isoPktInfos;
	delete[] contexts;


	bStreaming = false;

	
}

	//����һ���̶��ռ�Ķ���
	//ֻҪ��������push
	//ֻҪ�ǿվʹ��뵽usb��pop
const static int dacDataSize = sizeof(dacData);
bool initInerface()
{
	//����һ������ΪQUEUE_LEN��ѭ�����У������еĽڵ�Ԫ��ΪQUEUE_NODE����
	RingBufCPP<QUEUE_NODE,QUEUE_LEN> rQue;
	//��ʼ�����У���Ϊ��ѭ�����У����Բ��ó�ʼ������
	QUEUE_NODE qnode;
	qnode.datalen = QUEUE_NODE_LEN*dacDataSize;
	
	memset(qnode.data, 0xff, qnode.datalen);
	for (int i = 0; i < QUEUE_LEN; i++)
	{
		if (!rQue.add(qnode, true))
			printf("init queue faild \n");
	}


	
	return true;

}
void xferToUSBQueueNode(RingBufCPP<QUEUE_NODE, QUEUE_LEN> rQue)
{
	contexts[queueindex] = EndPt->BeginDataXfer(buffers[queueindex], len, &inOvLap[queueindex]);
	if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
	{
		printf("Xfer request rejected. NTSTATUS = %lld", EndPt->NtStatus);
		AbortXferLoop(EndPt, queueindex + 1, buffers, isoPktInfos, contexts, inOvLap);
		return;
	}
}