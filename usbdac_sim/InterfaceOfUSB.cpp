#include "stdafx.h"
#include "RingBufCPP.h"
#include "InterfaceOfUSB.h"
/*�������ݺ�usb�Ľӿ�
����USB��DAC��Ӳ���豸�������ղ��η����������ݣ���������͸�USB�豸���ӿ���2��������in_buf�����У���out_buf����������СΪn*len��lenΪһ������һ�εĻ��峤�ȣ���lenΪDAC_BIT���������������������´���
	1�����ղ��η�������n�����ݷ���in_buf�У���in_buf����ʱ���Ƴ���һ�����ݣ������µ�һ�����ݣ�ֱ����������ѭ��ִ�У�֪���յ�ֹͣ����
	2����x������ת�ú����out_buf.
**/




/*
InterfaceOfUSB::InterfaceOfUSB(
	QUEUE_NODE		*inNode,
	const UINT		inNodeLen,
	const UINT		outNodeLen,
	const UINT		inQueLen,
	const UINT		outQueLen,
	UINT			MaxPktSize, 
	UINT			PPX
	)
	//:
	//USBDevice(USBDevice)
{
	//��ʼ����Ա
	this->inNode = inNode;
	this->MaxPktSize = MaxPktSize;
	this->PPX = PPX;
	this->tempInNode->datalen = inNode->datalen;
	//��ʼ������
	//setToInterfaceNode(inNodeLen);//���ʼ��ǰ�����ⲿ����
	setToUSBNode(outNodeLen);
	setInRQueue(inQueLen);
	setOutRQueue(outQueLen);
	setUSBNodeBuf();
	
	
	*/
	
	/*
	//��ʼ��������У���Ϊ��ѭ�����У����Բ��ó�ʼ������
	QUEUE_NODE qnode;
	qnode.datalen = QUEUE_NODE_LEN * dacDataSize;
	memset(qnode.data, 0xff, qnode.datalen);
	for (int i = 0; i < QUEUE_LEN; i++)
	{
		if (!rQue.add(qnode, true))
			printf("init queue faild \n");
	}
	*/

//}

InterfaceOfUSB::InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * midNode, const UINT inNodeLen, const UINT outNodeLen, const UINT inQueLen, const UINT outQueLen):
	tempInNode(inNode),tempMidNode(midNode)
{
	index = 0;
}

//InterfaceOfUSB::InterfaceOfUSB(QUEUE_NODE * inNode, QUEUE_NODE * outNode) :
InterfaceOfUSB::InterfaceOfUSB() :
	index(0), 
	inQ2MidNodeSem(CreateSemaphore(NULL, 0, inRQueue.getMaxEles(), NULL)),
	midNode2OutQSem(CreateSemaphore(NULL, 0, outRQueue.getMaxEles(), NULL))
	
{
	tempInNode = new QUEUE_NODE;
	tempMidNode = new QUEUE_NODE;
	tempInNode->datalen = 4 * sizeof(dacData);
	tempInNode->data = (UCHAR *)malloc(tempInNode->datalen * sizeof(dacData));
	tempMidNode->datalen = tempInNode->datalen * 5;
	tempMidNode->data = (UCHAR *)malloc(tempMidNode->datalen * sizeof(dacData));
	//setInRQueue();
	//setOutRQueue();
}

