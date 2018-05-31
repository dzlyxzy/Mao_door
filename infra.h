#ifndef _INFRA_H
#define _INFRA_H

#include "common.h"

extern BYTE Parameter[PARAMETER_NUM];

#define HIGH LATAbits.LATA2=1
#define LOW LATAbits.LATA2=0
#define DEVIATION 0  //本参数可变
#define CODELENTH 400
#define HOT     1
#define COLD    2
#define OFF     3

extern BYTE dark;
extern WORD code_n;
extern BYTE code_flag;   //1:hot     2:cold     3:off
extern BYTE sent_flag;
extern WORD codearray_hot[CODELENTH];
extern WORD codearray_cold[CODELENTH];
extern WORD codearray_off[CODELENTH];
extern WORD count;
extern WORD sizecode_hot;
extern WORD sizecode_cold;
extern WORD sizecode_off;

void cleararray(void);
void infraInit(void);
BOOL hot_to_flash(void);
BOOL cold_to_flash(void);
BOOL off_to_flash(void);
void IC1_Init(void);
void T5_Init(void);
void T3_Init(void);
void PWM_Init(void);

void infra_open(void);
void InfraTask(void);
void infra_sent_hot(void);
void infra_sent_cold(void);
void infra_sent_off(void);
#endif


