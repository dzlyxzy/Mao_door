#ifndef _COMMON_H_
#define _COMMON_H_

#include "p24FJ64GA002.h"
#include "string.h"

#define IO_NUM 10
#define InValid_Index 0xFFFF
#define InValidVal	  0xFFFF
#define ConstCfgRspBufferSize 128
#define ConstCfgRspShareBufSize 16

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long DWORD;
typedef unsigned char BOOL;
typedef int	INT16S;
typedef char INT8S;

typedef short          Word16;
typedef unsigned short UWord16;
typedef long           Word32;
typedef unsigned long  UWord32;
typedef BYTE BUFFER;

typedef struct {
	unsigned char IO_Pro;
	unsigned char type;
	unsigned char val0[2];
	union{
		unsigned int Value;
	 	unsigned char val1[2];
	};
} IO_INFO;
typedef union _DWORD_VAL
{
	BYTE cVal[4];
	WORD nVal[2];
	DWORD dwVal;
}DWORD_VAL;

typedef union _WORD_VAL
{
	BYTE cVal[2];
	WORD nVal;
}WORD_VAL;

typedef struct _CFG_RSP_BUFFER
{
	BYTE Buffer[ConstCfgRspBufferSize];
	BYTE Ptr;
}CFG_RSP_BUFFER;

typedef struct _CFG_RSP_SHARE_BUFFER
{
	BYTE Ptr;
	BOOL bReady;
	BYTE Buffer[ConstCfgRspShareBufSize];
}CFG_RSP_SHARE_BUFFER;

/*typedef struct _SYS_LED
{
	WORD bWsnRx : 1;
	WORD bWsnTx : 1;
	WORD bBusRx : 1;
	WORD bBusTx : 1;
	WORD : 12;
}SYS_LED;*/

typedef struct _RTime{

	BYTE year;
	BYTE month;
	BYTE day;
	BYTE hour;
	BYTE minute;
	BYTE second;

}RTIME;
typedef struct _CONTime{

	BYTE hour;
	BYTE minute;
	BYTE second;

}CONTIME;
typedef struct _IO_CONTROL {
	BYTE Addr;			//1BYTE
	CONTIME StartTime;	//3BYTE
	CONTIME EndTime;	//3BYTE
	BYTE Times;			//1BYTE
	BYTE Action;		//1BYTE 在规定的时间段内采取的行动
}IO_CONINF;

typedef union tuReg32
{
	DWORD Val32;

	struct
	{
		UWord16 LW;//unsigned short
		UWord16 HW;
	} Word;

	BYTE Val[4];
} BInfo;


#define DEVICE_NUM  0		//1：有两个设备		0：有一个设备
#define SERVER_ID	0		//1：本机			0：服务器
#define OutPutCN 	8		//输出控制的通道数目

#define PARAMETER_NUM  29   //参数数量  (如实写，即开辟了多少就填充多少)
#define update         0x00
#define xzyReserved1   0x01
#define xzyReserved2   0x02
#define h1             0x03
#define m1             0x04
#define open_data1_0   0x05
#define open_data1     0x06
#define h2             0x07
#define m2             0x08
#define open_data2_0   0x09
#define open_data2     0x0a
#define h3             0x0b
#define m3             0x0c
#define open_data3_0   0x0d
#define open_data3     0x0e
#define h4             0x0f
#define m4             0x10
#define open_data4_0   0x11
#define open_data4     0x12
#define h5             0x13
#define m5             0x14
#define open_data5_0   0x15
#define open_data5     0x16
#define TransmitPower  0x17
#define _rssi_offset    0x18
#define _rssi_standard  0x19
#define _rssi_factor    0x1a
#define route_high     0x1b
#define route_low      0x1c

#define Tactics 	0xA400 	//定义策略存储的地址
#define code8000    0x8000
#define code8400    0x8400
#define code8800    0x8800


#define TRUE  1
#define FALSE 0

//以下定义消息的类型

//定义系统消息
#define RTU_OVERFLOW_SYS_EROR	0xF0
#define RF_OVERFLOW_SYS_EROR	0xF1

//定义基本消息
/*#define RTU_REV_MESSAGE_NOTIFY 	0x01
#define RTU_REV_MESSAGE_REQ		0x02
#define RTU_UPLOAD_MESSAGE_REQ	0x03
#define RTU_SENT_MESSAGE_REQ	0x04*/
#define RF_REV_MESSAGE_NOTIFY	0x05
#define RF_REV_MESSAGE_REQ		0x06
#define RF_UPLOAD_MESSAGE_REQ	0x07
#define RF_SENT_MESSAGE_REQ		0x08
#define RF_JOIN_NETWORK_REQ		0x09
     
