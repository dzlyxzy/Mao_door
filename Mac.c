#include "mac.h"
#include "timeds.h"
#include "bootloader.h"
#include "common.h"
#include "infra.h"
#include "Loc.h"

//��λ����
BYTE Loc_Buffer[20];
WORD distance_Buffer[20];

BYTE Loc_i=35,Loc_j=0;
BYTE Loc_Times=0;
BYTE Type = 0; //ϵͳ��ѯ��ϵͳ��ϢTEM_SENT_REQʱ������ȫ�ֱ���Type�������ݵĻظ�(���汾������))
extern BYTE infrared_correct;

//����Э�����б��С
#define ConstCoordQueSize	2 
extern IO_INFO AD_Value[IO_NUM];
BYTE CoordID[2]={0,0};
static BYTE PreviousRxmacDSN[4] = {0xff,0xff,0xff,0xff};
BYTE Route_flag = 0;    //0��ʾ���أ�1��ʾЭ������2��ʾת��Ŀ�Ľڵ㣻3��ʾ�㲥
/*
Ŀǰ����Mac��Flash�е����ݴӵ�ַ0X9000��ʼ��
0X9000~0X907E��ŵ��ǽڵ�ĳ���ַ
0X9080~0X90FE��ŵĽڵ��MAC������,Ҳ��������ַ��
0X9100~0X91AE��ŵ��ǽڵ��״̬
0X9280~0X92FE��ŵ�����������Э�����������Ϣ
*/

//�����������ı���
BYTE JoinNwkIndction = 0;//�������������ı�־λ��
//BYTE RssiLevel = 180;//ͨ�����ֵ�ڵ��ж���·������Ȼ��ѡ���������
BYTE RssiLevel = 160;//ͨ�����ֵ�ڵ��ж���·������Ȼ��ѡ���������


//���������ȫ�ֱ���
MAC_PIB macPIB;					//���ڼ�¼mac���PIB����
MAC_STATUS macStatus;			//��¼MAC״̬
CFG_RSP_BUFFER MACCfgRspBuf;
CFG_RSP_SHARE_BUFFER MACCfgRspSharBuf;
BYTE MACBuffer[ConstMacBufferSize];
extern PHY_PIB phyPIB;

//������ʱ�洢���͵����ݣ������ط�
MAC_TX_QUEUE TxFrameQue[ConstMacTxQueSize];
//�����γ�һ�������Ľ�������֡
MAC_RX_FRAME macCurrentRxFrame; 
//�������������ݵĻ�����
MAC_TX_BUFFER TxBuffer;
//����һ��Э�����б�
PAN_DESCRIPTOR CoordQue[ConstCoordQueSize];
//����������Ϣ
PAN_DESCRIPTOR PANDescriptor;


//��ʼ����ʱ�������Ҫ���ĵ��ǽڵ�ı�ʾ
void MACInitSetup(void)
{
	BYTE i;
	//��ʼ��
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		TxFrameQue[i].Flags.bits.bInUse=0;
	}
	
	MACCfgRspBuf.Ptr=0;
	MACCfgRspSharBuf.bReady=0;
	MACCfgRspSharBuf.Ptr=0;

	macPIB.macPANId.nVal=0;
	macPIB.macShortAddr.nVal=0;
	macPIB.macLongAddr.byte.dwLsb=0;
	macPIB.macLongAddr.byte.dwMsb=0;
	macPIB.macDSN=0;
	
		//״̬��ʼ��
	macStatus.nVal=0;
	macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
	
	macPIB.macCoordShortAddr.nVal=0;
	macPIB.macCoordLongAddr.byte.dwLsb=0;
	macPIB.macCoordLongAddr.byte.dwMsb=0;

    //�����ط���ʱ��
	macPIB.macAckWaitDuration=60;
	//GTS 
	macPIB.macGTSPermit=0;
    
	//�豸����
	macPIB.DeviceInfo.bits.StackProfile=0;
	macPIB.DeviceInfo.bits.ZigBeeVersion=ZIGBEE_VERSION;
	macPIB.DeviceInfo.bits.RxOnWhenIdle=1;
	macPIB.DeviceInfo.bits.AutoRequest=1;
	macPIB.DeviceInfo.bits.PromiscuousMode=1;
	macPIB.DeviceInfo.bits.BattLifeExtPeriods=0;
	macPIB.DeviceInfo.bits.PotentialParent=1;
	macPIB.DeviceInfo.bits.PermitJoin=1;
	//��������Ҫ�ı�Ĳ�������ʼ��ȫ��Ϊ0

	//��ʾ�豸��Э�������ǽڵ㡣
	macPIB.DeviceInfo.bits.DeviceType=ZIGBEE_RFD;

	//����������ʼ��
	PANDescriptor.CoordAddrMode=0;
	PANDescriptor.GTSPermit=0;
	PANDescriptor.SecurityUse=0;
	PANDescriptor.SecurityFailure=0;
	PANDescriptor.ACLEntry=0;

	PANDescriptor.CoordPANId.nVal=0;
	PANDescriptor.CoordShortAddr.nVal=0;
	PANDescriptor.CoordLongAddr.nVal[0]=0;
	PANDescriptor.CoordLongAddr.nVal[1]=0;
	PANDescriptor.CoordLongAddr.nVal[2]=0;
	PANDescriptor.CoordLongAddr.nVal[3]=0;
	PANDescriptor.SuperframeSpec.nVal=0;

	//MAC���������֡
	macCurrentRxFrame.packetSize=0;
	macCurrentRxFrame.frameCON.nVal=0;
	macCurrentRxFrame.sequenceNumber=0;
	macCurrentRxFrame.srcAddress.AddrMode=0;
	macCurrentRxFrame.srcAddress.PANId.nVal=0;
	macCurrentRxFrame.srcAddress.ShortAddr.nVal=0;
	macCurrentRxFrame.srcAddress.LongAddr.byte.dwLsb=0;
	macCurrentRxFrame.srcAddress.LongAddr.byte.dwMsb=0;
	
	macCurrentRxFrame.dstAddress.AddrMode=0;
	macCurrentRxFrame.dstAddress.PANId.nVal=0;
	macCurrentRxFrame.dstAddress.ShortAddr.nVal=0;
	macCurrentRxFrame.dstAddress.LongAddr.nVal[0]=0;
	macCurrentRxFrame.dstAddress.LongAddr.nVal[1]=0;
	macCurrentRxFrame.dstAddress.LongAddr.nVal[2]=0;
	macCurrentRxFrame.dstAddress.LongAddr.nVal[3]=0;

	macCurrentRxFrame.rssi=0;
	macCurrentRxFrame.crc=0;
	macCurrentRxFrame.bReady=0;

	//�ڵ���ݱ�ʶ
	macPIB.CDNum = 0xaa;

	//���ͻ�����
	for(i=0;i<ConstMacPacketSize;i++) 
	{
		TxBuffer.cTxBuffer[i]=0;
	}
	TxBuffer.cSize=0;
	
	//�����ж��Ƿ��Ѿ�����
	//�ϵ�,���ȶ�ȡMAC��״̬���鿴�Ƿ�����
	GetMACStatus();

	//���û������������һ���ϵ��ʱ��.FLASH������洢λȫ��Ϊ�ա�
	if(macStatus.nVal == 0XFFFF)
	{
		//״̬��ʼ��
		macStatus.nVal=0;
		macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
		macStatus.bits.isAssociated = 1;//����FLASH�����ڲ�д��ʱ��Ĭ����0XFFFF�����Բ�����0������ᱻ��Ϊ�Ѿ�����
		//��ַ��Ϣ����,��Ҫ�ǻ�ȡһ������ַ
		MACInitIEEEAddr();	
	}
	//����Ѿ������������Ӧ�ĵ�ַ��Flash�ж�����
	else
	{
		MACGetAddrInfo();
	}

/*	{	macStatus.nVal=0;
		macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;	
		macStatus.bits.isAssociated = 0;//����FLASH�����ڲ�д��ʱ��Ĭ����0XFFFF�����Բ�����0������ᱻ��Ϊ�Ѿ�����
			//��ַ��Ϣ����,��Ҫ�ǻ�ȡһ������ַ
		MACInitIEEEAddr();
	}
*/
	//��ʼ��Э�����б�
	MACFormatPANRecord();
}


//�������еĵ�ַ��Ϣ��״̬��Ϣ�ĳ�ʼ��
void MACInitIEEEAddr(void)
{
	WORD Index,Indication,Number;
	LONG_ADDR addr,allocAddr;
	BOOL bIndication;

	//���ȴӱ�־λ���������־λ
	bIndication=GetIEEEIndication((WORD *)&Index);
	//�ж��Ƿ��Ѿ��Զ���ȡ����ַ
	while((Index!=IEEE_VALID_CODE) || (!bIndication))	
	{
		AllocIEEEAddr(&addr);
		//�Ѿ������˵�ַ�����д洢
		Indication=IEEE_VALID_CODE;
		//�洢��־������ĵ�ַ
		PutIEEEAddrInfo(Indication,&addr);
		//���ȴӱ�־λ���������־λ�����������
		bIndication=GetIEEEIndication((WORD *)&Index);
	}
	//��������ĵ�ַ
	GetIEEEAddr(&allocAddr);
	//��������ַ
	GetMACLongAddr();
	//�ж������Ƿ�һ��
    Number=memcmp((BYTE *)&allocAddr,(BYTE *)&macPIB.macLongAddr,sizeof(LONG_ADDR));
	while(Number!=0)
	{
		//���ó���ַ
		memcpy((BYTE *)&macPIB.macLongAddr,(BYTE *)&allocAddr,sizeof(LONG_ADDR));
		//�����ʾ
		macPIB.macPANId.nVal=0xFFFF;
		//�̵�ַΪ0
		macPIB.macShortAddr.nVal=0;
		//Ŀ�ĵ�ַ
		macPIB.macCoordShortAddr.nVal=0xFFFF;
		//�豸����
		macPIB.DeviceInfo.bits.DeviceType=ZIGBEE_RFD;
		//����״̬
		macStatus.nVal=0;
		//�����ű�֡
		macStatus.bits.allowBeacon=1;
		//��ַģʽ
		macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
		//�Ƚ�
		Number=memcmp((BYTE *)&allocAddr,(BYTE *)&macPIB.macLongAddr,sizeof(LONG_ADDR));
	}
}

//�洢��ַ��Ϣ
void MACSetAddrInfo(void)
{
	//�洢�̵�ַ
	PutMACAddr();
	//�洢PAN������Ϣ		
	PutCoordDescriptor();	
	//�洢״̬
	PutMACStatus();	
	CLR_WDT();
}
//��ȡ��ַ��Ϣ
void MACGetAddrInfo(void)
{
	//��ȡ�����ַ
	GetMACPANId(); 	
	//��ȡ�̵�ַ	
	GetMACShortAddr();
	//��ȡ����ַ	
	GetMACLongAddr();
	//��ȡ��������		
	GetCoordDescriptor();
	CLR_WDT();
}

