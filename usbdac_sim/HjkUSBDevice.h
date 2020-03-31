#pragma once
#ifndef __HJKUSBDEVICE_H_
#define __HJKUSBDEVICE_H_
#include <wtypes.h>
#include <dbt.h>
#include "CyAPI.h"

#include "usbdac_sim.h"
#define USB_DEVICE_VENDOR_ID 0x04B4;
#define USB_DEVICE_PRODUCT_ID 0x00F1;


class HjkUSBDevice
{
public:
	//USB�豸
	CCyUSBDevice						*USBDevice;
private:
	int							USBDevcieCounts;
	const int					VENDOR_ID = USB_DEVICE_VENDOR_ID;
	const int					PRODUCT_ID = USB_DEVICE_PRODUCT_ID;
	//usb�˵�
	CCyUSBEndPoint				*EndPt;
	//Handle
	HANDLE										hjkHandle;


	bool							bHighSpeedDevice;
	bool							bSuperSpeedDevice;
public:
	HjkUSBDevice();
	//~HjkUSBDevice();
	//��ȡUSB�豸�������豸����,�豸�ź����Ʒ���devicelist��
	int getUSBDevice(char * deviceList);

	//��ȡUSB�˵���Ϣ�����ض˵�����
	int getUSBEndPt(CCyUSBDevice *usbDevice, int deviceNum, char *EndPtInfo);

	//����USB�豸
	bool setUSBDevice();

	//����USB�˵�
	bool setUSBEndPt();
	//���ô���ģʽ
	bool setUSBXferMode();



};
#endif
