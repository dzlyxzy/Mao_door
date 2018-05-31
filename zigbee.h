#ifndef _ZGB_H
#define _ZGB_H

#include "common.h"


#define macMinBE	2
#define macMaxBE	5
#define macMaxCSMABackoffs 5

//修改日期：2009-4-7
#define ZIGBEE_VERSION	0x02

//网络ID
#define IEEE_PAN_ID    0x0531
#define ZIGBEE_PAN	   4

#define CDrangeLow 0x00
#define CDrangeHigh 0x21


//定义长地址方式：固定值+可变部分，其中固定部分代表区域
/****************************************************/
//1Byte   1Byte   2Byte  2Byte 2Byte
//国家    省份    城市   PANID  Addr
/*****************************************************/
#define AREA_CODE  0x01030001             


//定义PHY层缓冲区大小
#define ConstPhyRxBufferSize  256 //表示长度，而非数组下标

//MAC层宏定义
#define ConstMacTxQueSize    5 		//定义MAC层重发队列大小
#define ConstMacPayloadSize  45 	//定义MAC层PAYLOAD的大小
#define ConstNeighborTableSize 20   //定义邻居表的大小

//网络层宏定义
#define ConstNwkPayloadSize  ConstMacPayloadSize-8
#define ConstNwkMaxResponseTime  20
#define ConstRouteTableSize    10
#define ConstRouteDiscSize		5

//默认的NIB属性
#define DEFAULT_nwkRadius  	  60      
#define DEFAULT_nwkMaxDepth   30
#define DEFAULT_nwkMaxRouters 30
#define DEFAULT_PathCost      40
#define DEFAULT_BRDCastAddress			(0xFFFF)
#define DEFAULT_NULLAddress				(0x0000)

//应用层定义
#define ConstAppPayloadSize  30
#define ConstRtuPayloadSize  36
//定义超帧GTS列表的大小
#define ConstGTSListSize	4     //范围是0-7
//定义挂起地址列表
#define ConstPendingListSize	4 //范围是0-7


//定义节点记录表的大小	
#define ConstRecordTableSize	60



//定义通用数据结构
//短地址数据结构
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
//长地址数据结构
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
//地址信息
//定义PAN地址信息数据结构
typedef SHORT_ADDR PAN_ADDR;

typedef struct _NODE_INFO
{
	BYTE AddrMode;
    PAN_ADDR PANId;
    SHORT_ADDR ShortAddr;
    LONG_ADDR LongAddr;
}NODE_INFO;

//定义设备类型
typedef enum _DEVICE_TYPE
{
	ZIGBEE_COORD=0x01,
	ZIGBEE_ROUTER=0x02,
	ZIGBEE_RFD=0x03,
}DEVICE_TYPE;

//PHY层的PIB属性结构
typedef struct _PHY_PIB			
{
	BYTE phyCurrentChannel;
	BYTE phyChannelSuppoerted;
	BYTE phyTransmitPower;
	BYTE phyCCAMode;
	BYTE phyBaudRate;
}PHY_PIB;

//MAC层的设备信息
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


//MAC层的PIB属性
typedef struct _MAC_PIB
{
	PAN_ADDR  macPANId;//存储网络标识
	SHORT_ADDR macShortAddr;//存储短地址
	LONG_ADDR macLongAddr;//存储长地址

	SHORT_ADDR macCoordShortAddr;
	LONG_ADDR macCoordLongAddr;

	BYTE macDSN;
//	BYTE macMaxCSMABackoffs;//CSMA-CA机制的退避时间
	BYTE macAckWaitDuration;//等待ACK确认的时间
//	BYTE macMinBE;
	BYTE macGTSPermit;
    DEVICE_INFO DeviceInfo;
    BYTE CDNum;
}MAC_PIB;


//MAC层的状态
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

//超帧说明
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

//网络描述,普通节点使用
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

//整个结构体设置成偶数字节，方便使用FLASH操作
typedef struct _NEIGHBOR_RECORD
{
    LONG_ADDR                   LongAddr;
    SHORT_ADDR                  ShortAddr;
    SHORT_ADDR                  PANId;
    NEIGHBOR_RECORD_INFO 		DeviceInfo;
    BYTE CdNum;
} NEIGHBOR_RECORD;  //


#endif

