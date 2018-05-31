#ifndef _TICK_H
#define _TICK_H

#include "common.h"
//定义时间结构
typedef DWORD TICK;

//获取当前时间
TICK GetTicks(void);
//计算时间间隔
TICK DiffTicks(TICK start,TICK end);
//同步时间
void SynTicks(TICK tick);

#endif
