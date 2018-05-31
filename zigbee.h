#ifndef _ZGB_H
#define _ZGB_H

#include "common.h"


#define macMinBE	2
#define macMaxBE	5
#define macMaxCSMABackoffs 5

//�޸����ڣ�2009-4-7
#define ZIGBEE_VERSION	0x02

//����ID
#define IEEE_PAN_ID    0x0531
#define ZIGBEE_PAN	   4

#define CDrangeLow 0x00
#define CDrangeHigh 0x21


//���峤��ַ��ʽ���̶�ֵ+�ɱ䲿�֣����й̶����ִ�������
/****************************************************/
//1Byte   1Byte   2Byte  2Byte 2Byte
//����    ʡ��    ����   PANID  Addr
/*****************************************************/
#define AREA_CODE  0x01030001             


//����PHY�㻺������С
#define ConstPhyRxBufferSize  256 //��ʾ���ȣ����������±�

//MAC��궨��
#define ConstMacTxQueSize    5 		//����MAC���ط����д�С
#define ConstMacPayloadSize  45 	//����MAC��PAYLOAD�Ĵ�С
#define ConstNeighborTableSize 20   //�����ھӱ�Ĵ�С

//�����궨��
#define ConstNwkPayloadSize  ConstMacPayloadSize-8
#define ConstNwkMaxResponseTime  20
#define ConstRouteTableSize    10
#define ConstRouteDiscSize		5

//Ĭ�ϵ�NIB����
#define DEFAULT_nwkRadius  	  60      
#define DEFAULT_nwkMaxDepth   30
#define DEFAULT_nwkMaxRouters 30
#define DEFAULT_PathCost      40
#define DEFAULT_BRDCastAddress			(0xFFFF)
#define DEFAULT_NULLAddress				(0x0000)

//Ӧ�ò㶨��
#define ConstAppPayloadSize  30
#define ConstRtuPayloadSize  36
//���峬֡GTS�б�Ĵ�С
#define ConstGTSListSize	4     //��Χ��0-7
//��������ַ�б�
#define ConstPendingListSize	4 //��Χ��0-7


//����ڵ��¼��Ĵ�С	
#define ConstRecordTableSize	60



//����ͨ�����ݽṹ
//�̵�ַ���ݽṹ
typedef union _SHORT_ADDR
{
    struct
    {
        BYTE cLsb;
        BYTE cMsb;
    }byte;
    WORD nVal;
    BYTE cVal[2];
}SHORT_ADDR;
//����ַ���ݽṹ
typedef union _LONG_ADDR
{
    struct 
    {
        DWORD dwLsb;
        DWORD dwMsb;
    }byte;
    WORD nVal[4];
    BYTE cVal[8];
} LONG_ADDR;
//��ַ��Ϣ
//����PAN��ַ��Ϣ���ݽṹ
typedef SHORT_ADDR PAN_ADDR;

typedef struct _NODE_INFO
{
	BYTE AddrMode;
    PAN_ADDR PANId;
    SHORT_ADDR ShortAddr;
    LONG_ADDR LongAddr;
}NODE_INFO;

//�����豸����
typedef enum _DEVICE_TYPE
{
	ZIGBEE_COORD=0x01,
	ZIGBEE_ROUTER=0x02,
	ZIGBEE_RFD=0x03,
}DEVICE_TYPE;

//PHY���PIB���Խṹ
typedef struct _PHY_PIB			
{
	BYTE phyCurrentChannel;
	BYTE phyChannelSuppoerted;
	BYTE phyTransmitPower;
	BYTE phyCCAMode;
	BYTE phyBaudRate;
}PHY_PIB;

