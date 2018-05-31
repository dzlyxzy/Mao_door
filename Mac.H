/************************************************************
�ļ����������ļ���Ҫ������ʵ�����޴����������MAC��Ļ������ܣ�
					�������ݰ��Ľ��������ݰ��ķ��ͣ�����Ĵ�����
�汾��Ϣ��v7.0
�޸�ʱ�䣺2017/03/
*************************************************************/

#ifndef _MAC_H
#define _MAC_H

#include "zigbee.h"
#include "tick.h"
#include "mcu.h"
#include "Interface.h"
#include "phy.h"
#include "addr.h"
#include "Nvm_Flash.h"

extern BYTE CoordID[2];
extern BYTE Route_flag; //0��ʾ���أ�1��ʾЭ������2��ʾת��Ŀ�Ľڵ㣻3��ʾ�㲥

//���巢��MAC���ĳ���
#define ConstMacPacketSize	(ConstMacPayloadSize+20)
#define ConstMacBufferSize	20

//MAC Constant
#define aMaxFrameRetries  3 //�ط�����
#define aMaxMACFrameSize  64 //MAC֡����󳤶�
#define aResponseWaitTime 10 //����������Ӧʱ��


//����CSMA���Ƶĳ���
#define aUnitBackoffPeriod 1
#define aMaxBE 5


//����MAC��Ļ�������
#define MAC_ASSOC_REQUEST 					0x01
#define MAC_ASSOC_RESPONSE 					0x02
#define MAC_DISASSOC_NOTIFY		 			0x03
#define MAC_DATA_REQUEST 					0x04
#define MAC_PANCONFLICT_NOTIFY				0x05
#define MAC_ORPHAN_NOTIFY 					0x06
#define MAC_BEACON_REQUEST 					0x07
#define MAC_COORD_REALIGN 					0x08
#define MAC_ASSOC_CHECK                     0x0A
#define MAC_CHECK_RSP                       0x0B
#define MAC_NEIGH_NOTIFY					0x40



//����֡����
#define MAC_FRAME_TYPE_MASK         (0x03)
#define MAC_FRAME_BEACON            (0x00)
#define MAC_FRAME_DATA              (0x01)
#define MAC_FRAME_ACK               (0x02)
#define MAC_FRAME_CMD               (0x03)

#define MAC_SECURITY_YES            (0x08)
#define MAC_SECURITY_NO             (0x00)

#define MAC_FRAME_PENDING_YES       (0x10)
#define MAC_FRAME_PENDING_NO        (0x00)

#define MAC_ACK_YES                 (0x20)
#define MAC_ACK_NO                  (0x00)

#define MAC_INTRA_PAN_YES           (0x40)
#define MAC_INTRA_PAN_NO            (0x00)

#define MAC_BEACON_YES              (0x40)
#define MAC_BEACON_NO               (0x00)

//��ַģʽ�ĺ궨��
#define MAC_DST_NO_ADDR             (0x00)
#define MAC_DST_SHORT_ADDR          (0x08)
#define MAC_DST_LONG_ADDR           (0x0c)
#define MAC_DST_ADDR_RESERVED       (0x04)

#define MAC_SRC_NO_ADDR             (0x00)
#define MAC_SRC_SHORT_ADDR          (0x80)
#define MAC_SRC_LONG_ADDR           (0xc0)
#define MAC_SRC_ADDR_RESERVED       (0x40)


//MAC֡ͷ�Ŀ����򲿷�
typedef union _MAC_FRAME_CONTROL
{
   	struct
   	{
	    WORD FrameType : 3;
	    WORD SecurityEnabled : 1;
	    WORD FramePending : 1;
	    WORD AckRequest : 1;
	    WORD IntraPAN : 1;//1��ʾ��һ������
	    WORD :1;
	    WORD :1;
	    WORD :1;
	    WORD DstAddrMode :2;//10�����̵�ַ��11��������ַ
	    WORD :1;
	    WORD :1;
	    WORD SrcAddrMode :2;
	  }bits;
	 WORD nVal;
	 BYTE cVal[2];
}MAC_FRAME_CONTROL;


typedef union _ASSOC_REQUEST_CAP
{
	BYTE cVal;
	struct
	{
		BYTE EnableCoord:1;	//�Ƿ���ܳ�ΪЭ����
		BYTE DeviceType:2;	//��FFD ����RFD
		BYTE PowerSource:1; //���緽ʽ
		BYTE ReceiveIdle:1;	//�Ƿ���һֱ���ڽ���״̬
		BYTE :1;
		BYTE EnableSecur:1; //�Ƿ�ȫ
		BYTE AllocAddress:1;//Ҫ�����һ���̵�ַ
	}bits;
}ASSOC_REQUEST_CAP;