//��ַ��ʶ
BOOL MACCheckAddress(NODE_INFO *pAddr)
{
	BYTE cNumber,cSize,cLength;
	BYTE cVal[8];
	BYTE cZero[8];
	BYTE i;
	for(i=0;i<8;i++)
	{
		cVal[i]=0xFF;
		cZero[i]=0;
	}
	//������������ʹ�������ʶ��֤
	if(macStatus.bits.isAssociated)
	{
		//��������ַ�Ȳ��ǹ㲥Ҳ���Ǳ�����ַ�����˳�
		if(((*pAddr).PANId.nVal!=macPIB.macPANId.nVal) && ((*pAddr).PANId.nVal!=0xFFFF))
			return FALSE;
	}
	
	//���Ƕ̵�ַ�����ж��Ƿ��Ǳ�����ַ�����ǹ㲥��ַ
	if((macStatus.bits.isAssociated) && (*pAddr).AddrMode==MAC_DST_SHORT_ADDR)
	{
		//��Ϊ�գ����˳�
		if((*pAddr).ShortAddr.nVal==macPIB.macShortAddr.nVal)
        {
            if(macCurrentRxFrame.sequenceNumber!= PreviousRxmacDSN[0])
            {
                Route_flag = 0;
                PreviousRxmacDSN[0] = macCurrentRxFrame.sequenceNumber;
                return TRUE;
            }
            else
                return FALSE;
        }	
		else if((*pAddr).ShortAddr.nVal==0xFFFF)
		{
            if(macCurrentRxFrame.sequenceNumber!= PreviousRxmacDSN[3])
            {
                Route_flag = 3;
                PreviousRxmacDSN[3] = macCurrentRxFrame.sequenceNumber;
                return TRUE;
            }
            else
                return FALSE;
        }	
        else if(macCurrentRxFrame.srcAddress.ShortAddr.nVal == (((WORD)Parameter[route_high]<<8&0xff00)|(WORD)Parameter[route_low]&0x00ff))
        {
            if(macCurrentRxFrame.sequenceNumber!= PreviousRxmacDSN[1])
            {
                Route_flag = 1; //��ʾ������Ϊ�ڵ��ϴ���Э����
                PreviousRxmacDSN[1] = macCurrentRxFrame.sequenceNumber;
                return TRUE;
            }
            else
                return FALSE;
        }	
        else if((*pAddr).ShortAddr.nVal==(((WORD)Parameter[route_high]<<8&0xff00)|(WORD)Parameter[route_low]&0x00ff))
        {
            if(macCurrentRxFrame.sequenceNumber!= PreviousRxmacDSN[2])
            {
                Route_flag = 2; //��ʾ������ΪЭ�����´����ڵ�
                PreviousRxmacDSN[2] = macCurrentRxFrame.sequenceNumber;
                return TRUE;
            }
            else
                return FALSE;
        }	
		else
			return FALSE;
	}
	else if((*pAddr).AddrMode==MAC_DST_LONG_ADDR)
	{
		//�ж��Ƿ�Ϊ��
		cLength=memcmp((BYTE *)&(*pAddr).LongAddr,(BYTE *)cZero,sizeof(LONG_ADDR));
		//�ж��Ƿ�Ϊ������ַ
		cNumber=memcmp((BYTE *)&(*pAddr).LongAddr,(BYTE *)&macPIB.macLongAddr,sizeof(LONG_ADDR));
		//�ж��Ƿ�Ϊ�㲥
		cSize=memcmp((BYTE *)&(*pAddr).LongAddr,(BYTE *)cVal,sizeof(LONG_ADDR));
		CLR_WDT();
		//����Ϊ�գ����˳�
		if(cLength==0)
			return FALSE;
		else if(cNumber==0)
			return TRUE;
		else if(cSize==0)	
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
} 

//����ACKȷ��֡
BOOL MACSendACK(BYTE dsn)
{
	BYTE cPtrTx[6];
	BYTE cPktSize;
	CRC_RESULT	crcRes;
	BYTE cSize=0;
	//���������ȣ�CRC��RSSI
	cPtrTx[cSize++]=3;
	cPtrTx[cSize++]=MAC_FRAME_ACK;
	cPtrTx[cSize++]=0;
	cPtrTx[cSize++]=dsn;
	//�����Լ��ӵ�CRCУ���
	crcRes=CrcCalc(cPtrTx,cSize);
	cPtrTx[cSize++]=crcRes.cVal[0];
	cPtrTx[cSize++]=crcRes.cVal[1];
	//�����޸ĳ���
	cPtrTx[0]+=sizeof(CRC_RESULT);

	//���ȣ�FCS��dsn
	PHYPutTxBuffer((BYTE *)&cPtrTx,cSize);
	//ι������ֹ����������븴λ
	CLR_WDT(); 
	cPktSize=PHYGetTxNumber();
	//��д�뷢�ͻ������ĳ��Ȳ���ȷ���˳�
	if(cPktSize!=6)
	{
		PHYClearTx();
		return FALSE;
	}
	CLR_WDT();
	//����
	return MACTransmitByCSMA();
}

//���ջ������ڣ�����һ�����ȵ�����
void MACDiscardRx(BYTE size)
{
	BYTE i;
	for(i=0;i<size;i++)
	{
		MACGet();
	}	
}

//ȡ��MAC֡
BOOL MACGetPacket(void) 
{
	BYTE i;
	BYTE cNumber=0;
	BYTE frameType;
	CRC_RESULT	crcResult;
	
	i=emSearchMesg(RF_REV_MESSAGE_NOTIFY);
	
	//�Ѿ��յ�������һ�����������ݰ�
	if((i!=InvalidMesg) && !macCurrentRxFrame.bReady)
	{
		emRxMesg(i,NRealTimeMesg,0,0);
		TxBuffer.cSize=0;
		//�����ֽڣ����������ȱ������ǲ������Լ��ӵ�crcУ�飬RSSI��CRC
		macCurrentRxFrame.packetSize=MACGet();
		macCurrentRxFrame.packetSize-=4;
		//ȥ��2���Լ��ӵ�CRCУ��ͣ�2���ֽ�Ӳ���Զ����ϵ�
		TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.packetSize; 
		//���������򣬵��ֽ���ǰ
		macCurrentRxFrame.frameCON.cVal[0]=MACGet();
		macCurrentRxFrame.frameCON.cVal[1]=MACGet();
		TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.frameCON.cVal[0];
		TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.frameCON.cVal[1];
		//ȡ�����к�
		macCurrentRxFrame.sequenceNumber=MACGet();
		TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.sequenceNumber;
		//ȡ��֡����
		frameType=macCurrentRxFrame.frameCON.bits.FrameType;
		cNumber+=3;
		//ι������ֹ����������븴λ
		CLR_WDT(); 

		//����ȷ��֡���д���
		if((frameType==MAC_FRAME_ACK) && (macCurrentRxFrame.packetSize==5))
		{
			macCurrentRxFrame.dstAddress.AddrMode=MAC_DST_SHORT_ADDR;
			macCurrentRxFrame.dstAddress.ShortAddr.cVal[0]=MACGet();
			macCurrentRxFrame.dstAddress.ShortAddr.cVal[1]=MACGet();
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.ShortAddr.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.ShortAddr.cVal[1];
			macCurrentRxFrame.crcRes.cVal[0]=MACGet();
			macCurrentRxFrame.crcRes.cVal[1]=MACGet();
			//����CRCУ���
			crcResult=CrcCalc(TxBuffer.cTxBuffer,TxBuffer.cSize);		
			macCurrentRxFrame.rssi=MACGet();
			macCurrentRxFrame.crc=MACGet();
			if(!MACCheckAddress(&macCurrentRxFrame.dstAddress)) 
			{
				macCurrentRxFrame.bReady=0;
				return FALSE;
			}
			//�ж��Լ����ϵ�CRCУ���
			if(crcResult.nVal==macCurrentRxFrame.crcRes.nVal)
			{
				//�ж�Ӳ����CRCУ����
				if(macCurrentRxFrame.crc & 0x80)
				{
					macCurrentRxFrame.bReady=1;
					return TRUE;
				}
			}

			//У������˳�
			macCurrentRxFrame.bReady=0;
			return FALSE;			
		}
		
		//��������֡������֡���ű�֡
		if((frameType==MAC_FRAME_DATA) || (frameType==MAC_FRAME_CMD) || (frameType==MAC_FRAME_BEACON))
		{
			//��ַģʽ������ڣ�Ҫô�ǳ���ַ��Ҫô�Ƕ̵�ַ����Ŀ�ĵ�ַģʽ�Ƕ̵�ַ
			if(macCurrentRxFrame.frameCON.bits.DstAddrMode==2)
			{
				//ȡ��Ŀ�������ַ�����ֽ���ǰ�����ֽ��ں�
				macCurrentRxFrame.dstAddress.PANId.cVal[0]=MACGet();
				macCurrentRxFrame.dstAddress.PANId.cVal[1]=MACGet();
				//ȡ��Ŀ�ĵ�ַ�����ֽ���ǰ�����ֽ��ں�
				macCurrentRxFrame.dstAddress.AddrMode=MAC_DST_SHORT_ADDR;
				macCurrentRxFrame.dstAddress.ShortAddr.cVal[0]=MACGet();
				macCurrentRxFrame.dstAddress.ShortAddr.cVal[1]=MACGet();
				cNumber+=4;
				TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.PANId.cVal[0];
				TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.PANId.cVal[1];
				TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.ShortAddr.cVal[0];
				TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.ShortAddr.cVal[1];
			}
			else if(macCurrentRxFrame.frameCON.bits.DstAddrMode==3)
			{
				//����ַģʽ�ǳ���ַ
				//ȡ��Ŀ�������ַ�����ֽ���ǰ�����ֽ��ں�
				macCurrentRxFrame.dstAddress.PANId.cVal[0]=MACGet();
				macCurrentRxFrame.dstAddress.PANId.cVal[1]=MACGet();
				//ȡ��Ŀ�ĵ�ַ�����ֽ���ǰ�����ֽ��ں�
				macCurrentRxFrame.dstAddress.AddrMode=MAC_DST_LONG_ADDR;
				macCurrentRxFrame.dstAddress.LongAddr.cVal[0]=MACGet();
				macCurrentRxFrame.dstAddress.LongAddr.cVal[1]=MACGet();
				macCurrentRxFrame.dstAddress.LongAddr.cVal[2]=MACGet();
				macCurrentRxFrame.dstAddress.LongAddr.cVal[3]=MACGet();
				macCurrentRxFrame.dstAddress.LongAddr.cVal[4]=MACGet();
				macCurrentRxFrame.dstAddress.LongAddr.cVal[5]=MACGet();
				macCurrentRxFrame.dstAddress.LongAddr.cVal[6]=MACGet();
				macCurrentRxFrame.dstAddress.LongAddr.cVal[7]=MACGet();
				cNumber+=10;
				TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.PANId.cVal[0];
				TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.PANId.cVal[1];
				memcpy((BYTE *)&TxBuffer.cTxBuffer[TxBuffer.cSize],(BYTE *)&macCurrentRxFrame.dstAddress.LongAddr,sizeof(LONG_ADDR));
				TxBuffer.cSize+=8;
			}
			else if(macCurrentRxFrame.frameCON.bits.DstAddrMode==0)
			{
				//��û��Ŀ�ĵ�ַ��Ϣ�������ű�֡,������������֡
				if(frameType!=MAC_FRAME_BEACON)
				{
					MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
					macCurrentRxFrame.bReady=0;
					return FALSE;
				}
			}
			else 
			{
				//���ǵ�ַģʽ��������������֡
				MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
				macCurrentRxFrame.bReady=0;
				return FALSE;	
			}

			//Դ��ַģʽ���Բ����ڣ���ʱ����Ϊ�ű�����ָ��
			if(macCurrentRxFrame.frameCON.bits.SrcAddrMode)
			{
				//����Դ��ַ��Ŀ�ĵ�ַ����һ�������ڣ�����Դ�����ַ
				if(!macCurrentRxFrame.frameCON.bits.IntraPAN)
				{
					macCurrentRxFrame.srcAddress.PANId.cVal[0]=MACGet();
					macCurrentRxFrame.srcAddress.PANId.cVal[1]=MACGet();
					cNumber+=2;
					TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.srcAddress.PANId.cVal[0];
					TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.srcAddress.PANId.cVal[1];					
				}

				//�жϵ�ַģʽ
				if(macCurrentRxFrame.frameCON.bits.SrcAddrMode==2)
				{
					//���Ƕ̵�ַģʽ��ȡ��Դ��ַ
					macCurrentRxFrame.srcAddress.AddrMode=MAC_SRC_SHORT_ADDR;
					macCurrentRxFrame.srcAddress.ShortAddr.cVal[0]=MACGet();
					macCurrentRxFrame.srcAddress.ShortAddr.cVal[1]=MACGet();
					cNumber+=2;
					TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.srcAddress.ShortAddr.cVal[0];
					TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.srcAddress.ShortAddr.cVal[1];					
				}
				else if(macCurrentRxFrame.frameCON.bits.SrcAddrMode==3)
				{
					//���ǳ���ַģʽ����������ַ
					macCurrentRxFrame.srcAddress.AddrMode=MAC_SRC_LONG_ADDR;
					macCurrentRxFrame.srcAddress.LongAddr.cVal[0]=MACGet();
					macCurrentRxFrame.srcAddress.LongAddr.cVal[1]=MACGet();
					macCurrentRxFrame.srcAddress.LongAddr.cVal[2]=MACGet();
					macCurrentRxFrame.srcAddress.LongAddr.cVal[3]=MACGet();
					macCurrentRxFrame.srcAddress.LongAddr.cVal[4]=MACGet();
					macCurrentRxFrame.srcAddress.LongAddr.cVal[5]=MACGet();
					macCurrentRxFrame.srcAddress.LongAddr.cVal[6]=MACGet();
					macCurrentRxFrame.srcAddress.LongAddr.cVal[7]=MACGet();
					cNumber+=8;
					memcpy((BYTE *)&TxBuffer.cTxBuffer[TxBuffer.cSize],(BYTE *)&macCurrentRxFrame.srcAddress.LongAddr,sizeof(LONG_ADDR));
					TxBuffer.cSize+=8;
				}
				else
				{
					//����Դ��ַģʽ��������������֡
					MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
					macCurrentRxFrame.bReady=0;
					return FALSE;	
				}
				
			}
			else
			{
				//����Դ��ַģʽΪ�գ����Ƿ����ű�����֡,�����˳�
				if(frameType!=MAC_FRAME_CMD)
				{
					MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
					macCurrentRxFrame.bReady=0;
					return FALSE;					
				}	
			}
			CLR_WDT();
			//���Ǹ�MAC֡�ĳ��ȳ������ޣ�����������֡
			if((macCurrentRxFrame.packetSize+4-cNumber)>ConstMacPayloadSize)
			{
				MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
				macCurrentRxFrame.bReady=0;
				return FALSE;	
			}
			
			//����װ�뵽��������֡�ṹ����
			for(i=0;i<macCurrentRxFrame.packetSize-cNumber;i++)
			{
				//pMsduָ��洢��payload�ĳ�ʼ��ַ
				macCurrentRxFrame.pMsdu[i]=MACGet();
				TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.pMsdu[i];
			}
			//ȡ���Լ����ӵ�CRCУ���
			macCurrentRxFrame.crcRes.cVal[0]=MACGet();
			macCurrentRxFrame.crcRes.cVal[1]=MACGet();
			//ȡ��RSSI��CRC���
			macCurrentRxFrame.rssi=MACGet();
			//macCurrentRxFrame.rssi=PHYGetLinkQuality(macCurrentRxFrame.rssi);
			macCurrentRxFrame.crc=MACGet();
			
			//�����ű�֡������Ҫ��ַ��ʶ,������Ҫ���е�ַ��ʶ
			if(frameType!=MAC_FRAME_BEACON)
			{
				if(!MACCheckAddress(&macCurrentRxFrame.dstAddress)) 
				{
					macCurrentRxFrame.bReady=0;
					return FALSE;
				}
			}

			//�����Լ��ӵ�CRCУ���
			crcResult=CrcCalc(TxBuffer.cTxBuffer,TxBuffer.cSize);
			CLR_WDT();
			//CRCУ����
			if((macCurrentRxFrame.crcRes.nVal==crcResult.nVal) && (macCurrentRxFrame.crc & 0x80))
			{
				if(macCurrentRxFrame.frameCON.bits.AckRequest)
				{
					//������Ҫ�ظ�ACk������ȷ��֡
					MACSendACK(macCurrentRxFrame.sequenceNumber);
				}
				//�޸Ľ��ձ�־λ�����س���
				macCurrentRxFrame.bReady=1;
				macCurrentRxFrame.packetSize=macCurrentRxFrame.packetSize-cNumber;
				LEDBlinkRed();
				//ι������ֹ����������븴λ
				CLR_WDT(); 
				return TRUE;
			}
			else
			{
				//CRCУ���������������֡
				macCurrentRxFrame.bReady=0;
				return FALSE;
			}
		}
		else
		{
			//֡���ͳ�������������֡
			MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
			macCurrentRxFrame.bReady=0;
			CLR_WDT(); 
			return FALSE;
		}
	}
	else if((i!=InvalidMesg)&&(macCurrentRxFrame.bReady==1))
	{
		emTxMesg(RF_OVERFLOW_SYS_EROR,RealTimeMesg,0,0);
		return FALSE;
	}
	
	return FALSE;
}

//��װMAC֡ͷ
BOOL MACPutHeader(NODE_INFO *pDestAddr, BYTE frameCON)
{
	BYTE IntraPAN;
	BYTE srcAddrMode,dstAddrMode;
	//��ȡ��ַģʽ
	srcAddrMode=macStatus.bits.addrMode; 
	dstAddrMode=(*pDestAddr).AddrMode;
	//�Ƿ�����һ��������
	IntraPAN=frameCON & MAC_INTRA_PAN_YES;
	//��շ��ͻ�����
	TxBuffer.cSize=0;
	//д�볤��
	TxBuffer.cTxBuffer[TxBuffer.cSize++]=0;
	//д��֡�����򣬵��ֽ���ǰ�����ֽ��ں�
	TxBuffer.cTxBuffer[TxBuffer.cSize++]=frameCON;
	TxBuffer.cTxBuffer[TxBuffer.cSize++]=srcAddrMode|dstAddrMode;
	//д��MAC��������	
    TxBuffer.cTxBuffer[TxBuffer.cSize++]=++macPIB.macDSN;
	CLR_WDT();
	//��Ŀ�ĵ�ַģʽΪ�գ������Ϊ�ű�֡
	if(dstAddrMode)
	{
		//�ж�Ŀ�ĵ�ַģʽ
		if(dstAddrMode==MAC_DST_SHORT_ADDR)
		{
			//�̵�ַģʽ
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).PANId.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).PANId.cVal[1];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).ShortAddr.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).ShortAddr.cVal[1];
			 //���ǹ㲥��ACKǿ�����óɲ�����
			if((*pDestAddr).ShortAddr.nVal==0xFFFF)
				TxBuffer.cTxBuffer[1]&=0xDF;
		}
		else if(dstAddrMode==MAC_DST_LONG_ADDR)
		{
			//����ַģʽ
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).PANId.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).PANId.cVal[1];
		    memcpy((BYTE *)&TxBuffer.cTxBuffer[TxBuffer.cSize],(BYTE *)&(*pDestAddr).LongAddr.cVal[0],8);
			TxBuffer.cSize+=8;
			//���ǹ㲥��ACKǿ�����óɲ�����
			if(((*pDestAddr).LongAddr.byte.dwLsb==0xFFFFFFFF) && ((*pDestAddr).LongAddr.byte.dwMsb==0xFFFFFFFF))
				TxBuffer.cTxBuffer[1]&=0xDF; 
		}
		else
		{
			//�����ַģʽ����ȷ�������ð�
			TxBuffer.cSize=0;
			return FALSE;
		}
	}
	
	//�̵�ַģʽ�����У�Ҳ����û�У������ű�����֡�У�Դ��ַ��Ϣ������
	if(srcAddrMode)
	{
		//��Ŀ�ĵ�ַ��Դ��ַ����һ�������ڣ�����Դ��ַģʽ
		if(!IntraPAN)
		{
		    TxBuffer.cTxBuffer[TxBuffer.cSize++]=macPIB.macPANId.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=macPIB.macPANId.cVal[1];
		}

		//���Ƕ̵�ַģʽ
		if(srcAddrMode==MAC_SRC_SHORT_ADDR)
		{
            switch(Route_flag)
            {
                case 0: //��ʾĿ�ĵ�ַΪ����
                case 2: //��ʾĿ�ĵ�ַΪ��һ��ڵ�
                    TxBuffer.cTxBuffer[TxBuffer.cSize++]=macPIB.macShortAddr.cVal[0];
                    TxBuffer.cTxBuffer[TxBuffer.cSize++]=macPIB.macShortAddr.cVal[1];
                    break;
                case 1: //��ʾʽת���²�ڵ����ݣ�Ŀ�ĵ�ַΪЭ����
                    TxBuffer.cTxBuffer[TxBuffer.cSize++]=Parameter[route_high];
                    TxBuffer.cTxBuffer[TxBuffer.cSize++]=Parameter[route_low];
                    break;
            }
		}
		else if(srcAddrMode==MAC_SRC_LONG_ADDR)
		{
			//���ǳ���ַģʽ
		    memcpy((BYTE *)&TxBuffer.cTxBuffer[TxBuffer.cSize],(BYTE *)&macPIB.macLongAddr.cVal[0],sizeof(LONG_ADDR));
			TxBuffer.cSize+=8;
		}
		else
		{
			//����Դ��ַģʽ����������֡
			TxBuffer.cSize=0;
			return FALSE;
		}
		
	}

	//�޸�MAC֡�ĳ���
	TxBuffer.cTxBuffer[0]=TxBuffer.cSize-1; 
	CLR_WDT();
	return TRUE;
}

