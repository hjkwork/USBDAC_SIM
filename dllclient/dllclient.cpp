﻿// dllclient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <time.h> 
#include <process.h>
#include <String>
#include <sstream>
#include <chrono> 
#include"hjkUsbXferdll.h"
using namespace std;
void xferToUSB(void *);
void dataProduce(void*);

int main()
{
	//std::cout << "Data producing..." << endl;

	//_beginthread(dataProduce, 0, NULL);
	//std::cout << "Xfer  data to usb device...\n";
	//初始化后启动传输线程
	_beginthread(xferToUSB, 0, NULL);
	while (1);
	/*********以下都是无用的测试*************/
	int test1 = 0x12345678;
	UCHAR *uct=(UCHAR *)malloc(4);
	memcpy(uct, &test1, 4);
	cout << *uct << endl;


	stringstream ss;
	short a = 1566;
	string str;
	int eptCnt;
	for (int i = 0; i < 2; i++)
	{
		

			eptCnt = 2;

			// Fill the EndPtInfo
			for (int e = 1; e < eptCnt; e++)
			{
				
				// INTR, BULK and ISO endpoints are supported.
				
					ss << ((false) ? "ISOC " :
						((true) ? "BULK " : "INTR "))
						<< (true ? "IN,       " : "OUT,   ")
						<< dec<< 1024
						<< " Bytes,";
					
						ss << 15 << " MaxBurst,";
					ss << "   (" << i << " - "
						<< "0x" << hex << 99 << ")"<<endl;
					str.append(ss.str());
					//cout << ss.str();
					ss.clear();
					ss.str("");
				
			}

		
	}
	str.append(ss.str());
	//cout << str;
	int slecd = -1;
	int n = 1;
	if (n > 0)
	{
		cout << str;
		while (slecd < 0 || slecd >= n)
		{
			
			cout<< "Slect one  hjkUSB device to xfer."<< endl;			
			cin >> slecd;
		}
			
		if (1)
		{
			
				cout << "The USB end point is null, program exit." << endl;
				exit(1);
			
			//long len = EndPt->MaxPktSize * PPX; // Each xfer request will get PPX isoc packets
		}
		else
		{
			cout << "USB device is not a hjkUSB device, program exit." << endl;
			exit(1);
		}

	}
	else
	{
		cout << "USB device is 0, program exit,please check the USB device or driver." << endl;
		exit(1);
	}
}
void dataProduce(void*)
{
	int i = 0x70000000;
	int dda = 0;
	double x = 0;
	__int32 data[64];
	std::cout << "Data producing and xfer data to usb device..." << endl;
	while (1)
	{
		for (i = 0; i < 64; i++) {
			//in[i] = ++dda;
			//in[i] = 0x7fffffff * sin(x += 0.01);//(sin(x)*0x7fffffff)
			data[i] = 0x7fffffff;
		}
		Sleep(1);
	}
}
void xferToUSB(void *)
{
	int i = 0x70000000;
	int dda = 0;
	double x = 0;
	__int32 in[64];
	__int32 out[512];
//	startHjkUSBdevice()
	int delay = 200;
	int f = 500;
	double pi = 3.141592654;
	std::cout << "Data producing and xfer data to usb device..." << endl;
	while (1)
	{
		for (i = 0; i < 64; i++) {
			//in[i] = ++dda;
			in[i] = 0x7fffffff * sin(x += 0.00001);//(sin(x)*0x7fffffff)
			//in[i] = 0x7fffffff;
		}
		
		queueBufStart(in, out,64,64);
		for (int t = 0; t < delay; t++)
			for (int j = 0; j < delay; j++);
	// Sleep(1);

	
		
	}

	
}
// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
