#ifndef _TICK_H
#define _TICK_H

#include "common.h"
//����ʱ��ṹ
typedef DWORD TICK;

//��ȡ��ǰʱ��
TICK GetTicks(void);
//����ʱ����
TICK DiffTicks(TICK start,TICK end);
//ͬ��ʱ��
void SynTicks(TICK tick);

#endif