//��װMAC֡���ز���
void MACPutTxBuffer(BYTE *ptr,BYTE cSize)
{
	WORD i;
	WORD Number;
	//��ȡ֡ͷ�ĳ���
	Number=TxBuffer.cTxBuffer[0];
	//�ж�֡ͷ��װ�Ƿ���ȷ
	if(Number+1==TxBuffer.cSize)
	{
		for(i=0;i<cSize;i++)
		{
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=*ptr;
			ptr++;
		}
		//�޸ĳ���
		TxBuffer.cTxBuffer[0]=TxBuffer.cSize-1;
		CLR_WDT();
	}
	else
	{
		//����֡ͷ��װ����ȷ���������η���
		TxBuffer.cSize=0;
	}
}

//MAC֡���ݷ���
BOOL MACTransmitPacket(void)
{
	BYTE cPktSize;
	BYTE cNumber;
	CRC_RESULT crcRes;
	WORD prio;
	//��ȡ����
	cNumber=TxBuffer.cSize;
	cPktSize=TxBuffer.cTxBuffer[0];	
	//����������߹�ϵ��һ�£������
	if(cNumber!=cPktSize+1)
	{
		TxBuffer.cSize=0;
		return FALSE;
	}
	emDint(&prio);
	//����CRCУ��
	crcRes=CrcCalc(TxBuffer.cTxBuffer,cPktSize+1);
	//д��CRCУ��
	MACPutTxBuffer(&crcRes.cVal[0],sizeof(CRC_RESULT));
	//��ȡ����
	cNumber=TxBuffer.cSize;
	//д����Ƶ
	PHYPutTxBuffer(TxBuffer.cTxBuffer,TxBuffer.cSize);
	//����д����Ƿ���ȷ
	cPktSize=PHYGetTxNumber();
	//����֡�ṹ�еĳ��Ȳ������ȱ���,�������������ֽڵ�CRCУ���
	if(cPktSize!=cNumber)
	{
   		PHYClearTx();
		emEint(&prio);
		return FALSE;//�������
	}
	//ι������ֹ����������븴λ
	CLR_WDT();
	emEint(&prio);
	//�����ط�����
	MACEnqueTxFrame(); 
	//���ͻ��������
	TxBuffer.cSize=0;
	//����
	return MACTransmitByCSMA();
}

