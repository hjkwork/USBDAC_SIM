#include "pch.h"
#include <stdlib.h>
#include <process.h>
#include <iostream>
#include <time.h>
#include "hjkUsbXferdll.h"
#include "HjkDataDefine.h"
#include "RingBufCPP.h"
#include "hjkDataFormat.h"
#include "HjkUSBDevice.h"

// DLL internal state variables:
static qnodeChInt32								inNode[IN_RQUEUE_LEN];
static qnodeChInt32								outNode[OUT_RQUEUE_LEN];
static RingBufCPP<qnodeChInt32, IN_RQUEUE_LEN>	inRQue;
static RingBufCPP<qnodeChInt32, OUT_RQUEUE_LEN>	outRQue;
static PUCHAR									*outBuf= new PUCHAR[OUT_RQUEUE_LEN];
static qnodeChInt32								*nodeIn;
static qnodeChInt32								*nodeOut;
static qnodeChInt32								*outQueNodeOut;
static qnodeChInt32								*inQueNodeOut;
static qnodeChInt32								*nodeOutbuf;
static __int32									*outIntBuf;
static __int32* dataOfOutQueNodeOut;
static __int32									*dataOfInQueNodeOut;
static __int32* pDataIn;
static __int32* dataInBuf;
static int										chNum;
static bool										haveInit;
static bool										isInFirst;
static bool										isOutFirst;
static bool										isXfer;

static UCHAR* dacDataBuf;
//需要初始化设置
static UCHAR									sizeOfInt32;
static int										MaxPktSize;
static long										xferUSBDataSize;
//需要放入初始化函数中
static HjkDataFormat							hjkdataf;
//同步相关
//只有获取数据后，in线程才能入队，且入队之前，数据不能变，
static HANDLE									dataInEvent;
static HANDLE									dataInMutex;
static HANDLE									outbufMutex = CreateMutex(NULL, false, NULL);
//在填充一个节点的时候不能pull；
static	UINT										inQIndex;//计数
static	UINT										outQIndex;
//usb相关
static string									hjkUSBdevices;
static HjkUSBDevice								hjkUSB;
static CCyBulkEndPoint							*hjkBulkOutEndpt;


static int xferIntNum;
static long ppx;

static PUCHAR* outBufs;
static UINT outBufsLen;
static UINT	outBufsIndex;
static CCyIsoPktInfo** isoPktInfos;
static PUCHAR* contexts;
static OVERLAPPED* inOvLap;

