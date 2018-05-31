#include "tick.h"

//设定一个时间片，每到中断，dwSeconds自增
#define MAX_COUNT_VALUE 0xFFFFFFFF

//用于实现定时器的中断次数累计
extern unsigned long mSeconds;  //全局变量在哪个文件中定义，一定在相应文件中初始化

//获取当前时间
TICK GetTicks(void)
{
	return mSeconds;
}

//计算时间间隔
TICK DiffTicks(TICK start,TICK end)
{
	TICK dwValue;
	if(end>=start)
		dwValue=end-start;
	else 
		dwValue=MAX_COUNT_VALUE+end-start;
	return dwValue;
}

//同步时间
void SynTicks(TICK tick)
{
	mSeconds=tick;
}

