#ifndef _MCU_H
#define _MCU_H

#include "common.h"

/************************************************************
*SPI通信的引脚定义，本次采用软件模拟SPI通信过程,引脚的定义是相对于单片机来说
*************************************************************/
#define CS          LATBbits.LATB9       // 片选信号
#define SCLK        LATBbits.LATB13       // 时钟信号
#define SO          LATBbits.LATB14       // 串行输出
#define SI          PORTBbits.RB12      // 串行输入

/************************************************************
*与CC2500有关的状态引脚宏定义
*************************************************************/
#define RF_CHIP_RDYn     PORTBbits.RB12
#define RF_GDO0 		 PORTBbits.RB10  
#define RF_GDO0_IE	     CNEN2bits.CN16IE 
#define RF_GDO0_IF		 IFS1bits.CNIF
#define RF_GDO2			 PORTBbits.RB11

/***********************************************************/
//禁止射频的接收和发送中断
#define RFDint()  IEC1bits.INT1IE=0
#define RFEint()         \
{						 \
	IFS1bits.INT1IF=0;   \
	IEC1bits.INT1IE=1;   \
}

//定义看门狗清除
#define CLR_WDT() ClrWdt()
#define RESET()	  asm("RESET")


//进行一定功能引脚映射时候，解锁寄存器和上锁寄存器
void UnlockREG(void);
void LockREG(void);

//禁止中断，允许中断,一定要把中断嵌套打开
void emDint(WORD *prio);
void emEint(WORD *prio);

//SPI通信初始化
void SPIInit(void);
//定时器通信初始化
void InitTicks(void);
//再开启一个定时器
void InitTime(void);
//AD采样通信初始化
void ADInitSetup(void);
//输出控制初始化
void DeviceInitSetup(void);


//LED指示灯闪烁函数
void LEDInitSetup(void);
void LEDBlinkRed(void);
void LEDBlinkYellow(void);


#endif




