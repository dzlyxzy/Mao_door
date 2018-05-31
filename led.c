#include "led.h"
#include "mac.h"
#include "common.h"
#include "bootloader.h"
#include "Mac.H"

WORD WenduSave[10]={0,0,0,0,0,0,0,0,0,0};
BYTE WenduCnt=0;
WORD tem_wendu;
extern BYTE Parameter[PARAMETER_NUM];
extern BYTE dark;
extern BYTE Type;//系统查询到系统消息TEM_SENT_REQ时，根据全局变量Type进行数据的回复
BYTE FLAG;
extern IO_INFO AD_Value[]; 

void Delay_nus(WORD i)
{	
	while(--i);
}


void DS18B20_Reset(void)
{
	WORD j=0;
	WORD k=0;
	DQ_LOW;
	Delay_nus(500);
	DQ_HIGH;
	Delay_nus(60);
	while(DQ)
	{
		j++;
		if(j>1000)break;//防死机
	}
	while(!DQ)
	{
		k++;
		if(k>1000)break;//防死机
	}
}
void DS18B20_Write_Byte(BYTE Data)
{
	BYTE i;
	for(i=0;i<8;i++)
	{
		DQ_LOW;
		Delay_nus(10);
		if(Data&0x01)
			DQ_HIGH;
		Delay_nus(40);
		DQ_HIGH;
		Delay_nus(2);
		Data>>=1;	
	}
}
BYTE DS18B20_Read_Byte(void)
{
	BYTE i,Data;
	for(i=0;i<8;i++)
	{
		Data>>=1;
		DQ_LOW;
		Delay_nus(1);
		DQ_HIGH;
		Delay_nus(15);
		if(DQ)
			Data|=0x80;
		Delay_nus(45);
	}
	return Data;
}

WORD DS18B20_Get_Temp(void)
{
	WORD temp1,temp2,temperature;


	DS18B20_Reset();
	DS18B20_Write_Byte(0xCC);
	DS18B20_Write_Byte(0x44);
	
	DS18B20_Reset();
	DS18B20_Write_Byte(0xCC);
	DS18B20_Write_Byte(0xBE);

	temp1=DS18B20_Read_Byte();
	temp2=DS18B20_Read_Byte();
	
	if(temp2&0xF0)
	{
		FLAG=1;
		temperature=(temp2<<8)|temp1;
		temperature=~temperature+1;
		temperature*=0.625;//扩大了10倍
	}
	else
	{
		temperature=(temp2<<8)|temp1;
		temperature*=0.625;
		FLAG=0;
	}
	return temperature;	
}

void GET_wendu(void)
{
	BYTE a,c=0;
	WORD i = 0;
	for(a=0;a<9;a++)
	{
		WenduSave[a]=WenduSave[a+1];
	}
	WenduSave[9]=DS18B20_Get_Temp();

	for(a=0;a<9;a++)
	{
		if( ((WenduSave[a]-WenduSave[a+1])< 50) || 
			((WenduSave[a]-WenduSave[a+1])>-50))	
		{
			c++;	
			i=i+WenduSave[a];
		}
	}
	if(c!=0)	
	{
		i=i/c;
		AD_Value[0].val1[0]=i&0x00ff;
		AD_Value[0].val1[1]=(i&0xff00)>>8;
		AD_Value[0].type=0x81;
        if(FLAG==1)     //flag为正，则为0x01
		{
			AD_Value[0].type=0x81;
		}
		else
		{
			AD_Value[0].type=0x01;
		}
	}
}

void Condi_Control(void)
{	
}

void TemTask(void)
{
	BYTE i;
	BYTE cSize = 0;
	BYTE cPtrTx[50];
	NODE_INFO macAddr;
	
	i=emWaitMesg(TEM_SENT_REQ,RealTimeMesg,0,0);
	if(i==1)
	{
   
	    macAddr.AddrMode = MAC_DST_SHORT_ADDR;
		macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
		macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
		MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);
        
            LATBbits.LATB1=0;//读取18B20的必备步骤
            tem_wendu=DS18B20_Get_Temp();

            cPtrTx[cSize++]=0x02;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
            cPtrTx[cSize++]=0x00; 
            cPtrTx[cSize++]=30;   //数据总长度30=0x1E
            cPtrTx[cSize++]=macPIB.macPANId.nVal>>8;
            cPtrTx[cSize++]=macPIB.macPANId.nVal;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal;

            cPtrTx[cSize++]=0x85;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=tem_wendu>>8;
            cPtrTx[cSize++]=tem_wendu;      //温度数据    

            cPtrTx[cSize++]=0x40;       //控制继电器状态
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=(BYTE)(AD_Value[1].Value>>8);
            //反馈为高电平表示上电
            cPtrTx[cSize++]=!(BYTE)(AD_Value[1].Value&0x00FF);

            cPtrTx[cSize++]=0x43;       //电灯控制继电器状态
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=(BYTE)(AD_Value[4].Value>>8);
            //反馈为低电平表示上电
            cPtrTx[cSize++]=!(BYTE)(AD_Value[4].Value&0x00FF);

		MACPutTxBuffer(cPtrTx,cSize);	
		MACTransmitPacket();
	}  
    i=emWaitMesg(NET_ADDR_ACK,RealTimeMesg,0,0);
	if(i==1)
	{
	    macAddr.AddrMode = MAC_DST_SHORT_ADDR;
		macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
		macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
		MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);
		
		cPtrTx[cSize++]=0xaa;
        cPtrTx[cSize++]=0xbb;
        cPtrTx[cSize++]=0xcc;
        cPtrTx[cSize++]=macPIB.macPANId.nVal>>8;
        cPtrTx[cSize++]=macPIB.macPANId.nVal;
        cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
        cPtrTx[cSize++]=macPIB.macShortAddr.nVal;

		MACPutTxBuffer(cPtrTx,cSize);	
		MACTransmitPacket();
        CoordID[0] = 0;
        CoordID[1] = 0;
	}
    
    i=emWaitMesg(TIME_DATA_REQ,RealTimeMesg,0,0);
	if(i==1)
	{
        macAddr.AddrMode = MAC_DST_SHORT_ADDR;
        macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
        macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
        MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);

        cPtrTx[cSize++]=0x0d;
        cPtrTx[cSize++]=CoordID[0];
        cPtrTx[cSize++]=CoordID[1];
        cPtrTx[cSize++]=0x00;
        cPtrTx[cSize++]=0x01;
        cPtrTx[cSize++]=CurrentSysTime.year;
        cPtrTx[cSize++]=CurrentSysTime.month;
        cPtrTx[cSize++]=CurrentSysTime.day;
        cPtrTx[cSize++]=CurrentSysTime.hour;
        cPtrTx[cSize++]=CurrentSysTime.minute;
        cPtrTx[cSize++]=CurrentSysTime.second;
        cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
        cPtrTx[cSize++]=macPIB.macShortAddr.nVal;

        MACPutTxBuffer(cPtrTx,cSize);	
        MACTransmitPacket();
        CoordID[0] = 0;
        CoordID[1] = 0;
	}
	
	CurrentTaskWait();
	SchedTask();
}