//�ű�֡����
void MACProcessBeacon(void)
{
	BYTE cSize=0;
	PAN_DESCRIPTOR PANDesc;

	//�ű�֡û��Ŀ�ĵ�ַ��Ϣ�����Ժ��Դ���
	//�ж�FFD�ĵ�ַģʽ,Э�����ظ��ĳ�֡�������if,��ΪЭ������Դ��ַģʽ��
	if(macCurrentRxFrame.srcAddress.AddrMode == MAC_SRC_SHORT_ADDR)
	{
		//�̵�ַģʽ
		PANDesc.CoordAddrMode=0;
		//�����ʶ
		PANDesc.CoordPANId.nVal=macCurrentRxFrame.srcAddress.PANId.nVal;
		//�̵�ַ
		PANDesc.CoordShortAddr.nVal=macCurrentRxFrame.srcAddress.ShortAddr.nVal;
	}
	else
	{
		//����ַģʽ
		PANDesc.CoordAddrMode=1;
		//�����ʶ
		PANDesc.CoordPANId.nVal=macCurrentRxFrame.srcAddress.PANId.nVal;
		//����ַ
		memcpy((BYTE *)&PANDesc.CoordLongAddr,(BYTE *)&macCurrentRxFrame.srcAddress.LongAddr,sizeof(LONG_ADDR));		
	}

	//��¼��֡����
	PANDesc.SuperframeSpec.cVal[0]=macCurrentRxFrame.pMsdu[cSize++];
	PANDesc.SuperframeSpec.cVal[1]=macCurrentRxFrame.pMsdu[cSize++];
	CLR_WDT();
	//��λ��ʾЭ�������ھӱ��Ƿ�����
	PANDesc.bits.allowJoin=PANDesc.SuperframeSpec.bits.AssociationPermit;
	
	//�޸���·����
	PANDesc.LinkQuality=PHYGetLinkQuality(macCurrentRxFrame.rssi);
	
	//��Э��������ڵ��Э�����б��У������ж����Ŀǰֻ��Ҫһ����
	MACRefreshPANRecord(&PANDesc);

	//�ͷ���Դ
	macCurrentRxFrame.bReady=0;
	CLR_WDT();
}

void MACFillSourAddr(BYTE *ptr)
{
	*ptr++=macPIB.macPANId.cVal[0];
	*ptr++=macPIB.macPANId.cVal[1];
	*ptr++=macPIB.macShortAddr.cVal[0];
	*ptr++=macPIB.macShortAddr.cVal[1];                                  
}

