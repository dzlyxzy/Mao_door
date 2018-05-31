/************************************************************
文件描述：本文件主要是用来实现无限传感器网络的PHY层的基本功能，包括信道选择，
					激活射频状态，能量检测，发送和接收数据
版本信息：v1.0
修改时间：2008/03/
*************************************************************/
#ifndef _PHY_H
#define _PHY_H

#include "zigbee.h"
#include "driver.h"
#include "mcu.h"
#include "em16RTOS24.h"
#include "Nvm_Flash.h"


//定义信道的最大值
#define ConstRFChannelSize  16
//定义功率的最大等级数		
#define ConstRFPowerSize	18		
//定义波特率的最大等级数
#define ConstRFBaudRateSize	4


/**********不同通信速率，RSSI_OFFSET的值不一样****************
  2.4K	  71
  10K	  69
  250K    72
  500K    72
***************************************************************/

typedef RF_TRX_STATE PHY_TRX_STATE;

typedef struct _RHY_RX_BUFFER    //接收缓冲区
{
    BYTE RxBuffer[ConstPhyRxBufferSize];
	struct
	{
		WORD cWrite;  				//记录写入的位置
        WORD cRead;						//记录读出的位置
	}Postion;							//记录接收缓冲区读写位置
}PHY_RX_BUFFER; 


/*********************************************************************
 * 函数名:    void PHYInitSetup(void)
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE *pTxBuffer，写入值的指针，BYTE size写入的字节数
 * 输出参数:  无
 * 注意事项： 无
 * 功能描述:  实现物理层的一些基本设置，phyPIB，信道，速率，校验，RF的激活
 ********************************************************************/
void PHYInitSetup(void);
/*********************************************************************
 * 函数名:    BYTE PHYDetectEnergy(void);
 * 前提条件:  PHYInitSetup()已经调用
 * 输入参数:  无
 * 输出参数:  返回当前信道的能量值
 * 注意事项： 无
 * 功能描述:  用来检测信号的强弱
 ********************************************************************/
BYTE PHYDetectEnergy(void);//能量检测

/*********************************************************************
 * 函数名:    void PHYSetTxPower(BYTE cPower)
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE cPower，发射的功率值
 * 输出参数:  无
 * 注意事项： 无
 * 功能描述:  用于设定发射功率值
 ********************************************************************/
//设置发送功率
WORD PHYSetTxPower(BYTE Index);

//设置波特率
void PHYSetBaudRate(BYTE BaudRate);


/*********************************************************************
 * 函数名:    BOOL PHYRevPacket(void)
 * 前提条件:  接收到数据包后触发中断
 * 输入参数:  无
 * 输出参数:  如果接收到一个正确的数据包，返回TRUE
 * 注意事项： 如果接收到一个错误的数据包后，进行错误处理是关键中的关键
 * 功能描述:  把接收到的数据包都放到自定义的接收缓冲区中去
 ********************************************************************/
void PHYPut(BYTE cVal);
BYTE PHYGet(void);
void PHYGetArray(BYTE *ptr,BYTE cSize);

WORD PHYSetChannel(BYTE Index);
BOOL PHYExistPacket(void);
BYTE PHYGetLinkQuality(BYTE rssi);


#define PHY_RF_GDO0 	RF_GDO0
#define PHY_RF_GDO2 	RF_GDO2
#define PHYSetTRxState(state) RFSetTRxState(state)			//设置射频的收发状态
#define PHYGetTRxState() 	  RFGetTRxState 				//读取射频的状态

#define PHYSetBaud(v)		RFSetBaudRate(v)
#define PHYSetTxPwr(v)		RFSetTxPower(v)
#define PHYSetChan(v)		RFSetChannel(v)
#define PHYGetChannel()	    phyPIB.phyCurrentChannel	 //获取信道
#define PHYGetTxPower()		phypIB.phyTransmitPower
#define PHYDetectStatus() 	RFDetectStatus()

#define PHYGetTxNumber()	RFGetStatus(REG_TXBYTES)&0x7F;
#define PHYGetRxStatus()	RFGetStatus(REG_RXBYTES);
#define PHYDetectChannels()	RFGetStatus(REG_PKTSTATUS)&0x10 //取出CCA位0001 0000

#define PHYClearTx()		RFClearTxBuffer()
#define PHYClearRx()		RFClearRxBuffer() //RF FlushFIFO
#define PHYReadRx()			RFReadRxFIFO()  //从RF接收缓冲区中读一个数

#define PHYPutTxBuffer(ptr,size) RFWriteTxFIFO(ptr,size)
#define PHYTranmitByCSMA()	   	 RFTranmitByCSMA()	

WORD PHYGetPosition(void);
BOOL PHYPutRxBuffer(void);

#endif
