#include "addr.h"
#include "Interface.h"
#include "stdlib.h"
#include "time.h"

WORD AllocAddr=1;
extern MAC_PIB macPIB;

//��ȡ���ֵ�㷨
/*WORD GetRANDValue(void)
{

	WORD Result;
	WORD regVal;
	regVal=AD1PCFG;		//�õ��Ĵ��������ã���AD1PCFG����Ϊ0ʱ��AD����Ϊģ��ģʽ��A/D�����ŵ�ѹ���в�����
	AD1PCFG=0xFFFE;       // ����ȫ���Ƴ�ANģʽ��AN0Ϊģ������                  
	AD1CON1=0; 			// ����ʱ��Դ
	AD1CON2=0;          // �����ο���ѹ                 
	AD1CON3=0x0002;     // Configure A/D conversion clock as Tcy/2
 	AD1CHS=0;        	// ����Ƶ��                  
	AD1CSSL=0;          // No inputs are scanned.
	IFS0bits.AD1IF=0;   // Clear A/D conversion interrupt.
	IEC0bits.AD1IE=0;   // Disable A/D conversion interrupt
	AD1CON1bits.ADON=1; // Turn on A/D

	AD1CON1bits.SAMP=1;                    //��ʼ����
	Delay(300);
	AD1CON1bits.SAMP=0;                
	while(!AD1CON1bits.DONE);               
	{
		AD1CON1bits.DONE=0;
		Result=ADC1BUF0;
	}
	AD1PCFG=regVal; //�ǳ��ؼ���������Ժ��ٻָ���ԭ�ȵ��趨ֵ
	return Result;	

}*/

//�Զ���ȡ��ַ�㷨
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

//�Զ������ַ�㷨
SHORT_ADDR AllocShortAddr(WORD InitialVal) 
{
	SHORT_ADDR ShortAddr;
	ShortAddr.nVal=InitialVal+AllocAddr;
	AllocAddr++;
	return ShortAddr;
}

