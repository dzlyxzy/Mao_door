#ifndef _TIMEDS_H
#define _TIMEDS_H
#include "p24FJ64GA002.h"    
/******************************************************************
�˺����������ʱ����˵�ģ���������  ʱ����
******************************************************************/
#include "common.h"
//������ں�ʱ��Ľṹ��
typedef struct _TIME_FLAG	
{
    BYTE  t[6];
}TIME_FLAG;

void CalCurSysTime(DWORD time , RTIME *p);
void SysRealTime(void);   
void CurSysTimeInit(void); 
#endif
   