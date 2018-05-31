#include "Nvm_flash.h"

extern PHY_PIB phyPIB;
extern MAC_PIB macPIB;
extern MAC_STATUS macStatus;
extern PAN_DESCRIPTOR PANDescriptor;

//存储分配的IEEE地址
void PutIEEEAddrInfo(WORD Indication,const LONG_ADDR *IEEEAddr)
{
	WORD OffSet;
	WORD cPtrTx[10];
	//全部读出来
	FlashReadMem(ConstIEEEAddrPN,0,cPtrTx,5);	
	//找到修改位置
	OffSet=0;
	//修改
	cPtrTx[OffSet]=Indication;
	//找到修改位置
	OffSet=1; 
	//修改
	memcpy((BYTE *)&cPtrTx[OffSet],(BYTE *)IEEEAddr,sizeof(LONG_ADDR));
	//重新写入
	FlashWriteMem(ConstIEEEAddrPN,0,cPtrTx,5);

	CLR_WDT();	

}

//读取存储地址标志
BOOL GetIEEEIndication(WORD *Indication)
{
	WORD Number;
	Number=FlashReadMem(ConstIEEEAddrPN,0,Indication,lengthof(WORD));	
	if(Number==lengthof(WORD))
		return TRUE;
	return FALSE;
}

//读取自动获得地址信息
BOOL GetIEEEAddr(LONG_ADDR *IEEEAddr)
{
	WORD Number;
	Number=FlashReadMem(ConstIEEEAddrPN,1,(WORD *)IEEEAddr,lengthof(LONG_ADDR));
	if(Number==lengthof(LONG_ADDR))
		return TRUE;
	return FALSE;
}

//读取状态
BOOL GetMACStatus(void)
{
	WORD Number;
	Number=FlashReadMem(ConstStatusAddrPN,0,(WORD *)&macStatus,lengthof(MAC_STATUS));
	if(Number==lengthof(MAC_STATUS))
		return TRUE;
	return FALSE;
}


//存储MAC层状态
void PutMACStatus(void)
{

	WORD OffSet;
	WORD cPtrTx[4];
	//全部读出来
	FlashReadMem(ConstStatusAddrPN,0,cPtrTx,lengthof(MAC_STATUS)); 
	//找到修改位置
	OffSet=0; 
 	//修改相应参数
	cPtrTx[OffSet++]=macStatus.nVal;
	//把所有的都重新写入	
	FlashWriteMem(ConstStatusAddrPN,0,cPtrTx,lengthof(MAC_STATUS)); 
	
}


//读出网络PANId
BOOL GetMACPANId(void)
{
	WORD Number;
	Number=FlashReadMem(ConstMacAddrPN,0,(WORD *)&macPIB.macPANId,lengthof(SHORT_ADDR));
	if(Number==lengthof(SHORT_ADDR))
		return TRUE;
	return FALSE;
}

//读出短地址
BOOL GetMACShortAddr(void)
{
	WORD Number;
	Number=FlashReadMem(ConstMacAddrPN,1,(WORD *)&macPIB.macShortAddr,lengthof(SHORT_ADDR));
	if(Number==lengthof(SHORT_ADDR))
		return TRUE;
	return FALSE;
}


//读出长地址
BOOL GetMACLongAddr(void)
{
	WORD Number;
	Number=FlashReadMem(ConstMacAddrPN,2,(WORD *)&macPIB.macLongAddr,lengthof(LONG_ADDR));
	if(Number==lengthof(LONG_ADDR))
		return TRUE;
	return FALSE;
}



//存储MAC地址
void PutMACAddr(void)
{

	WORD OffSet;
	WORD cPtrTx[6];
	//全部读出来
	FlashReadMem(ConstMacAddrPN,0,cPtrTx,6); //把该页全部读出来
	//找到修改位置
	OffSet=0; 
	//修改相应参数
	cPtrTx[OffSet]=macPIB.macPANId.nVal; 
	//找到修改位置
	OffSet=1; 
 	//修改相应参数
	cPtrTx[OffSet]=macPIB.macShortAddr.nVal;
	//找到修改位置，
	OffSet=2; 
	//修改
	memcpy((BYTE *)&cPtrTx[OffSet],(BYTE *)&macPIB.macLongAddr,sizeof(LONG_ADDR));
 	//把所有的都重新写入
	FlashWriteMem(ConstMacAddrPN,0,cPtrTx,6); 
	
}

