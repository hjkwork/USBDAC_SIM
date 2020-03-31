#pragma once
#ifndef __HJKUSBXFER_H_
#define __HJKUSBXFER_H_
#include <wtypes.h>
#include <dbt.h>
#include "CyAPI.h"


//#include "HjkDataDefine.h"
#define TIMEOUT_100_MS 100



class HjkUSBXfer
{
private:

	static int									MaxPktSize;
	static int									PPX;
	static int									QueueSize;
	static int									TimeOut;
	static int									dacDataSize;

	// Allocate the arrays needed for queueing
	//static PUCHAR			*buffers = new PUCHAR[QueueSize];
	//static CCyIsoPktInfo	**isoPktInfos = new CCyIsoPktInfo*[QueueSize];
	//��¼���Ͷ���
	PUCHAR								*contexts;
	OVERLAPPED							inOvLap;

	UCHAR								*context;

	

public:
	HjkUSBXfer();
	~HjkUSBXfer();




	
	//��USB�˵㴫�����ݣ����첽���ͷ�ʽ�������ͺ���ȴ�������Ϣ
	bool xferDataToUSBEp(CCyUSBEndPoint *EndPt, PUCHAR buf, LONG len,OVERLAPPED ov);
	//��ͬ���ķ�������USB�Ķ˵㴫�����ݣ����÷������ݶ�����Ҫ�ȴ������Ƿ����
	bool xferDataToUSBEp(CCyUSBEndPoint *EndPt, PUCHAR buf, LONG len);
	
	//ǿ��PPX��Ч
	void EnforceValidPPX();
	//��ֹ����
	void AbortXfer();

};
#endif
