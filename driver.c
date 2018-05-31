 #include "Tick.h"
#include "driver.h"

BYTE cDebug[5];

void RFWriteStrobe(BYTE addr)
{
    CS=0;         //Csn
    while(RF_CHIP_RDYn);     //Using SPI
    SPIPut(addr|CMD_WRITE);     //Strobe
    CS=1;
}
void RFWriteReg(BYTE addr, BYTE value)
{
    CS=0;
    while(RF_CHIP_RDYn);
    SPIPut(addr|CMD_WRITE);   //Address
    SPIPut(value);              //Value
    CS=1;
}
BYTE RFReadReg(BYTE addr)
{
    BYTE value,cCmd;
    CS=0;
    while(RF_CHIP_RDYn);
    cCmd = addr|CMD_READ;
    SPIPut(cCmd);
    value = SPIGet();
    CS=1;
    return value;
}

BYTE RFGetStatus(BYTE addr)
{
    BYTE value;
    CS=0;
    while(RF_CHIP_RDYn);
    SPIPut(addr|CMD_BURST_READ);
    value = SPIGet();
    CS=1;
    return value;
}

void RFWriteBurstReg(BYTE addr, BYTE *pWriteValue, BYTE size)
{
    BYTE i;
    CS=0;
    while(RF_CHIP_RDYn);
    SPIPut(addr|CMD_BURST_WRITE);   //Address
    for(i=0; i<size; i++)
    {
		SPIPut(*pWriteValue);
		pWriteValue++;
    }
    CS=1;
}

void RFReadBurstReg(BYTE addr, BYTE *pReadValue, BYTE size)
{
    BYTE i;
    CS=0;
    while(RF_CHIP_RDYn);
    SPIPut(addr|CMD_BURST_READ);   //Address
    for(i=0; i<size; i++)
    {
		*pReadValue = SPIGet();
		pReadValue++;
    }
    CS=1;
}



BYTE RFReadRxFIFO(void)
{
    BYTE value;
    CS=0;
    while(RF_CHIP_RDYn);
    SPIPut(REG_TRXFIFO|CMD_BURST_READ);   //Address
    value=SPIGet();
    CS=1;
    return value;
}
void RFWriteTxFIFO(BYTE *pTxBuffer,BYTE size)
{
	BYTE i;
    CS=0;
    while(RF_CHIP_RDYn);
    SPIPut(REG_TRXFIFO|CMD_BURST_WRITE); //Address
    for(i=0;i<size;i++)
    {
		SPIPut(pTxBuffer[i]);
    }
    CS=1;
}

void RFReset(void)				//��λ�����ڲ��Ĵ�����״̬���ص�Ĭ��
{
	SCLK=1;
	SO=0;	//��������ָ��������ֹǱ�ڵĴ������
	CS=1;
	Delay(30);
	CS=0;				//CS�ȵͺ��
	Delay(30);
	CS=1;			//�߱��ִ�Լ40us
	Delay(45);
	CS=0;   		//���ͣ��ȴ������ȶ����͸�λ����
	while(RF_CHIP_RDYn); 	//�ȶ�
	SPIPut(CMD_WRITE|STROBE_SRES);// ���͸�λ����
	while(RF_CHIP_RDYn); 	//�ٴεȴ������ȶ�
	CS=1; 					//�����˴β���
}

void RFClearRxBuffer(void)
{
	RFWriteStrobe(STROBE_SIDLE);
	RFWriteStrobe(STROBE_SFRX);
	RFWriteStrobe(STROBE_SRX);
}

void RFClearTxBuffer(void)
{
	RFWriteStrobe(STROBE_SIDLE);
	RFWriteStrobe(STROBE_SFTX);
	RFWriteStrobe(STROBE_SRX);
}

