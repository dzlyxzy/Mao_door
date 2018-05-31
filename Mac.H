/************************************************************
文件描述：本文件主要是用来实现无限传感器网络的MAC层的基本功能，
					包括数据包的解析，数据包的发送，命令的处理等
版本信息：v7.0
修改时间：2017/03/
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
extern BYTE Route_flag; //0表示本地；1表示协调器；2表示转发目的节点；3表示广播

//定义发送MAC包的长度
#define ConstMacPacketSize	(ConstMacPayloadSize+20)
#define ConstMacBufferSize	20

//MAC Constant
#define aMaxFrameRetries  3 //重发次数
#define aMaxMACFrameSize  64 //MAC帧的最大长度
#define aResponseWaitTime 10 //命令的最大响应时间


//用于CSMA机制的常量
#define aUnitBackoffPeriod 1
#define aMaxBE 5


//定义MAC层的基本命令
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



//定义帧类型
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

//地址模式的宏定义
#define MAC_DST_NO_ADDR             (0x00)
#define MAC_DST_SHORT_ADDR          (0x08)
#define MAC_DST_LONG_ADDR           (0x0c)
#define MAC_DST_ADDR_RESERVED       (0x04)

#define MAC_SRC_NO_ADDR             (0x00)
#define MAC_SRC_SHORT_ADDR          (0x80)
#define MAC_SRC_LONG_ADDR           (0xc0)
#define MAC_SRC_ADDR_RESERVED       (0x40)


//MAC帧头的控制域部分
typedef union _MAC_FRAME_CONTROL
{
   	struct
   	{
	    WORD FrameType : 3;
	    WORD SecurityEnabled : 1;
	    WORD FramePending : 1;
	    WORD AckRequest : 1;
	    WORD IntraPAN : 1;//1表示在一个网内
	    WORD :1;
	    WORD :1;
	    WORD :1;
	    WORD DstAddrMode :2;//10代表短地址，11代表长地址
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
		BYTE EnableCoord:1;	//是否可能成为协调器
		BYTE DeviceType:2;	//是FFD 还是RFD
		BYTE PowerSource:1; //供电方式
		BYTE ReceiveIdle:1;	//是否是一直处于接收状态
		BYTE :1;
		BYTE EnableSecur:1; //是否安全
		BYTE AllocAddress:1;//要求分配一个短地址
	}bits;
}ASSOC_REQUEST_CAP;

typedef enum _MAC_ASSOCIATION_RESPONSE_STATUS
{
	MAC_ASSOCIATION_PAN_SUCCESS,	//分配成功
	MAC_ASSOCIATION_PAN_EXITST,	//网络中已经有一个这样的短地址，主要是应用与不需要分配短地址的地方
	MAC_ASSOCIATION_PAN_REFUSE	//拒绝加入网络
}MAC_ASSOCIATION_RESPONSE_STATUS;


typedef enum _MAC_DISASSOCIATION_REASON
{
    MAC_COORDINATOR_FORCED_LEAVE=0x01,
    MAC_DEVICE_LEAVE=0x02
} MAC_DISASSOCIATION_REASON;


//接收数据帧结构
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



//发送数据帧结构
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
	BYTE cTxBuffer[ConstMacPacketSize];//存储数据
	DWORD dwStartTime;
	BYTE cRetries;//重发次数
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

//邻居节点之间关系结构体
typedef enum _RELATIONSHIP_TYPE
{
    NEIGHBOR_IS_PARENT = 0x00,//父节点
    NEIGHBOR_IS_CHILD  = 0x01,//子节点
    NEIGHBOR_IS_SIBLING= 0x02,//对等节点
    NEIGHBOR_IS_NONE   = 0x03,//普通节点
} RELATIONSHIP_TYPE;

//MAC层的初始化
void MACInitSetup(void);
//地址信息的初始化
void MACInitIEEEAddr(void);
//设置地址信息，网络描述信息，状态信息
void MACSetAddrInfo(void);
//读取地址信息，网络描述信息，状态信息
void MACGetAddrInfo(void);
//地址辨识
BOOL MACCheckAddress(NODE_INFO *pAddr);
//封装包头
BOOL MACPutHeader(NODE_INFO *pDestAddr, BYTE frameCON);
//写入发送缓冲区
void MACPutTxBuffer(BYTE *ptr,BYTE cSize);
//发送数据
BOOL MACTransmitPacket(void);

//读取一个完整的数据包
BOOL MACGetPacket(void);
//丢弃一定长度的数据
void MACDiscardRx(BYTE size);
//处理信标帧
void MACProcessBeacon(void);
//处理命令帧
void MACProcessCommand(void);
//处理确认帧
void MACProcessAck(void);
//封装地址信息
void MACFillSourAddr(BYTE *ptr);

//发送确认帧
BOOL MACSendACK(BYTE dsn);
//发送帧临时存储
WORD MACEnqueTxFrame(void);
//清除临时发送队列里一个数据帧
WORD MACRemoveTxFrame(BYTE dsn);
//从临时发送队列里找到一个数据帧
WORD MACSearchTxFrame(BYTE dsn);
//重发处理
void MACRefreshTxFrame(void);
//统计临时发送队列中数据帧个数
BYTE MACRecordTxFrame(void);
/****************************************************
//通过对协调器队列的管理，来选择一个可靠的网络加入
****************************************************/
//格式化列表
void MACFormatPANRecord(void);
//增加一个协调器节点
WORD MACAddPANRecord(PAN_DESCRIPTOR *Record);
//查找协调器节点
WORD MACSearchPANRecord(PAN_DESCRIPTOR *Record);
//修改协调器节点的通信次数
void MACRefreshPANRecord(PAN_DESCRIPTOR *Record);
//选择一个最好的网络
WORD MACSearchPrioPANRecord(PAN_DESCRIPTOR *Record);
//根据网络地址查询
WORD MACSearchPANRecordByPAN(SHORT_ADDR PANId);
//统计网络的数量
WORD MACCountPANRecord(void);
//超时处理
void MACProccessPANRecordByTimeout(void);

//建立网络
void MACEstablishPAN(void);
//加入网络
void MACJoinPAN(void);

//对射频接收缓冲区的操作
#define MACGet()  PHYGet() 
#define MACGetArray(ptr,cSize) PHYGetArray(ptr,cSize) 
//射频发送
#define MACTransmitByCSMA()	   PHYTranmitByCSMA()

//信标请求
BOOL MACSendBeaconReq(void); 
//入网请求
BOOL MACSendAssociationReq(NODE_INFO *macAddr,ASSOC_REQUEST_CAP CapInfo);
//离开网络
BOOL MACSendDisassociationNotify(LONG_ADDR *LongAddr,MAC_DISASSOCIATION_REASON Reason);

//API接口
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
extern MAC_PIB macPIB;					//用于记录mac层的PIB属性
extern MAC_STATUS macStatus;			//记录MAC状态
extern MAC_RX_FRAME macCurrentRxFrame;  //接收包的帧格式
#endif
