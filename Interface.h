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

//CRCУ�����
CRC_RESULT CrcCalc(const BYTE *pBuf,WORD nSize);
//ָ������
WORD CalcExponent(BYTE num,BYTE exp);
//���ֵ
//WORD GetRandom(WORD rnd);

#endif
