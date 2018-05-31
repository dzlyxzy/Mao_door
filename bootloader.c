#include "bootloader.h"
#include "mac.h"
#include "infra.h"


BYTE GetFlashFlag = 0;
BYTE RxBuf[96]={0};
BYTE Buffer[PM_ROW_SIZE*3];//����λ���յ���1536�ֽ�
BYTE Parameter[PARAMETER_NUM]=//����������Ĭ��ֵ
{
    10,     //��ʱ�ϴ�ʱ��
    0x00,      //xzyԤ��
    0x00,      //xzyԤ��
    5,
    0,
    0xff,
    0xff,   //ʱ���1�͵ƿ�ѡ��1
    11,
    0,
    0xff,
    0xff,
    14,
    0,
    0xff,
    0xff,
    17,
    0,
    0xff,
    0xff,
    22,
    0,      
    0xff,        //���ʱ���
    0xff,
    17,
    71,
    45,
    4,
    0x34,      //ת����ַroute_high
    0x83,      //ת����ַroute_low
    
};

BInfo SourceAddr;
BInfo TacticsAddr;
BInfo codeAddr_hot;
BInfo codeAddr_cold;
BInfo codeAddr_off;

void BufferInit(void)// ������BUffer���ΪFF
{
	WORD jj=0;
  for(jj=0;jj<96;jj++)
  {
   RxBuf[jj]=0xFF;
  }
  for(jj=0;jj<PM_ROW_SIZE*3;jj++)
  {
   Buffer[jj]=0xFF;
  }
    TacticsAddr.Val32=Tactics;
    codeAddr_hot.Val32=code8000;
    codeAddr_cold.Val32=code8400;
    codeAddr_off.Val32=code8800;
}					
void GetAddr(BYTE *addr)//��ȡPM�ĵ�ַ
{
	BYTE i_GA;
	SourceAddr.Val[3]=0x00;//����ֽڸ�ֵΪ0
	for(i_GA=0;i_GA<3;i_GA++)
	{
		SourceAddr.Val[i_GA]=*addr++;
	}
}

void ErasePage(void)	//����һҳ
{
	Erase(SourceAddr.Word.HW,SourceAddr.Word.LW,PM_ROW_ERASE); // ҳ����
}

void WritePM(BYTE * ptrData, BInfo SourceAddr)  //д����洢��
{
	int Size,Size1;
	BInfo Temp;

	for(Size = 0,Size1=0; Size < PM_ROW_SIZE; Size++)
	{
		
		Temp.Val[0]=ptrData[Size1+0];
		Temp.Val[1]=ptrData[Size1+1];
		Temp.Val[2]=ptrData[Size1+2];
		Temp.Val[3]=0;
		Size1+=3;
	    WriteLatch(SourceAddr.Word.HW, SourceAddr.Word.LW,Temp.Word.HW,Temp.Word.LW); // ��д��д��������

		if((Size !=0) && (((Size + 1) % 64) == 0)) // �ж��ǲ���������һ��
		{
			WriteMem(PM_ROW_WRITE); // ִ�� ��д ����
		}

		SourceAddr.Val32 = SourceAddr.Val32 + 2; //Ŀ���ַ��2 ָ����һ����ַ
	}  
}
void ReadPM(BYTE * ptrData, BInfo SourceAddr)   //��ȡ����洢��
{
	int    Size;
	//BInfo Temp;
 	WORD addrOffset;

	for(Size = 0; Size < PM_ROW_SIZE; Size++)  //��ȡ8������
	{
		//Temp.Val32 = ReadLatch(SourceAddr.Word.HW, SourceAddr.Word.LW);
		TBLPAG = ((SourceAddr.Val32 & 0x7F0000)>>16);
    	addrOffset = (SourceAddr.Val32 & 0x00FFFF);
		asm("tblrdl.b [%1], %0" : "=r"(ptrData[0]) : "r"(addrOffset));
		asm("tblrdl.b [%1], %0" : "=r"(ptrData[1]) : "r"(addrOffset + 1));
		asm("tblrdh.b [%1], %0" : "=r"(ptrData[2]) : "r"(addrOffset));
		/*ptrData[0] = Temp.Val[0];
		ptrData[1] = Temp.Val[1];
		ptrData[2] = Temp.Val[2];*/

		ptrData = ptrData + 3;

		SourceAddr.Val32 = SourceAddr.Val32 + 2;
	}
}

BOOL GetParameters(void)
{	
	BYTE i;
    WORD j;
    /**********************************
     ������bootloader.c���ز����й��й�
     **********************************/
	ReadPM(Buffer,(BInfo)TacticsAddr);//��Flash�ж���������RE��
	for(i=0;i<=PARAMETER_NUM;i++)
	{
		if(Buffer[i] != 0xFF)
		{
			Parameter[i]=Buffer[i];
			GetFlashFlag = 0x40;//�Ѿ���ȡ��Flash�п�����Ϣ	
		}
	}
	BufferInit();
    
    /**********************************
               �����ǿյ��й�
     **********************************/
    
    ReadPM(Buffer,(BInfo)codeAddr_hot);//��Flash�ж���������RE��
    if((Buffer[0] != 0xFF)&&(Buffer[1] != 0xFF))
        sizecode_hot=((WORD)Buffer[1]<<8&0xff00)|((WORD)Buffer[0]&0x00ff);
    else 
        return 0;
	for(j=1;j<=sizecode_hot;j++)
	{
		codearray_hot[j]=((WORD)Buffer[j*2+1]<<8&0xff00)|((WORD)Buffer[j*2+0]&0x00ff);
	}
    BufferInit();
    
    
    ReadPM(Buffer,(BInfo)codeAddr_cold);//��Flash�ж���������RE��
    if((Buffer[0] != 0xFF)&&(Buffer[1] != 0xFF))
        sizecode_cold=((WORD)Buffer[1]<<8&0xff00)|((WORD)Buffer[0]&0x00ff);
    else 
        return 0;
	for(j=1;j<=sizecode_cold;j++)
	{
		codearray_cold[j]=((WORD)Buffer[j*2+1]<<8&0xff00)|((WORD)Buffer[j*2+0]&0x00ff);
	}
    BufferInit();
    
    
    ReadPM(Buffer,(BInfo)codeAddr_off);//��Flash�ж���������RE��
    if((Buffer[0] != 0xFF)&&(Buffer[1] != 0xFF))
        sizecode_off=((WORD)Buffer[1]<<8&0xff00)|((WORD)Buffer[0]&0x00ff);
    else 
        return 0;
	for(j=1;j<=sizecode_off;j++)
	{
		codearray_off[j]=((WORD)Buffer[j*2+1]<<8&0xff00)|((WORD)Buffer[j*2+0]&0x00ff);
	}
    BufferInit();
    
    
    IEC0bits.IC1IE = 0;
    IEC0bits.T3IE = 0;
    return 1;
}