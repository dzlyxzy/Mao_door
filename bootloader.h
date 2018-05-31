#include <stdio.h>
#include "common.h"

#define PM_ROW_SIZE 64 * 8  //����洢�� size ���� 
#define CM_ROW_SIZE 8

#define PM_ROW_ERASE 		  0x4042  //����洢���������� ����Ҫд�� NVMCON ��ȥ��
#define PM_ROW_WRITE 		  0x4001  //write 1 row

extern BInfo SourceAddr;
extern BInfo TacticsAddr;
extern BInfo codeAddr_hot;
extern BInfo codeAddr_cold;
extern BInfo codeAddr_off;
extern BYTE Buffer[PM_ROW_SIZE*3];//����λ���յ���1536�ֽ�
extern BYTE Parameter[PARAMETER_NUM];

void GetAddr(BYTE *addr);
void BufferInit(void);
void WritePM(BYTE * ptrData, BInfo SourceAddr);
void ReadPM(BYTE * ptrData, BInfo SourceAddr) ;
void ErasePage(void);
BOOL GetParameters(void);
