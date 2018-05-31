#include "p24FJ64GA002.h"
#include "em16RTOS24.h"
#include "common.h"
#include "cc2500.h"
#include "spi.h"

extern BYTE Parameter[PARAMETER_NUM];
extern RTIME CurrentSysTime;

#define Caiyang_Times		5    //ÿ��AD�����Ĵ���
IO_INFO AD_Value[IO_NUM];   //�����������߿������ϵ�IO

BYTE dark=1;

void AD_Info(void)//����ַ������
{
	int i=0;
	for( i=0;i<5;i++)//��������0���¶ȣ�1-4�Ǹ�֪��ǿ��
	{
		AD_Value[i].IO_Pro= 0x80+i;
		AD_Value[i].type=0;
		AD_Value[i].Value=0;
	}
	for(i=5;i<10;i++)//�������5-8�Ǽ̵��������9�ǿյ�����
	{
		AD_Value[i].IO_Pro=0x40 + i - 4;
		AD_Value[i].type=0;
		AD_Value[i].Value=1;
	}
	AD_Value[0].type=0xFF;//��ʼ��Ϊ�Ƿ����������
}
void ADInit(void)                    //AD������ʼ��
{
  AD_Info();
  AD1CON2bits.VCFG = 0;              //�ο���ѹԴ
  AD1CON2 &= 0xEFFF;                 //��λ
  AD1CON2bits.CSCNA = 0;             //����ɨ��
  AD1CON2bits.SMPI = 0;              //����1���ж�
  AD1CON2bits.BUFM = 0;              //������ģʽ
  AD1CON2bits.ALTS = 0;              //MUXѡ��

  AD1CON3bits.ADRC = 0;              //ʱ��Դ��1 = A/D �ڲ���RC ʱ�ӣ�0 = ʱ����ϵͳʱ�Ӳ���
  AD1CON3bits.SAMC = 2;              //�Զ�����ʱ��
  AD1CON3bits.ADCS = 3;              //ת��ʱ��ѡ��λTad��ʱ�ӷ�Ƶ��
  AD1PCFG &= 0xFFF8;                  //��4��ANΪģ��ͨ��Ӧ����0xfff0����������james
                                                         
  AD1CON1bits.FORM = 0;              //�������
  AD1CON1bits.SSRC = 0;              //7�����ڲ���������������������ת�����Զ�ת������0Ϊ����SAMP                                     //λ��������������ת��
  AD1CON1bits.ADSIDL = 1;            //����ģʽֹͣ
  AD1CON1bits.ASAM = 0;              //0ΪSAMP λ��1 ʱ��ʼ������1Ϊת����ɺ��Զ�������SAMP λ�Զ���1
  AD1CON1bits.ADON = 1;              //����
}
void PutOut(void)//ѭ��ִ�е��������,ֱ�ӿ��Ƽ̵�����״̬
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
    AD1CHS = 0x00;      //ͨ��0   
    Delay(30);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    Tem1Dig[i] = ADC1BUF0;
    Tem1_sum += Tem1Dig[i];
    
    AD1CON1bits.SAMP = 1; //��ʼ����
    AD1CHS = 0x01;        //ͨ��1   
    Delay(30);            //��ʱ�Եȴ���������
    AD1CON1bits.SAMP = 0; //��������
    while (!AD1CON1bits.DONE);
    Tem2Dig[i] = ADC1BUF0; //ADת����ֵ����һ���������У����Ժ�ǰ���һ�� 
    Tem2_sum += Tem2Dig[i];

    AD1CON1bits.SAMP = 1;
    AD1CHS = 0x02;      //ͨ��2   
    Delay(30);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    Tem3Dig[i] = ADC1BUF0;
    Tem3_sum += Tem3Dig[i];
    
    AD1CON1bits.SAMP = 1; //��ʼ����
    AD1CHS = 0x03;        //ͨ��3   
    Delay(30);            //��ʱ�Եȴ���������
    AD1CON1bits.SAMP = 0; //��������
    while (!AD1CON1bits.DONE);
    Tem4Dig[i] = ADC1BUF0; //ADת����ֵ����һ���������У����Ժ�ǰ���һ�� 
    Tem4_sum += Tem4Dig[i];
  }

    Tem1_sum=Tem1_sum/Caiyang_Times;
	Tem2_sum=Tem2_sum/Caiyang_Times;
    Tem3_sum=Tem3_sum/Caiyang_Times;
	Tem4_sum=Tem4_sum/Caiyang_Times;
	AD_Value[1].Value=Tem2_sum;//��ů������
	AD_Value[2].Value=Tem1_sum;//�����Ӧ1
	AD_Value[3].Value=Tem4_sum;//
	AD_Value[4].Value=Tem3_sum;//��Ʒ���

	AD_Value[1].Value=(AD_Value[1].Value>200);//��ů������
	AD_Value[2].Value=(AD_Value[2].Value>200);//�����Ӧ1
	AD_Value[3].Value=(AD_Value[3].Value>200);//
	AD_Value[4].Value=(AD_Value[4].Value>200);//��Ʒ���
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