void RFSetBaudRate(BYTE BaudRate)
{
	switch (BaudRate)
	{
		case 0x01: //ͨѶ����Ϊ2.4K
		    //Ƶ��У��
		    RFWriteReg(REG_FSCTRL1,  0x08); // Freq synthesizer control.
		    RFWriteReg(REG_FSCTRL0,  0x00); // Freq synthesizer control.

			//ͨ��Ƶ���趨���ز�Ƶ�� 2433 MHz
		    RFWriteReg(REG_FREQ2,    0x5D); // Freq control word, high byte
		    RFWriteReg(REG_FREQ1,    0x93); // Freq control word, mid byte.
		    RFWriteReg(REG_FREQ0,    0xB1); // Freq control word, low byte.
		    
		    RFWriteReg(REG_MDMCFG4,  0x86); // Modem configuration.
		    RFWriteReg(REG_MDMCFG3,  0x83); // Modem configuration.
		    RFWriteReg(REG_MDMCFG2,  0x03); // Modem configuration.
		    RFWriteReg(REG_MDMCFG1,  0x22); // Modem configuration.
		    RFWriteReg(REG_MDMCFG0,  0xF8); // Modem configuration.
		    
		    //����ͨ�����ʺ͵��Ʒ�ʽ
			RFWriteReg(REG_DEVIATN,  0x44); // Modem dev (when FSK mod en)
		    RFWriteReg(REG_MCSM1,    0x3F); //ʼ�ճ��ڽ���״̬
		    RFWriteReg(REG_MCSM0,    0x18); //MainRadio Cntrl State Machine


		    RFWriteReg(REG_AGCCTRL2, 0x03); // AGC control.
		    RFWriteReg(REG_AGCCTRL1, 0x40); // AGC control.
		    RFWriteReg(REG_AGCCTRL0, 0x91); // AGC control.

		    RFWriteReg(REG_FREND1,   0x56); // Front end RX configuration.
		    RFWriteReg(REG_FREND0,   0x10); // Front end RX configuration.
		    
		    RFWriteReg(REG_FOCCFG,   0x16); // Freq Offset Compens. Config
		    RFWriteReg(REG_BSCFG,    0x6C); //  Bit synchronization config.

		    RFWriteReg(REG_FSCAL3,   0xA9); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL2,   0x0A); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL1,   0x00); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL0,   0x11); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSTEST,   0x59); // Frequency synthesizer cal.

		    RFWriteReg(REG_TEST2,    0x88); // Various test settings.
		    RFWriteReg(REG_TEST1,    0x31); // Various test settings.
		    RFWriteReg(REG_TEST0,    0x0B);  // Various test settings.
			CLR_WDT();
			break;
		case 0x02: //ͨѶ����Ϊ10K
		    //Ƶ��У��
		    RFWriteReg(REG_FSCTRL1,  0x06); // Freq synthesizer control.
		    RFWriteReg(REG_FSCTRL0,  0x00); // Freq synthesizer control.

			//ͨ��Ƶ���趨���ز�Ƶ�� 2433 MHz
		    RFWriteReg(REG_FREQ2,    0x5D); // Freq control word, high byte
		    RFWriteReg(REG_FREQ1,    0x93); // Freq control word, mid byte.
		    RFWriteReg(REG_FREQ0,    0xB1); // Freq control word, low byte.
		    
		    RFWriteReg(REG_MDMCFG4,  0x78); // Modem configuration.
		    RFWriteReg(REG_MDMCFG3,  0x93); // Modem configuration.
		    RFWriteReg(REG_MDMCFG2,  0x03); // Modem configuration.
		    RFWriteReg(REG_MDMCFG1,  0x22); // Modem configuration.
		    RFWriteReg(REG_MDMCFG0,  0xF8); // Modem configuration.
		    
		    //����ͨ�����ʺ͵��Ʒ�ʽ
			RFWriteReg(REG_DEVIATN,  0x44); // Modem dev (when FSK mod en)
		    RFWriteReg(REG_MCSM1,    0x3F); //ʼ�ճ��ڽ���״̬
		    RFWriteReg(REG_MCSM0,    0x18); //MainRadio Cntrl State Machine

		    RFWriteReg(REG_AGCCTRL2, 0x43); // AGC control.
		    RFWriteReg(REG_AGCCTRL1, 0x40); // AGC control.
		    RFWriteReg(REG_AGCCTRL0, 0x91); // AGC control.

		    RFWriteReg(REG_FREND1,   0x56); // Front end RX configuration.
		    RFWriteReg(REG_FREND0,   0x10); // Front end RX configuration.
		    
		    RFWriteReg(REG_FOCCFG,   0x16); // Freq Offset Compens. Config
		    RFWriteReg(REG_BSCFG,    0x6C); //  Bit synchronization config.

		    RFWriteReg(REG_FSCAL3,   0xA9); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL2,   0x0A); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL1,   0x00); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL0,   0x11); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSTEST,   0x59); // Frequency synthesizer cal.

		    RFWriteReg(REG_TEST2,    0x88); // Various test settings.
		    RFWriteReg(REG_TEST1,    0x31); // Various test settings.
		    RFWriteReg(REG_TEST0,    0x0B);  // Various test settings.
			CLR_WDT();
			break;	
		case 0x03: //ͨ������Ϊ250K
			//ͨ��Ƶ���趨���ز�Ƶ�� 2433 MHz
		    RFWriteReg(REG_FREQ2,    0x5D); // Freq control word, high byte
		    RFWriteReg(REG_FREQ1,    0x93); // Freq control word, mid byte.
		    RFWriteReg(REG_FREQ0,    0xB1); // Freq control word, low byte.
		
		    //Ƶ��У��
		    RFWriteReg(REG_FSCTRL1,  0x07); // Freq synthesizer control.
		    RFWriteReg(REG_FSCTRL0,  0x00); // Freq synthesizer control.
		    
		    RFWriteReg(REG_MDMCFG4,  0x2D); // Modem configuration.
		    RFWriteReg(REG_MDMCFG3,  0x3B); // Modem configuration.
		    RFWriteReg(REG_MDMCFG2,  0x73); // Modem configuration.
		    RFWriteReg(REG_MDMCFG1,  0x22); // Modem configuration.
		    RFWriteReg(REG_MDMCFG0,  0xF8); // Modem configuration.
		    
		    //����ͨ�����ʺ͵��Ʒ�ʽ
			RFWriteReg(REG_DEVIATN,  0x01); // Modem dev (when FSK mod en)
		    RFWriteReg(REG_MCSM1,    0x3F); //ʼ�ճ��ڽ���״̬
		    RFWriteReg(REG_MCSM0,    0x18); //MainRadio Cntrl State Machine
		    
		    RFWriteReg(REG_AGCCTRL2, 0xC7); // AGC control.
		    RFWriteReg(REG_AGCCTRL1, 0x00); // AGC control.
		    RFWriteReg(REG_AGCCTRL0, 0xB2); // AGC control.

		    RFWriteReg(REG_FREND1,   0xB6); // Front end RX configuration.
		    RFWriteReg(REG_FREND0,   0x10); // Front end RX configuration.

		    RFWriteReg(REG_FOCCFG,   0x1D); // Ƶ��ƫ�Ʋ���
		    RFWriteReg(REG_BSCFG,    0x1C); // λͬ������

		    RFWriteReg(REG_FSCAL3,   0xEA); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL2,   0x0A); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL1,   0x00); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL0,   0x11); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSTEST,   0x59); // Frequency synthesizer cal.

		    RFWriteReg(REG_TEST2,    0x88); // Various test settings.
		    RFWriteReg(REG_TEST1,    0x31); // Various test settings.
		    RFWriteReg(REG_TEST0,    0x0B);  // Various test settings.
			CLR_WDT();
			break;
		case 0x04: //ͨѶ����Ϊ500K
			//ͨ��Ƶ���趨���ز�Ƶ�� 2433 MHz
		    RFWriteReg(REG_FREQ2,    0x5D); // Freq control word, high byte
		    RFWriteReg(REG_FREQ1,    0x93); // Freq control word, mid byte.
		    RFWriteReg(REG_FREQ0,    0xB1); // Freq control word, low byte.

			 //Ƶ��У��
		    RFWriteReg(REG_FSCTRL1,  0x10); // Freq synthesizer control.
		    RFWriteReg(REG_FSCTRL0,  0x00); // Freq synthesizer control.

		    
		    RFWriteReg(REG_MDMCFG4,  0x0E); // Modem configuration.
		    RFWriteReg(REG_MDMCFG3,  0x3B); // Modem configuration.
		    RFWriteReg(REG_MDMCFG2,  0x73); // Modem configuration.
		    RFWriteReg(REG_MDMCFG1,  0x42); // Modem configuration.
		    RFWriteReg(REG_MDMCFG0,  0xF8); // Modem configuration.
		    
		    //����ͨ�����ʺ͵��Ʒ�ʽ
			RFWriteReg(REG_DEVIATN,  0x00); // Modem dev (when FSK mod en)
		    RFWriteReg(REG_MCSM1,    0x3F); //ʼ�ճ��ڽ���״̬
		    RFWriteReg(REG_MCSM0,    0x18); //MainRadio Cntrl State Machine

		    
		    RFWriteReg(REG_FOCCFG,   0x1D); // Freq Offset Compens. Config
		    RFWriteReg(REG_BSCFG,    0x1C); //  Bit synchronization config.

		    RFWriteReg(REG_AGCCTRL2, 0xC7); // AGC control.
		    RFWriteReg(REG_AGCCTRL1, 0x40); // AGC control.
		    RFWriteReg(REG_AGCCTRL0, 0xB0); // AGC control.

		    RFWriteReg(REG_FREND1,   0xB6); // Front end RX configuration.
		    RFWriteReg(REG_FREND0,   0x10); // Front end RX configuration.

		    RFWriteReg(REG_FSCAL3,   0xEA); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL2,   0x0A); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL1,   0x00); // Frequency synthesizer cal.
		    RFWriteReg(REG_FSCAL0,   0x19); // Frequency synthesizer cal.

		    RFWriteReg(REG_FSTEST,   0x59); // Frequency synthesizer cal.
		    RFWriteReg(REG_TEST2,    0x88); // Various test settings.
		    RFWriteReg(REG_TEST1,    0x31); // Various test settings.
		    RFWriteReg(REG_TEST0,    0x0B);  // Various test settings.
			CLR_WDT();
			break;
		default:
			break;
	}
}


