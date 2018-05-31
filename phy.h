/************************************************************
�ļ����������ļ���Ҫ������ʵ�����޴����������PHY��Ļ������ܣ������ŵ�ѡ��
					������Ƶ״̬��������⣬���ͺͽ�������
�汾��Ϣ��v1.0
�޸�ʱ�䣺2008/03/
*************************************************************/
#ifndef _PHY_H
#define _PHY_H

#include "zigbee.h"
#include "driver.h"
#include "mcu.h"
#include "em16RTOS24.h"
#include "Nvm_Flash.h"


//�����ŵ������ֵ
#define ConstRFChannelSize  16
//���幦�ʵ����ȼ���		
#define ConstRFPowerSize	18		
//���岨���ʵ����ȼ���
#define ConstRFBaudRateSize	4


/**********��ͬͨ�����ʣ�RSSI_OFFSET��ֵ��һ��****************
  2.4K	  71
  10K	  69
  250K    72
  500K    72
***************************************************************/

typedef RF_TRX_STATE PHY_TRX_STATE;

typedef struct _RHY_RX_BUFFER    //���ջ�����
{
    BYTE RxBuffer[ConstPhyRxBufferSize];
	struct
	{
		WORD cWrite;  				//��¼д���λ��
        WORD cRead;						//��¼������λ��
	}Postion;							//��¼���ջ�������дλ��
}PHY_RX_BUFFER; 


/*********************************************************************
 * ������:    void PHYInitSetup(void)
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE *pTxBuffer��д��ֵ��ָ�룬BYTE sizeд����ֽ���
 * �������:  ��
 * ע����� ��
 * ��������:  ʵ��������һЩ�������ã�phyPIB���ŵ������ʣ�У�飬RF�ļ���
 ********************************************************************/
void PHYInitSetup(void);
/*********************************************************************
 * ������:    BYTE PHYDetectEnergy(void);
 * ǰ������:  PHYInitSetup()�Ѿ�����
 * �������:  ��
 * �������:  ���ص�ǰ�ŵ�������ֵ
 * ע����� ��
 * ��������:  ��������źŵ�ǿ��
 ********************************************************************/
BYTE PHYDetectEnergy(void);//�������

/*********************************************************************
 * ������:    void PHYSetTxPower(BYTE cPower)
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE cPower������Ĺ���ֵ
 * �������:  ��
 * ע����� ��
 * ��������:  �����趨���书��ֵ
 ********************************************************************/
//���÷��͹���
WORD PHYSetTxPower(BYTE Index);

//���ò�����
void PHYSetBaudRate(BYTE BaudRate);


/*********************************************************************
 * ������:    BOOL PHYRevPacket(void)
 * ǰ������:  ���յ����ݰ��󴥷��ж�
 * �������:  ��
 * �������:  ������յ�һ����ȷ�����ݰ�������TRUE
 * ע����� ������յ�һ����������ݰ��󣬽��д������ǹؼ��еĹؼ�
 * ��������:  �ѽ��յ������ݰ����ŵ��Զ���Ľ��ջ�������ȥ
 ********************************************************************/
void PHYPut(BYTE cVal);
BYTE PHYGet(void);
void PHYGetArray(BYTE *ptr,BYTE cSize);

WORD PHYSetChannel(BYTE Index);
BOOL PHYExistPacket(void);
BYTE PHYGetLinkQuality(BYTE rssi);


#define PHY_RF_GDO0 	RF_GDO0
#define PHY_RF_GDO2 	RF_GDO2
#define PHYSetTRxState(state) RFSetTRxState(state)			//������Ƶ���շ�״̬
#define PHYGetTRxState() 	  RFGetTRxState 				//��ȡ��Ƶ��״̬

#define PHYSetBaud(v)		RFSetBaudRate(v)
#define PHYSetTxPwr(v)		RFSetTxPower(v)
#define PHYSetChan(v)		RFSetChannel(v)
#define PHYGetChannel()	    phyPIB.phyCurrentChannel	 //��ȡ�ŵ�
#define PHYGetTxPower()		phypIB.phyTransmitPower
#define PHYDetectStatus() 	RFDetectStatus()

#define PHYGetTxNumber()	RFGetStatus(REG_TXBYTES)&0x7F;
#define PHYGetRxStatus()	RFGetStatus(REG_RXBYTES);
#define PHYDetectChannels()	RFGetStatus(REG_PKTSTATUS)&0x10 //ȡ��CCAλ0001 0000

#define PHYClearTx()		RFClearTxBuffer()
#define PHYClearRx()		RFClearRxBuffer() //RF FlushFIFO
#define PHYReadRx()			RFReadRxFIFO()  //��RF���ջ������ж�һ����

#define PHYPutTxBuffer(ptr,size) RFWriteTxFIFO(ptr,size)
#define PHYTranmitByCSMA()	   	 RFTranmitByCSMA()	

WORD PHYGetPosition(void);
BOOL PHYPutRxBuffer(void);

#endif
