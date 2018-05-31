#include "timeds.h"
#include "p24FJ64GA002.h"   
#include "tick.h"

/******************************************************************
此时已经可以AD转换了，转换后将得到的数据进行一一校正，进而再进行传递
调用方法：只需要调用ADgetData（）就可以了。
******************************************************************/
#include "common.h"
BYTE       Second;
BYTE       Minute;
BYTE       Hour;
BYTE       Day;
BYTE       Month;
BYTE       Year;
extern DWORD mSeconds;
TICK	   StartmSeconds;
RTIME ServerTime;
RTIME CurrentSysTime;

/********************时间初始化，防防止动作*************************/
void CurSysTimeInit(void)
{
	CurrentSysTime.year = 	0xff;
	CurrentSysTime.month = 	0x07;
	CurrentSysTime.day = 	0x07;
	CurrentSysTime.hour = 	0x12;
	CurrentSysTime.minute = 0x01;
	CurrentSysTime.second = 0x01;
	
}

/****************计算当前时间******************************/

void CalCurSysTime(DWORD time_sys , RTIME *p)//当前的系统时间 
{
	WORD hour,minute,second;

	time_sys /= 100;			//将毫秒换成秒
	hour = (WORD)(time_sys / 3600);//计算小时数 
	minute = (WORD)(time_sys / 60);//计算分钟数
	minute = minute % 60;//计算显示的分钟数即去掉了小时 
	second = (WORD)(time_sys % 60); //计算显示的秒数
	
	p->day = ServerTime.day;
	p->hour = ServerTime.hour + (BYTE)hour;
	p->minute = ServerTime.minute + (BYTE)minute;
	p->second = ServerTime.second + (BYTE)second;
	
	if(p->second >= 60)
	{
		p->second %= 60;
		p->minute += 1;
	}
	if (p->minute >= 60)
	{
		p->minute %= 60;
		p->hour += 1; 
	}
	if(p->hour >= 24)
	{
		p->hour %= 24;
		p->day += 1;
	}

}
/********************系统真实时间**************************/
void SysRealTime(void)
{	
	if(CurrentSysTime.year!=0xFF)
	{
		CalCurSysTime((mSeconds-StartmSeconds) , &CurrentSysTime);
	}
}              

TICK GetmSecinds(void)
{
	return mSeconds;
}  