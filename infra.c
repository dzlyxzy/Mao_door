#include "common.h"
#include "infra.h"
#include "Mac.H"
#include "bootloader.h"

WORD code_n=0;
BYTE code_flag=0;   //1:hot     2:cold     3:off
BYTE sent_flag=0;
WORD codearray_hot[CODELENTH]={0};
WORD codearray_cold[CODELENTH]={0};
WORD codearray_off[CODELENTH]={0};
WORD count=0;
WORD sizecode_hot = 0;
WORD sizecode_cold = 0;
WORD sizecode_off = 0;

void T3_Init(void)
{
	IPC2bits.T3IP = 6;
	IFS0bits.T3IF = 0;
	IEC0bits.T3IE = 1;	
	
	T3CONbits.TON = 0;
	T3CONbits.TCS = 0;
	T3CONbits.TSIDL = 0;
	T3CONbits.TGATE = 0;
	T3CONbits.TCKPS1 = 0;
	T3CONbits.TCKPS0 = 1;
	
	//内部晶振是8MHZ，Fcy=Fosc/2
	//准备定时n(ms)=x/(8MHZ/2)
	TMR3=0x0000;
	PR3=0xffff;
}

void IC1_Init(void)
{
    TRISBbits.TRISB4 = 1;
    RPINR7bits.IC1R = 0x04;
    IC1CON = 0x0000;
    IC1CON = 0x0001;
    IFS0bits.IC1IF = 0;
    IEC0bits.IC1IE = 1;
    IPC0bits.IC1IP = 6;
}

void T5_Init(void)
{
	IPC7bits.T5IP = 6;
	IFS1bits.T5IF = 0;
	IEC1bits.T5IE = 1;	
	
	T5CONbits.TON = 0;
	T5CONbits.TCS = 0;
	T5CONbits.TSIDL = 0;
	T5CONbits.TGATE = 0;
	T5CONbits.TCKPS1 = 0;
	T5CONbits.TCKPS0 = 1;
	
	//内部晶振是8MHZ，Fcy=Fosc/2
	//准备定时n(ms)=x/(8MHZ/2)
	TMR5=0x0000;
	PR5=0x0020;
}

void PWM_Init(void)
{
	TRISBbits.TRISB3=0;  
	RPOR1bits.RP3R =18;
	T2CON=0;
	TMR2=0;
	OC1CON=0x0000;  
	OC1R=0x0022;
	OC1RS=0x0022;
	OC1CON=0x0006;
	PR2=0x0067; 
	IPC1bits.T2IP = 6; 
	IFS0bits.T2IF = 0; 
	IEC0bits.T2IE = 1; 
}

void cleararray(void)
{
    WORD i;
    for(i=0;i<CODELENTH;i++)
    {
        codearray_hot[i]=0;
        codearray_cold[i]=0;
        codearray_off[i]=0;
    }
}


void infraInit(void)
{
    cleararray();
    IC1_Init();
    T3_Init();
    TRISAbits.TRISA2=0; //红外编码引脚设置为输出
    LOW;
	PWM_Init(); //pwm功能初始化
	T5_Init(); //定时器5初始化
    HC595Put(data);
    HC595Out();
}

BOOL hot_to_flash(void)
{
    WORD i=0;
    GetAddr(&codeAddr_hot);
    ReadPM(Buffer,SourceAddr);
    Buffer[0]=sizecode_hot;
    Buffer[1]=sizecode_hot>>8;
    for(i=1;i<=sizecode_hot;i++)
    {
        Buffer[i*2+0]=codearray_hot[i];     //先写低位
        Buffer[i*2+1]=codearray_hot[i]>>8;      //后写高位
    }  
    ErasePage();
    WritePM(Buffer, SourceAddr);
    BufferInit();//清空Buffer
    return 1;
}
BOOL cold_to_flash(void)
{
    WORD i=0;
    GetAddr(&codeAddr_cold);
    ReadPM(Buffer,SourceAddr);
    Buffer[0]=sizecode_cold;
    Buffer[1]=sizecode_cold>>8;
    for(i=1;i<=sizecode_cold;i++)
    {
        Buffer[i*2+0]=codearray_cold[i];     //先写低位
        Buffer[i*2+1]=codearray_cold[i]>>8;      //后写高位
    }  
    ErasePage();
    WritePM(Buffer, SourceAddr);
    BufferInit();//清空Buffer
    return 1;
}
BOOL off_to_flash(void)
{
    WORD i=0;
    GetAddr(&codeAddr_off);
    ReadPM(Buffer,SourceAddr);
    Buffer[0]=sizecode_off;
    Buffer[1]=sizecode_off>>8;
    for(i=1;i<=sizecode_off;i++)
    {
        Buffer[i*2+0]=codearray_off[i];     //先写低位
        Buffer[i*2+1]=codearray_off[i]>>8;      //后写高位
    }  
    ErasePage();
    WritePM(Buffer, SourceAddr);
    BufferInit();//清空Buffer
    return 1;
}

void infra_open(void)
{
    code_n=1;
    T1CONbits.TON=0;
	T2CONbits.TON=1;  //定时器2开启
	LATAbits.LATA2=0; //引脚拉低初始化
	T5CONbits.TON = 1;//开启定时器
}
 
void infra_sent_hot(void)
{
    if(code_n<=sizecode_hot)
    {
        if(1==code_n%2)
            {HIGH;PR5=codearray_hot[code_n++];}
        else if(0==code_n%2)
            {LOW;PR5=codearray_hot[code_n++];}     
    }
    else
        {T2CONbits.TON=0;T5CONbits.TON = 0;TMR5=0;T1CONbits.TON=1;code_flag=0;LEDBlinkYellow();LOW;}
}

void infra_sent_cold(void)
{
    if(code_n<=sizecode_cold)
    {
        if(1==code_n%2)
            {HIGH;PR5=codearray_cold[code_n++];}
        else if(0==code_n%2)
            {LOW;PR5=codearray_cold[code_n++];}     
    }
    else
        {T2CONbits.TON=0;T5CONbits.TON = 0;TMR5=0;T1CONbits.TON=1;code_flag=0;LEDBlinkYellow();LOW;}
}

void infra_sent_off(void)
{
    if(code_n<=sizecode_off)
    {
        if(1==code_n%2)
            {HIGH;PR5=codearray_off[code_n++];}
        else if(0==code_n%2)
            {LOW;PR5=codearray_off[code_n++];}     
    }
    else
        {T2CONbits.TON=0;T5CONbits.TON = 0;TMR5=0;T1CONbits.TON=1;code_flag=0;LEDBlinkYellow();LOW;}
}

void InfraTask(void)
{
	BYTE i,j,k;
    BYTE cSize = 0;
	BYTE cPtrTx[10];
	NODE_INFO macAddr;
    
	i=emWaitMesg(INFRAHOT_SENT_REQ,RealTimeMesg,0,0);
	if(i==1)
	{
        if(sizecode_hot>50)
        {
            sent_flag=HOT;  
            infra_open();
            Delay(0xffff);
        }        
	}
	
	j=emWaitMesg(INFRACOLD_SENT_REQ,RealTimeMesg,0,0);
	if(j==1)
	{        
        if(sizecode_cold>50)
        {
            sent_flag=COLD;
            infra_open();
            Delay(0xffff);
        }
	}
		
	k=emWaitMesg(INFRAOFF_SENT_REQ,RealTimeMesg,0,0);
	if(k==1)
	{
        if(sizecode_off>50)
        {
            sent_flag=OFF;
            infra_open();
            Delay(0xffff);
        }   
	}

	CurrentTaskWait();
	SchedTask();
}

