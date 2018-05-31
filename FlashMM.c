#include "FlashMM.h"

/*
�޸����ڣ�2016��8��8��
�޸��ߣ����ӳ�
�ر�˵����
һ������FLASH��д�뺯����FlashWriteMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number)
�ú���ʵ���������Լ�д�ġ�
�������趨��ҳ��д���ݣ�����ҳʱ�����Ƚ���һ��д��ҳ��ΪDirtyҳ��Ȼ����д��һҳ��Dirty
��Ϊ��ҳ��һ����־�����ԣ����ڵ�һ��bug�������д����һҳ��Ȼ��д��һҳ���ٻ���д��һҳ��
ʱ����޷�д�롣���⣬��Ҫ�޸ĳ�ʼ������FlashCreateMapInfo()������dirtyҳʱ��ҲҪ�����߼�ҳ
������ҳ֮��Ķ�Ӧ�������ȡ����ȷ��

*/

//��λҪ���ж�λ�ĵ�ַ������ҳ��ʼ�ĵط���Ϊ�˽��в�����
WORD LctionAddr = 296; //��Ӧ�ĵ�ַΪ0X9400��ʵ�����Ǵӵ����鿪ʼ�ģ���Ϊ���ݿ�ʹ�á�


//����FLASH������Ϣ
FLASH_INFO MemInfo; 
//����ӳ���,ӳ���������������������ʼ��������������
MEM_MAP_INFO MemMapInfo;

//����
void FlashEraseMem(WORD LogicBlockNum) //����Ϊ�߼����
{
	uReg32 addr;
	//��ַ=��ʼ��ַ+���*ÿ���ֽ���
	addr.dwVal=ConstPhyBeginAddr+LogicBlockNum*ConstBlockSize*ConstPageSize;
	FLASHErasePage(addr); //�ú������Ѿ���ʱ��
}


//OffSetΪ�ֵĿ�ʼ
WORD FlashPutMem(WORD PhySector,WORD OffSet,WORD *Ptr,WORD Number) //����Ϊ����ҳ
{
	WORD Size;
	uReg32 addr;

	//��ַ=��ʼ��ַҳ��*ÿҳ�ֽ�������ʼ��ַ�����˹�����
	addr.dwVal=PhySector*ConstPageSize+2*OffSet; //
	if(Number<=ConstDataPageSize) //ÿҳȥ��8���ֽ�״̬λ���߼���Ԫ
		Size=Number;
	else
		Size=ConstDataPageSize; //������
	FLASHPutArray(addr,Ptr,Size);
	CLR_WDT();
	return Size;
}

WORD FlashGetMem(WORD PhySector,WORD OffSet,WORD *Ptr,WORD Number) //����Ϊ����ҳ
{
	WORD Size;
	uReg32 addr;
	//��ַ=��ʼ��ַҳ��*ÿҳ�ֽ�������ʼ��ַ������
	addr.dwVal=PhySector*ConstPageSize+2*OffSet;
	if(Number<=ConstDataPageSize) //��Ҫȥ������״̬�ֽ�
		Size=Number;
	else
		Size=ConstDataPageSize;//�����ֵĴ�С
	FLASHGetArray(addr,Ptr,Size);
	CLR_WDT();
	return Size;
}

//д��ÿҳ��״̬
void FlashPutStatus(WORD PhySector,WORD Status) //����Ϊ����ҳ
{
	FlashPutMem(PhySector,ConstStatusOffSet,(WORD *)&Status,lengthof(WORD));
}


WORD FlashGetStatus(WORD PhySector) //����Ϊ����ҳ
{
	WORD Status;
	FlashGetMem(PhySector,ConstStatusOffSet,(WORD *)&Status,lengthof(WORD));
	return Status;

}

//д��ÿҳ��CRCУ���
void FlashPutCheckSum(WORD PhySector,WORD CheckSum)
{
	FlashPutMem(PhySector,ConstCheckSumOffSet,(WORD *)&CheckSum,lengthof(WORD));	
}

WORD FlashGetCheckSum(WORD PhySector) //����Ϊ����ҳ
{
	WORD CheckSum;
	FlashGetMem(PhySector,ConstStatusOffSet,(WORD *)&CheckSum,lengthof(WORD));
	return CheckSum;

}

//д��ÿҳ���߼���
void FlashPutLogicSector(WORD PhySector,WORD LogicSector)
{
	FlashPutMem(PhySector,ConstLogicOffSet,(WORD *)&LogicSector,lengthof(WORD));	
}

WORD FlashGetLogicSector(WORD PhySector) //����Ϊ����ҳ
{
	WORD LogicSector;
	FlashGetMem(PhySector,ConstLogicOffSet,(WORD *)&LogicSector,lengthof(WORD));
	return LogicSector;

}

