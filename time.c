#include "timeds.h"
#include "p24FJ64GA002.h"   
#include "tick.h"

/******************************************************************
��ʱ�Ѿ�����ADת���ˣ�ת���󽫵õ������ݽ���һһУ���������ٽ��д���
���÷�����ֻ��Ҫ����ADgetData�����Ϳ����ˡ�
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

/********************ʱ���ʼ��������ֹ����*************************/
void CurSysTimeInit(void)
{
	CurrentSysTime.year = 	0xff;
	CurrentSysTime.month = 	0x07;
	CurrentSysTime.day = 	0x07;
	CurrentSysTime.hour = 	0x12;
	CurrentSysTime.minute = 0x01;
	CurrentSysTime.second = 0x01;
	
}

/****************���㵱ǰʱ��******************************/

void CalCurSysTime(DWORD time_sys , RTIME *p)//��ǰ��ϵͳʱ�� 
{
	WORD hour,minute,second;

	time_sys /= 100;			//�����뻻����
	hour = (WORD)(time_sys / 3600);//����Сʱ�� 
	minute = (WORD)(time_sys / 60);//���������
	minute = minute % 60;//������ʾ�ķ�������ȥ����Сʱ 
	second = (WORD)(time_sys % 60); //������ʾ������
	
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
/********************ϵͳ��ʵʱ��**************************/
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