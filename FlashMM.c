#include "FlashMM.h"

/*
修改日期：2016年8月8日
修改者：李延超
特别说明：
一个是向FLASH中写入函数。FlashWriteMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number)
该函数实现流程是自己写的。
首先向设定的页中写数据，当换页时，首先将上一次写的页置为Dirty页，然后再写下一页。Dirty
作为换页的一个标志。所以，存在的一个bug是如果你写完这一页，然后写另一页，再回来写这一页的
时候会无法写入。另外，需要修改初始化函数FlashCreateMapInfo()。当是dirty页时，也要增加逻辑页
和物理页之间的对应，否则读取不正确。

*/

//定位要进行定位的地址，备份页开始的地方，为了进行擦除。
WORD LctionAddr = 296; //对应的地址为0X9400，实际上是从第三块开始的，作为备份块使用。


//定义FLASH属性信息
FLASH_INFO MemInfo; 
//定义映射表,映射表物理扇区从数据区开始，不包含管理区
MEM_MAP_INFO MemMapInfo;

//擦除
void FlashEraseMem(WORD LogicBlockNum) //输入为逻辑块号
{
	uReg32 addr;
	//地址=起始地址+块号*每块字节数
	addr.dwVal=ConstPhyBeginAddr+LogicBlockNum*ConstBlockSize*ConstPageSize;
	FLASHErasePage(addr); //该函数中已经延时了
}


//OffSet为字的开始
WORD FlashPutMem(WORD PhySector,WORD OffSet,WORD *Ptr,WORD Number) //输入为物理页
{
	WORD Size;
	uReg32 addr;

	//地址=起始地址页号*每页字节数，起始地址包含了管理部分
	addr.dwVal=PhySector*ConstPageSize+2*OffSet; //
	if(Number<=ConstDataPageSize) //每页去掉8个字节状态位和逻辑单元
		Size=Number;
	else
		Size=ConstDataPageSize; //这是字
	FLASHPutArray(addr,Ptr,Size);
	CLR_WDT();
	return Size;
}

WORD FlashGetMem(WORD PhySector,WORD OffSet,WORD *Ptr,WORD Number) //输入为物理页
{
	WORD Size;
	uReg32 addr;
	//地址=起始地址页号*每页字节数，起始地址包含了
	addr.dwVal=PhySector*ConstPageSize+2*OffSet;
	if(Number<=ConstDataPageSize) //需要去掉几个状态字节
		Size=Number;
	else
		Size=ConstDataPageSize;//这是字的大小
	FLASHGetArray(addr,Ptr,Size);
	CLR_WDT();
	return Size;
}

//写入每页的状态
void FlashPutStatus(WORD PhySector,WORD Status) //输入为物理页
{
	FlashPutMem(PhySector,ConstStatusOffSet,(WORD *)&Status,lengthof(WORD));
}


WORD FlashGetStatus(WORD PhySector) //输入为物理页
{
	WORD Status;
	FlashGetMem(PhySector,ConstStatusOffSet,(WORD *)&Status,lengthof(WORD));
	return Status;

}

//写入每页的CRC校验和
void FlashPutCheckSum(WORD PhySector,WORD CheckSum)
{
	FlashPutMem(PhySector,ConstCheckSumOffSet,(WORD *)&CheckSum,lengthof(WORD));	
}

WORD FlashGetCheckSum(WORD PhySector) //输入为物理页
{
	WORD CheckSum;
	FlashGetMem(PhySector,ConstStatusOffSet,(WORD *)&CheckSum,lengthof(WORD));
	return CheckSum;

}

//写入每页的逻辑号
void FlashPutLogicSector(WORD PhySector,WORD LogicSector)
{
	FlashPutMem(PhySector,ConstLogicOffSet,(WORD *)&LogicSector,lengthof(WORD));	
}

WORD FlashGetLogicSector(WORD PhySector) //输入为物理页
{
	WORD LogicSector;
	FlashGetMem(PhySector,ConstLogicOffSet,(WORD *)&LogicSector,lengthof(WORD));
	return LogicSector;

}

//坏块标志设定
void FlashSetBadSector(WORD PhySector) //根据物理页
{
	WORD i,j;
	WORD Index;
	WORD OffSet;
	WORD Status;
	//首先把物理页转换成坏区表中的位置,按照顺序排第几个扇区
	Index=PhySector-ConstDataBeginSector; //去掉开始，去掉管理区
	i=Index/16;//计算出数组坐标
	j=Index%16; //计算出是一个自己的第几位
	OffSet=4+i; //去掉四个字，因为偏移量是按照字来进行的
	Status=~(0x01<<j); //0表示坏区
	Nop();
	//找到管理区存储位置，ConstBeginSector
	FlashPutMem(ConstPhyBeginSector,OffSet,(WORD *)&Status,lengthof(WORD));
	Nop();
}

