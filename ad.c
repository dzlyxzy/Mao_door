#include "p24FJ64GA002.h"
#include "em16RTOS24.h"
#include "common.h"
#include "cc2500.h"
#include "spi.h"

extern BYTE Parameter[PARAMETER_NUM];
extern RTIME CurrentSysTime;

#define Caiyang_Times		5    //每个AD采样的次数
IO_INFO AD_Value[IO_NUM];   //定义网络总线控制器上的IO

BYTE dark=1;

void AD_Info(void)//赋地址和类型
{
	int i=0;
	for( i=0;i<5;i++)//数字输入0是温度，1-4是感知和强制
	{
		AD_Value[i].IO_Pro= 0x80+i;
		AD_Value[i].type=0;
		AD_Value[i].Value=0;
	}
	for(i=5;i<10;i++)//数字输出5-8是继电器输出，9是空调控制
	{
		AD_Value[i].IO_Pro=0x40 + i - 4;
		AD_Value[i].type=0;
		AD_Value[i].Value=1;
	}
	AD_Value[0].type=0xFF;//初始化为非法避免误控制
}
void ADInit(void)                    //AD采样初始化
{
  AD_Info();
  AD1CON2bits.VCFG = 0;              //参考电压源
  AD1CON2 &= 0xEFFF;                 //置位
  AD1CON2bits.CSCNA = 0;             //输入扫描
  AD1CON2bits.SMPI = 0;              //采样1次中断
  AD1CON2bits.BUFM = 0;              //缓冲器模式
  AD1CON2bits.ALTS = 0;              //MUX选择

  AD1CON3bits.ADRC = 0;              //时钟源；1 = A/D 内部的RC 时钟；0 = 时钟由系统时钟产生
  AD1CON3bits.SAMC = 2;              //自动采样时间
  AD1CON3bits.ADCS = 3;              //转换时钟选择位Tad，时钟分频器
  AD1PCFG &= 0xFFF8;                  //低4个AN为模拟通道应该是0xfff0或者是其他james
                                                         
  AD1CON1bits.FORM = 0;              //输出整数
  AD1CON1bits.SSRC = 0;              //7代表内部计数器结束采样并启动转换（自动转换）；0为清零SAMP                                     //位结束采样并启动转换
  AD1CON1bits.ADSIDL = 1;            //空闲模式停止
  AD1CON1bits.ASAM = 0;              //0为SAMP 位置1 时开始采样；1为转换完成后自动启动，SAMP 位自动置1
  AD1CON1bits.ADON = 1;              //启动
}
void PutOut(void)//循环执行的输出任务,直接控制继电器的状态
{
    HC595Put(data);
    HC595Out();
}
void GET_Ganzhi(void)
{
  
  BYTE i;
  WORD Tem1Dig[Caiyang_Times]={0};
  WORD Tem2Dig[Caiyang_Times]={0};
  WORD Tem3Dig[Caiyang_Times]={0};
  WORD Tem4Dig[Caiyang_Times]={0};

  WORD Tem1_sum = 0;
  WORD Tem2_sum = 0;
  WORD Tem3_sum = 0;
  WORD Tem4_sum = 0;  

  for (i = 0; i < Caiyang_Times; i ++)
  {
    AD1CON1bits.SAMP = 1;
    AD1CHS = 0x00;      //通道0   
    Delay(30);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    Tem1Dig[i] = ADC1BUF0;
    Tem1_sum += Tem1Dig[i];
    
    AD1CON1bits.SAMP = 1; //开始采样
    AD1CHS = 0x01;        //通道1   
    Delay(30);            //延时以等待采样结束
    AD1CON1bits.SAMP = 0; //结束采样
    while (!AD1CON1bits.DONE);
    Tem2Dig[i] = ADC1BUF0; //AD转换的值都在一个缓冲器中，所以和前面的一样 
    Tem2_sum += Tem2Dig[i];

    AD1CON1bits.SAMP = 1;
    AD1CHS = 0x02;      //通道2   
    Delay(30);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    Tem3Dig[i] = ADC1BUF0;
    Tem3_sum += Tem3Dig[i];
    
    AD1CON1bits.SAMP = 1; //开始采样
    AD1CHS = 0x03;        //通道3   
    Delay(30);            //延时以等待采样结束
    AD1CON1bits.SAMP = 0; //结束采样
    while (!AD1CON1bits.DONE);
    Tem4Dig[i] = ADC1BUF0; //AD转换的值都在一个缓冲器中，所以和前面的一样 
    Tem4_sum += Tem4Dig[i];
  }

    Tem1_sum=Tem1_sum/Caiyang_Times;
	Tem2_sum=Tem2_sum/Caiyang_Times;
    Tem3_sum=Tem3_sum/Caiyang_Times;
	Tem4_sum=Tem4_sum/Caiyang_Times;
	AD_Value[1].Value=Tem2_sum;//供暖阀反馈
	AD_Value[2].Value=Tem1_sum;//人体感应1
	AD_Value[3].Value=Tem4_sum;//
	AD_Value[4].Value=Tem3_sum;//电灯反馈

	AD_Value[1].Value=(AD_Value[1].Value>200);//供暖阀反馈
	AD_Value[2].Value=(AD_Value[2].Value>200);//人体感应1
	AD_Value[3].Value=(AD_Value[3].Value>200);//
	AD_Value[4].Value=(AD_Value[4].Value>200);//电灯反馈
/**/
}

void InOut_Control(void)
{
	if((CurrentSysTime.hour == Parameter[h1])&&(CurrentSysTime.minute == Parameter[m1]))
        data = ((WORD)Parameter[open_data1_0]<<8&0xff00)|((WORD)Parameter[open_data1]&0x00ff);
    if((CurrentSysTime.hour == Parameter[h2])&&(CurrentSysTime.minute == Parameter[m2]))
        data = ((WORD)Parameter[open_data2_0]<<8&0xff00)|((WORD)Parameter[open_data2]&0x00ff);
    if((CurrentSysTime.hour == Parameter[h3])&&(CurrentSysTime.minute == Parameter[m3]))
        data = ((WORD)Parameter[open_data3_0]<<8&0xff00)|((WORD)Parameter[open_data3]&0x00ff);
    if((CurrentSysTime.hour == Parameter[h4])&&(CurrentSysTime.minute == Parameter[m4]))
        data = ((WORD)Parameter[open_data4_0]<<8&0xff00)|((WORD)Parameter[open_data4]&0x00ff);
    if((CurrentSysTime.hour == Parameter[h5])&&(CurrentSysTime.minute == Parameter[m5]))   
        data = ((WORD)Parameter[open_data5_0]<<8&0xff00)|((WORD)Parameter[open_data5]&0x00ff);
}
