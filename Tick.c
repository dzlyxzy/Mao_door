#include "tick.h"

//�趨һ��ʱ��Ƭ��ÿ���жϣ�dwSeconds����
#define MAX_COUNT_VALUE 0xFFFFFFFF

//����ʵ�ֶ�ʱ�����жϴ����ۼ�
extern unsigned long mSeconds;  //ȫ�ֱ������ĸ��ļ��ж��壬һ������Ӧ�ļ��г�ʼ��

//��ȡ��ǰʱ��
TICK GetTicks(void)
{
	return mSeconds;
}

//����ʱ����
TICK DiffTicks(TICK start,TICK end)
{
	TICK dwValue;
	if(end>=start)
		dwValue=end-start;
	else 
		dwValue=MAX_COUNT_VALUE+end-start;
	return dwValue;
}

//ͬ��ʱ��
void SynTicks(TICK tick)
{
	mSeconds=tick;
}