//增加映射表的一条记录
WORD AddMemMapRecord(WORD LogicSector,WORD PageNum)
{
	//if((LogicSector<ConstDataSectorSize) && (MemMapInfo.PhySector[LogicSector]==MEM_NULL)) 
    if(LogicSector<ConstDataSectorSize)    //修改的数据要覆盖上次写的数据，所以不能加空的判断
	{
		MemMapInfo.PhySector[LogicSector]=PageNum;
		return LogicSector;
	}
	return MEM_NULL;
}
//删除映射表的一条记录
WORD RemoveMemMapRecord(WORD LogicSector)
{
	if(LogicSector<ConstDataSectorSize)
	{
		MemMapInfo.PhySector[LogicSector]=MEM_NULL;
		return LogicSector;
	}
	return MEM_NULL;
}
//修改映射表记录
WORD RefreshMemMapRecord(WORD LogicSector,WORD PageNum)
{
	if(LogicSector<ConstDataSectorSize)
	{
		if(MemMapInfo.PhySector[LogicSector]!=MEM_NULL)
		{
			MemMapInfo.PhySector[LogicSector]=PageNum;
			return LogicSector;
		}
	}
	return MEM_NULL;
}
//根据逻辑扇区得到物理扇区
WORD GetMemMapRecord(WORD LogSector)
{
	if(LogSector<ConstDataSectorSize)
	{
		return MemMapInfo.PhySector[LogSector];
	}
	return MEM_NULL;
}

//查询一个为空闲的物理扇区
WORD SearchFreePhySector(void)
{
	WORD Status,i;
	WORD PhySector;
	for(i=0;i<ConstDataSectorSize;i++) //数据扇区范围内搜索
	{
		PhySector=ConstDataBeginSector+i; //从数据扇区开始查找，从288页开始,也就是从物理地址0X9000开始。
		Status=FlashGetStatus(PhySector);
		if(Status != DIRTY)
		{
			FlashPutStatus(PhySector,FREE); //将该页修改为空闲,其实只有第一次可以写入，其它的时候不起作用。
			return PhySector; //返回物理页号
		}
	}
	return MEM_NULL;
}

void FlashSearchMemStatus(void)
{
	WORD i;
	WORD PhySector;
	//定义每页状态表
	WORD MemStatus[ConstDataSectorSize];
	for(i=0;i<ConstDataSectorSize;i++) //数据扇区范围内搜索
	{
		PhySector=ConstDataBeginSector+i; //从数据扇区开始查找
		MemStatus[i]=FlashGetStatus(PhySector);
	}
	Nop();	
}

//创建映射表
void FlashCreateMapInfo(void)
{
	WORD i;
	WORD PhySector,LogSector;
	WORD Status,Number;
	//首先把空闲扇区数量清空
	MemMapInfo.FreeNumber=0;
	//把需要擦除的数量清空
	MemMapInfo.DirtyNumber=0;
	//从数据物理扇区开始依次检索
	Number=ConstDataSectorSize;
	for(i=0;i<Number;i++)
	{
		PhySector=ConstDataBeginSector+i;//从数据扇区开始查找，跳过管理部分//ConstDataBeginSector81
		//存储状态的偏移量
		FlashGetMem(PhySector,ConstStatusOffSet,(WORD *)&Status,lengthof(WORD));//ConstStatusOffSet62
		//存储逻辑扇区的偏移量
		FlashGetMem(PhySector,ConstLogicOffSet,(WORD *)&LogSector,lengthof(WORD)); //读取逻辑地址
		if(Status==INUSE)
		{
			AddMemMapRecord(LogSector,PhySector);
		}
		else if(Status==MEM_NULL)
		{
			MemMapInfo.FreeNumber++; //记录空闲的扇区数量
		}
		else if(Status==DIRTY)
		{
			MemMapInfo.DirtyNumber++;//仍然要读取地址的对应关系
			AddMemMapRecord(LogSector,PhySector);
		}
	}
}

void FlashFormatMem(void)		//格式化存储区
{
	WORD i;
	MemInfo.EraseNumber++; //擦除次数增加
	//擦除所有的块
	for(i=0;i<ConstTotalPhyPages/ConstBlockSize;i++)
		FlashEraseMem(i);
	//管理区存储位置是ConstBeginSector，写入FLASH信息
	FlashPutMem(ConstPhyBeginSector,0,(WORD *)&MemInfo,lengthof(FLASH_INFO));
	//修改映射表，都置为空，表明存储区空间是空的
	for(i=0;i<ConstDataSectorSize;i++)
		MemMapInfo.PhySector[i]=MEM_NULL; //i表示逻辑扇区
	MemMapInfo.FreeNumber=ConstDataSectorSize; //记录空闲扇区数量
	MemMapInfo.DirtyNumber=0;	//需要擦除的数量为0
}