//定义系统报警信息
/*#define RTU_FLUSH_RSPBUFFER_REQ 0x0A
#define RTU_FLUSH_RSPBUFFER_RSP	0x0B*/
#define RF_FLUSH_RSPBUFFER_REQ 	0x0C
#define RF_FLUSH_RSPBUFFER_RSP	0x0D

//定义经串口可配置的命令
/*#define RTU_SET_FREQ_REQ		0x10
#define RTU_SET_FREQ_RSP		0x11
#define RTU_SET_POWER_REQ		0x12
#define RTU_SET_POWER_RSP		0x13
#define RTU_SET_RATE_REQ		0x14
#define RTU_SET_RATE_RSP		0x15
#define RTU_SET_IEEEADDR_REQ	0x16
#define RTU_SET_IEEEADDR_RSP	0x17
#define RTU_SET_PANSADDR_REQ	0x18
#define RTU_SET_PANSADDR_RSP	0x19
#define RTU_SET_SHORTADDR_REQ	0x1A
#define RTU_SET_SHORTADDR_RSP	0X1B
#define RTU_SET_BUSADDR_REQ		0x1C
#define RTU_SET_BUSADDR_RSP		0X1D
#define RTU_SET_PHY_REQ			0x1E
#define RTU_SET_PHY_RSP			0X1F
#define RTU_RESET_REQ			0x20
#define RTU_RESET_RSP			0X21
#define RTU_GET_PHY_REQ			0x22
#define RTU_GET_PHY_RSP			0x23
#define RTU_GET_ADDR_REQ		0x24
#define RTU_GET_ADDR_RSP		0x25
#define RTU_GET_BUSADDR_REQ		0x26
#define RTU_GET_BUSADDR_RSP		0x27*/
#define RTU_SET_SHORTADDR_RSP	0X1B

//定义经射频可配置的命令
#define RF_SET_FREQ_REQ			0x30
#define RF_SET_FREQ_RSP			0x31
#define RF_SET_POWER_REQ		0x32
#define RF_SET_POWER_RSP		0x33
#define RF_SET_RATE_REQ			0x34
#define RF_SET_RATE_RSP			0x35
#define RF_SET_IEEEADDR_REQ		0x36
#define RF_SET_IEEEADDR_RSP		0x37
#define RF_SET_PANSADDR_REQ		0x38
#define RF_SET_PANSADDR_RSP		0x39
#define RF_SET_SHORTADDR_REQ	0x3A
#define RF_SET_SHORTADDR_RSP	0x3B
#define RF_SET_BUSADDR_REQ		0x3C
#define RF_SET_BUSADDR_RSP		0x3D
#define RF_SET_PHY_REQ			0x3E
#define RF_SET_PHY_RSP			0x3F
#define RF_RESET_REQ			0x40
#define RF_RESET_RSP			0x41
#define RF_GET_PHY_REQ			0x42
#define RF_GET_PHY_RSP			0x43
#define RF_GET_ADDR_REQ			0x44
#define RF_GET_ADDR_RSP			0x45
#define RF_GET_BUSADDR_REQ		0x46
#define RF_GET_BUSADDR_RSP		0x47

#define RF_CMD_PARA_REQ			0x50
#define RF_CMD_PARA_RSP			0x51

#define TEM_SENT_REQ			0x60 //温度
#define INFRAHOT_SENT_REQ		0x61
#define INFRACOLD_SENT_REQ		0x62
#define INFRAOFF_SENT_REQ		0x63
#define TIME_SENT_REQ			0x64

//定位任务创建的系统消息
#define LOC_HEART_REQ           0x65//心跳包
#define LOC_RSSI_REQ            0x66//协调器和锚节点之间的RSSI
#define LOC_DEL_REQ             0x67//删除锚节点历史记录
#define LOC_DIS_REQ             0x68//请求定位数据


#define INFRAHOT_DATA_REQ		0x71
#define INFRACOLD_DATA_REQ		0x72
#define INFRAOFF_DATA_REQ		0x73
#define TIME_DATA_REQ			0x74

#define NET_ADDR_ACK     			0x88


void Delay_2us(void);
void Delay(WORD t);





#endif