//MAC������֡����
void MACProcessCommand(void)
{
	BYTE command;
	BYTE flag;
	BYTE i;
	BYTE PramaNum;
	BYTE Channel,TxPower,BaudRate;
	BYTE offset=0;
	BYTE cSize=0;
	NODE_INFO DestShortAddr;
	BYTE cPtrTx[20];
//	WORD Index,prio;
	WORD NodeId;
	BYTE Status;
	SHORT_ADDR shortAddr;
//	ASSOC_REQUEST_CAP CapInfo;
//	NEIGHBOR_RECORD_INFO DeviceInfo;
	BYTE DeviceType;
	command=macCurrentRxFrame.pMsdu[offset++];
	DestShortAddr.PANId.nVal=macPIB.macPANId.nVal;
	DestShortAddr.AddrMode=MAC_DST_SHORT_ADDR;
	DestShortAddr.ShortAddr.cVal[0]=macCurrentRxFrame.srcAddress.ShortAddr.cVal[0];
	DestShortAddr.ShortAddr.cVal[1]=macCurrentRxFrame.srcAddress.ShortAddr.cVal[1];
                   	switch(command)
	{
		case RF_SET_FREQ_REQ:
		 	cSize=0;
		 	cPtrTx[cSize++]=RF_SET_FREQ_RSP;
			Channel=macCurrentRxFrame.pMsdu[offset++];
		 	if((Channel <= ConstRFChannelSize) && (Channel > 0))
			{
				phyPIB.phyCurrentChannel = Channel;
				PHYSetChannel(phyPIB.phyCurrentChannel);
				//PutPHYFreq(Channel);
				cPtrTx[cSize++]=0x01;
			}
			else
			{
				cPtrTx[cSize++]=0x00;
			}
			MACFillSourAddr(&cPtrTx[cSize]);
			cSize+=4;
			cPtrTx[cSize++]=Channel;
		    MACPutHeader(&DestShortAddr,MAC_FRAME_CMD|MAC_ACK_NO);
			MACPutTxBuffer(cPtrTx,cSize);
			MACTransmitPacket();
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_FREQ_RSP:
			flag=macCurrentRxFrame.pMsdu[offset++];
			if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)>=6)
			{
				MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=RF_SET_FREQ_RSP;
				for(i=0;i<5;i++)
				{
					MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
			}
			else if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)<6)
			{
				MACCfgRspSharBuf.bReady=1;
				MACCfgRspSharBuf.Ptr=0;
				MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=RF_SET_FREQ_RSP;
				for(i=0;i<5;i++)
				{
					MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
				emTxMesg(RF_FLUSH_RSPBUFFER_REQ,RealTimeMesg,0,0);
			}
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_POWER_REQ:
			cSize=0;
		 	cPtrTx[cSize++]=RF_SET_POWER_RSP;
			TxPower=macCurrentRxFrame.pMsdu[offset++];
		 	if((TxPower <= ConstRFPowerSize) && (TxPower > 0))
			{
				phyPIB.phyTransmitPower=TxPower;
				PHYSetTxPower(TxPower);
				//PutPHYTxPower(TXPOWERIndex);
				cPtrTx[cSize++]=0x01;
			}	
			else
			{
				cPtrTx[cSize++]=0x00;
			}
			MACFillSourAddr(&cPtrTx[cSize]);
			cSize+=4;
			cPtrTx[cSize++]=TxPower;	
		    MACPutHeader(&DestShortAddr,MAC_FRAME_CMD|MAC_ACK_YES);
			MACPutTxBuffer(cPtrTx,cSize);
			MACTransmitPacket();
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_POWER_RSP:
			flag=macCurrentRxFrame.pMsdu[offset++];
			if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)>=6)
			{
				MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=RF_SET_POWER_RSP;
				for(i=0;i<5;i++)
				{
					MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
			}
			else if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)<6)
			{
				MACCfgRspSharBuf.bReady=1;
				MACCfgRspSharBuf.Ptr=0;
				MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=RF_SET_POWER_RSP;
				for(i=0;i<5;i++)
				{
					MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
				emTxMesg(RF_FLUSH_RSPBUFFER_REQ,RealTimeMesg,0,0);
			}
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_RATE_REQ:
			cSize=0;
		 	cPtrTx[cSize++]=RF_SET_RATE_RSP;
			BaudRate=macCurrentRxFrame.pMsdu[offset++];
			if((BaudRate > 0) && (BaudRate <= ConstRFBaudRateSize))
			{
				phyPIB.phyBaudRate = BaudRate;
				PHYSetBaudRate(BaudRate);
				Delay(1500);
				PHYSetTRxState(RF_TRX_RX);
				PHYSetTRxState(RF_TRX_IDLE);
				PHYSetTRxState(RF_TRX_RX);
				//PutPHYBaudRate(BAUDRATEIndex);
				cPtrTx[cSize++]=0x01;
			}	
			else
			{
				cPtrTx[cSize++]=0x00;
			}
			MACFillSourAddr(&cPtrTx[cSize]);
			cSize+=4;
			cPtrTx[cSize++]=BaudRate;
			
		    MACPutHeader(&DestShortAddr,MAC_FRAME_CMD|MAC_ACK_NO);
			MACPutTxBuffer(cPtrTx,cSize);
			MACTransmitPacket();
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_RATE_RSP:
			flag=macCurrentRxFrame.pMsdu[offset++];
			if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)>=6)
			{
				MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=RF_SET_RATE_RSP;
				for(i=0;i<5;i++)
				{
					MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
			}
			else if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)<6)
			{
				MACCfgRspSharBuf.bReady=1;
				MACCfgRspSharBuf.Ptr=0;
				MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=RF_SET_RATE_RSP;
				for(i=0;i<5;i++)
				{
					MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
				emTxMesg(RF_FLUSH_RSPBUFFER_REQ,RealTimeMesg,0,0);
			}
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_IEEEADDR_REQ:
			cSize=0;
		 	cPtrTx[cSize++]=RF_SET_IEEEADDR_RSP;
		 	PramaNum=macCurrentRxFrame.packetSize-1;
			if(PramaNum==sizeof(LONG_ADDR))
			{
				memcpy(macPIB.macLongAddr.cVal,(BYTE *)&macCurrentRxFrame.pMsdu[offset],PramaNum);
				PutMACAddr();
			//	PutIEEEAddrInfo(IEEE_VALID_CODE, &macPIB.macLongAddr);
				cPtrTx[cSize++]=0x01;
			}	
			else
			{
				cPtrTx[cSize++]=0x00;
			}
			MACFillSourAddr(&cPtrTx[cSize]);
			cSize+=4;
			memcpy(&cPtrTx[cSize],(BYTE *)&macCurrentRxFrame.pMsdu[offset],PramaNum);
			cSize+=PramaNum;
		    MACPutHeader(&DestShortAddr,MAC_FRAME_CMD|MAC_ACK_YES);
			MACPutTxBuffer(cPtrTx,cSize);
			MACTransmitPacket();
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_IEEEADDR_RSP:
			flag=macCurrentRxFrame.pMsdu[offset++];
			PramaNum=macCurrentRxFrame.packetSize-1;
			if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)>=13)
			{
				MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=RF_SET_IEEEADDR_RSP;
				for(i=0;i<PramaNum;i++)
				{
					MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
			}
			else if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)<13)
			{
				MACCfgRspSharBuf.bReady=1;
				MACCfgRspSharBuf.Ptr=0;
				MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=RF_SET_IEEEADDR_RSP;
				for(i=0;i<PramaNum;i++)
				{
					MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
				emTxMesg(RF_FLUSH_RSPBUFFER_REQ,RealTimeMesg,0,0);
			}
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_PANSADDR_REQ:
			cSize=0;
		 	cPtrTx[cSize++]=RF_SET_PANSADDR_RSP;
		 	PramaNum=macCurrentRxFrame.packetSize-1;
			if(PramaNum==sizeof(SHORT_ADDR))
			{
				macPIB.macPANId.cVal[0]=macCurrentRxFrame.pMsdu[offset++];
				macPIB.macPANId.cVal[1]=macCurrentRxFrame.pMsdu[offset++];
				//��ʼ����
				macStatus.bits.isAssociated=1;
				PutMACStatus();
				PutMACAddr();
				cPtrTx[cSize++]=0x01;
			}	
			else
			{
				cPtrTx[cSize++]=0x00;
			}
			MACFillSourAddr(&cPtrTx[cSize]);
			cSize+=4;
			memcpy(&cPtrTx[cSize],(BYTE *)&macCurrentRxFrame.pMsdu[offset],PramaNum);
			cSize+=PramaNum;
		    MACPutHeader(&DestShortAddr,MAC_FRAME_CMD|MAC_ACK_YES);
			MACPutTxBuffer(cPtrTx,cSize);
			MACTransmitPacket();
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_PANSADDR_RSP:
			flag=macCurrentRxFrame.pMsdu[offset++];
			PramaNum=macCurrentRxFrame.packetSize-1;
			if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)>=7)
			{
				MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=RF_SET_PANSADDR_RSP;
				for(i=0;i<PramaNum;i++)
				{
					MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
			}
			else if(flag==1&&(ConstCfgRspBufferSize-MACCfgRspBuf.Ptr)<7)
			{
				MACCfgRspSharBuf.bReady=1;
				MACCfgRspSharBuf.Ptr=0;
				MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=RF_SET_PANSADDR_RSP;
				for(i=0;i<PramaNum;i++)
				{
					MACCfgRspSharBuf.Buffer[MACCfgRspSharBuf.Ptr++]=macCurrentRxFrame.pMsdu[offset++];
				}
				emTxMesg(RF_FLUSH_RSPBUFFER_REQ,RealTimeMesg,0,0);
			}
			macCurrentRxFrame.bReady=0;
			break;
		case RF_SET_SHORTADDR_REQ:
			cSize=0;
			cPtrTx[cSize++]=RTU_SET_SHORTADDR_RSP;
			
			//macCurrentRxFrame.packetSize���ʱ���ʾ����ʵ�Ǹ��صĳ��ȣ�ǰ���Ѿ������������֡ͷ�ĳ����Ѿ�ȥ��
			PramaNum=macCurrentRxFrame.packetSize-1;
			if(PramaNum==sizeof(SHORT_ADDR))
			{
				macPIB.macShortAddr.cVal[0]=macCurrentRxFrame.pMsdu[offset++];
				macPIB.macShortAddr.cVal[1]=macCurrentRxFrame.pMsdu[offset++];
				macStatus.bits.addrMode = MAC_SRC_SHORT_ADDR; //���˶̵�ַģʽ˵���������ö̵�ַͨ����
				PutMACStatus();
				PutMACAddr();
				cPtrTx[cSize++]=0x01;	//1��ʾ��ַ����ɹ�
			}	
			else 
			{
				cPtrTx[cSize++]=0x00;   //0��ʾ��ַû�з���ɹ�
			}
			cPtrTx[cSize++] = macPIB.CDNum;	
			MACFillSourAddr(&cPtrTx[cSize]); //��������������ַ�Ͷ̵�ַ����Ա㷢�͸�Э����
			cSize+=4;

			//�ֿ����������ֽڣ����ʱ���������ֽ�Ӧ��Ϊ�գ��ѵ�����Ϊ������־����
			//memcpy(&cPtrTx[cSize],(BYTE *)&macCurrentRxFrame.pMsdu[offset],PramaNum);
			//cSize+=PramaNum;
		    MACPutHeader(&DestShortAddr,MAC_FRAME_CMD|MAC_INTRA_PAN_YES|MAC_ACK_NO);
			MACPutTxBuffer(cPtrTx,cSize);
			MACTransmitPacket();
			macCurrentRxFrame.bReady=0;
			break;
		case MAC_ASSOC_RESPONSE://�ڵ��ַҪд��FLASH��
			DeviceType=macPIB.DeviceInfo.bits.DeviceType;
			//if((DeviceType==ZIGBEE_RFD || DeviceType==ZIGBEE_ROUTER) && (!macStatus.bits.isAssociated))
            if((DeviceType==ZIGBEE_RFD || DeviceType==ZIGBEE_ROUTER) )
			{
                BYTE inum;
				//����������Լ��ĵ�ַ
				shortAddr.cVal[0]=macCurrentRxFrame.pMsdu[offset++];
				shortAddr.cVal[1]=macCurrentRxFrame.pMsdu[offset++];
				//�������������״̬
				Status=macCurrentRxFrame.pMsdu[offset++];
				//˵����������ɹ������Ҹ�������һ���̵�ַ
				if((Status==MAC_ASSOCIATION_PAN_SUCCESS) && (shortAddr.nVal!=0xFFFF))
				{
					//�ӽ���֡��ȡ�������ʶ
					macPIB.macPANId.nVal=macCurrentRxFrame.srcAddress.PANId.nVal;
					//��¼����ĵ�ַ
					macPIB.macShortAddr.nVal=shortAddr.nVal;
                    memcpy((BYTE *)&macPIB.macLongAddr,(BYTE *)&macCurrentRxFrame.dstAddress.LongAddr,sizeof(LONG_ADDR));
					for(inum=0;inum<8;inum++)
                    {
                        macPIB.macLongAddr.cVal[inum]=macCurrentRxFrame.pMsdu[offset++];
                    }
                    PANDescriptor.CoordPANId.nVal = macCurrentRxFrame.srcAddress.PANId.nVal;
                    PANDescriptor.CoordShortAddr.nVal = macCurrentRxFrame.srcAddress.ShortAddr.nVal;
                    //�洢�̵�ַ
					PutMACAddr();
					//�޸ĵ�ַģʽ	
					macStatus.bits.addrMode=MAC_SRC_SHORT_ADDR;
					//�޸�����״̬
					macStatus.bits.isAssociated=1;	
					//�洢MAC״̬
					PutMACStatus(); 
					//���游�ڵ���Ϣ
					PutCoordDescriptor();
					emTxMesg(NET_ADDR_ACK,RealTimeMesg,0,0); 
				}
				else if((Status==MAC_ASSOCIATION_PAN_SUCCESS) && (shortAddr.nVal==0xFFFF))
				{
					//�����Լ�����Ҫ����̵�ַ�����ó���ַͨ��
					macPIB.macPANId.nVal=macCurrentRxFrame.srcAddress.PANId.nVal;
					//�洢
					//PutMACAddr();
					//�޸ĵ�ַģʽ	
					macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
					//�޸�������־
					macStatus.bits.isAssociated=1;			
					//�洢
					//PutMACStatus(); 
					//���游�ڵ���Ϣ
					//PutCoordDescriptor();
				}
			}
			macCurrentRxFrame.bReady=0;
			break;
		case MAC_ASSOC_CHECK:
			cSize = 0;
			NodeId = macCurrentRxFrame.pMsdu[offset];
			cPtrTx[cSize++]=MAC_CHECK_RSP;
			cPtrTx[cSize++]=NodeId;
			if(NodeId == macPIB.CDNum)
			{
				MACPutHeader(&DestShortAddr,MAC_FRAME_CMD|MAC_ACK_NO);
				MACPutTxBuffer(cPtrTx,cSize);
				MACTransmitPacket();
			}
			macCurrentRxFrame.bReady=0;
			break;
		//�յ���������
		case MAC_DISASSOC_NOTIFY:
			JoinNwkIndction = 0;//��־λ����
			//��ʼ������Э�����б�
			MACFormatPANRecord();
			//�޸ı�־λ
			macStatus.bits.isAssociated=0;
			//��ַģʽҲ�ص�����ַģʽ
			macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
			macPIB.macShortAddr.nVal=0;
			macPIB.macPANId.nVal = 0;
			macCurrentRxFrame.bReady=0;
			break;
		default:
			break;
	}
}

//MACȷ��֡����
void MACProcessAck(void)
{
	BYTE dsn;
	dsn=macCurrentRxFrame.sequenceNumber;
	MACRemoveTxFrame(dsn);
	macCurrentRxFrame.bReady=0;
	CLR_WDT();
}