//获取协调器信息
BOOL GetCoordDescriptor(void)
{
	WORD Number;
	Number=FlashReadMem(ConstCoordAddrPN,0,(WORD *)&PANDescriptor,lengthof(PAN_DESCRIPTOR));
	if(Number==lengthof(PAN_DESCRIPTOR))
		return TRUE;
	return FALSE;
}

//存储协调器信息
void PutCoordDescriptor(void)
{

	WORD OffSet;
	WORD cPtrTx[20];
	//把该页全部读出来
	FlashReadMem(ConstCoordAddrPN,0,cPtrTx,20); 
	//找到修改位置
	OffSet=0; 
	//修改相应参数
	memcpy((BYTE *)&cPtrTx[OffSet],(BYTE *)&PANDescriptor,sizeof(PAN_DESCRIPTOR));
	//从新写入
	FlashWriteMem(ConstCoordAddrPN,0,cPtrTx,lengthof(PAN_DESCRIPTOR)); 
	
}


BOOL GetPHYFreq(WORD *Index)	
{
	WORD OffSet;
	WORD Number;
	WORD cPtrTx[4];
	Number=FlashReadMem(ConstPhyFreqAddrPN,0,(WORD *)&cPtrTx[0],2);
	OffSet=0;
	*Index=cPtrTx[OffSet++];
	phyPIB.phyCurrentChannel=(BYTE)cPtrTx[OffSet++];
	if(Number==2)
		return TRUE;
	return FALSE;	
}

void PutPHYFreq(WORD FreqIndex)
{
	WORD OffSet;
	WORD cPtrTx[4];
	//把该页全部读出来
	FlashReadMem(ConstPhyFreqAddrPN,0,cPtrTx,2); 
	//找到修改位置
	OffSet=0; 
	//修改相应参数
	cPtrTx[OffSet++]=FreqIndex; 
	cPtrTx[OffSet++]=phyPIB.phyCurrentChannel; 
	//从新写入
	FlashWriteMem(ConstPhyFreqAddrPN,0,cPtrTx,2);
	CLR_WDT(); 		
}

BOOL GetPHYTxPower(WORD *Index)
{
	WORD OffSet;
	WORD Number;
	WORD cPtrTx[4];
	Number=FlashReadMem(ConstPhyTxPowerAddrPN,0,(WORD *)&cPtrTx[0],2);
	OffSet=0;
	*Index=cPtrTx[OffSet++];
	phyPIB.phyTransmitPower=(BYTE)cPtrTx[OffSet++];
	if(Number==2)
		return TRUE;
	return FALSE;	
}

void PutPHYTxPower(WORD TxPowerIndex)
{
	WORD OffSet;
	WORD cPtrTx[4];
	//把该页全部读出来
	FlashReadMem(ConstPhyTxPowerAddrPN,0,cPtrTx,2); 
	//找到修改位置
	OffSet=0; 
	//修改相应参数
	cPtrTx[OffSet++]=TxPowerIndex; 
	cPtrTx[OffSet++]=phyPIB.phyTransmitPower; 
	//从新写入
	FlashWriteMem(ConstPhyTxPowerAddrPN,0,cPtrTx,2);
	CLR_WDT(); 	
}

BOOL GetPHYBaudRate(WORD *Index)
{	  
	WORD OffSet;
	WORD Number;
	WORD cPtrTx[4];
	Number=FlashReadMem(ConstPhyBaudRateAddrPN,0,(WORD *)&cPtrTx[0],2);
	OffSet=0;
	*Index=cPtrTx[OffSet++];	
	phyPIB.phyBaudRate=(BYTE)cPtrTx[OffSet++];
	if(Number==2)
		return TRUE;
	return FALSE;	
}

void PutPHYBaudRate(WORD BaudRateIndex)
{
	WORD OffSet;
	WORD cPtrTx[4];
	//把该页全部读出来
	FlashReadMem(ConstPhyBaudRateAddrPN,0,cPtrTx,2); 
	//找到修改位置
	OffSet=0; 
	//修改相应参数
	cPtrTx[OffSet++]=BaudRateIndex; 
	cPtrTx[OffSet++]=phyPIB.phyBaudRate; 
	//从新写入
	FlashWriteMem(ConstPhyBaudRateAddrPN,0,cPtrTx,2);
	CLR_WDT(); 		
}
