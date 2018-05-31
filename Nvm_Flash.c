#include "Nvm_flash.h"

extern PHY_PIB phyPIB;
extern MAC_PIB macPIB;
extern MAC_STATUS macStatus;
extern PAN_DESCRIPTOR PANDescriptor;

//�洢�����IEEE��ַ
void PutIEEEAddrInfo(WORD Indication,const LONG_ADDR *IEEEAddr)
{
	WORD OffSet;
	WORD cPtrTx[10];
	//ȫ��������
	FlashReadMem(ConstIEEEAddrPN,0,cPtrTx,5);	
	//�ҵ��޸�λ��
	OffSet=0;
	//�޸�
	cPtrTx[OffSet]=Indication;
	//�ҵ��޸�λ��
	OffSet=1; 
	//�޸�
	memcpy((BYTE *)&cPtrTx[OffSet],(BYTE *)IEEEAddr,sizeof(LONG_ADDR));
	//����д��
	FlashWriteMem(ConstIEEEAddrPN,0,cPtrTx,5);

	CLR_WDT();	

}

//��ȡ�洢��ַ��־
BOOL GetIEEEIndication(WORD *Indication)
{
	WORD Number;
	Number=FlashReadMem(ConstIEEEAddrPN,0,Indication,lengthof(WORD));	
	if(Number==lengthof(WORD))
		return TRUE;
	return FALSE;
}

//��ȡ�Զ���õ�ַ��Ϣ
BOOL GetIEEEAddr(LONG_ADDR *IEEEAddr)
{
	WORD Number;
	Number=FlashReadMem(ConstIEEEAddrPN,1,(WORD *)IEEEAddr,lengthof(LONG_ADDR));
	if(Number==lengthof(LONG_ADDR))
		return TRUE;
	return FALSE;
}

//��ȡ״̬
BOOL GetMACStatus(void)
{
	WORD Number;
	Number=FlashReadMem(ConstStatusAddrPN,0,(WORD *)&macStatus,lengthof(MAC_STATUS));
	if(Number==lengthof(MAC_STATUS))
		return TRUE;
	return FALSE;
}


//�洢MAC��״̬
void PutMACStatus(void)
{

	WORD OffSet;
	WORD cPtrTx[4];
	//ȫ��������
	FlashReadMem(ConstStatusAddrPN,0,cPtrTx,lengthof(MAC_STATUS)); 
	//�ҵ��޸�λ��
	OffSet=0; 
 	//�޸���Ӧ����
	cPtrTx[OffSet++]=macStatus.nVal;
	//�����еĶ�����д��	
	FlashWriteMem(ConstStatusAddrPN,0,cPtrTx,lengthof(MAC_STATUS)); 
	
}


//��������PANId
BOOL GetMACPANId(void)
{
	WORD Number;
	Number=FlashReadMem(ConstMacAddrPN,0,(WORD *)&macPIB.macPANId,lengthof(SHORT_ADDR));
	if(Number==lengthof(SHORT_ADDR))
		return TRUE;
	return FALSE;
}

//�����̵�ַ
BOOL GetMACShortAddr(void)
{
	WORD Number;
	Number=FlashReadMem(ConstMacAddrPN,1,(WORD *)&macPIB.macShortAddr,lengthof(SHORT_ADDR));
	if(Number==lengthof(SHORT_ADDR))
		return TRUE;
	return FALSE;
}


//��������ַ
BOOL GetMACLongAddr(void)
{
	WORD Number;
	Number=FlashReadMem(ConstMacAddrPN,2,(WORD *)&macPIB.macLongAddr,lengthof(LONG_ADDR));
	if(Number==lengthof(LONG_ADDR))
		return TRUE;
	return FALSE;
}



//�洢MAC��ַ
void PutMACAddr(void)
{

	WORD OffSet;
	WORD cPtrTx[6];
	//ȫ��������
	FlashReadMem(ConstMacAddrPN,0,cPtrTx,6); //�Ѹ�ҳȫ��������
	//�ҵ��޸�λ��
	OffSet=0; 
	//�޸���Ӧ����
	cPtrTx[OffSet]=macPIB.macPANId.nVal; 
	//�ҵ��޸�λ��
	OffSet=1; 
 	//�޸���Ӧ����
	cPtrTx[OffSet]=macPIB.macShortAddr.nVal;
	//�ҵ��޸�λ�ã�
	OffSet=2; 
	//�޸�
	memcpy((BYTE *)&cPtrTx[OffSet],(BYTE *)&macPIB.macLongAddr,sizeof(LONG_ADDR));
 	//�����еĶ�����д��
	FlashWriteMem(ConstMacAddrPN,0,cPtrTx,6); 
	
}

//��ȡЭ������Ϣ
BOOL GetCoordDescriptor(void)
{
	WORD Number;
	Number=FlashReadMem(ConstCoordAddrPN,0,(WORD *)&PANDescriptor,lengthof(PAN_DESCRIPTOR));
	if(Number==lengthof(PAN_DESCRIPTOR))
		return TRUE;
	return FALSE;
}

//�洢Э������Ϣ
void PutCoordDescriptor(void)
{

	WORD OffSet;
	WORD cPtrTx[20];
	//�Ѹ�ҳȫ��������
	FlashReadMem(ConstCoordAddrPN,0,cPtrTx,20); 
	//�ҵ��޸�λ��
	OffSet=0; 
	//�޸���Ӧ����
	memcpy((BYTE *)&cPtrTx[OffSet],(BYTE *)&PANDescriptor,sizeof(PAN_DESCRIPTOR));
	//����д��
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
	//�Ѹ�ҳȫ��������
	FlashReadMem(ConstPhyFreqAddrPN,0,cPtrTx,2); 
	//�ҵ��޸�λ��
	OffSet=0; 
	//�޸���Ӧ����
	cPtrTx[OffSet++]=FreqIndex; 
	cPtrTx[OffSet++]=phyPIB.phyCurrentChannel; 
	//����д��
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
	//�Ѹ�ҳȫ��������
	FlashReadMem(ConstPhyTxPowerAddrPN,0,cPtrTx,2); 
	//�ҵ��޸�λ��
	OffSet=0; 
	//�޸���Ӧ����
	cPtrTx[OffSet++]=TxPowerIndex; 
	cPtrTx[OffSet++]=phyPIB.phyTransmitPower; 
	//����д��
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
	//�Ѹ�ҳȫ��������
	FlashReadMem(ConstPhyBaudRateAddrPN,0,cPtrTx,2); 
	//�ҵ��޸�λ��
	OffSet=0; 
	//�޸���Ӧ����
	cPtrTx[OffSet++]=BaudRateIndex; 
	cPtrTx[OffSet++]=phyPIB.phyBaudRate; 
	//����д��
	FlashWriteMem(ConstPhyBaudRateAddrPN,0,cPtrTx,2);
	CLR_WDT(); 		
}