//MAC��״̬��
void MACProcessPacket(void)
{
	BYTE cNumber;
	BYTE DeviceType;
	BYTE FrameType;
	BYTE i,j;   //����������
    BYTE cSize = 0;
	BYTE cPtrTx[40];
	NODE_INFO macAddr;
	//��ʼ��
	cNumber=0; 
	//���ǽ��յ�һ��MAC֡
	i = MACGetPacket();
	if(i == TRUE)
	{
        //ȡ��MAC֡����
        FrameType=macCurrentRxFrame.frameCON.bits.FrameType;
        if((0 == Route_flag)||(3 == Route_flag)) //  ����������֡�Ǹ����ڵ�Ļ�㲥��
        {
            switch (FrameType)
            {
                //��������֡
                case MAC_FRAME_DATA:
                    DeviceType=macPIB.DeviceInfo.bits.DeviceType;
                    if((DeviceType==ZIGBEE_COORD) && (!macStatus.bits.bEstablishPAN))	
                    {
                        cNumber=0;
                        macCurrentRxFrame.bReady=0;					
                    }
                    else if((DeviceType==ZIGBEE_RFD) && (!macStatus.bits.isAssociated))	
                    {
                        cNumber=0;
                        macCurrentRxFrame.bReady=0;					
                    }
                    else if(DeviceType==0x00)
                    {
                        cNumber=0;
                        macCurrentRxFrame.bReady=0;					
                    }
                    else
                    {	
    //					emTxMesg(RF_UPLOAD_MESSAGE_REQ,RealTimeMesg,&macCurrentRxFrame.pMsdu[0],macCurrentRxFrame.packetSize);
                        MACProcessData();
                    }
                    break;
                //�����ű�֡
                case MAC_FRAME_BEACON:
                    MACProcessBeacon();
                    break;
                //����ȷ��֡
                case MAC_FRAME_ACK:
                    MACProcessAck();
                    break;
                //��������֡
                case MAC_FRAME_CMD:
                    MACProcessCommand();
                    break;
                //������
                default:
                    macCurrentRxFrame.bReady=0;
                    break;	
            }
        }
        else if((1 == Route_flag)||(2 == Route_flag))        //1����Ŀ��ΪЭ������2����ڵ㣻3����㲥
        {
            switch(Route_flag)
            {
                case 1:     //Ŀ�ĵ�ַΪЭ����
                    macAddr.ShortAddr.nVal = macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;;
                    break;
                case 2:     //Э�����������ݣ�ת�����²�ڵ�
                    macAddr.ShortAddr.nVal = ((WORD)Parameter[route_high]<<8&0xff00)|(WORD)Parameter[route_low]&0x00ff;
                    break;
                default:
                    macCurrentRxFrame.bReady=0;
                    return 0;
                    break;
            }
            //·������ת��
            {
                macAddr.AddrMode = MAC_DST_SHORT_ADDR;
                macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
                macPIB.macDSN = macCurrentRxFrame.sequenceNumber;
                
                MACPutHeader(&macAddr,FrameType | MAC_INTRA_PAN_YES | MAC_ACK_NO);

                for(j=0;j<macCurrentRxFrame.packetSize;j++)
                {
                    cPtrTx[cSize++] = macCurrentRxFrame.pMsdu[j];    
                }

                MACPutTxBuffer(cPtrTx,cSize);	
                MACTransmitPacket();
                macCurrentRxFrame.bReady=0;
            }	//�˻�����Ϊ����ת��
        }		
	}
	else
	{
  	Nop();
  	Nop();
    } 	
    Route_flag = 0;
	//�����ط�����
	MACRefreshTxFrame();
	//��ʱ����Э��������
//	MACProccessPANRecordByTimeout();
	CLR_WDT();
}

void MACProcessData(void)
{
    
    BYTE CMDData[3] = {0,0,0};
    BYTE ErrorFlag = 0;
    BYTE TACTICE_NUM,TACTICE_DATA[4];
    BYTE i_Local = 0;
    BYTE i=0;
    BYTE cSize = 0;
	BYTE cPtrTx[40];
	NODE_INFO macAddr;
    
    Type = 	macCurrentRxFrame.pMsdu[0];	
    CoordID[0] = macCurrentRxFrame.pMsdu[1];
    CoordID[1] = macCurrentRxFrame.pMsdu[2];
    if(macCurrentRxFrame.packetSize >=6)
    {
        CMDData[0] = macCurrentRxFrame.pMsdu[3];
        CMDData[1] = macCurrentRxFrame.pMsdu[4];
        CMDData[2] = macCurrentRxFrame.pMsdu[5];
    }
    
    switch(Type)
    {
        case 0x00:
            emTxMesg(LOC_HEART_REQ,RealTimeMesg,0,0);
        break; 
        case 0x01:
            emTxMesg(TEM_SENT_REQ,RealTimeMesg,0,0);
        break;         
        case 0x04:
            emTxMesg(INFRACOLD_SENT_REQ,RealTimeMesg,0,0);
	    break;
	    
	    case 0x02:
            emTxMesg(INFRAHOT_SENT_REQ,RealTimeMesg,0,0);
	    break;
	    
	    case 0x0f:
            emTxMesg(INFRAOFF_SENT_REQ,RealTimeMesg,0,0);
	    break;
        
        case 0x12:
            code_flag=HOT;
	    break;
        
        case 0x14:
            code_flag=COLD;
	    break;
        
        case 0x1f:
            code_flag=OFF;
	    break;
        
        case 0x2f:
            IEC0bits.IC1IE = 0;
            IEC0bits.T3IE = 0;
            break;
        case 0x2e:
            IC1_Init();
            T3_Init();
            break;
        
        case 0x06:
            RESET();//��λ
        break;
        case 0x20://��һ·�̵�����Ȧͨ��
             data = data | 0x0020;
             HC595Put(data);
             HC595Out();
        break;        
        case 0x21://��һ·�̵�����Ȧ�ϵ�
             data = data & 0xffdf;
             HC595Put(data);
             HC595Out();
        break;
        case 0x22://�ڶ�·�̵�����Ȧͨ��
             data = data | 0x0010;
             HC595Put(data);
             HC595Out();
        break;
        case 0x23://�ڶ�·�̵�����Ȧ�ϵ�
             data = data & 0xffef;
             HC595Put(data);
             HC595Out();
        break;
        case 0x24://����·�̵�����Ȧͨ��
             data = data | 0x0008;
             HC595Put(data);
             HC595Out();
        break;
        case 0x25://����·�̵�����Ȧ�ϵ�
             data = data & 0xfff7;
             HC595Put(data);
             HC595Out();
        break;
        case 0x26://����·�̵�����Ȧͨ��
             data = data | 0x0004;
             HC595Put(data);
             HC595Out();
        break; 
        case 0x27://����·�̵�����Ȧ�ϵ�
             data = data & 0xfffb;
             HC595Put(data);
             HC595Out();
        break;        
        case 0xBB:
            if(6 == macCurrentRxFrame.packetSize)
            {
                TACTICE_NUM = CMDData[0];
                TACTICE_DATA[0] = CMDData[1];
                if(TACTICE_NUM<PARAMETER_NUM)
                    i_Local = 1;
                else
                    i_Local = 0;	
                
                {//��ȫ���Ʋ���
                    if((TACTICE_NUM==0x85)&&(TACTICE_DATA[0]==0x92))
                        {data = data | 0xffff;emDelHardTimer(1);emDelHardTimer(3);IEC1bits.T5IE = 0;	}
                    else if((TACTICE_NUM==0x47)&&(TACTICE_DATA[0]==0x31))
                        {data = data & 0x0000;emDelHardTimer(1);emDelHardTimer(3);IEC1bits.T5IE = 0;	}
                    else if((TACTICE_NUM==0x72)&&(TACTICE_DATA[0]==0x64))
                        {data = data | 0xffff;emStartHardTimer(1);emStartHardTimer(3);T5_Init();	}
                    else if((TACTICE_NUM==0x95)&&(TACTICE_DATA[0]==0x48))
                        {data = data & 0x0000;emStartHardTimer(1);emStartHardTimer(3);T5_Init();	}
                }
                    
                if(i_Local == 1)
                {
                    GetAddr(&TacticsAddr);
                    ReadPM(Buffer,SourceAddr);

                    Buffer[TACTICE_NUM]=TACTICE_DATA[0];//����д�뻺����
                    Parameter[TACTICE_NUM]=TACTICE_DATA[0];

                    ErasePage();
                    WritePM(Buffer, SourceAddr);
                    BufferInit();//���Buffer                 
                }
            }
            else
                ErrorFlag = 1;
        break;
        
        case 0xbc:
            macAddr.AddrMode = MAC_DST_SHORT_ADDR;
            macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
            macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
            MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);

            cPtrTx[cSize++]=Type+1;
            cPtrTx[cSize++]=CoordID[0];
            cPtrTx[cSize++]=CoordID[1];
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
            for(i=0;i<PARAMETER_NUM;i++)
            {
                cPtrTx[cSize++] = Parameter[i];
            }
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal;

            MACPutTxBuffer(cPtrTx,cSize);	
            MACTransmitPacket();  
            CoordID[0] = 0;
            CoordID[1] = 0;
        break;

	    case 0x0b:
            if(10 == macCurrentRxFrame.packetSize)
            {
                //Delay(0xffff);
                ServerTime.year   = macCurrentRxFrame.pMsdu[3];
                ServerTime.month  = macCurrentRxFrame.pMsdu[4];	
                ServerTime.day    = macCurrentRxFrame.pMsdu[5];
                ServerTime.hour   = macCurrentRxFrame.pMsdu[6];	
                ServerTime.minute = macCurrentRxFrame.pMsdu[7];
                ServerTime.second = macCurrentRxFrame.pMsdu[8];
                dark = macCurrentRxFrame.pMsdu[9];
                StartmSeconds = GetmSecinds();
                memcpy(&CurrentSysTime,&ServerTime,sizeof(RTIME));//��������ʱ�丳ֵ�����ʱ�̵�ϵͳʱ��

                emTxMesg(TIME_DATA_REQ,RealTimeMesg,0,0);
            }
            else 
                ErrorFlag = 1;
		break;
        
        case 0x0d:

           emTxMesg(TIME_DATA_REQ,RealTimeMesg,0,0);
        break;
        
        case 0xee:      //����������͹㲥�����㲥�Լ��������硢�����Э�����̵�ַ���Լ�macPIB�̵�ַ
            macAddr.AddrMode = MAC_DST_SHORT_ADDR;
            macAddr.PANId.nVal = 0xffff;
            macAddr.ShortAddr.nVal = 0xffff;
            MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);

            cPtrTx[cSize++]=Type+1;
            cPtrTx[cSize++]=CoordID[0];
            cPtrTx[cSize++]=CoordID[1];
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
            cPtrTx[cSize++]=macPIB.macPANId.nVal>>8;
            cPtrTx[cSize++]=macPIB.macPANId.nVal;
            cPtrTx[cSize++]=PANDescriptor.CoordShortAddr.nVal>>8;
            cPtrTx[cSize++]=PANDescriptor.CoordShortAddr.nVal;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal;

            MACPutTxBuffer(cPtrTx,cSize);	
            MACTransmitPacket();  
        break;
        
        case 0x66:
            emTxMesg(LOC_RSSI_REQ,RealTimeMesg,0,0);
        break;  
        
        case 0x68:
            emTxMesg(LOC_DEL_REQ,RealTimeMesg,0,0);
            memset(Loc_Buffer,0,20*sizeof(unsigned char));
            memset(distance_Buffer,0,20*sizeof(unsigned int));
            infrared_correct = 0;
        break;
		case 0x6A:
			if(Loc_Times<20)
			{
                Loc_Buffer[Loc_Times] = macCurrentRxFrame.rssi;
                distance_Buffer[Loc_Times] = register2distance(Loc_Buffer[Loc_Times]) ; 
                Loc_Times++;
            }
		else Loc_Times = 0;
			
		break;
		case 0x6C:
            Loc_Times = 0;
		     emTxMesg(LOC_DIS_REQ,RealTimeMesg,0,0);
        break;   
        
	    default:
	        ErrorFlag = 1;	   		
	    break;
	 }
    
    if(0 == ErrorFlag)
    {
        
            macAddr.AddrMode = MAC_DST_SHORT_ADDR;
            macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
            macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
            MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);
