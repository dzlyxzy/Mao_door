#ifndef _ADDR_H
#define _ADDR_H

#include "common.h"
#include "zigbee.h"
#include "Interface.h"


//自动获取长地址算法
//void AllocIEEEAddr(LONG_ADDR *LongAddr);
//指数运算，Rm^(Lm-d-1)
WORD CalcExponentVal(WORD v,BYTE exp);
//自动分配地址算法
SHORT_ADDR AllocShortAddr(WORD InitialVal) ;


#endif