//通道缓存
static __int32* mutiChsBuf;
static __int32* oddSetBuf;
static __int32* evenSetBuf;
//调试相关
int debugindex = 0;
int t1, t2;
//static OVERLAPPED								hjkOutOvLap[OUT_RQUEUE_LEN];
//static UCHAR									hjkOutContxt[OUT_RQUEUE_LEN];
//函数声明
//void queueBufStart(__int32 *allChIn1, int ch, __int32 *out);
void testIn(void *);
void testFormat(void *);
void testOut(void *);
void toDacData(void*);
void dacDataToOutQue(UCHAR* dacDataBuf);
void dacDataToOutQueT(void*);
bool xferOutBulkdataAsy(CCyBulkEndPoint* endPt, PUCHAR bufer, long xferSize, UCHAR* context, OVERLAPPED ovLap);
bool xferBuf(PUCHAR buf);
void debugThead(void*);
void abortXfer(int pending);
/*
usb设备初始化操作
查找usb设备，如果设备数小于1，则退出
等待选择要使用到设备
选择后，如果设备不可用，退出
若设备可用
设置端口和传输帧信息
初始化完成返回
*/
long  initHjkUSBXfer(string usbdevices, int chs)
{
	
	string endinfos;
	long len ;
	long ppx;
	int slecd = -1;
	int n = hjkUSB.getHjkUSBDevice(&usbdevices);
	if (n > 0)
	{
#ifdef CMD
		cout << usbdevices;
		while (slecd < 0 || slecd >= n)
		{

			cout << "Slect one  hjkUSB device to xfer." << endl;
			cin >> slecd;
		}
#else 
		slecd = 0;
#endif
		if (hjkUSB.USBDevice->Open(slecd))
		{
			//获取块端口，设置最大传输字节数
			hjkBulkOutEndpt = hjkUSB.USBDevice->BulkOutEndPt;			

			if (hjkBulkOutEndpt == NULL)
			{
				cout << "The USB bulk end point is null, program exit." << endl;
				return 0;
			}
			//清空重置端点
			hjkBulkOutEndpt->Abort();
			hjkBulkOutEndpt->Reset();
			//每次传输的字节数为xferlen，不超过4m bytes。
			ppx = chs * 3;
			len = hjkBulkOutEndpt->MaxPktSize*ppx;
			//len = 192 * 16;
			if (len > 0x400000)
			{
				len = 4177920;
			}
			hjkBulkOutEndpt->SetXferSize(len);
			
			return len;
		}
		else
		{
			cout << "USB device is not a hjkUSB device, program exit." << endl;
			return 0;
		}

	}
	else
	{
		cout << "USB device is 0, program exit,please check the USB device or driver."<<endl;
		return 0;
	}
	//

}
void initQueueBuf(int ch, int queueSize)
{
	haveInit = false;
	chNum = ch;

	sizeOfInt32 = sizeof(__int32);
	//连接USB设置该值
	
	xferUSBDataSize=initHjkUSBXfer(hjkUSBdevices, chNum);
	if (xferUSBDataSize == 0)
	{
		cout << "USB device init failed, progarma exit. " << endl;
		exit(1);
	}
	//hjkOutOvLap.hEvent = CreateEvent(NULL, false, false, L"hjk_bulk_out");

	//MaxPktSize = outBytes;
	//输入数据处理方法
	//hjkdataf = HjkDataFormat(ch, 0);

	//2.初始化节点缓存
	//*inNode = new qnodeChInt32;
	

	inQueNodeOut = new qnodeChInt32;
	inQueNodeOut->datalen = chNum;
	dataOfInQueNodeOut = (__int32*)malloc(inQueNodeOut->datalen * sizeOfInt32);
	inQueNodeOut->data = dataOfInQueNodeOut;

	outQueNodeOut = new qnodeChInt32;
	outQueNodeOut->datalen = xferUSBDataSize / sizeOfInt32;
	dataOfOutQueNodeOut = (__int32*)malloc(outQueNodeOut->datalen * sizeOfInt32);
	outQueNodeOut->data = dataOfOutQueNodeOut;


	nodeOutbuf = new qnodeChInt32;


	//1.初始化输入节点
	for (inQIndex = 0; inQIndex < IN_RQUEUE_LEN; inQIndex++)
	{
		//datalen一般为通道数
		inNode[inQIndex].datalen = chNum ;
		inNode[inQIndex].data = (__int32 *)malloc(inNode[inQIndex].datalen*sizeOfInt32);
		inNode[inQIndex].mutex = CreateMutex(NULL, false, NULL);		
	}
	inQIndex = 0;

	dataInBuf = new __int32[chNum];
	//初始化输出
	outBufsLen = queueSize;
	outBufsIndex = 0;
	outBufs = new PUCHAR[outBufsLen];
	isoPktInfos = new CCyIsoPktInfo * [outBufsLen];
	contexts = new PUCHAR[outBufsLen];
	inOvLap = new OVERLAPPED[outBufsLen];
	
	/*mutiChsBuf = new __int32[chNum];
	oddSetBuf = new __int32[chNum /2];
	evenSetBuf = new __int32[chNum /2];*/


	//_beginthread(testIn, 0, NULL);
	//_beginthread(testFormat, 0, NULL);
	//_beginthread(dacDataToOutQueT, 0, NULL);
	//_beginthread(testOut, 0, NULL);
	_beginthread(toDacData, 0, NULL);
	
	//_beginthread(debugThead, 0, NULL);
	haveInit = true;
	isInFirst = true;
	isOutFirst = true;
	isXfer = true;
}
//必须首先设置通道数
//allChIn1是输入的地址，ch是出入到32位int数据的个数（一个节点的int个数），
//out是输出地址，outBytes是输出（一个节点）的字节数
 void queueBufStart(__int32 *allChIn1,__int32 *out, int ch, int outQueSize)
{
	 pDataIn = allChIn1;
	 if (!haveInit)
	 {
		initQueueBuf(ch, outQueSize);
		out[0] = 123456;//测试数据
	 }	
}

