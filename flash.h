#ifndef _FLASH_H
#define _FLASH_H
#include "common.h"
#include "mcu.h"


typedef union flashtuReg32
{
	DWORD dwVal;
	struct
	{
		WORD LW;
		WORD HW;
	}byte;

	BYTE cVal[4];
} uReg32;

/*********************************************************************
 * 函数名:    FLASHErasePage(uReg32 addr);
 * 前提条件:  无
 * 输入参数:  uReg32 addr，要擦除的页地址，也就是512条指令的起始地址，
 * 输出参数:  无
 * 注意事项： addr,要能够被512整除
 * 功能描述:  擦除内部FLASH的一页
 ********************************************************************/
void FLASHErasePage(uReg32 addr);
/*********************************************************************
 * 函数名:    void FLASHReadValue(uReg32 addr,WORD *pRxBuf, WORD size);
 * 前提条件:  无
 * 输入参数:  uReg32 addr，要读取的地址；WORD *pRxBuf，读取数据存储指针；WORD size，读取数据长度
 * 输出参数:  无
 * 注意事项： 内部FLASH只存储在低字中，高字不存储数据
 * 功能描述:  从内部FLASH中读取一定长度的数据
 ********************************************************************/
void FLASHGetArray(uReg32 addr,WORD *pRxBuf, WORD size);
/*********************************************************************
 * 函数名:    void FLASHWriteValue(uReg32 addr,WORD *pTxBuf, WORD size);
 * 前提条件:  无
 * 输入参数:  uReg32 addr，要读取的地址；WORD *pTxBuf，写入数据存储指针；WORD size，数据长度
 * 输出参数:  无
 * 注意事项： 内部FLASH只存储在低字中，高字不存储数据
 * 功能描述:  往从内部FLASH中读取写入一定长度的数据
 ********************************************************************/
void FLASHPutArray(uReg32 addr,const WORD *pTxBuf, WORD size);


void FLASHReadBytes(uReg32 addr,BYTE *pRxBuf, WORD size);
void FLASHWriteBytes(uReg32 addr,const BYTE *pTxBuf,WORD size);
#endif