//初始化
void FlashInitSetup(void)
{
	WORD i;
	//初始化MemInfo
	MemInfo.Version=0;
	MemInfo.EraseNumber=0;
	MemInfo.Identifier=0;
	for(i=0;i<ConstBadSectorSize;i++)
		MemInfo.BadSector[i]=MEM_NULL;
	//初始化MemMapInfo
	MemMapInfo.FreeNumber=0;
	MemMapInfo.DirtyNumber=0;
	for(i=0;i<ConstDataSectorSize;i++)
		MemMapInfo.PhySector[i]=MEM_NULL; //i表示逻辑扇区
	//读取ID扇区
	FlashCreateMemInfo();
	//如果是未使用，那么格式化
	if((MemInfo.Version==MEM_NULL) && (MemInfo.Identifier==MEM_NULL))
	{
		MemInfo.Version=0x01;
		MemInfo.EraseNumber=1;
		MemInfo.Identifier=0x9999;
		//每位代表一个扇区，0为坏扇区，1为正常
		for(i=0;i<ConstBadSectorSize;i++)
			MemInfo.BadSector[i]=MEM_NULL;
		//管理区存储位置是ConstBeginSector，写入信息
		FlashPutMem(ConstPhyBeginSector,0,(WORD *)&MemInfo,lengthof(FLASH_INFO));
	}
	else
	{
		//创建映射表
		FlashCreateMapInfo();
	}
}


//存储区整理，把Dirty的擦除,输入是逻辑块号
BOOL FlashCleanMem(WORD nBlkNum)
{
	WORD i;
	WORD BlockNumber;
	WORD Status;
	WORD LogSector;
	WORD PhySector,PageNum;
	WORD PtrRd[ConstWordPageSize];
	for(i=0;i<ConstBlockSize;i++)
	{
		PhySector=ConstDataBeginSector+nBlkNum*ConstBlockSize+i; //找到物理页号
		Status=FlashGetStatus(PhySector);
		if(Status!=INUSE)
			continue; //说明本页没有使用，跳出本次循环
		//找到一个使用的页，把该页呢绒读出来写到另一个块的空闲页
		FlashGetMem(PhySector,0,(WORD *)PtrRd,ConstWordPageSize); //按照字来读
		//读出对应的逻辑页面
		LogSector=FlashGetLogicSector(PhySector);
		//找到一个空闲页，保证不要再次写入本块
		while(1)
		{
			//找到一个空闲页
			PageNum=SearchFreePhySector(); //再找到一个空闲页，保证不要再次写入本块
			//如果为空，则说明已经没有空闲页了，整理得太晚了
			if(PageNum==MEM_NULL) 
				return FALSE;
			//计算空闲页所属的逻辑块号
			BlockNumber=(PageNum-ConstDataBeginSector)/ConstBlockSize;
			if(BlockNumber==nBlkNum)
				continue; //跳出本次循环，重新开始
			//写到另一块的新空闲页
			FlashPutMem(PageNum,0,(WORD *)PtrRd,ConstWordPageSize); 
			//并将页置成使用
			FlashPutStatus(PageNum,INUSE);
			//写入逻辑扇区号
			FlashPutLogicSector(PageNum,LogSector);
			//将原来的页置成DIRTY 
			FlashPutStatus(PhySector,DIRTY); 
			//修改映射表
			RefreshMemMapRecord(LogSector,PageNum);	
			CLR_WDT();
			break; //成功就跳出循环		
		}
	}
	//开始擦除
	FlashEraseMem(nBlkNum+1); //将逻辑块转成物理块，要抛去管理块
	#ifdef I_NEED_DEBUG
		DebugPrintf("FlashErase!",11);
	#endif
	return TRUE;
}

void FlashManageMem(void)
{
	WORD i,j;
	WORD Number=0;
	float Percent;
	WORD PhySector,nBlockNum;
	WORD Status;
	Percent=((float)MemMapInfo.FreeNumber)/(float)ConstDataSectorSize;
	//共有5块，最多剩一块时候，开始整理存储区
	if((Percent>=0) && (Percent<=0.2))
	{
		//所有的块都要查询
		for(i=0;i<ConstDataSectorSize/ConstBlockSize;i++)
		{
			//将数据逻辑块，转成物理块号
			nBlockNum=ConstDataBeginSector/ConstBlockSize+i;
			//检索每块内状态为Dirty的页数量
			for(j=0;j<ConstBlockSize;j++)
			{
				//计算物理页
				PhySector=nBlockNum*ConstBlockSize+j;
				Status=FlashGetStatus(PhySector);
				if(Status==DIRTY)
					Number++;
			}
			//如果每块内，DIRTY的数量超过一半，则进行整理
			if(Number>=(ConstBlockSize/2))
			{	
				Number=0; //重新清零，再次计数
				FlashCleanMem(i);
			}
			CLR_WDT();
		}
		//整理完毕，重新创建一下映射表
		FlashCreateMapInfo();
	}
}