//�����־�趨
void FlashSetBadSector(WORD PhySector) //��������ҳ
{
	WORD i,j;
	WORD Index;
	WORD OffSet;
	WORD Status;
	//���Ȱ�����ҳת���ɻ������е�λ��,����˳���ŵڼ�������
	Index=PhySector-ConstDataBeginSector; //ȥ����ʼ��ȥ��������
	i=Index/16;//�������������
	j=Index%16; //�������һ���Լ��ĵڼ�λ
	OffSet=4+i; //ȥ���ĸ��֣���Ϊƫ�����ǰ����������е�
	Status=~(0x01<<j); //0��ʾ����
	Nop();
	//�ҵ��������洢λ�ã�ConstBeginSector
	FlashPutMem(ConstPhyBeginSector,OffSet,(WORD *)&Status,lengthof(WORD));
	Nop();
}

//����ӳ����һ����¼
WORD AddMemMapRecord(WORD LogicSector,WORD PageNum)
{
	//if((LogicSector<ConstDataSectorSize) && (MemMapInfo.PhySector[LogicSector]==MEM_NULL)) 
    if(LogicSector<ConstDataSectorSize)    //�޸ĵ�����Ҫ�����ϴ�д�����ݣ����Բ��ܼӿյ��ж�
	{
		MemMapInfo.PhySector[LogicSector]=PageNum;
		return LogicSector;
	}
	return MEM_NULL;
}
//ɾ��ӳ����һ����¼
WORD RemoveMemMapRecord(WORD LogicSector)
{
	if(LogicSector<ConstDataSectorSize)
	{
		MemMapInfo.PhySector[LogicSector]=MEM_NULL;
		return LogicSector;
	}
	return MEM_NULL;
}
//�޸�ӳ����¼
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
//�����߼������õ���������
WORD GetMemMapRecord(WORD LogSector)
{
	if(LogSector<ConstDataSectorSize)
	{
		return MemMapInfo.PhySector[LogSector];
	}
	return MEM_NULL;
}

//��ѯһ��Ϊ���е���������
WORD SearchFreePhySector(void)
{
	WORD Status,i;
	WORD PhySector;
	for(i=0;i<ConstDataSectorSize;i++) //����������Χ������
	{
		PhySector=ConstDataBeginSector+i; //������������ʼ���ң���288ҳ��ʼ,Ҳ���Ǵ������ַ0X9000��ʼ��
		Status=FlashGetStatus(PhySector);
		if(Status != DIRTY)
		{
			FlashPutStatus(PhySector,FREE); //����ҳ�޸�Ϊ����,��ʵֻ�е�һ�ο���д�룬������ʱ�������á�
			return PhySector; //��������ҳ��
		}
	}
	return MEM_NULL;
}

void FlashSearchMemStatus(void)
{
	WORD i;
	WORD PhySector;
	//����ÿҳ״̬��
	WORD MemStatus[ConstDataSectorSize];
	for(i=0;i<ConstDataSectorSize;i++) //����������Χ������
	{
		PhySector=ConstDataBeginSector+i; //������������ʼ����
		MemStatus[i]=FlashGetStatus(PhySector);
	}
	Nop();	
}

//����ӳ���
void FlashCreateMapInfo(void)
{
	WORD i;
	WORD PhySector,LogSector;
	WORD Status,Number;
	//���Ȱѿ��������������
	MemMapInfo.FreeNumber=0;
	//����Ҫ�������������
	MemMapInfo.DirtyNumber=0;
	//����������������ʼ���μ���
	Number=ConstDataSectorSize;
	for(i=0;i<Number;i++)
	{
		PhySector=ConstDataBeginSector+i;//������������ʼ���ң�����������//ConstDataBeginSector81
		//�洢״̬��ƫ����
		FlashGetMem(PhySector,ConstStatusOffSet,(WORD *)&Status,lengthof(WORD));//ConstStatusOffSet62
		//�洢�߼�������ƫ����
		FlashGetMem(PhySector,ConstLogicOffSet,(WORD *)&LogSector,lengthof(WORD)); //��ȡ�߼���ַ
		if(Status==INUSE)
		{
			AddMemMapRecord(LogSector,PhySector);
		}
		else if(Status==MEM_NULL)
		{
			MemMapInfo.FreeNumber++; //��¼���е���������
		}
		else if(Status==DIRTY)
		{
			MemMapInfo.DirtyNumber++;//��ȻҪ��ȡ��ַ�Ķ�Ӧ��ϵ
			AddMemMapRecord(LogSector,PhySector);
		}
	}
}

