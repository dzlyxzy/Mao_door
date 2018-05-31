#ifndef _DRIVER_H
#define _DRIVER_H

#include "cc2500.h"
#include "common.h"
#include "spi.h"
#include "zigbee.h"
#include "Interface.h"
#include "Tick.h"

#define RSSI_OFFSET 71


typedef enum _RF_TRX_STATE	//射频芯片的状态
{
    RF_TRX_RX,
    RF_TRX_OFF,
    RF_TRX_IDLE,
    RF_TRX_TX
} RF_TRX_STATE;

/************************************************************
														driver.h
文件描述：本文件主要是用来实现对CC2500的基本操作，也是硬件驱动部分之一
版本信息：v1.0
修改时间：2008/03/
*************************************************************/

/*********************************************************************
 * 函数名:    void RFReset(void)
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  无
 * 输出参数:  无
 * 注意事项： 无
 * 功能描述:  初始化变量
 ********************************************************************/
void RFReset(void);
/*********************************************************************
 * 函数名:    void RFWriteStrobe(BYTE cmd)
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE cmd，需要写入的命令
 * 输出参数:  无
 * 注意事项： 无。
 * 功能描述:  对CC2500设置射命令
 ********************************************************************/
void RFWriteStrobe(BYTE cmd);//写CC2500命令
/*********************************************************************
 * 函数名:    void RFWriteReg(BYTE addr, BYTE value);
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE addr，寄存器地址，BYTE addr，需要配置的值
 * 输出参数:  无
 * 注意事项： 无
 * 功能描述:  对CC2500配置寄存器
 ********************************************************************/
void RFWriteReg(BYTE addr, BYTE value);//写寄存器值
/*********************************************************************
 * 函数名:    BYTE RFReadReg(BYTE addr);
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE addr，寄存器地址
 * 输出参数:  返回寄存器的值
 * 注意事项： 无
 * 功能描述:  读取CC2500配置寄存器
 ********************************************************************/
BYTE RFReadReg(BYTE addr);//读寄存器的值
/*********************************************************************
 * 函数名:    BYTE RFGetStatus(BYTE addr);
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE addr，寄存器地址
 * 输出参数:  返回CC2500当前的状态
 * 注意事项： 无
 * 功能描述:  读取CC2500状态
 ********************************************************************/
BYTE RFGetStatus(BYTE addr);
/*********************************************************************
 * 函数名:    void RFWriteBurstReg(BYTE addr,BYTE *pWriteValue,BYTE size);
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE addr，寄存器的初始地址；BYTE *pWriteValue存储写入数据值指针,
 *						BYTE size，是写入寄存器的个数
 * 输出参数:  无
 * 注意事项： 无
 * 功能描述:  连续写入CC2500配置寄存器
 ********************************************************************/
void RFWriteBurstReg(BYTE addr,BYTE *pWriteValue,BYTE size);//连续写几个寄存器的值
/*********************************************************************
 * 函数名:    void RFReadBurstReg(BYTE addr,BYTE *pReadValue,BYTE size);
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE addr，寄存器的初始地址；BYTE *pWriteValue存储读取的值指针,
 *						BYTE size，是读取寄存器的个数
 * 输出参数:  无
 * 注意事项： 无
 * 功能描述:  连续读取CC2500配置寄存器
 ********************************************************************/
void RFReadBurstReg(BYTE addr,BYTE *pReadValue,BYTE size);//连续读几个寄存器的值
/*********************************************************************
 * 函数名:    BYTE RFReadRxFIFO(void)
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  无
 * 输出参数:  BYTE，从RXFIFO中读取一个字节
 * 注意事项： 无
 * 功能描述:  从RXFIFO寄存器中读取一个字节
 ********************************************************************/
BYTE RFReadRxFIFO(void);//把接收到的数据读入接收缓冲区
/*********************************************************************
 * 函数名:    void RFWriteTxFIFO(BYTE *pTxBuffer,BYTE size)
 * 前提条件:  SPIInit()已经调用
 * 输入参数:  BYTE *pTxBuffer，写入值的指针，BYTE size写入的字节数
 * 输出参数:  无
 * 注意事项： 无
 * 功能描述:  往TXFIFO寄存器中写入数据
 ********************************************************************/
void RFWriteTxFIFO(BYTE *pTxBuffer,BYTE size);

void RFClearTxBuffer(void); //清空接收缓冲区
void RFClearRxBuffer(void); //清空发送缓冲区
void RFInitSetup(void);		//寄存器设置
BYTE RFDetectEnergy(void);	//能量检测
void RFSetChannel(BYTE channel);	//设置信道
void RFSetTxPower(BYTE power);		//设置发射功率
void RFSetBaudRate(BYTE BaudRate);

void RFSetTRxState(RF_TRX_STATE state); //设置RF状态
RF_TRX_STATE  RFGetTRxState(void);
void RFDetectStatus(void); //RF状态检测

BOOL RFTranmitByCSMA(void); //利用CSMA发送数据
#endif
