#ifndef _NVM_FLASH_H
#define _NVM_FLASH_H

#include "common.h"
#include "zigbee.h"
#include "mcu.h"
#include "flash.h"
#include "FlashMM.h"

#define IEEE_VALID_CODE 0x9968
#define FREQIndex		0x4452
#define TXPOWERIndex	0x3658
#define BAUDRATEIndex	0x7639


/**********************************************************************************************/
//利用程序存储器作为存储时的操作函数
/**********************************************************************************************/


//定义存储位置,逻辑扇区号
#define ConstIEEEAddrPN  		0
#define ConstMacAddrPN   		1
#define ConstStatusAddrPN  	 	2
#define ConstCoordAddrPN		3
#define ConstRTUAddrPN			4
#define ConstPhyFreqAddrPN		5
#define ConstPhyBaudRateAddrPN	6
#define ConstPhyTxPowerAddrPN	7
#define CntNeighborTabNO		8	

//存储分配的IEEE地址
void PutIEEEAddrInfo(WORD Indication,const LONG_ADDR *IEEEAddr);
//读取存储地址标志
BOOL GetIEEEIndication(WORD *Indication);
//读取自动获得地址信息
BOOL GetIEEEAddr(LONG_ADDR *IEEEAddr);
//读取状态
BOOL GetMACStatus(void);
//存储MAC层状态
void PutMACStatus(void);
//读出网络PANId
BOOL GetMACPANId(void);
//读出短地址
BOOL GetMACShortAddr(void);
//读出长地址
BOOL GetMACLongAddr(void);
//存储MAC地址
void PutMACAddr(void); 
//获取协调器信息
BOOL GetCoordDescriptor(void);
//存储协调器信息
void PutCoordDescriptor(void);
//获取信道
BOOL GetPHYFreq(WORD *Index);
//存储信道
void PutPHYFreq(WORD FreqIndex);
//获取发射功率
BOOL GetPHYTxPower(WORD *Index);
//存储发射功率
void PutPHYTxPower(WORD TxPowerIndex);
//获取波特率
BOOL GetPHYBaudRate(WORD *Index);
//存储波特率
void PutPHYBaudRate(WORD BaudRateIndex);

#endif