void FlashFormatMem(void)		//��ʽ���洢��
{
	WORD i;
	MemInfo.EraseNumber++; //������������
	//�������еĿ�
	for(i=0;i<ConstTotalPhyPages/ConstBlockSize;i++)
		FlashEraseMem(i);
	//�������洢λ����ConstBeginSector��д��FLASH��Ϣ
	FlashPutMem(ConstPhyBeginSector,0,(WORD *)&MemInfo,lengthof(FLASH_INFO));
	//�޸�ӳ�������Ϊ�գ������洢���ռ��ǿյ�
	for(i=0;i<ConstDataSectorSize;i++)
		MemMapInfo.PhySector[i]=MEM_NULL; //i��ʾ�߼�����
	MemMapInfo.FreeNumber=ConstDataSectorSize; //��¼������������
	MemMapInfo.DirtyNumber=0;	//��Ҫ����������Ϊ0
}


//��ʼ��
void FlashInitSetup(void)
{
	WORD i;
	//��ʼ��MemInfo
	MemInfo.Version=0;
	MemInfo.EraseNumber=0;
	MemInfo.Identifier=0;
	for(i=0;i<ConstBadSectorSize;i++)
		MemInfo.BadSector[i]=MEM_NULL;
	//��ʼ��MemMapInfo
	MemMapInfo.FreeNumber=0;
	MemMapInfo.DirtyNumber=0;
	for(i=0;i<ConstDataSectorSize;i++)
		MemMapInfo.PhySector[i]=MEM_NULL; //i��ʾ�߼�����
	//��ȡID����
	FlashCreateMemInfo();
	//�����δʹ�ã���ô��ʽ��
	if((MemInfo.Version==MEM_NULL) && (MemInfo.Identifier==MEM_NULL))
	{
		MemInfo.Version=0x01;
		MemInfo.EraseNumber=1;
		MemInfo.Identifier=0x9999;
		//ÿλ����һ��������0Ϊ��������1Ϊ����
		for(i=0;i<ConstBadSectorSize;i++)
			MemInfo.BadSector[i]=MEM_NULL;
		//�������洢λ����ConstBeginSector��д����Ϣ
		FlashPutMem(ConstPhyBeginSector,0,(WORD *)&MemInfo,lengthof(FLASH_INFO));
	}
	else
	{
		//����ӳ���
		FlashCreateMapInfo();
	}
}


//�洢��������Dirty�Ĳ���,�������߼����
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
		PhySector=ConstDataBeginSector+nBlkNum*ConstBlockSize+i; //�ҵ�����ҳ��
		Status=FlashGetStatus(PhySector);
		if(Status!=INUSE)
			continue; //˵����ҳû��ʹ�ã���������ѭ��
		//�ҵ�һ��ʹ�õ�ҳ���Ѹ�ҳ���޶�����д����һ����Ŀ���ҳ
		FlashGetMem(PhySector,0,(WORD *)PtrRd,ConstWordPageSize); //����������
		//������Ӧ���߼�ҳ��
		LogSector=FlashGetLogicSector(PhySector);
		//�ҵ�һ������ҳ����֤��Ҫ�ٴ�д�뱾��
		while(1)
		{
			//�ҵ�һ������ҳ
			PageNum=SearchFreePhySector(); //���ҵ�һ������ҳ����֤��Ҫ�ٴ�д�뱾��
			//���Ϊ�գ���˵���Ѿ�û�п���ҳ�ˣ������̫����
			if(PageNum==MEM_NULL) 
				return FALSE;
			//�������ҳ�������߼����
			BlockNumber=(PageNum-ConstDataBeginSector)/ConstBlockSize;
			if(BlockNumber==nBlkNum)
				continue; //��������ѭ�������¿�ʼ
			//д����һ����¿���ҳ
			FlashPutMem(PageNum,0,(WORD *)PtrRd,ConstWordPageSize); 
			//����ҳ�ó�ʹ��
			FlashPutStatus(PageNum,INUSE);
			//д���߼�������
			FlashPutLogicSector(PageNum,LogSector);
			//��ԭ����ҳ�ó�DIRTY 
			FlashPutStatus(PhySector,DIRTY); 
			//�޸�ӳ���
			RefreshMemMapRecord(LogSector,PageNum);	
			CLR_WDT();
			break; //�ɹ�������ѭ��		
		}
	}
	//��ʼ����
	FlashEraseMem(nBlkNum+1); //���߼���ת������飬Ҫ��ȥ�����
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
	//����5�飬���ʣһ��ʱ�򣬿�ʼ����洢��
	if((Percent>=0) && (Percent<=0.2))
	{
		//���еĿ鶼Ҫ��ѯ
		for(i=0;i<ConstDataSectorSize/ConstBlockSize;i++)
		{
			//�������߼��飬ת��������
			nBlockNum=ConstDataBeginSector/ConstBlockSize+i;
			//����ÿ����״̬ΪDirty��ҳ����
			for(j=0;j<ConstBlockSize;j++)
			{
				//��������ҳ
				PhySector=nBlockNum*ConstBlockSize+j;
				Status=FlashGetStatus(PhySector);
				if(Status==DIRTY)
					Number++;
			}
			//���ÿ���ڣ�DIRTY����������һ�룬���������
			if(Number>=(ConstBlockSize/2))
			{	
				Number=0; //�������㣬�ٴμ���
				FlashCleanMem(i);
			}
			CLR_WDT();
		}
		//������ϣ����´���һ��ӳ���
		FlashCreateMapInfo();
	}
}