void RFInitSetup(void)
{
	WORD prio; //�����жϼĴ�����ֵ
	emDint(&prio);
	//����Ҫ��λ
	RFReset();
	//��ʱһ��
	Delay(20);
	//�ı�һ�·�ʽ��GDO0ֻ�������գ�GDO2��������ز�CS
    RFWriteReg(REG_IOCFG2,   0x0E);  // �ߵ�ƽ˵�������ز�
    RFWriteReg(REG_IOCFG0,   0x06);  // GDO0 ��������.
    RFWriteReg(REG_PKTLEN,   0x3F);  // Packet length,�����64�ֽڼ��ϳ��ȹ�64�ֽ�
    RFWriteReg(REG_PKTCTRL1, 0x0C);  // CRCУ��ʧ�ܣ��Զ����������������RSSI��CRCУ����
                                     //��ַУ�飬0x00��0xFF�ǹ㲥��ַ
    RFWriteReg(REG_PKTCTRL0, 0x05);  // �ɱ䳤�ȵ����ݱ�
	RFWriteReg(REG_ADDR,     0x00);  // ID���Ѿ��������ã���ַ��ʶ�ر�
	//Ĭ����ͨ������10K
	RFSetBaudRate(1); 
	//Ĭ���ŵ���0x00
	RFSetChannel(0x00);
	//Ĭ�Ϸ��书����0xFE
	RFSetTxPower(0xFF);
	cDebug[0]=RFReadReg(REG_IOCFG2);
	cDebug[1]=RFReadReg(REG_IOCFG0);
	cDebug[2]=RFReadReg(REG_PKTLEN);
	cDebug[3]=RFReadReg(REG_PKTCTRL1);
	Nop();
	CLR_WDT();
	emEint(&prio);

}

