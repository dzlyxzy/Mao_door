#ifndef _ADDR_H
#define _ADDR_H

#include "common.h"
#include "zigbee.h"
#include "Interface.h"


//�Զ���ȡ����ַ�㷨
//void AllocIEEEAddr(LONG_ADDR *LongAddr);
//ָ�����㣬Rm^(Lm-d-1)
WORD CalcExponentVal(WORD v,BYTE exp);
//�Զ������ַ�㷨
SHORT_ADDR AllocShortAddr(WORD InitialVal) ;


#endif