//д�洢���ǰ����߼�����������д�ģ�ƫ�������ǰ�����������
WORD FlashWriteMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number)
{
	WORD Size=0;
	WORD MaxSector;
	WORD NowLogicSector;
	WORD PhySector,PageNum;
	WORD CntNodeBackInfo = 18;//�������򣬴ӵ�18ҳ��ʼ����ɾ����ʱ��ʹ�á�
	WORD Status,i;
	//��ӳ����в����������������Ϊ�ձ������߼���������
	PhySector=GetMemMapRecord(LogSector);
	MaxSector=ConstDataBeginSector+ConstDataSectorSize;//ֵ��328.
	//����߼�������Ӧ��������Ϊ�գ������Ѿ�ʹ��
	if((PhySector==MEM_NULL) || (PhySector<MaxSector))
	{
		//���������ڴ�
		//FlashManageMem();
        FlashManageMem();
		//����������ݿ���д��Ϣ
		if(LogSector >= CntNodeBackInfo)
		{
			for(i=0;i<8;i++) //�ڵڶ�������������308ҳ��ʼ
			{
				PageNum = LctionAddr + i; 
				Status=FlashGetStatus(PageNum);
				if(Status != DIRTY)
				{
					FlashPutStatus(PhySector,FREE); //����ҳ�޸�Ϊ����,��ʵֻ�е�һ�ο���д�룬������ʱ�������á�
					break;
				}
			}
		}
		//������������ݿ���д��Ϣ��ֱ�Ӵ�ͷ��ʼ���ң����߼�ҳ�Ĵ�С�޹أ����Ǵӿ��е�Flash�����ʼ���ҡ�
		else
		{  
			PageNum=SearchFreePhySector();
		}
		NowLogicSector = FlashGetLogicSector(PageNum); //�õ���������Ӧ���߼�ҳ��
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
		//���û�п��е�������������ôдʧ��
		if(PageNum==MEM_NULL)
		{
			return 0; 
		}
		//���������д������    
		Size=FlashPutMem(PageNum,OffSet,Ptr,Number);
		//���е���������һ��
		//MemMapInfo.FreeNumber--; 
        MemMapInfo.FreeNumber--;
		//�޸ĵ�ǰ������״̬
		//FlashPutStatus(PageNum,INVALID);
		//������߼�������һ���յ�,ֱ�Ӽ���ӳ��� 	 
		if(PhySector==MEM_NULL)			 
		{
			//�޸�ӳ����еĶ�Ӧֵ
			AddMemMapRecord(LogSector,PageNum);//���߼������������������ж�Ӧ��
			//�ٰѸ�д������ҳ���״̬�޸ĳ�InUSe 
			FlashPutStatus(PageNum,INUSE);
			//д���߼�������
			FlashPutLogicSector(PageNum,LogSector);		
		}
		else
		{
            //�޸�ӳ���ԭ����Ӧ����������Dirty  
			//FlashPutStatus(PhySector,DIRTY);
            FlashPutStatus(PhySector,DIRTY);
			//Dirty״̬��ҳ�ֶ���һ��
			//MemMapInfo.DirtyNumber++; 
            MemMapInfo.DirtyNumber++; 
			//�޸��߼�������Ӧ����������ֵ
			RefreshMemMapRecord(LogSector,PageNum);
 			//�ٰѸ�д������ҳ���״̬�޸ĳ�InUSe
			FlashPutStatus(PageNum,INUSE);
			//д���߼�������
			FlashPutLogicSector(PageNum,LogSector);		
		}
	}
	return Size;
}

WORD FlashReadMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number)
{
	WORD Size,i;
	WORD PhySector;
	//���߼������õ���������
	PhySector=GetMemMapRecord(LogSector); 
	//�����������û��ʹ�ã���ôȫ��Ϊ0xFFFF
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




