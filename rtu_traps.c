/**********************************************************************
* Microchip Technology Inc.
*
* FileName:        traps.c
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       PIC24Fxxxx
* Compiler:        MPLAB?C30 v3.00 or higher
*
**********************************************************************/

#include "common.h"
#include "driver.h"
#include "infra.h"
#include "Phy.h"
#include "Mac.H"

unsigned char infrared_times = 0;
unsigned char infrared_correct = 0;

unsigned char testnumber=0;

void __attribute__((__interrupt__)) _OscillatorFail(void);
void __attribute__((__interrupt__)) _AddressError(void);
void __attribute__((__interrupt__)) _StackError(void);
void __attribute__((__interrupt__)) _MathError(void);

void __attribute__((__interrupt__)) _AltOscillatorFail(void);
void __attribute__((__interrupt__)) _AltAddressError(void);
void __attribute__((__interrupt__)) _AltStackError(void);
void __attribute__((__interrupt__)) _AltMathError(void);

//中断入口

void __attribute__((__interrupt__)) _T1Interrupt(void);
void __attribute__((__interrupt__)) _INT1Interrupt(void);

extern DWORD mSeconds;
extern DWORD dwTimes;


void __attribute__((interrupt, no_auto_psv)) _OscillatorFail(void)
{
        INTCON1bits.OSCFAIL = 0;        //Clear the trap flag
        while (1);
}

void __attribute__((interrupt, no_auto_psv)) _AddressError(void)
{
        INTCON1bits.ADDRERR = 0;        //Clear the trap flag
        while (1);
}
void __attribute__((interrupt, no_auto_psv)) _StackError(void)
{
        INTCON1bits.STKERR = 0;         //Clear the trap flag
        while (1);
}

void __attribute__((interrupt, no_auto_psv)) _MathError(void)
{
        INTCON1bits.MATHERR = 0;        //Clear the trap flag
        while (1);
}



void __attribute__((interrupt, no_auto_psv)) _AltOscillatorFail(void)
{
        INTCON1bits.OSCFAIL = 0;
        while (1);
}

void __attribute__((interrupt, no_auto_psv)) _AltAddressError(void)
{
        INTCON1bits.ADDRERR = 0;
        while (1);
}

void __attribute__((interrupt, no_auto_psv)) _AltStackError(void)
{
        INTCON1bits.STKERR = 0;
        while (1);
}

void __attribute__((interrupt, no_auto_psv)) _AltMathError(void)
{
        INTCON1bits.MATHERR = 0;
       while (1);
}
//定义中断入口
void __attribute__((interrupt,no_auto_psv)) _T1Interrupt(void)
{
	TMR1=0x0000;
	mSeconds++;
	IFS0bits.T1IF=0;//这个一定要
}

void __attribute__((interrupt,no_auto_psv)) _INT1Interrupt(void)
{
//////    if(PHYPutRxBuffer())//否则是接收进入的中断
//////	{
//////		Nop();
//////	}
//////	IFS1bits.INT1IF=0;
    emDelHardTimer(5);
    emStartHardTimer(5);
    IFS1bits.INT1IF=0;
}

void __attribute__((interrupt,no_auto_psv)) _INT2Interrupt(void)
{
    //第一路继电器线圈通电
    data=(data|0x0008);
    HC595Put(data);
    HC595Out();
    emStartHardTimer(7);

	IFS1bits.INT2IF=0;

}


void __attribute__((interrupt,no_auto_psv)) _T2Interrupt(void)
{
	TMR2=0;
	IFS0bits.T2IF = 0; 
}

void __attribute__((interrupt,no_auto_psv)) _T5Interrupt(void)
{
    switch(sent_flag)
    {
        case HOT:
            infra_sent_hot();
            break;
        case COLD:
            infra_sent_cold();
            break;
        case OFF:
            infra_sent_off();
            break;
        default:
            break;
    }
	IFS1bits.T5IF = 0; 
}

/*
 *****************************空调自学习得输入捕捉中断服务子程序****************************
 */
//void __attribute__((interrupt,no_auto_psv)) _IC1Interrupt(void)
//{
//    T3CONbits.TON = 1;
//    switch(code_flag)
//    {
//        case HOT:
//            codearray_hot[count++] = IC1BUF;
//            break;
//        case COLD:
//            codearray_cold[count++] = IC1BUF;
//            break;
//        case OFF:
//            codearray_off[count++] = IC1BUF;
//            break;
//        default:
//            break;
//    }
//    TMR3 = 0;
//	IFS0bits.IC1IF = 0; 
//}

/*
 *****************************红外辅助定位输入捕捉中断服务子程序****************************
 */
void __attribute__((interrupt,no_auto_psv)) _IC1Interrupt(void)
{
//    T3CONbits.TON = 1;
//////    capture1 = IC1BUF;
//////	if((capture1>0x1200)&&(capture1<0x1500))
//////	{
//////		infrared_times++;
//////        if(4 == infrared_times)
//////        {
//////            infrared_times = 1;
//////            LEDBlinkYellow();
//////            infrared_times = 0;
//////            
//////        }
//////	}		
//////    else
//////    {
//////        infrared_correct = 0;
//////        infrared_times = 0;
//////    }
//////    TMR3 = 0;
    infrared_times++;
    if(infrared_times == 4)
    {    
        LEDBlinkYellow();
        infrared_times = 0;
        infrared_correct = 1;
    }
    
        IFS0bits.IC1IF = 0; 
}

void __attribute__((interrupt,no_auto_psv)) _T3Interrupt(void)
{
	IFS0bits.T3IF = 0; 
}
