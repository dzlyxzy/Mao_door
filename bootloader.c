#include "bootloader.h"
#include "mac.h"
#include "infra.h"


BYTE GetFlashFlag = 0;
BYTE RxBuf[96]={0};
BYTE Buffer[PM_ROW_SIZE*3];//从上位机收到的1536字节
BYTE Parameter[PARAMETER_NUM]=//各个参数的默认值
{
    10,     //定时上传时间
    0x00,      //xzy预留
    0x00,      //xzy预留
    5,
    0,
    0xff,
    0xff,   //时间段1和灯控选择1
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
    0xff,        //五个时间点
    0xff,
    17,
    71,
    45,
    4,
    0x34,      //转发地址route_high
    0x83,      //转发地址route_low
    
};

BInfo SourceAddr;
BInfo TacticsAddr;
BInfo codeAddr_hot;
BInfo codeAddr_cold;
BInfo codeAddr_off;

void BufferInit(void)// 将所有BUffer填充为FF
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
void GetAddr(BYTE *addr)//获取PM的地址
{
	BYTE i_GA;
	SourceAddr.Val[3]=0x00;//最高字节赋值为0
	for(i_GA=0;i_GA<3;i_GA++)
	{
		SourceAddr.Val[i_GA]=*addr++;
	}
}

void ErasePage(void)	//擦除一页
{
	Erase(SourceAddr.Word.HW,SourceAddr.Word.LW,PM_ROW_ERASE); // 页擦除
}

void WritePM(BYTE * ptrData, BInfo SourceAddr)  //写程序存储器
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
	    WriteLatch(SourceAddr.Word.HW, SourceAddr.Word.LW,Temp.Word.HW,Temp.Word.LW); // 先写到写锁存器中

		if((Size !=0) && (((Size + 1) % 64) == 0)) // 判断是不是正好是一行
		{
			WriteMem(PM_ROW_WRITE); // 执行 行写 操作
		}

		SourceAddr.Val32 = SourceAddr.Val32 + 2; //目标地址加2 指向下一个地址
	}  
}
void ReadPM(BYTE * ptrData, BInfo SourceAddr)   //读取程序存储器
{
	int    Size;
	//BInfo Temp;
 	WORD addrOffset;

	for(Size = 0; Size < PM_ROW_SIZE; Size++)  //读取8行数据
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
     以下是bootloader.c加载参数有关有关
     **********************************/
	ReadPM(Buffer,(BInfo)TacticsAddr);//从Flash中读出到数组RE中
	for(i=0;i<=PARAMETER_NUM;i++)
	{
		if(Buffer[i] != 0xFF)
		{
			Parameter[i]=Buffer[i];
			GetFlashFlag = 0x40;//已经获取到Flash中控制信息	
		}
	}
	BufferInit();
    
    /**********************************
               以下是空调有关
     **********************************/
    
    ReadPM(Buffer,(BInfo)codeAddr_hot);//从Flash中读出到数组RE中
    if((Buffer[0] != 0xFF)&&(Buffer[1] != 0xFF))
        sizecode_hot=((WORD)Buffer[1]<<8&0xff00)|((WORD)Buffer[0]&0x00ff);
    else 
        return 0;
	for(j=1;j<=sizecode_hot;j++)
	{
		codearray_hot[j]=((WORD)Buffer[j*2+1]<<8&0xff00)|((WORD)Buffer[j*2+0]&0x00ff);
	}
    BufferInit();
    
    
    ReadPM(Buffer,(BInfo)codeAddr_cold);//从Flash中读出到数组RE中
    if((Buffer[0] != 0xFF)&&(Buffer[1] != 0xFF))
        sizecode_cold=((WORD)Buffer[1]<<8&0xff00)|((WORD)Buffer[0]&0x00ff);
    else 
        return 0;
	for(j=1;j<=sizecode_cold;j++)
	{
		codearray_cold[j]=((WORD)Buffer[j*2+1]<<8&0xff00)|((WORD)Buffer[j*2+0]&0x00ff);
	}
    BufferInit();
    
    
    ReadPM(Buffer,(BInfo)codeAddr_off);//从Flash中读出到数组RE中
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