//MAC����豸��Ϣ
typedef union _DEVICE_INFO
{
	struct
    {
        WORD StackProfile:4;    // Needed for network discovery
        WORD ZigBeeVersion:4;    // Needed for network discovery
        WORD DeviceType:2;
        WORD RxOnWhenIdle:1;
        WORD PermitJoin:1;
		WORD AutoRequest;
		WORD PromiscuousMode:1;
		WORD BattLifeExtPeriods:1;
		WORD PotentialParent:1;
    } bits;
    WORD nVal;
}DEVICE_INFO;


//MAC���PIB����
typedef struct _MAC_PIB
{
	PAN_ADDR  macPANId;//�洢�����ʶ
	SHORT_ADDR macShortAddr;//�洢�̵�ַ
	LONG_ADDR macLongAddr;//�洢����ַ

	SHORT_ADDR macCoordShortAddr;
	LONG_ADDR macCoordLongAddr;

	BYTE macDSN;
//	BYTE macMaxCSMABackoffs;//CSMA-CA���Ƶ��˱�ʱ��
	BYTE macAckWaitDuration;//�ȴ�ACKȷ�ϵ�ʱ��
//	BYTE macMinBE;
	BYTE macGTSPermit;
    DEVICE_INFO DeviceInfo;
    BYTE CDNum;
}MAC_PIB;


//MAC���״̬
typedef union _MAC_STATUS
{
	WORD nVal;
	struct
	{
		WORD addrMode:8;
		WORD bEstablishPAN:1;
		WORD isAssociated:1;
		WORD allowBeacon:1;
		WORD nwkDepth:5;
	}bits;
}MAC_STATUS;

//��֡˵��
typedef union _SUPERFRAME_SPEC
{
    WORD nVal;
	BYTE cVal[2];
    struct 
    {
        WORD BeaconOrder:4;
        WORD SuperframeOrder:4;
        WORD FinalCAPSlot:4;
        WORD BatteryLifeExtension:1;
        WORD :1;
        WORD PANCoordinator:1;
        WORD AssociationPermit:1;
    } bits;
} SUPERFRAME_SPEC;

//��������,��ͨ�ڵ�ʹ��
typedef struct _PAN_DESCRIPTOR
{
    BYTE  CoordAddrMode   : 1;    // spec uses 0x02 and 0x03, we'll use 0 and 1 (short/long)
    BYTE  GTSPermit       : 1;
    BYTE  SecurityUse     : 1;
    BYTE  SecurityFailure : 1;
    BYTE  ACLEntry        : 4;

    BYTE  LogicalChannel;
    SHORT_ADDR    CoordPANId;
    SHORT_ADDR    CoordShortAddr;
	LONG_ADDR	  CoordLongAddr;
    SUPERFRAME_SPEC    SuperframeSpec;
    DWORD         TimeStamp;
    BYTE          LinkQuality;
	BYTE 		  NwkDepth;
	struct
	{
		WORD RxNumber:14;
		WORD allowJoin:1;
		WORD bInUse:1;
	}bits;
	BYTE CoorCDNum;
} PAN_DESCRIPTOR;


typedef union _NEIGHBOR_RECORD_INFO
{
    struct
    {
        BYTE LQI                : 8;
        BYTE Depth              : 4;
        BYTE StackProfile       : 4;    // Needed for network discovery
        BYTE ZigBeeVersion      : 4;    // Needed for network discovery
        BYTE DeviceType         : 2;
        BYTE Relationship       : 2;
        BYTE RxOnWhenIdle       : 1;
        BYTE bInUse             : 1;
        BYTE PermitJoining      : 1;
        BYTE PotentialParent    : 1;
    } bits;
    DWORD dwVal;
} NEIGHBOR_RECORD_INFO;

//�����ṹ�����ó�ż���ֽڣ�����ʹ��FLASH����
typedef struct _NEIGHBOR_RECORD
{
    LONG_ADDR                   LongAddr;
    SHORT_ADDR                  ShortAddr;
    SHORT_ADDR                  PANId;
    NEIGHBOR_RECORD_INFO 		DeviceInfo;
    BYTE CdNum;
} NEIGHBOR_RECORD;  //


#endif

