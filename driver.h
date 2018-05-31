#ifndef _DRIVER_H
#define _DRIVER_H

#include "cc2500.h"
#include "common.h"
#include "spi.h"
#include "zigbee.h"
#include "Interface.h"
#include "Tick.h"

#define RSSI_OFFSET 71


typedef enum _RF_TRX_STATE	//��ƵоƬ��״̬
{
    RF_TRX_RX,
    RF_TRX_OFF,
    RF_TRX_IDLE,
    RF_TRX_TX
} RF_TRX_STATE;

/************************************************************
														driver.h
�ļ����������ļ���Ҫ������ʵ�ֶ�CC2500�Ļ���������Ҳ��Ӳ����������֮һ
�汾��Ϣ��v1.0
�޸�ʱ�䣺2008/03/
*************************************************************/

/*********************************************************************
 * ������:    void RFReset(void)
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  ��
 * �������:  ��
 * ע����� ��
 * ��������:  ��ʼ������
 ********************************************************************/
void RFReset(void);
/*********************************************************************
 * ������:    void RFWriteStrobe(BYTE cmd)
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE cmd����Ҫд�������
 * �������:  ��
 * ע����� �ޡ�
 * ��������:  ��CC2500����������
 ********************************************************************/
void RFWriteStrobe(BYTE cmd);//дCC2500����
/*********************************************************************
 * ������:    void RFWriteReg(BYTE addr, BYTE value);
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE addr���Ĵ�����ַ��BYTE addr����Ҫ���õ�ֵ
 * �������:  ��
 * ע����� ��
 * ��������:  ��CC2500���üĴ���
 ********************************************************************/
void RFWriteReg(BYTE addr, BYTE value);//д�Ĵ���ֵ
/*********************************************************************
 * ������:    BYTE RFReadReg(BYTE addr);
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE addr���Ĵ�����ַ
 * �������:  ���ؼĴ�����ֵ
 * ע����� ��
 * ��������:  ��ȡCC2500���üĴ���
 ********************************************************************/
BYTE RFReadReg(BYTE addr);//���Ĵ�����ֵ
/*********************************************************************
 * ������:    BYTE RFGetStatus(BYTE addr);
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE addr���Ĵ�����ַ
 * �������:  ����CC2500��ǰ��״̬
 * ע����� ��
 * ��������:  ��ȡCC2500״̬
 ********************************************************************/
BYTE RFGetStatus(BYTE addr);
/*********************************************************************
 * ������:    void RFWriteBurstReg(BYTE addr,BYTE *pWriteValue,BYTE size);
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE addr���Ĵ����ĳ�ʼ��ַ��BYTE *pWriteValue�洢д������ֵָ��,
 *						BYTE size����д��Ĵ����ĸ���
 * �������:  ��
 * ע����� ��
 * ��������:  ����д��CC2500���üĴ���
 ********************************************************************/
void RFWriteBurstReg(BYTE addr,BYTE *pWriteValue,BYTE size);//����д�����Ĵ�����ֵ
/*********************************************************************
 * ������:    void RFReadBurstReg(BYTE addr,BYTE *pReadValue,BYTE size);
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE addr���Ĵ����ĳ�ʼ��ַ��BYTE *pWriteValue�洢��ȡ��ֵָ��,
 *						BYTE size���Ƕ�ȡ�Ĵ����ĸ���
 * �������:  ��
 * ע����� ��
 * ��������:  ������ȡCC2500���üĴ���
 ********************************************************************/
void RFReadBurstReg(BYTE addr,BYTE *pReadValue,BYTE size);//�����������Ĵ�����ֵ
/*********************************************************************
 * ������:    BYTE RFReadRxFIFO(void)
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  ��
 * �������:  BYTE����RXFIFO�ж�ȡһ���ֽ�
 * ע����� ��
 * ��������:  ��RXFIFO�Ĵ����ж�ȡһ���ֽ�
 ********************************************************************/
BYTE RFReadRxFIFO(void);//�ѽ��յ������ݶ�����ջ�����
/*********************************************************************
 * ������:    void RFWriteTxFIFO(BYTE *pTxBuffer,BYTE size)
 * ǰ������:  SPIInit()�Ѿ�����
 * �������:  BYTE *pTxBuffer��д��ֵ��ָ�룬BYTE sizeд����ֽ���
 * �������:  ��
 * ע����� ��
 * ��������:  ��TXFIFO�Ĵ�����д������
 ********************************************************************/
void RFWriteTxFIFO(BYTE *pTxBuffer,BYTE size);

void RFClearTxBuffer(void); //��ս��ջ�����
void RFClearRxBuffer(void); //��շ��ͻ�����
void RFInitSetup(void);		//�Ĵ�������
BYTE RFDetectEnergy(void);	//�������
void RFSetChannel(BYTE channel);	//�����ŵ�
void RFSetTxPower(BYTE power);		//���÷��书��
void RFSetBaudRate(BYTE BaudRate);

void RFSetTRxState(RF_TRX_STATE state); //����RF״̬
RF_TRX_STATE  RFGetTRxState(void);
void RFDetectStatus(void); //RF״̬���

BOOL RFTranmitByCSMA(void); //����CSMA��������
#endif