//////////        if( (Type!=0x0c)&&(Type!=0xbc)&&(Type!=0x0d)&&(Type!=0xee)&&(Type!=0x66)&&(Type!=0xbb)&&(Type!=0x03)&&(Type!=0x01)&&(Type!=0x03)&&(Type!=0x6A)&&(Type!=0x6c))
            if(Type!=0x6A)
       {
            cPtrTx[cSize++]=Type+1;
            cPtrTx[cSize++]=CoordID[0];
            cPtrTx[cSize++]=CoordID[1];
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
       }
//////////        else if((0xbb==Type)||(0x03==Type))//��Ϊ������ָ��ĳ��ȱȽϳ�����Ҫר�ŵĻظ���ʽ
//////////        {
//////////            cPtrTx[cSize++]=Type+1;
//////////            cPtrTx[cSize++]=CoordID[0];
//////////            cPtrTx[cSize++]=CoordID[1];
//////////            cPtrTx[cSize++]=CMDData[0];
//////////            cPtrTx[cSize++]=CMDData[1];
//////////            cPtrTx[cSize++]=CMDData[2];
//////////            cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
//////////            cPtrTx[cSize++]=macPIB.macShortAddr.nVal;
//////////            cPtrTx[cSize++]=0x00;
//////////            cPtrTx[cSize++]=0x01;
//////////        }
//////////            
            MACPutTxBuffer(cPtrTx,cSize);	
            MACTransmitPacket();   
            CoordID[0] = 0;
            CoordID[1] = 0;
        
    }
    else if(1 == ErrorFlag)
    {
        macAddr.AddrMode = MAC_DST_SHORT_ADDR;
        macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
        macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
        MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);
//////////        
//////////        if((Type!=0xbb)&&(Type!=0x03))//��Ϊ������ָ��ĳ��ȱȽϳ�����Ҫר�ŵĻظ���ʽ
//////////        {
        cPtrTx[cSize++]=Type+1;
        cPtrTx[cSize++]=CoordID[0];
        cPtrTx[cSize++]=CoordID[1];
        cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
        cPtrTx[cSize++]=macPIB.macShortAddr.nVal;
        cPtrTx[cSize++]=0x00;
        cPtrTx[cSize++]=0x00;
//////////        }
//////////        else
//////////        {
//////////        cPtrTx[cSize++]=Type+1;
//////////        cPtrTx[cSize++]=CoordID[0];
//////////        cPtrTx[cSize++]=CoordID[1];
//////////        cPtrTx[cSize++]=CMDData[0];
//////////        cPtrTx[cSize++]=CMDData[1];
//////////        cPtrTx[cSize++]=CMDData[2];
//////////        cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
//////////        cPtrTx[cSize++]=macPIB.macShortAddr.nVal;
//////////        cPtrTx[cSize++]=0x00;
//////////        cPtrTx[cSize++]=0x00;
//////////        }
        
        MACPutTxBuffer(cPtrTx,cSize);	
        MACTransmitPacket();   
        CoordID[0] = 0;
        CoordID[1] = 0;
        ErrorFlag = 0;
    }
	macCurrentRxFrame.bReady=0;
}

//�ط��������
WORD MACEnqueTxFrame(void)
{
	BYTE i;
	BYTE AckRequest;
	//�ж��Ƿ���ҪACKȷ��
	AckRequest=TxBuffer.cTxBuffer[1] & MAC_ACK_YES;
	if(!AckRequest)
	{
		//���ǲ���ҪACK�����ô洢
		return InValid_Index;
	}

	for(i=0;i<ConstMacTxQueSize;i++)
	{
		if(!TxFrameQue[i].Flags.bits.bInUse)
		{
			memcpy(TxFrameQue[i].cTxBuffer,TxBuffer.cTxBuffer,TxBuffer.cSize);
			//��¼����ʱ�䣬���г�ʱ�ж�
			TxFrameQue[i].dwStartTime=GetTicks();
			//�ط�����
			TxFrameQue[i].cRetries=0;
			//�޸Ĵ洢��־λ
			TxFrameQue[i].Flags.bits.bInUse=1;
			//ι������ֹ����������븴λ
			CLR_WDT(); 
			return i;
		}
	}
	return InValid_Index;
}

//ɾ���ط�������һ����¼
WORD MACRemoveTxFrame(BYTE dsn)
{
	BYTE i;
	BYTE SequenceNumber;
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		//ȡ��MAC֡��DSN
		SequenceNumber=TxFrameQue[i].cTxBuffer[3];
		//����DSN����MAC֡
		if((SequenceNumber==dsn) && (TxFrameQue[i].Flags.bits.bInUse))
		{
			TxFrameQue[i].Flags.bits.bInUse=0;
			//��֡���ȡ���MAC��֡ͷ���������óɿ�
			TxFrameQue[i].cTxBuffer[TxBuffer.cSize++]=0; 
			TxFrameQue[i].cTxBuffer[TxBuffer.cSize++]=0;
			TxFrameQue[i].cTxBuffer[TxBuffer.cSize++]=0;
			TxFrameQue[i].cTxBuffer[TxBuffer.cSize++]=0;
			CLR_WDT();
			return i;
		}
	}
	return InValid_Index;
}

//�����ط���¼
WORD MACSearchTxFrame(BYTE dsn)
{
	BYTE i;
	BYTE SequenceNumber;
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		//ȡ��MAC֡��DSN
		SequenceNumber=TxFrameQue[i].cTxBuffer[3];
		//����DSN����MAC֡
		if((SequenceNumber==dsn) && (TxFrameQue[i].Flags.bits.bInUse))
			return i;
	}
	return InValid_Index;
}

//MAC֡�ط�����ʱ������
void MACRefreshTxFrame(void)
{
	BYTE i;
	BYTE PacketSize;
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		//���ط����������涨ֵ������
		if(TxFrameQue[i].Flags.bits.bInUse)
		{
			if(TxFrameQue[i].cRetries>=aMaxFrameRetries)
				TxFrameQue[i].Flags.bits.bInUse=0;
			else if(DiffTicks(TxFrameQue[i].dwStartTime,GetTicks())>macPIB.macAckWaitDuration)
			{
				//����Զ�ʱ���ڣ�û�н��յ�ACK,�ط�
				//ȡ������
				PacketSize=TxFrameQue[i].cTxBuffer[0];
				//д����Ƶ,�Ѿ�������crcУ��
				PHYPutTxBuffer(TxFrameQue[i].cTxBuffer,PacketSize);
				//����
				MACTransmitByCSMA(); 
				//��¼����ʱ��
				TxFrameQue[i].dwStartTime=GetTicks();
				//�ط�������1
				TxFrameQue[i].cRetries++;
				CLR_WDT(); 
			}
		}
	}
}

//MAC֡�ط�����ͳ��
BYTE MACRecordTxFrame(void)
{
	BYTE i;
	BYTE Number=0;
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		//ͳ���ط����еļ�¼
		if(TxFrameQue[i].Flags.bits.bInUse)
			Number++;
	}
	return Number;
}

/****************************************************
//ͨ����Э�������еĹ�����ѡ��һ���ɿ����������
****************************************************/

//��ʽ��Э�����б�
void MACFormatPANRecord(void)
{
	WORD i;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		CoordQue[i].bits.bInUse=0;
		CoordQue[i].LinkQuality=0;
		CoordQue[i].bits.RxNumber=0;
	}
}

//����һ��Э�����ڵ�
WORD MACAddPANRecord(PAN_DESCRIPTOR *Record)
{
	WORD i;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		if(!CoordQue[i].bits.bInUse)
		{
			CoordQue[i].bits.bInUse=1;
			//���մ���
			CoordQue[i].bits.RxNumber=1;
			//�������
			CoordQue[i].bits.allowJoin=1;
			//��ַģʽ
			CoordQue[i].CoordAddrMode=(*Record).CoordAddrMode;
			//����ֵĬ��Ϊ0
			CoordQue[i].SecurityUse=0;
			CoordQue[i].SecurityFailure=0;
			CoordQue[i].ACLEntry=0;
			//�ŵ�
			CoordQue[i].LogicalChannel=PHYGetChannel();
			//��ַ
			CoordQue[i].CoordPANId.nVal=(*Record).CoordPANId.nVal;
			CoordQue[i].CoordShortAddr.nVal=(*Record).CoordShortAddr.nVal;
			memcpy((BYTE *)&CoordQue[i].CoordLongAddr,(BYTE *)&((*Record).CoordLongAddr),sizeof(LONG_ADDR));
			//��֡����
			CoordQue[i].SuperframeSpec.nVal=(*Record).SuperframeSpec.nVal;
			//ʱ���
			CoordQue[i].TimeStamp=GetTicks();
			//��·����
			CoordQue[i].LinkQuality=(*Record).LinkQuality;
			//�������
			CoordQue[i].NwkDepth=(*Record).NwkDepth;
			CoordQue[i].CoorCDNum=(*Record).NwkDepth;
			CLR_WDT();
			return i;
		}
	}
	return InValid_Index;
}

//����Э�����ڵ�
WORD MACSearchPANRecord(PAN_DESCRIPTOR *Record)
{
	WORD i;
	BYTE Number;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		if(CoordQue[i].bits.bInUse)
		{
			//���Ƕ̵�ַ,0����̵�ַ��1������ַ
			if(!CoordQue[i].CoordAddrMode)
			{
				(*Record).CoordAddrMode=0;
				if((*Record).CoordPANId.nVal == CoordQue[i].CoordPANId.nVal)
				{
					return i;
				}
			}
			else
			{
				//���ǳ���ַ
				(*Record).CoordAddrMode=1;
				Number=memcmp((BYTE *)&CoordQue[i].CoordLongAddr,
						(BYTE *)&((*Record).CoordLongAddr),sizeof(LONG_ADDR));
				if(Number==0)
					return i;
			}
		}
	}
	return InValid_Index;
}

//���������ʾ��ѯЭ�����б�
WORD MACSearchPANRecordByPAN(SHORT_ADDR PANId)
{
	WORD i;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		if(CoordQue[i].bits.bInUse)
		{
			if(CoordQue[i].CoordPANId.nVal==PANId.nVal)
				return i;
		}
	}
	return InValid_Index;	
}

//�޸�Э�����ڵ��ͨ�Ŵ���
void MACRefreshPANRecord(PAN_DESCRIPTOR *Record)
{
	WORD Index;
//	WORD LinkQuality;
//	WORD NwkNumber;
	//��ѯĿǰ�ڵ���б����Ƿ��Ѿ����ڸ�����
	Index=MACSearchPANRecord(Record);
	//������������磬���Э���������б���
	if(Index==InValid_Index)
	{
		//������·�����ж��Ƿ���롣
		if((*Record).LinkQuality >= RssiLevel)
		{
			MACAddPANRecord(Record);
			JoinNwkIndction = 1;
		}
	}
}

//�������������磬���뵽������
WORD MACSearchSuitPANRecord(PAN_DESCRIPTOR *Record)
{
	WORD i;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		if(CoordQue[i].bits.bInUse)
		{
			memcpy((BYTE *)Record,(BYTE *)&CoordQue[i],sizeof(PAN_DESCRIPTOR));
			return i;
		}
	}
	return InValid_Index;
}

