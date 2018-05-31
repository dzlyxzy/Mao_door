#include "mcu.h"


//���ĵ��Ĺ����ǰ���Ӳ���йصĳ�ʼ�����ٴε������

//��CC2500��ͨ�Žӿڳ�ʼ��
void SPIInit(void)
{
	TRISBbits.TRISB15=0; //PA
	TRISBbits.TRISB14=0; //SI
	TRISBbits.TRISB13=0;	//SCLK
	TRISBbits.TRISB12=1;	//SO
	TRISBbits.TRISB11=1;//����ΪGDO2����
	TRISBbits.TRISB10=1; //GDO0���ţ��ⲿ�ж�
	TRISBbits.TRISB9=0; //CS	
	TRISBbits.TRISB2=0; //LNA
}
void SPIInitCC2500off(void)
{
	TRISBbits.TRISB15=0; //PA
	TRISBbits.TRISB14=0; //SI
	TRISBbits.TRISB13=0;	//SCLK
	TRISBbits.TRISB12=0;	//SO
	TRISBbits.TRISB11=0;//����ΪGDO2����
	TRISBbits.TRISB10=0; //GDO0���ţ��ⲿ�ж�
	TRISBbits.TRISB9=0; //CS	
	TRISBbits.TRISB2=0; //LNA
}

//ʹ�ö�ʱ��1�ĳ�ʼ��
void InitTicks(void)
{
	INTCON1bits.NSTDIS=0;//1��ֹ�ж�Ƕ��

	IFS0bits.T1IF=0;//�����ʱ���жϱ�־λ
	IEC0bits.T1IE=1;//��ʱ��1����λ
	//���ö�ʱ��1�����ȼ�Ϊ7��������ȼ�
	IPC0bits.T1IP=7;


	T1CONbits.TON=0;//�رն�ʱ������ʼ���üĴ���
	T1CONbits.TCS=0;//�ڲ�ʱ��Դ
	T1CONbits.TSIDL=0;//����ģʽ�¼�������
	T1CONbits.TGATE=0;
	T1CONbits.TSYNC=0;
	//64��Ƶ
	T1CONbits.TCKPS1=1;
	T1CONbits.TCKPS0=0;
	
	//�ڲ�������8MHZ��Fcy=Fosc/2
	//׼����ʱ10ms=x/(8MHZ/2/64)
	TMR1=0x0000;
	PR1=0x0271;
	T1CONbits.TON=1;//������ʱ��
}


//AD�����йصĳ�ʼ��

void ADInitSetup(void)
{
	AD1CON2bits.VCFG=0;          //�ο���ѹԴ
	AD1CON2&=0xEFFF;             //��λ
	AD1CON2bits.CSCNA=0;       //����ɨ��
	AD1CON2bits.SMPI=0;           //����1���ж�
	AD1CON2bits.BUFM=0;         //������ģʽ
	AD1CON2bits.ALTS=0;          //MUXѡ��

	AD1CON3bits.ADRC=0;         //ʱ��Դ��1 = A/D �ڲ���RC ʱ�ӣ�0 = ʱ����ϵͳʱ�Ӳ���
	AD1CON3bits.SAMC=2;         //�Զ�����ʱ��
	AD1CON3bits.ADCS=3;         //ת��ʱ��ѡ��λTad��ʱ�ӷ�Ƶ��
	AD1PCFG=0xF9FD;                 //1,9,10ͨ��

	AD1CON1bits.FORM=0;         //�������
	AD1CON1bits.SSRC=0;     //7�����ڲ���������������������ת�����Զ�ת������0Ϊ����SAMP λ��������������ת��
	AD1CON1bits.ADSIDL=1;      //����ģʽֹͣ
	AD1CON1bits.ASAM=0;         //0ΪSAMP λ��1 ʱ��ʼ������1Ϊת����ɺ��Զ�������SAMP λ�Զ���1
	AD1CON1bits.ADON=1;         //����
}

//����������ų�ʼ��
void DeviceInitSetup(void)
{ 
	TRISAbits.TRISA3=0;
	TRISAbits.TRISA4=0;
}


void UnlockREG(void)
{
	asm("push w1");
	asm("push w2");
    asm("push w3");
    asm("mov #OSCCON, w1");
    asm("mov #0x46, w2");
    asm("mov #0x57, w3");
    asm("mov.b w2, [w1]");
    asm("mov.b w3, [w1]");
    asm("bclr OSCCON, #6");
    asm("pop w3");
    asm("pop w2");
    asm("pop w1");
}

void LockREG(void)
{
	asm("push w1");
	asm("push w2");
    asm("push w3");
    asm("mov #OSCCON, w1");
    asm("mov #0x46, w2");
    asm("mov #0x57, w3");
    asm("mov.b w2, [w1]");
    asm("mov.b w3, [w1]");
    asm("bset OSCCON, #6");
    asm("pop w3");
    asm("pop w2");
    asm("pop w1");
}

void emDint(WORD *prio)
{
	*prio=SRbits.IPL;
	INTCON1bits.NSTDIS=0;
	SRbits.IPL=7;
}

void emEint(WORD *prio)
{
	INTCON1bits.NSTDIS=0;
	SRbits.IPL=*prio; //�����ж�Դ�����ȼ�Ҫ����Ϊ7
	INTCON1bits.NSTDIS=1;
}

void LEDInitSetup(void)
{
    TRISBbits.TRISB8=0;  //��ʼ��LED P1����
    TRISBbits.TRISB7=0;  //��ʼ��LED P2����
    PORTBbits.RB8=1;
    PORTBbits.RB7=1;
}

void LEDBlinkYellow(void)
{
    PORTBbits.RB8=0;
   	Delay(0x3000);
    PORTBbits.RB8=1;	//����ͬ���������ŵ����ò�ͬ 
}  
  
void LEDBlinkRed(void)
{
    PORTBbits.RB7=0;
   	Delay(0x3000);
    PORTBbits.RB7=1;   
}   