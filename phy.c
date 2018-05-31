#include "zigbee.h"
#include "phy.h"
#include "bootloader.h"
/*
信道忙碌状态的相关信息被删除，一直没有用到。
物理层的最大包长为64，最小包长为7
*/
//PHY Constant
#define aMaxPHYPacketSize   64 //物理层最大的包长度

//全局变量，要在相应定义文件中进行初始化
PHY_PIB phyPIB;					//物理层PIB属性
PHY_RX_BUFFER PhyRxBuffer;		//接收缓冲区

BYTE ChanTab[ConstRFChannelSize]={0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0xA0,0xB0,0xC0,0xD0,0xE0,0xF0};
BYTE PowerTab[ConstRFPowerSize]={0x00,0x50,0x44,0xC0,0x84,0x81,0x46,0x93,0x55,0x8D,0xC6,0x97,0x6E,0x7F,0xA9,0xBB,0xFE,0xFF};


//关于频率和信道扫描的事情
/***********************************************************************
//基带频率:2433MHZ
//每两个信道间频带宽度：200KHZ
//将256个信道重新划分，每隔16个为一个，信道间频宽为200*16=3200KHZ，共16个信道
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

//设置发送功率
WORD PHYSetTxPower(BYTE Index)
{
	if((Index<ConstRFPowerSize) && (Index>0))
	{
		PHYSetTxPwr(PowerTab[Index-1]);
		return Index;
	}
	return InValid_Index;
}

//设置波特率
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
	//PIB属性初始化
	phyPIB.phyCurrentChannel=5;
	phyPIB.phyChannelSuppoerted=16; //支持16个信道
//////////	phyPIB.phyTransmitPower=Parameter[route_high];//0-17
    phyPIB.phyTransmitPower=17;
	phyPIB.phyCCAMode=2;//取值范围1-3
	phyPIB.phyBaudRate=1;//1-4
	//接收缓冲区初始化
	PhyRxBuffer.Postion.cWrite=0;
	PhyRxBuffer.Postion.cRead=0;
	
	PHYSetBaud(phyPIB.phyBaudRate);
	PHYSetTxPwr(PowerTab[phyPIB.phyTransmitPower]);
	PHYSetChannel(phyPIB.phyCurrentChannel);
	
	PHYSetTRxState(RF_TRX_RX);
}

void PHYPut(BYTE cVal)
{
	//往接收缓冲区写入数据
	PhyRxBuffer.RxBuffer[PhyRxBuffer.Postion.cWrite]=cVal;
	//修改写入指针
	PhyRxBuffer.Postion.cWrite=(PhyRxBuffer.Postion.cWrite+1)%ConstPhyRxBufferSize;
}

BYTE PHYGet(void)
 {
	BYTE cReturn;
	//从接收缓冲区中读取数据
  cReturn=PhyRxBuffer.RxBuffer[PhyRxBuffer.Postion.cRead];
	//修改读出指针
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
	
    //首先判断是否接收溢出
    cValue=PHYGetRxStatus();
	cValue=cValue&0x80;
	
    if(cValue==0x80)//接收溢出
    {
		PHYClearRx();
		emEint(&prio);
		return FALSE;
    }

	cSize=PHYReadRx();
	cSize=cSize+2;			//长度包含2个字节的CRC和RSSI
			
	if((cSize >= aMaxPHYPacketSize)||(cSize<7))
	{
		PHYClearRx();
		emEint(&prio);
		return FALSE;
	}	
	
	if(PhyRxBuffer.Postion.cWrite<PhyRxBuffer.Postion.cRead)
		cRest=PhyRxBuffer.Postion.cRead-PhyRxBuffer.Postion.cWrite-1;//减一的目的是去掉长度字节
	else
		cRest=ConstPhyRxBufferSize-PhyRxBuffer.Postion.cWrite+PhyRxBuffer.Postion.cRead-1;//写时先写再增
	if(cRest>=cSize+1) //因为cSize指示的是不包含长度字节的剩余字节数，所以要加1
	{
		PHYPut(cSize);	//写入长度
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
		PhyRxBuffer.Postion.cWrite=0;//若缓冲区满，则清除缓冲区，和消息
		PhyRxBuffer.Postion.cRead=0;
		emDelMesg(RF_REV_MESSAGE_NOTIFY);
		emEint(&prio);
		return FALSE;
	}

}