BYTE RFDetectEnergy(void)
{
	BYTE cVal,cRssi;//RSSI�ļ��㷽��
	WORD prio;
	emDint(&prio);						
	cVal=RFReadReg(CMD_READ|REG_RSSI);
	emEint(&prio);
	if(cVal>=128)
		cRssi=(cVal-256)/2-RSSI_OFFSET;
	else
		cRssi=cVal/2-RSSI_OFFSET;
	return cRssi;
}

void RFSetChannel(BYTE channel)
{
	WORD prio;
	emDint(&prio);
	RFWriteStrobe(STROBE_SIDLE); //��ת�ɽ���״̬
	RFWriteReg(CMD_WRITE|REG_CHANNR,channel);//�ŵ�����
	//����״̬��
	RFWriteStrobe(STROBE_SIDLE);
	RFWriteStrobe(STROBE_SFRX);
	RFWriteStrobe(STROBE_SRX);

	emEint(&prio);

}

void RFSetTxPower(BYTE power)
{
	WORD prio;
	emDint(&prio);
	RFWriteBurstReg(REG_PATABLE,&power,1);//��������
	emEint(&prio);
}

void RFSetTRxState(RF_TRX_STATE state)
{
	WORD prio;
	emDint(&prio);
	switch (state)
	{
		case RF_TRX_RX:
			RFWriteStrobe(STROBE_SIDLE);
			RFWriteStrobe(STROBE_SRX);	//����״̬
			break;
		case RF_TRX_OFF:
			RFWriteStrobe(STROBE_SIDLE);
			RFWriteStrobe(STROBE_SXOFF);//��Ƶ�ر�
			break;
		case RF_TRX_IDLE:
			RFWriteStrobe(STROBE_SIDLE);//����״̬
			break;
		case RF_TRX_TX:
			RFWriteStrobe(STROBE_SIDLE);
			RFWriteStrobe(STROBE_STX);	//����״̬
			break;
		default:
			break;
	}
	emEint(&prio);
}