//输入为所有用到的一次采样数据,输入为chNum个__int32的数组
 void testIn(void *)
{
	while(1)
	{

		if (chNum > 0)
		{
			//WaitForSingleObject(dataInEvent, INFINITE);
			//inRQue.pull(nodeIn);
			/*
			WaitForSingleObject(dataInMutex, INFINITE);
			nodeIn = (qnodeChInt32 *)inRQue.getHead();
			memcpy(nodeIn->data,inData,nodeIn->datalen * sizeOfInt32);
			
			//add第二个参数为true，可能造成数据丢失！！
			if (!inRQue.add(*nodeIn, false))
			{
				cout << "Data in false." << endl;
			}
			ReleaseMutex(dataInMutex);
			*/
			/*	
			if (num < IN_RQUEUE_LEN)
				num++;
			else num = IN_RQUEUE_LEN;
			*/
			/*
			if (inRQue.add(*nodeIn,true))
			{
				if(num < IN_RQUEUE_LEN)
					num++;
				else num = IN_RQUEUE_LEN;
				
			}
			else
			{
				//printf("add head fail");
			}
			*/
		}
	}
}
 void  testFormat(void *)
{
	UINT index = 0;
	UINT x = 0;
	
	while (1)
	{
		if (!isInFirst)
		{
			while (inRQue.isEmpty())
			{
				//等待超时，输出debug信息等，时间太长USB设备进入省电模式等其他操作	
			}
			inRQue.pull(inQueNodeOut);
			//因为pull操作，只是改变了noInbuf中data指针的值，并没有将实际数据拷贝一份，所以要做内容复制。
			//在复制完数据前，需要保护nodeInbuf->data中的数据。然后将节点中的数据放入缓存节点数据区
			//WaitForSingleObject(inQueNodeOut->mutex, INFINITE);
			memcpy(dataOfInQueNodeOut, inQueNodeOut->data, inQueNodeOut->datalen * sizeOfInt32);
			inQueNodeOut->data = dataOfInQueNodeOut;
			//ReleaseMutex(inQueNodeOut->mutex);

			if (isOutFirst)
			{
				index += hjkdataf.waveDataFormat
				(inQueNodeOut->data, (UCHAR*)outNode[outQIndex].data + index, inQueNodeOut->datalen, DAC_24BIT);
				if (index == outNode[outQIndex].datalen * sizeOfInt32)
				{
					outNode[outQIndex].context = hjkBulkOutEndpt->BeginDataXfer(
					(PUCHAR)outNode[outQIndex].data, xferUSBDataSize, &outNode[outQIndex].ovLap);
					if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
					{
						printf("Xfer request rejected.\n");
						return;
					}
					/*if (!outRQue.add(outNode[outQIndex], false))
						printf("Out queue first add node false.\n");*/
					index = 0;
					outQIndex++;
					if (outQIndex == OUT_RQUEUE_LEN)
					{
						outQIndex = 0;
						isOutFirst = false;
					}
				}
			}
			else
			{
				
				if(index == 0)
					xferOutBulkdataAsy(hjkBulkOutEndpt, (PUCHAR)outNode[outQIndex].data, xferUSBDataSize,
						outNode[outQIndex].context, outNode[outQIndex].ovLap);

			/*	while (outRQue.isFull());*/
			//	WaitForSingleObject(outNode[outQIndex].mutex, INFINITE);
				index += hjkdataf.waveDataFormat
				(inQueNodeOut->data, (UCHAR*)outNode[outQIndex].data + index, inQueNodeOut->datalen, DAC_24BIT);
				if (debugindex == 0)
					t1 = GetTickCount();
				debugindex++;
				if (debugindex == 100000)
				{
					t2 = GetTickCount();
					printf("Average across time is %d ms per waveDataFormat.\n", (t2 - t1) / 1000);
					debugindex = 0;
				}
				if (index == outNode[outQIndex].datalen * sizeOfInt32)
				{
					index = 0;
					
					outNode[outQIndex].context = hjkBulkOutEndpt->BeginDataXfer(
					(PUCHAR)outNode[outQIndex].data, xferUSBDataSize, &outNode[outQIndex].ovLap);
					if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
					{
						printf("Xfer request rejected.\n");
						return;
					}

				/*	if (!outRQue.add(outNode[outQIndex], false))
						printf("Out queue add node false.\n");*/
				//	ReleaseMutex(outNode[outQIndex].mutex);
					outQIndex++;
					if (outQIndex == OUT_RQUEUE_LEN)
					{
						outQIndex = 0;
						isOutFirst = false;
					}
				}

			}
		}
	}
		
}

 void  testOut(void *)
 {

	 int i = 0;	
	// HANDLE oMutex = CreateMutex(NULL, false, NULL);

	 while (1)
	 {
		 //if (isOutFirst && outRQue.isFull())
		 //{
			// for (outQIndex = 0; outQIndex < OUT_RQUEUE_LEN; outQIndex++)
			// {
			//	 outNode[outQIndex].context = hjkBulkOutEndpt->BeginDataXfer(
			//		 (PUCHAR)outNode[outQIndex].data, xferUSBDataSize, &outNode[outQIndex].ovLap);
			//	 if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
			//	 {
			//		 //Display(String::Concat("Xfer request rejected. NTSTATUS = ", EndPt->NtStatus.ToString("x")));
			//	 //	AbortXferLoop(i + 1, buffers, isoPktInfos, contexts, inOvLap);
			//		 printf("Xfer request rejected.\n");
			//		 return ;
			//	 }
			// }
			// outQIndex = 0;

		 //}
		 //
		// if (!isOutFirst)
		// {
			 while (outRQue.isEmpty());
			 //队列非空就pull
			 if (outRQue.pull(outQueNodeOut))
			 {
				//  WaitForSingleObject(outQueNodeOut->mutex, INFINITE);
				// xferOutBulkdataAsy(hjkBulkOutEndpt, (PUCHAR)outQueNodeOut->data, xferUSBDataSize,
				//	 outQueNodeOut->context, outQueNodeOut->ovLap);
				 //outQueNodeOut->context = hjkBulkOutEndpt->BeginDataXfer(
				 //(PUCHAR)outQueNodeOut->data, xferUSBDataSize, &outQueNodeOut->ovLap);
				 //if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
				 //{
					// printf("Xfer request rejected.\n");
					// return;
				 //}
				//  ReleaseMutex(outQueNodeOut->mutex);			 
			 }
			 else
				 printf("Out queue pull node false.\n");
		 //}
		 

	 }
		//清理函数 
		 abortXfer(OUT_RQUEUE_LEN);	 
 }
 void dacDataToOutQue(UCHAR* dacDataBuf)
 {
	 while (outRQue.isFull())
	 {
		 //等待输出队列有空余空间
	 }
	 nodeOut = outRQue.getHead();	
	 WaitForSingleObject(nodeOut->mutex, INFINITE);
	 memcpy(nodeOut->data, dacDataBuf, xferUSBDataSize);	
	 if(!outRQue.add(*nodeOut, false)) 
		 printf("In queue add node false.\n");
	 ReleaseMutex(nodeOut->mutex);

 }
 void dacDataToOutQueT(void *)
 {
	 while (1)
	 {
		 //初始化输出节点数据
		 if (isOutFirst)
		 {
			 memcpy(outNode[outQIndex].data, dacDataBuf, outNode[inQIndex].datalen * sizeOfInt32);
			 outQIndex++;
		 }
		 else 
		 {
			 while (outRQue.isFull())
			 {
				 //等待输出队列有空余空间
			 }

			 nodeOut = outRQue.getHead();
			 WaitForSingleObject(nodeOut->mutex, INFINITE);
			 // WaitForSingleObject(nodeOut->mutex, INFINITE);
			 WaitForSingleObject(outbufMutex, INFINITE);
			 memcpy(nodeOut->data, dacDataBuf, xferUSBDataSize);
			 ReleaseMutex(outbufMutex);

			 if (!outRQue.add(*nodeOut, false))
				 printf("In queue add node false.\n");
			 ReleaseMutex(nodeOut->mutex);
		 }
		
			
		 
		 
		// ReleaseMutex(nodeOut->mutex);
			

	 }
	 

 }
 //usb bulk异步输出
 bool xferOutBulkdataAsy(CCyBulkEndPoint *endPt, PUCHAR bufer, long xferSize, UCHAR * context,OVERLAPPED ovLap)
 {
	
	
	 if (!endPt->WaitForXfer(&ovLap, XFER_TIME_OUT))
	 {
		 endPt->Abort();
		 if (endPt->LastError == ERROR_IO_PENDING)
			 WaitForSingleObject(ovLap.hEvent, 2000);
		 printf("WaitForXfer error.\n");
	 }

	 if (!endPt->FinishDataXfer(bufer, xferSize, &ovLap, context))
	 {
		 printf("FinishDataXfer error.\n");
		 return false;
	 }
	 //context = endPt->BeginDataXfer(
		// bufer, xferSize, &ovLap);
	 //if (endPt->NtStatus || endPt->UsbdStatus) // BeginDataXfer failed
	 //{
		// //Display(String::Concat("Xfer request rejected. NTSTATUS = ", EndPt->NtStatus.ToString("x")));
	 ////	AbortXferLoop(i + 1, buffers, isoPktInfos, contexts, inOvLap);
		// return false;
	 //}

	 return true;
 }

 void abortXfer(int pending)
 {
	// xferUSBDataSize = hjkBulkOutEndpt->MaxPktSize * chNum;
	 hjkBulkOutEndpt->Abort();
	 for (int i = 0; i < OUT_RQUEUE_LEN; i++)
	 {
		 if (i < pending)
		 {
			 hjkBulkOutEndpt->WaitForXfer(&outNode[i].ovLap, XFER_TIME_OUT);
			 hjkBulkOutEndpt->FinishDataXfer(
				 (UCHAR *)outNode[i].data, xferUSBDataSize, &outNode[i].ovLap, outNode[i].context);
		 }
		 CloseHandle(outNode[i].ovLap.hEvent);
	 }
	 delete[] outNode;

 }