typedef enum _MAC_ASSOCIATION_RESPONSE_STATUS
{
	MAC_ASSOCIATION_PAN_SUCCESS,	//����ɹ�
	MAC_ASSOCIATION_PAN_EXITST,	//�������Ѿ���һ�������Ķ̵�ַ����Ҫ��Ӧ���벻��Ҫ����̵�ַ�ĵط�
	MAC_ASSOCIATION_PAN_REFUSE	//�ܾ���������
}MAC_ASSOCIATION_RESPONSE_STATUS;


typedef enum _MAC_DISASSOCIATION_REASON
{
    MAC_COORDINATOR_FORCED_LEAVE=0x01,
    MAC_DEVICE_LEAVE=0x02
} MAC_DISASSOCIATION_REASON;


//��������֡�ṹ
typedef struct _MAC_RX_FRAME
{
	BYTE packetSize;
	MAC_FRAME_CONTROL frameCON;
	BYTE sequenceNumber;
	NODE_INFO srcAddress;
	NODE_INFO dstAddress;
	BYTE pMsdu[ConstMacPayloadSize];
	CRC_RESULT crcRes;
	BYTE rssi;
	BYTE crc;
	BOOL bReady;
}MAC_RX_FRAME;



//��������֡�ṹ
typedef struct _MAC_TX_FRAME
{
	BYTE packetSize;
	MAC_FRAME_CONTROL frameCON;
	BYTE sequenceNumber;
	NODE_INFO srcAddress;
	NODE_INFO dstAddress;
	union
	{
		BYTE cVal;
		struct
		{
			BYTE bFinishHeader:1;
			BYTE bReadyTransmit:1;
			BYTE bTransmitOver:1;
			BYTE :5;	
		}bits;
	}Flags;
}MAC_TX_FRAME;


typedef struct _MAC_TX_QUEUE
{
	BYTE cTxBuffer[ConstMacPacketSize];//�洢����
	DWORD dwStartTime;
	BYTE cRetries;//�ط�����
	union
	{
		BYTE cVal;
		struct
		{
			BYTE bInUse:1;
			BYTE :7;	
		}bits;
	}Flags;
}MAC_TX_QUEUE;

typedef struct _MAC_TX_BUFFER
{
	BYTE cTxBuffer[ConstMacPacketSize];
	BYTE cSize;
}MAC_TX_BUFFER;

//�ھӽڵ�֮���ϵ�ṹ��
typedef enum _RELATIONSHIP_TYPE
{
    NEIGHBOR_IS_PARENT = 0x00,//���ڵ�
    NEIGHBOR_IS_CHILD  = 0x01,//�ӽڵ�
    NEIGHBOR_IS_SIBLING= 0x02,//�ԵȽڵ�
    NEIGHBOR_IS_NONE   = 0x03,//��ͨ�ڵ�
} RELATIONSHIP_TYPE;

//MAC��ĳ�ʼ��
void MACInitSetup(void);
//��ַ��Ϣ�ĳ�ʼ��
void MACInitIEEEAddr(void);
//���õ�ַ��Ϣ������������Ϣ��״̬��Ϣ
void MACSetAddrInfo(void);
//��ȡ��ַ��Ϣ������������Ϣ��״̬��Ϣ
void MACGetAddrInfo(void);
//��ַ��ʶ
BOOL MACCheckAddress(NODE_INFO *pAddr);
//��װ��ͷ
BOOL MACPutHeader(NODE_INFO *pDestAddr, BYTE frameCON);
//д�뷢�ͻ�����
void MACPutTxBuffer(BYTE *ptr,BYTE cSize);
//��������
BOOL MACTransmitPacket(void);

//��ȡһ�����������ݰ�
BOOL MACGetPacket(void);
//����һ�����ȵ�����
void MACDiscardRx(BYTE size);
//�����ű�֡
void MACProcessBeacon(void);
//��������֡
void MACProcessCommand(void);
//����ȷ��֡
void MACProcessAck(void);
//��װ��ַ��Ϣ
void MACFillSourAddr(BYTE *ptr);

