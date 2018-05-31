#ifndef _FLASH_MM_H
#define _FLASH_MM_H

#include "common.h"
#include "flash.h"

/*
Flash总共分为4块
第一块：0X8C00~0X8FFF
第二块：0X9000~0X93FF（其中，0X9000 = 0X8C00+8*128，一块有8页，一页有128个字节）
第三块：0X9400~0X97FF
第四块：0X9800~0X9C00
依次类推
*/

//定义存储区的起始物理地址、起始物理页
#define ConstPhyBeginAddr	0x008C00 				//定义FLASH的起始地址


//定义存储区的页数、块数、每页大小
#define ConstBlockSize		8       //定义块大小，每块包含的页数	
#define ConstTotalPhyPages	48	    //总共页数,6块
#define ConstPageSize		128     //每页大小按照字节来计算

//确定初始扇区,扇区实际上就是页
#define ConstPhyBeginSector	(ConstPhyBeginAddr/ConstPageSize)  //计算是按照字节来计算扇区
#define ConstDataBeginSector (ConstPhyBeginSector+ConstBlockSize)

#define ConstWordPageSize   (ConstPageSize/2)     //每页大小，按照字来计算，带计算的宏定义一定加括号
#define ConstDataPageSize	(ConstWordPageSize-4) //把128个字节中的最后8个字节拿出来存储CRC校验和状态和逻辑号

//定义坏扇区的大小
#define ConstBadSectorSize	(ConstTotalPhyPages-ConstBlockSize)		//最大为数据扇区大小，带计算的宏定义一定加括号
//定义数据页数多少，存储区页数去掉管理扇区页数
#define ConstDataSectorSize		(ConstTotalPhyPages-ConstBlockSize) //定义数据区大小带计算的宏定义一定加括号

//定义状态存储偏移

#define ConstStatusOffSet	(ConstWordPageSize-2) //偏移量是按照字来计算的，带计算的宏定义一定加括号，目前为62
#define ConstCheckSumOffSet	(ConstWordPageSize-3)	//定义存储CRC校验和的位置
#define ConstLogicOffSet	(ConstWordPageSize-1) //每页中存储逻辑地址的偏移量，按照字来计算，目前为63

//定义空
#define MEM_NULL	   0xFFFF //代表为空

//定义在此函数中调用的长度计算函数
#define lengthof(a)		(sizeof(a)/2)		//计算长度

//定义每页扇区状态,状态依次变迁
#define FREE    0x0F //扇区是空闲的
#define INVALID 0x0E //扇区是无效的
#define INUSE	0x0C //扇区是正在使用的
#define DIRTY	0x08 //扇区是可以擦除的


//定义整体FLASH信息
typedef struct _FLASH_INFO
{
	WORD  Version;	   //版本号
	WORD  EraseNumber; //擦除次数
	WORD  Identifier;  //用户标示
	WORD  BadSector[ConstDataSectorSize/sizeof(WORD)+1];//无效扇区，每位代表一个扇区
}FLASH_INFO;

//定义映射表
typedef struct _MEM_MAP_INFO
{
	WORD PhySector[ConstDataSectorSize]; //物理扇区,逻辑扇区用数组下标代替
	WORD FreeNumber;
	WORD DirtyNumber;
}MEM_MAP_INFO;

//擦数块，根据逻辑块，可以擦除管理块，也可以擦除数据块
void FlashEraseMem(WORD LogicBlockNum); 
//读写扇区，物理扇区号、偏移，偏移是按照字计算
WORD FlashPutMem(WORD PhySector,WORD OffSet,WORD *Ptr,WORD Number); 
WORD FlashGetMem(WORD PhySector,WORD OffSet,WORD *Ptr,WORD Number); 

//设置数据扇区的状态，输入参数是物理扇区号
void FlashPutStatus(WORD PhySector,WORD Status); 
WORD FlashGetStatus(WORD PhySector); 
void FlashSearchMemStatus(void);

//设置数据扇区的校验和，输入参数是物理扇区号
void FlashPutCheckSum(WORD PhySector,WORD CheckSum); 
WORD FlashGetCheckSum(WORD PhySector); 

//在管理扇区坏区表中，根据物理页来设置其出现坏区标志，0表示坏区，1表示正常
void FlashSetBadSector(WORD PhySector); 

//格式化存储区
void FlashFormatMem(void);		
//增加映射表的一条记录
WORD AddMemMapRecord(WORD LogicSector,WORD PageNum);
//删除映射表的一条记录
WORD RemoveMemMapRecord(WORD LogicSector);
//修改映射表记录
WORD RefreshMemMapRecord(WORD LogicSector,WORD PageNum);
//根据逻辑扇区得到物理扇区
WORD GetMemMapRecord(WORD LogSector);
//查询一个为空闲的物理扇区
WORD SearchFreePhySector(void);
//创建管理扇区表
#define FlashCreateMemInfo()	FlashGetMem(ConstPhyBeginSector,0,(WORD *)&MemInfo,lengthof(FLASH_INFO))
//创建映射表
void FlashCreateMapInfo(void);
//初始化
void FlashInitSetup(void);
//整理块把Dirty的擦除,逻辑块号
BOOL FlashCleanMem(WORD nBlkNum);
//存储区整理
void FlashManageMem(void);

//确定整理数据块的函数,思想是先找空闲扇区的数量，看能否把需要整理的内容写到空闲区域
//第二个是先整理那些每页中DIRTY的数量超过INUSE的
WORD FlashRefreshMem(void);

//读写数据，按照逻辑扇区来读写。提供给上层使用
WORD FlashWriteMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number); //写数据，根据逻辑扇区
WORD FlashReadMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number);  //读数据，根据逻辑扇区


#endif




