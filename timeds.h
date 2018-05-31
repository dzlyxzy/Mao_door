#ifndef _TIMEDS_H
#define _TIMEDS_H
#include "p24FJ64GA002.h"    
/******************************************************************
此函数是针对于时钟来说的，有年月日  时分秒
******************************************************************/
#include "common.h"
//存放日期和时间的结构体
typedef struct _TIME_FLAG	
{
    BYTE  t[6];
}TIME_FLAG;

void CalCurSysTime(DWORD time , RTIME *p);
void SysRealTime(void);   
void CurSysTimeInit(void); 
#endif
   