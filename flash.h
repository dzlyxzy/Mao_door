#ifndef _FLASH_H
#define _FLASH_H
#include "common.h"
#include "mcu.h"


typedef union flashtuReg32
{
	DWORD dwVal;
	struct
	{
		WORD LW;
		WORD HW;
	}byte;

	BYTE cVal[4];
} uReg32;

/*********************************************************************
 * ������:    FLASHErasePage(uReg32 addr);
 * ǰ������:  ��
 * �������:  uReg32 addr��Ҫ������ҳ��ַ��Ҳ����512��ָ�����ʼ��ַ��
 * �������:  ��
 * ע����� addr,Ҫ�ܹ���512����
 * ��������:  �����ڲ�FLASH��һҳ
 ********************************************************************/
void FLASHErasePage(uReg32 addr);
/*********************************************************************
 * ������:    void FLASHReadValue(uReg32 addr,WORD *pRxBuf, WORD size);
 * ǰ������:  ��
 * �������:  uReg32 addr��Ҫ��ȡ�ĵ�ַ��WORD *pRxBuf����ȡ���ݴ洢ָ�룻WORD size����ȡ���ݳ���
 * �������:  ��
 * ע����� �ڲ�FLASHֻ�洢�ڵ����У����ֲ��洢����
 * ��������:  ���ڲ�FLASH�ж�ȡһ�����ȵ�����
 ********************************************************************/
void FLASHGetArray(uReg32 addr,WORD *pRxBuf, WORD size);
/*********************************************************************
 * ������:    void FLASHWriteValue(uReg32 addr,WORD *pTxBuf, WORD size);
 * ǰ������:  ��
 * �������:  uReg32 addr��Ҫ��ȡ�ĵ�ַ��WORD *pTxBuf��д�����ݴ洢ָ�룻WORD size�����ݳ���
 * �������:  ��
 * ע����� �ڲ�FLASHֻ�洢�ڵ����У����ֲ��洢����
 * ��������:  �����ڲ�FLASH�ж�ȡд��һ�����ȵ�����
 ********************************************************************/
void FLASHPutArray(uReg32 addr,const WORD *pTxBuf, WORD size);


void FLASHReadBytes(uReg32 addr,BYTE *pRxBuf, WORD size);
void FLASHWriteBytes(uReg32 addr,const BYTE *pTxBuf,WORD size);
#endif
