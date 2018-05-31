#include "common.h"
#include "flash.h"


//用户一次可编程闪存程序存储器的一行。 要实现此操作，有必要擦除包含该行在内的一个8行大小的块。
//程序存储器操作定义 将来要写到 NVMCON 中去的
#define PM_BLOCK_ERASE 		  0x404F  //整体擦除，只有在ICSP模式下使用
#define PM_PAGE_ERASE 		  0x4042  //页擦除命令,每页512条指令，8行
#define PM_ROW_WRITE 		  0x4001  //行写命令
#define PM_WORD_WRITE		  0x4003  //字写命令

#define CONFIG_WORD_WRITE	  0x4000  //寄存器配置


//引用汇编文件里的函数
extern void WriteLatch(WORD highAddr,WORD lowAddr,WORD highVal,WORD lowVal);
extern DWORD ReadLatch(WORD highAddr,WORD lowAddr);
extern void WriteMem(WORD cmd); 
extern void Erase(WORD highAddr,WORD lowAddr,WORD cmd);


void FLASHErasePage(uReg32 addr)
{
	WORD prio;
	emDint(&prio);
	Erase(addr.byte.HW,addr.byte.LW,PM_PAGE_ERASE);
	Delay(10);//20us
	emEint(&prio);
}

void FLASHGetArray(uReg32 addr,WORD *pRxBuf, WORD size)
{
	WORD i;
	uReg32 Temp;
	WORD prio;
	emDint(&prio);
	for(i=0;i<size;i++)
	{
		Temp.dwVal = ReadLatch(addr.byte.HW, addr.byte.LW);
		pRxBuf[i]=Temp.byte.LW;
		addr.dwVal+=2;
	}
	emEint(&prio);
	Nop();
}

void FLASHPutArray(uReg32 addr,const WORD *pTxBuf,WORD size)
{
	WORD i;
	uReg32 Temp;
	WORD prio;
	emDint(&prio);
	for(i=0;i<size;i++)
	{
		
		Temp.byte.LW=pTxBuf[i];
		Temp.byte.HW=0;
	    WriteLatch(addr.byte.HW, addr.byte.LW,Temp.byte.HW,Temp.byte.LW); // 先写到写锁存器中
		WriteMem(PM_WORD_WRITE); // 执行 行写 操作
		addr.dwVal+=2; //目标地址加2 指向下一个地 址
	}
	emEint(&prio);
	Nop();
}

void FLASHReadBytes(uReg32 addr,BYTE *pRxBuf, WORD size)
{
	WORD i;
	uReg32 Temp;
	WORD prio;
	emDint(&prio);
	for(i=0;i<size;i++)
	{
		Temp.dwVal = ReadLatch(addr.byte.HW, addr.byte.LW);
		pRxBuf[i]=(BYTE)Temp.byte.LW; //将WORD型截取低字节部分
		addr.dwVal+=2;
	}
	Nop();
	emEint(&prio);
	Nop();
}
void FLASHWriteBytes(uReg32 addr,const BYTE *pTxBuf,WORD size)
{
	WORD i;
	uReg32 Temp;
	WORD prio;
	emDint(&prio);
	for(i=0;i<size;i++)
	{
		
		Temp.byte.LW=(WORD)pTxBuf[i];//高字节部分补0
		Temp.byte.HW=0;
	    WriteLatch(addr.byte.HW, addr.byte.LW,Temp.byte.HW,Temp.byte.LW); // 先写到写锁存器中
		WriteMem(PM_WORD_WRITE); // 执行 行写 操作
		addr.dwVal+=2; //目标地址加2 指向下一个地址
	}
	Nop();
	emEint(&prio);
	Nop();
}



