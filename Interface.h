#ifndef _CRC_H
#define _CRC_H


#include "common.h"


typedef union _CRC_RESULT
{
	WORD nVal;
	BYTE cVal[2];
	struct
	{
		BYTE cLsb;
		BYTE cMsb;
	}byte;
}CRC_RESULT;

//CRC校验程序
CRC_RESULT CrcCalc(const BYTE *pBuf,WORD nSize);
//指数计算
WORD CalcExponent(BYTE num,BYTE exp);
//随机值
//WORD GetRandom(WORD rnd);

#endif
