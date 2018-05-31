#include "common.h"
#include "flash.h"


//�û�һ�οɱ���������洢����һ�С� Ҫʵ�ִ˲������б�Ҫ���������������ڵ�һ��8�д�С�Ŀ顣
//����洢���������� ����Ҫд�� NVMCON ��ȥ��
#define PM_BLOCK_ERASE 		  0x404F  //���������ֻ����ICSPģʽ��ʹ��
#define PM_PAGE_ERASE 		  0x4042  //ҳ��������,ÿҳ512��ָ�8��
#define PM_ROW_WRITE 		  0x4001  //��д����
#define PM_WORD_WRITE		  0x4003  //��д����

#define CONFIG_WORD_WRITE	  0x4000  //�Ĵ�������


//���û���ļ���ĺ���
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
	    WriteLatch(addr.byte.HW, addr.byte.LW,Temp.byte.HW,Temp.byte.LW); // ��д��д��������
		WriteMem(PM_WORD_WRITE); // ִ�� ��д ����
		addr.dwVal+=2; //Ŀ���ַ��2 ָ����һ���� ַ
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
		pRxBuf[i]=(BYTE)Temp.byte.LW; //��WORD�ͽ�ȡ���ֽڲ���
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
		
		Temp.byte.LW=(WORD)pTxBuf[i];//���ֽڲ��ֲ�0
		Temp.byte.HW=0;
	    WriteLatch(addr.byte.HW, addr.byte.LW,Temp.byte.HW,Temp.byte.LW); // ��д��д��������
		WriteMem(PM_WORD_WRITE); // ִ�� ��д ����
		addr.dwVal+=2; //Ŀ���ַ��2 ָ����һ����ַ
	}
	Nop();
	emEint(&prio);
	Nop();
}