RF_TRX_STATE RFGetTRxState(void)
{
	BYTE cVal,cState;
	WORD prio;
	emDint(&prio);
	cVal=RFGetStatus(REG_MARCSTATE)&0x1F;
	emEint(&prio);
	switch (cVal)
	{
		case 0x01:cState=RF_TRX_IDLE;break;
		case 0x02:cState=RF_TRX_OFF;break;
		case 0x0D:cState=RF_TRX_RX;break;
		case 0x13:	cState=RF_TRX_TX;break;
		default:cState=0xFF;break;		
	}
	return cState;
}

void RFDetectStatus(void)
{
	BYTE cState;
	WORD prio;
	emDint(&prio);
	cState=RFGetStatus(REG_MARCSTATE)&0x1F;
	emEint(&prio);
	CLR_WDT(); //ι������ֹ����������븴λ
	if(cState==0x11)//�������
	{
		RFWriteStrobe(STROBE_SFRX);
		RFWriteStrobe(STROBE_SRX);
	}
	else if(cState==0x16)//�������
	{
		RFWriteStrobe(STROBE_SFTX);
		RFWriteStrobe(STROBE_SRX);
	}
	else if(cState==0x01)//����״̬
	{
		RFWriteStrobe(STROBE_SRX);//�������״̬ת������״̬
	}
	else if(cState==0x00)//˯��״̬
	{
		RFWriteStrobe(STROBE_SIDLE);
		RFWriteStrobe(STROBE_SRX);
	}
	else if(cState==0x0D)//����״̬
	{
		Nop();
		Nop();
	}
}

BOOL RFTranmitByCSMA(void)
{
	BYTE status;
	WORD prio;
	BOOL bSucceed=TRUE;
	DWORD dwStartTick;
	
	LATBbits.LATB2=0; //LNA	
	LATBbits.LATB15=1; //PA
    //Delay(0x5000);
	
	RFSetTRxState(RF_TRX_RX);
	RFDint();
	emDint(&prio);
	status=RFGetStatus(REG_PKTSTATUS)&0x10; //ȡ��CCAλ0001 0000
	emEint(&prio);
	dwStartTick=GetTicks();
	while(!status)//cca is not clear
	{
		if(DiffTicks(dwStartTick,GetTicks())>50)//��ʱ////ZXY MODIFIED
		{
			RFSetTRxState(RF_TRX_RX);
			break;
		}
		emDint(&prio);
		status=RFGetStatus(REG_PKTSTATUS)&0x10; 
		emEint(&prio);
		Delay(100);
	}

	dwStartTick=GetTicks();
	RFSetTRxState(RF_TRX_TX);
	while(!RF_GDO0)//�ȴ����ͳɹ�
	{
		if(DiffTicks(dwStartTick,GetTicks())>50)//ZXY MODIFIED
		{
			RFSetTRxState(RF_TRX_RX);
			bSucceed=FALSE;
			break;
		}
	}
	dwStartTick=GetTicks();
	while(bSucceed && RF_GDO0)
	{
		if(DiffTicks(dwStartTick,GetTicks())>50)//ZXY MODIFIED
		{
			RFSetTRxState(RF_TRX_RX);
			bSucceed=FALSE;
			break;
		}
	}
	CLR_WDT(); //ι������ֹ����������븴λ
	RFEint();        //�򿪽����жϣ������������������ж�
	if(bSucceed)
	{
		LEDBlinkYellow();
		LATBbits.LATB15=0; //PA
		LATBbits.LATB2=1; //LNA
		return TRUE;
	}
	else
	{
		emDint(&prio);
		RFClearTxBuffer();
		emEint(&prio);
		return FALSE;
	}
}
