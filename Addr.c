#include "addr.h"
#include "Interface.h"
#include "stdlib.h"
#include "time.h"

WORD AllocAddr=1;
extern MAC_PIB macPIB;

//获取随机值算法
/*WORD GetRANDValue(void)
{

	WORD Result;
	WORD regVal;
	regVal=AD1PCFG;		//得到寄存器的设置；当AD1PCFG引脚为0时，AD引脚为模拟模式，A/D对引脚电压进行采样。
	AD1PCFG=0xFFFE;       // 首先全部制成AN模式，AN0为模拟输入                  
	AD1CON1=0; 			// 采样时钟源
	AD1CON2=0;          // 采样参考电压                 
	AD1CON3=0x0002;     // Configure A/D conversion clock as Tcy/2
 	AD1CHS=0;        	// 采样频道                  
	AD1CSSL=0;          // No inputs are scanned.
	IFS0bits.AD1IF=0;   // Clear A/D conversion interrupt.
	IEC0bits.AD1IE=0;   // Disable A/D conversion interrupt
	AD1CON1bits.ADON=1; // Turn on A/D

	AD1CON1bits.SAMP=1;                    //开始采样
	Delay(300);
	AD1CON1bits.SAMP=0;                
	while(!AD1CON1bits.DONE);               
	{
		AD1CON1bits.DONE=0;
		Result=ADC1BUF0;
	}
	AD1PCFG=regVal; //非常关键啊，完成以后再恢复到原先的设定值
	return Result;	

}*/

//自动获取地址算法
void AllocIEEEAddr(LONG_ADDR *LongAddr)
{
	srand(time(NULL));
	LongAddr->cVal[0]=rand()%256;
	LongAddr->cVal[1]=rand()%256;
	LongAddr->cVal[2]=rand()%256;
	LongAddr->cVal[3]=rand()%256;
	LongAddr->cVal[4]=rand()%256;
	LongAddr->cVal[5]=rand()%256;
	LongAddr->cVal[6]=rand()%256;
	LongAddr->cVal[7]=macPIB.CDNum;
/*
	BYTE i;
	WORD nTemp;
	BYTE cResult[16];
	CRC_RESULT checksum;
	srand(time(NULL));
	
	for(i=0;i<16;i++)
	{
//		cResult[i]=GetRANDValue();
		cResult[i]=rand()%256;
	}
	checksum=CrcCalc((BYTE *)cResult,16);
	while(checksum.nVal==0xFFFF)
	{
		for(i=0;i<5;i++)
		{
//			cResult[i]=GetRANDValue();
			cResult[i]=rand()%256;
		}
		checksum=CrcCalc((BYTE *)cResult,5);
	}

	LongAddr->cVal[0]=checksum.cVal[0];
	LongAddr->cVal[1]=checksum.cVal[1];

	nTemp=IEEE_PAN_ID;
	LongAddr->cVal[2]=nTemp;
	LongAddr->cVal[3]=nTemp>>8;
	nTemp=(WORD)AREA_CODE;
	LongAddr->cVal[4]=nTemp;
	LongAddr->cVal[5]=nTemp>>8;
	nTemp=(WORD)(AREA_CODE>>16);
	LongAddr->cVal[6]=nTemp;
	LongAddr->cVal[7]=nTemp>>8;
*/
}

//自动分配地址算法
SHORT_ADDR AllocShortAddr(WORD InitialVal) 
{
	SHORT_ADDR ShortAddr;
	ShortAddr.nVal=InitialVal+AllocAddr;
	AllocAddr++;
	return ShortAddr;
}

