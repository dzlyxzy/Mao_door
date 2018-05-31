#include <stdio.h>
#include "common.h"

#define PM_ROW_SIZE 64 * 8  //程序存储器 size 定义 
#define CM_ROW_SIZE 8

#define PM_ROW_ERASE 		  0x4042  //程序存储器操作定义 将来要写到 NVMCON 中去的
#define PM_ROW_WRITE 		  0x4001  //write 1 row

extern BInfo SourceAddr;
extern BInfo TacticsAddr;
extern BInfo codeAddr_hot;
extern BInfo codeAddr_cold;
extern BInfo codeAddr_off;
extern BYTE Buffer[PM_ROW_SIZE*3];//从上位机收到的1536字节
extern BYTE Parameter[PARAMETER_NUM];

void GetAddr(BYTE *addr);
void BufferInit(void);
void WritePM(BYTE * ptrData, BInfo SourceAddr);
void ReadPM(BYTE * ptrData, BInfo SourceAddr) ;
void ErasePage(void);
BOOL GetParameters(void);