//ѡ��һ����õ����磬ѡ�����������·������
WORD MACSearchPrioPANRecord(PAN_DESCRIPTOR *Record)
{
	WORD i;
	WORD MaxCount=0;
	BOOL allowCoord;
	WORD Index=InValid_Index;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		allowCoord=CoordQue[i].SuperframeSpec.bits.AssociationPermit & CoordQue[i].bits.allowJoin;
		if(CoordQue[i].bits.bInUse && allowCoord && CoordQue[i].LinkQuality>MaxCount)
		{
			MaxCount=CoordQue[i].LinkQuality;
			Index=i;
		}
	}
	if((MaxCount>0) && (Index!=InValid_Index))
	{
		memcpy((BYTE *)Record,(BYTE *)&CoordQue[Index],sizeof(PAN_DESCRIPTOR));
		return Index;
	}
	return InValid_Index;
}

//ͳ�����������
WORD MACCountPANRecord(void)
{
	WORD i;
	WORD Number=0;
	BOOL allowCoord;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		allowCoord=CoordQue[i].SuperframeSpec.bits.AssociationPermit & CoordQue[i].bits.allowJoin;
		if(CoordQue[i].bits.bInUse && allowCoord)
		{
			Number++;
		}
	}
	return Number;
}

//��ʱ����
void MACProccessPANRecordByTimeout(void)
{
	WORD i;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		if(CoordQue[i].bits.bInUse)
		{
			if(DiffTicks(CoordQue[i].TimeStamp,GetTicks())>1000)
			{
				CoordQue[i].bits.bInUse=0;
			}
		}
	}
}

//·�������ڵ�������纯��
void MACJoinPAN(void)
{
	WORD Index;
	NODE_INFO Record;
	ASSOC_REQUEST_CAP CapInfo;
	//ֻҪ�յ���Э�����������ű�֡���򲻻��ٷ����ű�����ֱ�Ӳ����Լ��ļ�¼���Լ�������Э������
	if(JoinNwkIndction == 1)
	{
		//�����Լ�������Э����
		Index=MACSearchPrioPANRecord(&PANDescriptor);
		if(Index!=InValid_Index)
		{
			CapInfo.cVal=0;
			//���ܳ�ΪЭ����
			CapInfo.bits.EnableCoord=0; 
			//RFD
			CapInfo.bits.DeviceType=MACGetDeviceType(); 
			//���緽ʽ���� 
			CapInfo.bits.PowerSource=1; 
			//���д��ڽ���״̬
			CapInfo.bits.ReceiveIdle=1;	
			//û��ʹ�ð�ȫģʽ
			CapInfo.bits.EnableSecur=0;
			//Ҫ��Э���������ַ 
			CapInfo.bits.AllocAddress=1; 
			//���������������Ƕ̵�ַ��Ӧ�ý������if
			if(PANDescriptor.CoordAddrMode==0)
			{
				Record.AddrMode=MAC_DST_SHORT_ADDR;
				Record.PANId.nVal=PANDescriptor.CoordPANId.nVal;
				Record.ShortAddr.nVal=PANDescriptor.CoordShortAddr.nVal;
				memcpy((BYTE *)&PANDescriptor,(BYTE *)&PANDescriptor,sizeof(PAN_DESCRIPTOR));
				MACSendAssociationReq(&Record,CapInfo);
			}
			else if(PANDescriptor.CoordAddrMode==1)
			{
				Record.AddrMode=MAC_DST_LONG_ADDR;
				Record.PANId.nVal=PANDescriptor.CoordPANId.nVal;
				Record.PANId.nVal=PANDescriptor.CoordPANId.nVal;
				memcpy((BYTE *)&Record.LongAddr,(BYTE *)&PANDescriptor.CoordLongAddr,sizeof(LONG_ADDR));
				memcpy((BYTE *)&PANDescriptor,(BYTE *)&PANDescriptor,sizeof(PAN_DESCRIPTOR));
				MACSendAssociationReq(&Record,CapInfo);
			}
		}
	}
	else
	{
		if((!macStatus.bits.isAssociated))
		{
			//����Ҫ����һ���ű�����
			MACSendBeaconReq();
		}
	}
}


//�ڵ㷢���ű���������
BOOL MACSendBeaconReq(void)
{
	BYTE cSize=0;
	BYTE cPtrTx[5];
	BYTE addrMode;
	NODE_INFO macAddr;
	//��װMAC֡ͷ
	//Ŀ�ĵ�ַģʽΪ�̵�ַ
	macAddr.AddrMode = MAC_DST_SHORT_ADDR;
	//Ŀ�������ʶΪ�㲥
	macAddr.PANId.nVal=0xFFFF;
	//Ŀ�ĵ�ַΪ�㲥
	macAddr.ShortAddr.nVal=0xFFFF;
	//��ʱ�洢Դ��ַģʽ,MAC���ʼ����ʱ��Ĭ��Ϊ����ַģʽ��
	addrMode=macStatus.bits.addrMode;
	//Դ��ַģʽΪ�գ����ű�����֡�в�����Դ��ַ��Ϣ
	macStatus.bits.addrMode=0;
	MACPutHeader(&macAddr,MAC_FRAME_CMD);
	//�ָ���ַģʽ
	macStatus.bits.addrMode=addrMode;
	//��װ����
	cPtrTx[cSize++]=MAC_BEACON_REQUEST;
	MACPutTxBuffer(cPtrTx,cSize);
	CLR_WDT();
	//����
	return MACTransmitPacket();		
}

/**************************************************************************/
//					�ڵ㷢����������
//Ŀ�ĵ�ַ������PAN��Ŀ�ĵ�ַ�����ű�֡ȷ��
//Դ��ַ��ԴPANΪ�㲥������ַģʽ
//��������Ϊ0��ȷ��֡Ϊ1
/**************************************************************************/

BOOL MACSendAssociationReq(NODE_INFO *macAddr,ASSOC_REQUEST_CAP CapInfo)
{
	BYTE cSize=0;
	BYTE cPtrTx[5];
	SHORT_ADDR PANId;
	//��װMAC֡ͷ
	//��ʱ�洢Դ�����ʶ
	PANId.nVal=macPIB.macPANId.nVal;
	//Դ��ַ�����ʾΪ�㲥
	macPIB.macPANId.nVal=0xFFFF;
	//Ŀ�ĵ�ַģʽ�����ű�֡��ȷ��
	MACPutHeader(macAddr,MAC_FRAME_CMD); //����������������֡
	//�ָ�Դ�����ʶ
	macPIB.macPANId.nVal=PANId.nVal;
	//��װ����
	cPtrTx[cSize++]=MAC_ASSOC_REQUEST;
	cPtrTx[cSize++]=CapInfo.cVal;
	cPtrTx[cSize++]=macPIB.CDNum;
	MACPutTxBuffer(cPtrTx,cSize);
	//����
	return MACTransmitPacket();		
}

/******************************************************************************/
// 					�뿪��������
//Э������RFD����
//Ŀ�ĵ�ַ������ַģʽ�������ʶΪmacPANId
//Դ��ַ������ַģʽ
//����Ϊ0��ACK��Ϊ1��
/******************************************************************************/

BOOL MACSendDisassociationNotify(LONG_ADDR *LongAddr,MAC_DISASSOCIATION_REASON Reason)
{
	BYTE cSize=0;
	BYTE cPtrTx[5];
	BYTE addrMode;
	NODE_INFO macAddr;
	//��װMAC֡ͷ
	//��ַģʽҪ�ó���ַ
	macAddr.AddrMode=MAC_DST_LONG_ADDR;
	macAddr.PANId.nVal=0xFFFF;
	memcpy((BYTE *)&macAddr.LongAddr,(BYTE *)LongAddr,sizeof(LONG_ADDR));
	//��ʱ�洢��ַģʽ
	addrMode=macStatus.bits.addrMode;
	//Դ��ַģʽ��ʱ��Ϊ����ַ
	macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
	MACPutHeader(&macAddr,MAC_FRAME_CMD|MAC_ACK_YES);
	//�ָ�Դ��ַģʽ
	macStatus.bits.addrMode=addrMode;
	//��װ����
	cPtrTx[cSize++]=MAC_DISASSOC_NOTIFY; 
	cPtrTx[cSize++]=Reason; 
	MACPutTxBuffer(cPtrTx,cSize);
	CLR_WDT();
	//����
	return MACTransmitPacket();		
}


void MACFlushTxFrame(void)
{
	macCurrentRxFrame.bReady=0;
}

void MACTask(void)
{
	int i;
	BYTE size;
	BYTE *ptr;
	BYTE **pptr;
	BYTE *psize;
	pptr=&ptr;
	psize=&size;
	BYTE DeviceType;
	NODE_INFO DestShortAddr;
	BYTE frameCON;
	BYTE cSize=0;
	
	i=emWaitMesg(RF_REV_MESSAGE_REQ,RealTimeMesg,0,0);
	if(i==1)
	{
		MACProcessPacket();
	}
	                                  
	i=emWaitMesg(RF_SENT_MESSAGE_REQ,RealTimeMesg,pptr,psize);
	if(i==1)
	{
		DestShortAddr.AddrMode=*ptr++;
		DestShortAddr.PANId.cVal[0]=*ptr++;
		DestShortAddr.PANId.cVal[1]=*ptr++;
		DestShortAddr.ShortAddr.cVal[0]=*ptr++;
		DestShortAddr.ShortAddr.cVal[1]=*ptr++;
		//�������ݰ���
		memcpy((BYTE *)&DestShortAddr.LongAddr.cVal[0],ptr,8);
		ptr+=8;
		size=size-13;
		frameCON=*ptr++;
		size--;
		MACPutHeader(&DestShortAddr,frameCON);
		MACPutTxBuffer(ptr,size);
		MACTransmitPacket();
	}
	
	//MAC�������֮һ������������
	i=emWaitMesg(RF_JOIN_NETWORK_REQ,RealTimeMesg,0,0);
	if(i==1)
	{
		DeviceType=macPIB.DeviceInfo.bits.DeviceType;
		//����豸�����ǽڵ㲢��û������
		if((DeviceType==ZIGBEE_RFD) && (!MACIsNetworkJoined()))
		{
			MACJoinPAN();
		}
	}
	
	i=emWaitMesg(RF_FLUSH_RSPBUFFER_RSP,RealTimeMesg,0,0);
	if(i==1)
	{
		//���������
		MACCfgRspBuf.Ptr=0;
		cSize=0;
		if(MACCfgRspSharBuf.bReady==1)
		{
			for(i=0;i<MACCfgRspSharBuf.Ptr;i++)
			{
				MACCfgRspBuf.Buffer[MACCfgRspBuf.Ptr++]=MACCfgRspSharBuf.Buffer[cSize++];
			}
			MACCfgRspSharBuf.bReady=0;
			MACCfgRspSharBuf.Ptr=0;
		}
	}
	
	MACRefreshTxFrame();
	
	PHYDetectStatus();
	
	CurrentTaskWait();
	SchedTask();
}