//写存储器是按照逻辑扇区来进行写的，偏移量则是按照字来进行
WORD FlashWriteMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number)
{
	WORD Size=0;
	WORD MaxSector;
	WORD NowLogicSector;
	WORD PhySector,PageNum;
	WORD CntNodeBackInfo = 18;//备份区域，从第18页开始，在删除的时候使用。
	WORD Status,i;
	//从映射表中查找物理扇区，如果为空表明该逻辑扇区空闲
	PhySector=GetMemMapRecord(LogSector);
	MaxSector=ConstDataBeginSector+ConstDataSectorSize;//值是328.
	//如果逻辑扇区对应物理扇区为空，或者已经使用
	if((PhySector==MEM_NULL) || (PhySector<MaxSector))
	{
		//进行整理内存
		//FlashManageMem();
        FlashManageMem();
		//如果是往备份块中写信息
		if(LogSector >= CntNodeBackInfo)
		{
			for(i=0;i<8;i++) //在第二块中搜索，从308页开始
			{
				PageNum = LctionAddr + i; 
				Status=FlashGetStatus(PageNum);
				if(Status != DIRTY)
				{
					FlashPutStatus(PhySector,FREE); //将该页修改为空闲,其实只有第一次可以写入，其它的时候不起作用。
					break;
				}
			}
		}
		//如果不是往备份快中写信息，直接从头开始查找，与逻辑页的大小无关，都是从空闲的Flash区域最开始查找。
		else
		{  
			PageNum=SearchFreePhySector();
		}
		NowLogicSector = FlashGetLogicSector(PageNum); //得到该扇区对应的逻辑页号
		if((NowLogicSector != LogSector) && (NowLogicSector != MEM_NULL))
		{
			MemMapInfo.FreeNumber--;
			FlashPutStatus(PageNum,DIRTY);
			MemMapInfo.DirtyNumber++;
			if(LogSector > CntNodeBackInfo)
			{
				PageNum = LctionAddr+(LogSector - CntNodeBackInfo);	
			} 
			else
			{
				PageNum = SearchFreePhySector();
			} 
		}
		//如果没有空闲的物理扇区，那么写失败
		if(PageNum==MEM_NULL)
		{
			return 0; 
		}
		//向该扇区中写入数据    
		Size=FlashPutMem(PageNum,OffSet,Ptr,Number);
		//空闲的扇区减少一个
		//MemMapInfo.FreeNumber--; 
        MemMapInfo.FreeNumber--;
		//修改当前扇区的状态
		//FlashPutStatus(PageNum,INVALID);
		//如果本逻辑扇区是一个空的,直接加入映射表 	 
		if(PhySector==MEM_NULL)			 
		{
			//修改映射表中的对应值
			AddMemMapRecord(LogSector,PageNum);//把逻辑扇区和物理扇区进行对应。
			//再把刚写的物理页面的状态修改成InUSe 
			FlashPutStatus(PageNum,INUSE);
			//写入逻辑扇区号
			FlashPutLogicSector(PageNum,LogSector);		
		}
		else
		{
            //修改映射表原来对应物理扇区成Dirty  
			//FlashPutStatus(PhySector,DIRTY);
            FlashPutStatus(PhySector,DIRTY);
			//Dirty状态的页又多了一个
			//MemMapInfo.DirtyNumber++; 
            MemMapInfo.DirtyNumber++; 
			//修改逻辑扇区对应的物理扇区值
			RefreshMemMapRecord(LogSector,PageNum);
 			//再把刚写的物理页面的状态修改成InUSe
			FlashPutStatus(PageNum,INUSE);
			//写入逻辑扇区号
			FlashPutLogicSector(PageNum,LogSector);		
		}
	}
	return Size;
}

WORD FlashReadMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number)
{
	WORD Size,i;
	WORD PhySector;
	//由逻辑扇区得到物理扇区
	PhySector=GetMemMapRecord(LogSector); 
	//如果该扇区还没有使用，那么全部为0xFFFF
	if(PhySector==MEM_NULL)
	{
		Size=Number;
		for(i=0;i<Number;i++)
			*Ptr++=0xFFFF;
		#ifdef I_NEED_DEBUG
			DebugPrintf("FlashRead is error!",17);
		#endif
	}
	else
	{
		Size=FlashGetMem(PhySector,OffSet,Ptr,Number);
	}
	return Size;
}