//����ȷ��֡
BOOL MACSendACK(BYTE dsn);
//����֡��ʱ�洢
WORD MACEnqueTxFrame(void);
//�����ʱ���Ͷ�����һ������֡
WORD MACRemoveTxFrame(BYTE dsn);
//����ʱ���Ͷ������ҵ�һ������֡
WORD MACSearchTxFrame(BYTE dsn);
//�ط�����
void MACRefreshTxFrame(void);
//ͳ����ʱ���Ͷ���������֡����
BYTE MACRecordTxFrame(void);
/****************************************************
//ͨ����Э�������еĹ�������ѡ��һ���ɿ����������
****************************************************/
//��ʽ���б�
void MACFormatPANRecord(void);
//����һ��Э�����ڵ�
WORD MACAddPANRecord(PAN_DESCRIPTOR *Record);
//����Э�����ڵ�
WORD MACSearchPANRecord(PAN_DESCRIPTOR *Record);
//�޸�Э�����ڵ��ͨ�Ŵ���
void MACRefreshPANRecord(PAN_DESCRIPTOR *Record);
//ѡ��һ����õ�����
WORD MACSearchPrioPANRecord(PAN_DESCRIPTOR *Record);
//���������ַ��ѯ
WORD MACSearchPANRecordByPAN(SHORT_ADDR PANId);
//ͳ�����������
WORD MACCountPANRecord(void);
//��ʱ����
void MACProccessPANRecordByTimeout(void);

//��������
void MACEstablishPAN(void);
//��������
void MACJoinPAN(void);

//����Ƶ���ջ������Ĳ���
#define MACGet()  PHYGet() 
#define MACGetArray(ptr,cSize) PHYGetArray(ptr,cSize) 
//��Ƶ����
#define MACTransmitByCSMA()	   PHYTranmitByCSMA()

//�ű�����
BOOL MACSendBeaconReq(void); 
//��������
BOOL MACSendAssociationReq(NODE_INFO *macAddr,ASSOC_REQUEST_CAP CapInfo);
//�뿪����
BOOL MACSendDisassociationNotify(LONG_ADDR *LongAddr,MAC_DISASSOCIATION_REASON Reason);

//API�ӿ�
#define MACGetDeviceType()				(macPIB.DeviceInfo.bits.DeviceType)
#define MACSetDeviceType(DeviceType)	(macPIB.DeviceInfo.bits.DeviceType=DeviceType)
#define MACIsNetworkEstablished()   (macStatus.bits.bEstablishPAN)
#define MACIsNetworkJoined()		(macStatus.bits.isAssociated)

#define MACGetAddrMode()			(macStatus.bits.addrMode)
#define MACGetPANId()				(macPIB.macPANId)	
#define MACGetShortAddr()			(macPIB.macShortAddr)
#define MACGetLongAddr()			(macPIB.macLongAddr)

#define MACSetAddrMode(v)			(macStatus.bits.addrMode=v)	
#define MACSetPANId(v)				(macPIB.macPANId.nVal=v.nVal)
#define MACSetShortAddr(v)			(macPIB.macShortAddr.nVal=v.nVal)
#define MACSetLongAddr(v)			(memcpy((BYTE *)&macPIB.macLongAddr,(BYTE *)&v,sizeof(LONG_ADDR)))

#define MACGetCoordAddrMode()		(PANDescriptor.CoordAddrMode)
#define MACGetCoordPANId()			(PANDescriptor.CoordPANId)
#define MACGetCoordShortAddr()		(PANDescriptor.CoordShortAddr)
#define MACGetCoordLongAddr()		(PANDescriptor.CoordLongAddr)

#define MACSetCoordAddrMode(v)		(PANDescriptor.CoordAddrMode=v)
#define MACSetCoordPANId(v)			(PANDescriptor.CoordPANId.nVal=v.nVal)
#define MACSetCoordShortAddr(v)		(PANDescriptor.CoordShortAddr.nVal=v.nVal)
#define MACSetCoordLongAddr(v)		(memcpy((BYTE *)&PANDescriptor.CoordLongAddr,(BYTE *)&v,sizeof(LONG_ADDR)))

void MACFlushTxFrame(void);
void MACTask(void);
void MACProcessData(void);

extern RTIME CurrentSysTime;
extern DWORD mSeconds;
extern RTIME CurrentSysTime;
extern RTIME ServerTime;
extern TICK StartmSeconds;
extern TICK GetmSecinds(void);
extern PAN_DESCRIPTOR PANDescriptor;
extern MAC_PIB macPIB;					//���ڼ�¼mac���PIB����
extern MAC_STATUS macStatus;			//��¼MAC״̬
extern MAC_RX_FRAME macCurrentRxFrame;  //���հ���֡��ʽ
#endif