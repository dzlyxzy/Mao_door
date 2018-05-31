#ifndef _NVM_FLASH_H
#define _NVM_FLASH_H

#include "common.h"
#include "zigbee.h"
#include "mcu.h"
#include "flash.h"
#include "FlashMM.h"

#define IEEE_VALID_CODE 0x9968
#define FREQIndex		0x4452
#define TXPOWERIndex	0x3658
#define BAUDRATEIndex	0x7639


/**********************************************************************************************/
//���ó���洢����Ϊ�洢ʱ�Ĳ�������
/**********************************************************************************************/


//����洢λ��,�߼�������
#define ConstIEEEAddrPN  		0
#define ConstMacAddrPN   		1
#define ConstStatusAddrPN  	 	2
#define ConstCoordAddrPN		3
#define ConstRTUAddrPN			4
#define ConstPhyFreqAddrPN		5
#define ConstPhyBaudRateAddrPN	6
#define ConstPhyTxPowerAddrPN	7
#define CntNeighborTabNO		8	

//�洢�����IEEE��ַ
void PutIEEEAddrInfo(WORD Indication,const LONG_ADDR *IEEEAddr);
//��ȡ�洢��ַ��־
BOOL GetIEEEIndication(WORD *Indication);
//��ȡ�Զ���õ�ַ��Ϣ
BOOL GetIEEEAddr(LONG_ADDR *IEEEAddr);
//��ȡ״̬
BOOL GetMACStatus(void);
//�洢MAC��״̬
void PutMACStatus(void);
//��������PANId
BOOL GetMACPANId(void);
//�����̵�ַ
BOOL GetMACShortAddr(void);
//��������ַ
BOOL GetMACLongAddr(void);
//�洢MAC��ַ
void PutMACAddr(void); 
//��ȡЭ������Ϣ
BOOL GetCoordDescriptor(void);
//�洢Э������Ϣ
void PutCoordDescriptor(void);
//��ȡ�ŵ�
BOOL GetPHYFreq(WORD *Index);
//�洢�ŵ�
void PutPHYFreq(WORD FreqIndex);
//��ȡ���书��
BOOL GetPHYTxPower(WORD *Index);
//�洢���书��
void PutPHYTxPower(WORD TxPowerIndex);
//��ȡ������
BOOL GetPHYBaudRate(WORD *Index);
//�洢������
void PutPHYBaudRate(WORD BaudRateIndex);

#endif
