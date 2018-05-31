#include "mcu.h"


//本文档的功能是把与硬件有关的初始化都再次调用完成

//对CC2500的通信接口初始化
void SPIInit(void)
{
	TRISBbits.TRISB15=0; //PA
	TRISBbits.TRISB14=0; //SI
	TRISBbits.TRISB13=0;	//SCLK
	TRISBbits.TRISB12=1;	//SO
	TRISBbits.TRISB11=1;//定义为GDO2输入
	TRISBbits.TRISB10=1; //GDO0引脚，外部中断
	TRISBbits.TRISB9=0; //CS	
	TRISBbits.TRISB2=0; //LNA
}
void SPIInitCC2500off(void)
{
	TRISBbits.TRISB15=0; //PA
	TRISBbits.TRISB14=0; //SI
	TRISBbits.TRISB13=0;	//SCLK
	TRISBbits.TRISB12=0;	//SO
	TRISBbits.TRISB11=0;//定义为GDO2输入
	TRISBbits.TRISB10=0; //GDO0引脚，外部中断
	TRISBbits.TRISB9=0; //CS	
	TRISBbits.TRISB2=0; //LNA
}

//使用定时器1的初始化
void InitTicks(void)
{
	INTCON1bits.NSTDIS=0;//1禁止中断嵌套

	IFS0bits.T1IF=0;//清除定时器中断标志位
	IEC0bits.T1IE=1;//定时器1允许位
	//设置定时器1的优先级为7，最高优先级
	IPC0bits.T1IP=7;


	T1CONbits.TON=0;//关闭定时器，开始设置寄存器
	T1CONbits.TCS=0;//内部时钟源
	T1CONbits.TSIDL=0;//空闲模式下继续工作
	T1CONbits.TGATE=0;
	T1CONbits.TSYNC=0;
	//64分频
	T1CONbits.TCKPS1=1;
	T1CONbits.TCKPS0=0;
	
	//内部晶振是8MHZ，Fcy=Fosc/2
	//准备定时10ms=x/(8MHZ/2/64)
	TMR1=0x0000;
	PR1=0x0271;
	T1CONbits.TON=1;//开启定时器
}


//AD采样有关的初始化

void ADInitSetup(void)
{
	AD1CON2bits.VCFG=0;          //参考电压源
	AD1CON2&=0xEFFF;             //置位
	AD1CON2bits.CSCNA=0;       //输入扫描
	AD1CON2bits.SMPI=0;           //采样1次中断
	AD1CON2bits.BUFM=0;         //缓冲器模式
	AD1CON2bits.ALTS=0;          //MUX选择

	AD1CON3bits.ADRC=0;         //时钟源；1 = A/D 内部的RC 时钟；0 = 时钟由系统时钟产生
	AD1CON3bits.SAMC=2;         //自动采样时间
	AD1CON3bits.ADCS=3;         //转换时钟选择位Tad，时钟分频器
	AD1PCFG=0xF9FD;                 //1,9,10通道

	AD1CON1bits.FORM=0;         //输出整数
	AD1CON1bits.SSRC=0;     //7代表内部计数器结束采样并启动转换（自动转换）；0为清零SAMP 位结束采样并启动转换
	AD1CON1bits.ADSIDL=1;      //空闲模式停止
	AD1CON1bits.ASAM=0;         //0为SAMP 位置1 时开始采样；1为转换完成后自动启动，SAMP 位自动置1
	AD1CON1bits.ADON=1;         //启动
}

//输出控制引脚初始化
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
	SRbits.IPL=*prio; //其它中断源的优先级要设置为7
	INTCON1bits.NSTDIS=1;
}

void LEDInitSetup(void)
{
    TRISBbits.TRISB8=0;  //初始化LED P1引脚
    TRISBbits.TRISB7=0;  //初始化LED P2引脚
    PORTBbits.RB8=1;
    PORTBbits.RB7=1;
}

void LEDBlinkYellow(void)
{
    PORTBbits.RB8=0;
   	Delay(0x3000);
    PORTBbits.RB8=1;	//方向不同，到底引脚的设置不同 
}  
  
void LEDBlinkRed(void)
{
    PORTBbits.RB7=0;
   	Delay(0x3000);
    PORTBbits.RB7=1;   
}   