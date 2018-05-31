#include "zigbee.h"
#include "phy.h"
#include "bootloader.h"
/*
�ŵ�æµ״̬�������Ϣ��ɾ����һֱû���õ���
������������Ϊ64����С����Ϊ7
*/
//PHY Constant
#define aMaxPHYPacketSize   64 //��������İ�����

//ȫ�ֱ�����Ҫ����Ӧ�����ļ��н��г�ʼ��
PHY_PIB phyPIB;					//�����PIB����
PHY_RX_BUFFER PhyRxBuffer;		//���ջ�����

BYTE ChanTab[ConstRFChannelSize]={0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0xA0,0xB0,0xC0,0xD0,0xE0,0xF0};
BYTE PowerTab[ConstRFPowerSize]={0x00,0x50,0x44,0xC0,0x84,0x81,0x46,0x93,0x55,0x8D,0xC6,0x97,0x6E,0x7F,0xA9,0xBB,0xFE,0xFF};


//����Ƶ�ʺ��ŵ�ɨ�������
/***********************************************************************
//����Ƶ��:2433MHZ
//ÿ�����ŵ���Ƶ����ȣ�200KHZ
//��256���ŵ����»��֣�ÿ��16��Ϊһ�����ŵ���Ƶ��Ϊ200*16=3200KHZ����16���ŵ�
***********************************************************************/

WORD PHYSetChannel(BYTE Index)
{
	if((Index<ConstRFChannelSize) && (Index>0))
	{
		PHYSetChan(ChanTab[Index-1]);
		return Index;
	}
	return InValid_Index;
}

//���÷��͹���
WORD PHYSetTxPower(BYTE Index)
{
	if((Index<ConstRFPowerSize) && (Index>0))
	{
		PHYSetTxPwr(PowerTab[Index-1]);
		return Index;
	}
	return InValid_Index;
}

//���ò�����
void PHYSetBaudRate(BYTE BaudRate)
{
	PHYSetBaud(BaudRate);	
}

BYTE PHYGetLinkQuality(BYTE rssi)
{
	BYTE cResult;
	INT8S cTemp;
	if(rssi>=128)
		cTemp=(rssi-256)/2-RSSI_OFFSET;
	else
		cTemp=rssi/2-RSSI_OFFSET;
	cResult=(BYTE)cTemp;
	return cResult;
}

	
void PHYInitSetup(void)
{
//	BYTE i;
//	BYTE index;
	//PIB���Գ�ʼ��
	phyPIB.phyCurrentChannel=5;
	phyPIB.phyChannelSuppoerted=16; //֧��16���ŵ�
//////////	phyPIB.phyTransmitPower=Parameter[route_high];//0-17
    phyPIB.phyTransmitPower=17;
	phyPIB.phyCCAMode=2;//ȡֵ��Χ1-3
	phyPIB.phyBaudRate=1;//1-4
	//���ջ�������ʼ��
	PhyRxBuffer.Postion.cWrite=0;
	PhyRxBuffer.Postion.cRead=0;
	
	PHYSetBaud(phyPIB.phyBaudRate);
	PHYSetTxPwr(PowerTab[phyPIB.phyTransmitPower]);
	PHYSetChannel(phyPIB.phyCurrentChannel);
	
	PHYSetTRxState(RF_TRX_RX);
}

void PHYPut(BYTE cVal)
{
	//�����ջ�����д������
	PhyRxBuffer.RxBuffer[PhyRxBuffer.Postion.cWrite]=cVal;
	//�޸�д��ָ��
	PhyRxBuffer.Postion.cWrite=(PhyRxBuffer.Postion.cWrite+1)%ConstPhyRxBufferSize;
}

BYTE PHYGet(void)
 {
	BYTE cReturn;
	//�ӽ��ջ������ж�ȡ����
  cReturn=PhyRxBuffer.RxBuffer[PhyRxBuffer.Postion.cRead];
	//�޸Ķ���ָ��
	PhyRxBuffer.Postion.cRead=(PhyRxBuffer.Postion.cRead+1)%ConstPhyRxBufferSize;
    return cReturn;
}

void PHYGetArray(BYTE *ptr,BYTE cSize)
{
	WORD i;
	for(i=0;i<cSize;i++)
		*ptr++=PHYGet();
}


BOOL PHYPutRxBuffer(void)
{
	WORD i;
	WORD prio;
	WORD cRest=0;
	BYTE cTemp;
    BYTE cSize;
	BYTE cValue;
	emDint(&prio);
	
    //�����ж��Ƿ�������
    cValue=PHYGetRxStatus();
	cValue=cValue&0x80;
	
    if(cValue==0x80)//�������
    {
		PHYClearRx();
		emEint(&prio);
		return FALSE;
    }

	cSize=PHYReadRx();
	cSize=cSize+2;			//���Ȱ���2���ֽڵ�CRC��RSSI
			
	if((cSize >= aMaxPHYPacketSize)||(cSize<7))
	{
		PHYClearRx();
		emEint(&prio);
		return FALSE;
	}	
	
	if(PhyRxBuffer.Postion.cWrite<PhyRxBuffer.Postion.cRead)
		cRest=PhyRxBuffer.Postion.cRead-PhyRxBuffer.Postion.cWrite-1;//��һ��Ŀ����ȥ�������ֽ�
	else
		cRest=ConstPhyRxBufferSize-PhyRxBuffer.Postion.cWrite+PhyRxBuffer.Postion.cRead-1;//дʱ��д����
	if(cRest>=cSize+1) //��ΪcSizeָʾ���ǲ����������ֽڵ�ʣ���ֽ���������Ҫ��1
	{
		PHYPut(cSize);	//д�볤��
		for(i=0;i<cSize;i++)
		{	cTemp=PHYReadRx();
			PHYPut(cTemp);
		}
		//ZXY MODIFIED
		emTxMesg(RF_REV_MESSAGE_NOTIFY,NRealTimeMesg,0,0);
		emEint(&prio); 
		return TRUE;
	}
	else
	{
		PHYClearRx();
		PhyRxBuffer.Postion.cWrite=0;//�����������������������������Ϣ
		PhyRxBuffer.Postion.cRead=0;
		emDelMesg(RF_REV_MESSAGE_NOTIFY);
		emEint(&prio);
		return FALSE;
	}

}
