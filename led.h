#ifndef _LED_H
#define _LED_H


#include "mcu.h"

void Light_Control(void);
void InOut_Control(void);
void Condi_Control(void);
void ADInit(void);
#define DQ_HIGH   TRISBbits.TRISB1=1
#define DQ_LOW    TRISBbits.TRISB1=0
#define DQ        PORTBbits.RB1


WORD DS18B20_Get_Temp(void);
void wendu_change(void);
void TemTask(void);

#endif