InterfaceOfUSB::~InterfaceOfUSB()
{
	CloseHandle(inQ2MidNodeSem);
	CloseHandle(midNode2OutQSem);
}
bool InterfaceOfUSB::interfaceForIn(QUEUE_NODE inNode, HANDLE *mutex)
{
	if (!inRQueue.isFull())//add�����������жϣ���������2 falseʱ���������᷵�ش���
	{
		
		if (inRQueue.add(inNode, false))
		{
			ReleaseSemaphore(inQ2MidNodeSem, 1, NULL);//��Ӻ��ź�����1
			return true;
		}
			
		else//����ִ�е��Ĳ���
		{
			printf("interfaceForIn rToInterfaceQue.add() failed.\n");
			return false;
		}
	}
	else
	{
		if (inRQueue.add(inNode, true))
		{
			ReleaseSemaphore(inQ2MidNodeSem, 1, NULL);//��Ӻ��ź�����1
			printf("add inNode overwrite!\n");
			return true;
		}
			
		else
		{
			printf("interfaceForIn rToInterfaceQue.add() overwrite failed.\n");
			return false;
		}
		
	}
}
bool InterfaceOfUSB::interfaceStart(HjkDataFormat *hjkDataFormat, HANDLE *inMutex, HANDLE *outMutex)
{
	
	//��ͷ���ݴ���ʱ��Ҫ��֤����ʱ���зǿգ��Ҷ�ͷ�ڵ�û����д�����
	if (!inRQueue.isEmpty())
	{
		
		
		if (index < tempMidNode->datalen)
		{	
			WaitForSingleObject(inQ2MidNodeSem, TIMEOUT_MS);//�ź���Ϊ0ʱ�޷�����ִ��
			inRQueue.pull(tempInNode);//����ͷ�����ݴ���һ������
			//��������
			//����inNode��ֱ��tempMidNode��
			index += hjkDataFormat->waveDataFormat(tempInNode, tempMidNode, index);
			
		}
			
		else ////��tempMidNode�е�������ʱ�������������
		{
			//��������
			index = 0;

			if (!outRQueue.isFull())
			{
				//д��ʱ����
				
				outRQueue.add(*tempMidNode, false);
				ReleaseSemaphore(midNode2OutQSem, 1, NULL);//������к��ź���+1
			}
			else
			{
				outRQueue.add(*tempMidNode, true);
				ReleaseSemaphore(midNode2OutQSem, 1, NULL);//������к��ź���+1
				printf("add tempMidNode overwrite!\n");
			}
		}

	}
	else
	{
		//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������
	}
	return true;
}
bool InterfaceOfUSB::interfaceForUSBOut(QUEUE_NODE outNode)
{
	//rToUSBQue.pull(tempOutNode);//����ͷ�����ݴ���һ������
		
	if (!outRQueue.isEmpty())//pull�����������жϣ������п�ʱ������false
	{
		WaitForSingleObject(midNode2OutQSem, TIMEOUT_MS);//�ź�������0�������ͷ���û�н������޷�����
		if (outRQueue.pull(&outNode))//����ڵ��ַ
		{
			return true;
		}
		else
		{
			printf("interfaceForUSBOut rToUSBQue.pull() failed.\n");
			return false;
		}
	}
	else
	{
		printf("rToUSBQue is empty.\n");
		return true;
	}
		
}
/*
bool InterfaceOfUSB::interfaceForUSBOut( CCyUSBEndPoint * EndPt,OVERLAPPED inOvLap)
{
	if (!outRQueue.isEmpty)
		{
			//rToUSBQue.pull(tempOutNode);//����ͷ�����ݴ���һ������
			//�������ݵ�sub
			if (outRQueue.pull(tempOutNode))//����ڵ��ַ
			{
				
			}
			else printf("xferQueueNodeToUSB rToUSBQue.pull() failed.\n");

		}
		else
		{
			//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������
		}
	return true;
}
*/
//�����߼�����
/*
void xferThread()
{
	while (1) //����inNode��rToInterfaceQue���У�ֻҪ������в�Ϊ�վ���ӽڵ�,
	{
		if(!rToInterfaceQue.isFull)
			rToInterfaceQue.add(*inNode,false);
		else
		{
			rToInterfaceQue.add(*inNode, true);
			printf("add in node overwrite!\n");
		}
	}
	while (1)//���rToInterfaceQue����ͷ�ڵ㵽tempInNode���������ݡ�����tempMidNode��rToUSBQue����
	{
		if (!rToInterfaceQue.isEmpty)
		{
			rToInterfaceQue.pull(tempInNode);//����ͷ�����ݴ���һ������
			//��������
			//ѭ�����룬ֱ��tempMidNode��
			for(UINT index = 0; index < tempMidNode->datalen;)
				index+=waveDataFormat(tempInNode, tempMidNode,index);

			//��tempMidNode�е�������ʱ�������������
			if (!rToUSBQue.isFull)
			{
				rToUSBQue.add(*tempMidNode, false);
			}
			else
			{
				rToUSBQue.add(*tempMidNode, true);
				printf("add tempMidNode overwrite!\n");
			}

		}
		else
		{
			//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������
		}
	}
	while (1)//���rToUSBQue����ͷ�ڵ�
	{
		
		if (!rToUSBQue.isEmpty)
		{
			//rToUSBQue.pull(tempOutNode);//����ͷ�����ݴ���һ������
			//�������ݵ�sub
			xferQueueNodeToUSB(tempOutNode);

		}
		else
		{
			//�ȴ���ʱ�����debug��Ϣ�ȣ�ʱ��̫��USB�豸����ʡ��ģʽ����������
		}

		//
	}

}
*/

UINT InterfaceOfUSB::setInRQueue()
{
	//inRQueue.setMaxEles(qLen);
	//inRQueue = malloc(sizeof(QUEUE_NODE)*inRQueue.getMaxEles());
	inQueBuf = (UCHAR *)malloc(inRQueue.getMaxEles()*tempInNode->datalen);
	return sizeof(*inQueBuf);
}
UINT InterfaceOfUSB::setOutRQueue()
{
	//outRQueue.setMaxEles(qLen);
	outQueBuf = (UCHAR *)malloc(outRQueue.getMaxEles()*tempMidNode->datalen);

	return sizeof(*outQueBuf);
}
/*
QUEUE_NODE * InterfaceOfUSB::waveDataFormat()
{
	//��ȡinNode������
	if(!inRQueue.isEmpty())
	{
		inRQueue.pull(tempInNode);//����ͷ�����ݴ���һ������
		//��������
		{
			UINT size_x = tempInNode->datalen;
			//תΪ��ΪDACʹ�õ����ݣ�
			//��������ݵĳ���Ϊsize_x
		}
		
	}
	return nullptr;
}
*/
UINT InterfaceOfUSB::waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index)
{
	//����inNode�е�����
	//תΪ��ΪDACʹ�õ����ݣ�
	//��������ݵĳ���Ϊsize_x,����tem��
	
	UINT size_x = inNode->datalen;
	UCHAR *tem = inNode->data;

	//�������


	//�������index�����ܳ���midNode��datalen
	if (index + size_x > midNode->datalen -1)
	{
		printf("SERROR:out of the len of midNode's data.\n");
		exit(1);
	}
		
	//��������д���indexλ�ÿ�ʼ��midNode��
	(UCHAR*)memcpy(midNode->data + index, tem, size_x);	
	
	return size_x;
}/*
UINT InterfaceOfUSB::setUSBNodeBuf()
{
	//��ʼ����ʱ�ڵ�
	tempOutNode->datalen = MaxPktSize * PPX;
	memset(tempOutNode->data, 0xff, tempOutNode->datalen);
	return tempOutNode->datalen;
}
*/





