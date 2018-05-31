#ifndef _MCU_H
#define _MCU_H

#include "common.h"

/************************************************************
*SPIͨ�ŵ����Ŷ��壬���β������ģ��SPIͨ�Ź���,���ŵĶ���������ڵ�Ƭ����˵
*************************************************************/
#define CS          LATBbits.LATB9       // Ƭѡ�ź�
#define SCLK        LATBbits.LATB13       // ʱ���ź�
#define SO          LATBbits.LATB14       // �������
#define SI          PORTBbits.RB12      // ��������

/************************************************************
*��CC2500�йص�״̬���ź궨��
*************************************************************/
#define RF_CHIP_RDYn     PORTBbits.RB12
#define RF_GDO0 		 PORTBbits.RB10  
#define RF_GDO0_IE	     CNEN2bits.CN16IE 
#define RF_GDO0_IF		 IFS1bits.CNIF
#define RF_GDO2			 PORTBbits.RB11

/***********************************************************/
//��ֹ��Ƶ�Ľ��պͷ����ж�
#define RFDint()  IEC1bits.INT1IE=0
#define RFEint()         \
{						 \
	IFS1bits.INT1IF=0;   \
	IEC1bits.INT1IE=1;   \
}

//���忴�Ź����
#define CLR_WDT() ClrWdt()
#define RESET()	  asm("RESET")


//����һ����������ӳ��ʱ�򣬽����Ĵ����������Ĵ���
void UnlockREG(void);
void LockREG(void);

//��ֹ�жϣ������ж�,һ��Ҫ���ж�Ƕ�״�
void emDint(WORD *prio);
void emEint(WORD *prio);

//SPIͨ�ų�ʼ��
void SPIInit(void);
//��ʱ��ͨ�ų�ʼ��
void InitTicks(void);
//�ٿ���һ����ʱ��
void InitTime(void);
//AD����ͨ�ų�ʼ��
void ADInitSetup(void);
//������Ƴ�ʼ��
void DeviceInitSetup(void);


//LEDָʾ����˸����
void LEDInitSetup(void);
void LEDBlinkRed(void);
void LEDBlinkYellow(void);


#endif