//输入为一次采样所有通道，且不用建立新的队列

 /*将输入队列中的数据处理后放入缓存*/

 void  toDacData(void*)
 {
	 UINT index = 0;
	 __int32* pTstIn;
	 //填满队列
	 for (outBufsIndex = 0; outBufsIndex < outBufsLen; outBufsIndex++)
	 {
		 outBufs[outBufsIndex] = new UCHAR[xferUSBDataSize];
		 isoPktInfos[outBufsIndex] = new CCyIsoPktInfo[ppx];
		 inOvLap[outBufsIndex].hEvent = CreateEvent(NULL, false, false, NULL);
		 index = 0;

		 while (index < xferUSBDataSize)//填满一次发送的数据
		 {
			 //处理过程中，输入数据的内容不能变。同步问题,考虑同步方法，或检测出来
			 //pTstIn = pDataIn;
			 memcpy(dataInBuf, pDataIn, chNum * sizeOfInt32);
			 index += hjkdataf.waveDataFormat(dataInBuf, outBufs[outBufsIndex] + index,chNum , DAC_24BIT);
			 //输出后检查是否改变
			//

		 }
	 }
	 //第一次发送
	 for (outBufsIndex = 0; outBufsIndex < outBufsLen; outBufsIndex++)
	 {
		 contexts[outBufsIndex] = hjkBulkOutEndpt->
			 BeginDataXfer(outBufs[outBufsIndex], xferUSBDataSize, &inOvLap[outBufsIndex]);
		 if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
		 {
			 printf("Xfer request rejected.\n");
			 return;
		 }
	 }

	 outBufsIndex = 0;
	 while (isXfer)
	 {
		 long rLen = xferUSBDataSize;	// Reset this each time through because
			// FinishDataXfer may modify it

		 if (!hjkBulkOutEndpt->WaitForXfer(&inOvLap[outBufsIndex], XFER_TIME_OUT))
		 {
			 hjkBulkOutEndpt->Abort();
			 if (hjkBulkOutEndpt->LastError == ERROR_IO_PENDING)
				 WaitForSingleObject(inOvLap[outBufsIndex].hEvent, 2000);
			 printf("WaitForXfer error.\n");
		 }


		 if (hjkBulkOutEndpt->Attributes == 1) // ISOC Endpoint
		 {
			 /*
			  if (EndPt->FinishDataXfer(buffers[q], rLen, &inOvLap[q], contexts[q], isoPktInfos[q]))
			  {
				  CCyIsoPktInfo *pkts = isoPktInfos[q];

			  }
			  */
		 }
		 else // BULK Endpoint
		 {

			 if (!(hjkBulkOutEndpt->FinishDataXfer(outBufs[outBufsIndex], rLen, &inOvLap[outBufsIndex], contexts[outBufsIndex])))
				 return ;
		 }
		 // Re-submit this queue element to keep the queue full
		 index = 0;
		 while (index < xferUSBDataSize)//填满一次发送的数据
		 {
			 // 处理过程中，输入数据的内容不能变。同步问题, 考虑同步方法，或检测出来
			 memcpy(dataInBuf, pDataIn, chNum * sizeOfInt32);
			 index += hjkdataf.waveDataFormat(dataInBuf, outBufs[outBufsIndex] + index, chNum, DAC_24BIT);
		 }


		contexts[outBufsIndex] = hjkBulkOutEndpt->
			BeginDataXfer(outBufs[outBufsIndex], xferUSBDataSize, &inOvLap[outBufsIndex]);
		if (hjkBulkOutEndpt->NtStatus || hjkBulkOutEndpt->UsbdStatus) // BeginDataXfer failed
		{
			printf("Xfer request rejected.\n");
			return;
		}

		outBufsIndex++;
		if (outBufsIndex == outBufsLen) //
			 outBufsIndex = 0;
	 }
	 abortXfer(outBufsLen);
 }

 bool xferBuf(PUCHAR buf)
 {
	 if (hjkBulkOutEndpt->XferData(buf, xferUSBDataSize))
		 return true;
 }
 void debugThead(void*)
 {
	 nodeOutbuf->data = (int*)malloc(xferUSBDataSize);
	 nodeOutbuf->datalen = xferUSBDataSize / sizeOfInt32;	 
	 nodeOutbuf->ovLap.hEvent = CreateEvent(NULL, false, false, NULL);
	 for (int i = 0; i < nodeOutbuf->datalen; i++)
	 {
		 if (i % 24 == 0)
			 nodeOutbuf->data[i] = 0;
		 else  nodeOutbuf->data[i] = 0xffffffff;
	 }
	 while(1)
	 xferOutBulkdataAsy(hjkBulkOutEndpt, (PUCHAR)nodeOutbuf->data, xferUSBDataSize,
		 nodeOutbuf->context, nodeOutbuf->ovLap);
 }