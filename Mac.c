#include "mac.h"
#include "timeds.h"
#include "bootloader.h"
#include "common.h"
#include "infra.h"
#include "Loc.h"

//定位变量
BYTE Loc_Buffer[20];
WORD distance_Buffer[20];

BYTE Loc_i=35,Loc_j=0;
BYTE Loc_Times=0;
BYTE Type = 0; //系统查询到系统消息TEM_SENT_REQ时，根据全局变量Type进行数据的回复(本版本已弃用))
extern BYTE infrared_correct;

//定义协调器列表大小
#define ConstCoordQueSize	2 
extern IO_INFO AD_Value[IO_NUM];
BYTE CoordID[2]={0,0};
static BYTE PreviousRxmacDSN[4] = {0xff,0xff,0xff,0xff};
BYTE Route_flag = 0;    //0表示本地；1表示协调器；2表示转发目的节点；3表示广播
/*
目前关于Mac层Flash中的内容从地址0X9000开始。
0X9000~0X907E存放的是节点的长地址
0X9080~0X90FE存放的节点的MAC层属性,也包括长地址。
0X9100~0X91AE存放的是节点的状态
0X9280~0X92FE存放的是所属网络协调器的相关信息
*/

//用于自组网的变量
BYTE JoinNwkIndction = 0;//可以申请入网的标志位。
//BYTE RssiLevel = 180;//通过这个值节点判断链路质量，然后选择加入网络
BYTE RssiLevel = 160;//通过这个值节点判断链路质量，然后选择加入网络


//定义基本的全局变量
MAC_PIB macPIB;					//用于记录mac层的PIB属性
MAC_STATUS macStatus;			//记录MAC状态
CFG_RSP_BUFFER MACCfgRspBuf;
CFG_RSP_SHARE_BUFFER MACCfgRspSharBuf;
BYTE MACBuffer[ConstMacBufferSize];
extern PHY_PIB phyPIB;

//用来临时存储发送的数据，用来重发
MAC_TX_QUEUE TxFrameQue[ConstMacTxQueSize];
//用于形成一个完整的接收数据帧
MAC_RX_FRAME macCurrentRxFrame; 
//用于做发送数据的缓冲区
MAC_TX_BUFFER TxBuffer;
//定义一个协调器列表
PAN_DESCRIPTOR CoordQue[ConstCoordQueSize];
//定义网络信息
PAN_DESCRIPTOR PANDescriptor;


//初始化的时候可能需要更改的是节点的标示
void MACInitSetup(void)
{
	BYTE i;
	//初始化
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
	
		//状态初始化
	macStatus.nVal=0;
	macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
	
	macPIB.macCoordShortAddr.nVal=0;
	macPIB.macCoordLongAddr.byte.dwLsb=0;
	macPIB.macCoordLongAddr.byte.dwMsb=0;

    //控制重发的时间
	macPIB.macAckWaitDuration=60;
	//GTS 
	macPIB.macGTSPermit=0;
    
	//设备属性
	macPIB.DeviceInfo.bits.StackProfile=0;
	macPIB.DeviceInfo.bits.ZigBeeVersion=ZIGBEE_VERSION;
	macPIB.DeviceInfo.bits.RxOnWhenIdle=1;
	macPIB.DeviceInfo.bits.AutoRequest=1;
	macPIB.DeviceInfo.bits.PromiscuousMode=1;
	macPIB.DeviceInfo.bits.BattLifeExtPeriods=0;
	macPIB.DeviceInfo.bits.PotentialParent=1;
	macPIB.DeviceInfo.bits.PermitJoin=1;
	//下面是需要改变的参数，初始化全部为0

	//标示设备是协调器还是节点。
	macPIB.DeviceInfo.bits.DeviceType=ZIGBEE_RFD;

	//网络描述初始化
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

	//MAC层接收数据帧
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

	//节点身份标识
	macPIB.CDNum = 0xaa;

	//发送缓冲区
	for(i=0;i<ConstMacPacketSize;i++) 
	{
		TxBuffer.cTxBuffer[i]=0;
	}
	TxBuffer.cSize=0;
	
	//首先判断是否已经入网
	//上电,首先读取MAC层状态，查看是否入网
	GetMACStatus();

	//如果没有入网，即第一次上电的时候.FLASH的这个存储位全部为空。
	if(macStatus.nVal == 0XFFFF)
	{
		//状态初始化
		macStatus.nVal=0;
		macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
		macStatus.bits.isAssociated = 1;//由于FLASH区域在不写的时候默认是0XFFFF，所以不能是0，否则会被认为已经入网
		//地址信息设置,主要是获取一个长地址
		MACInitIEEEAddr();	
	}
	//如果已经入网，则把相应的地址从Flash中读出来
	else
	{
		MACGetAddrInfo();
	}

/*	{	macStatus.nVal=0;
		macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;	
		macStatus.bits.isAssociated = 0;//由于FLASH区域在不写的时候默认是0XFFFF，所以不能是0，否则会被认为已经入网
			//地址信息设置,主要是获取一个长地址
		MACInitIEEEAddr();
	}
*/
	//初始化协调器列表
	MACFormatPANRecord();
}


//程序运行的地址信息及状态信息的初始化
void MACInitIEEEAddr(void)
{
	WORD Index,Indication,Number;
	LONG_ADDR addr,allocAddr;
	BOOL bIndication;

	//首先从标志位区域读出标志位
	bIndication=GetIEEEIndication((WORD *)&Index);
	//判断是否已经自动获取过地址
	while((Index!=IEEE_VALID_CODE) || (!bIndication))	
	{
		AllocIEEEAddr(&addr);
		//已经分配了地址，进行存储
		Indication=IEEE_VALID_CODE;
		//存储标志及分配的地址
		PutIEEEAddrInfo(Indication,&addr);
		//首先从标志位区域读出标志位，来看看情况
		bIndication=GetIEEEIndication((WORD *)&Index);
	}
	//读出分配的地址
	GetIEEEAddr(&allocAddr);
	//读出长地址
	GetMACLongAddr();
	//判断两者是否一致
    Number=memcmp((BYTE *)&allocAddr,(BYTE *)&macPIB.macLongAddr,sizeof(LONG_ADDR));
	while(Number!=0)
	{
		//设置长地址
		memcpy((BYTE *)&macPIB.macLongAddr,(BYTE *)&allocAddr,sizeof(LONG_ADDR));
		//网络标示
		macPIB.macPANId.nVal=0xFFFF;
		//短地址为0
		macPIB.macShortAddr.nVal=0;
		//目的地址
		macPIB.macCoordShortAddr.nVal=0xFFFF;
		//设备类型
		macPIB.DeviceInfo.bits.DeviceType=ZIGBEE_RFD;
		//设置状态
		macStatus.nVal=0;
		//允许信标帧
		macStatus.bits.allowBeacon=1;
		//地址模式
		macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
		//比较
		Number=memcmp((BYTE *)&allocAddr,(BYTE *)&macPIB.macLongAddr,sizeof(LONG_ADDR));
	}
}

//存储地址信息
void MACSetAddrInfo(void)
{
	//存储短地址
	PutMACAddr();
	//存储PAN描述信息		
	PutCoordDescriptor();	
	//存储状态
	PutMACStatus();	
	CLR_WDT();
}
//读取地址信息
void MACGetAddrInfo(void)
{
	//读取网络地址
	GetMACPANId(); 	
	//读取短地址	
	GetMACShortAddr();
	//读取长地址	
	GetMACLongAddr();
	//读取网络描述		
	GetCoordDescriptor();
	CLR_WDT();
}

//地址辨识
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
	//若已入网，则使用网络标识验证
	if(macStatus.bits.isAssociated)
	{
		//如果网络地址既不是广播也不是本机地址，则退出
		if(((*pAddr).PANId.nVal!=macPIB.macPANId.nVal) && ((*pAddr).PANId.nVal!=0xFFFF))
			return FALSE;
	}
	
	//若是短地址，则判断是否是本机地址或者是广播地址
	if((macStatus.bits.isAssociated) && (*pAddr).AddrMode==MAC_DST_SHORT_ADDR)
	{
		//若为空，则退出
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
                Route_flag = 1; //表示该数据为节点上传给协调器
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
                Route_flag = 2; //表示该数据为协调器下传给节点
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
		//判断是否为空
		cLength=memcmp((BYTE *)&(*pAddr).LongAddr,(BYTE *)cZero,sizeof(LONG_ADDR));
		//判断是否为本机地址
		cNumber=memcmp((BYTE *)&(*pAddr).LongAddr,(BYTE *)&macPIB.macLongAddr,sizeof(LONG_ADDR));
		//判断是否为广播
		cSize=memcmp((BYTE *)&(*pAddr).LongAddr,(BYTE *)cVal,sizeof(LONG_ADDR));
		CLR_WDT();
		//若是为空，则退出
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

//发送ACK确认帧
BOOL MACSendACK(BYTE dsn)
{
	BYTE cPtrTx[6];
	BYTE cPktSize;
	CRC_RESULT	crcRes;
	BYTE cSize=0;
	//不包含长度，CRC，RSSI
	cPtrTx[cSize++]=3;
	cPtrTx[cSize++]=MAC_FRAME_ACK;
	cPtrTx[cSize++]=0;
	cPtrTx[cSize++]=dsn;
	//计算自己加的CRC校验和
	crcRes=CrcCalc(cPtrTx,cSize);
	cPtrTx[cSize++]=crcRes.cVal[0];
	cPtrTx[cSize++]=crcRes.cVal[1];
	//重新修改长度
	cPtrTx[0]+=sizeof(CRC_RESULT);

	//长度，FCS，dsn
	PHYPutTxBuffer((BYTE *)&cPtrTx,cSize);
	//喂狗，防止正常情况进入复位
	CLR_WDT(); 
	cPktSize=PHYGetTxNumber();
	//若写入发送缓冲区的长度不正确则退出
	if(cPktSize!=6)
	{
		PHYClearTx();
		return FALSE;
	}
	CLR_WDT();
	//发送
	return MACTransmitByCSMA();
}

//接收缓冲区内，丢弃一定长度的数据
void MACDiscardRx(BYTE size)
{
	BYTE i;
	for(i=0;i<size;i++)
	{
		MACGet();
	}	
}

//取出MAC帧
BOOL MACGetPacket(void) 
{
	BYTE i;
	BYTE cNumber=0;
	BYTE frameType;
	CRC_RESULT	crcResult;
	
	i=emSearchMesg(RF_REV_MESSAGE_NOTIFY);
	
	//已经收到不少于一个完整的数据包
	if((i!=InvalidMesg) && !macCurrentRxFrame.bReady)
	{
		emRxMesg(i,NRealTimeMesg,0,0);
		TxBuffer.cSize=0;
		//长度字节，不包含长度本身，但是不包含自己加的crc校验，RSSI，CRC
		macCurrentRxFrame.packetSize=MACGet();
		macCurrentRxFrame.packetSize-=4;
		//去掉2个自己加的CRC校验和，2个字节硬件自动加上的
		TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.packetSize; 
		//读出控制域，低字节在前
		macCurrentRxFrame.frameCON.cVal[0]=MACGet();
		macCurrentRxFrame.frameCON.cVal[1]=MACGet();
		TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.frameCON.cVal[0];
		TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.frameCON.cVal[1];
		//取出序列号
		macCurrentRxFrame.sequenceNumber=MACGet();
		TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.sequenceNumber;
		//取出帧类型
		frameType=macCurrentRxFrame.frameCON.bits.FrameType;
		cNumber+=3;
		//喂狗，防止正常情况进入复位
		CLR_WDT(); 

		//若是确认帧进行处理
		if((frameType==MAC_FRAME_ACK) && (macCurrentRxFrame.packetSize==5))
		{
			macCurrentRxFrame.dstAddress.AddrMode=MAC_DST_SHORT_ADDR;
			macCurrentRxFrame.dstAddress.ShortAddr.cVal[0]=MACGet();
			macCurrentRxFrame.dstAddress.ShortAddr.cVal[1]=MACGet();
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.ShortAddr.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.dstAddress.ShortAddr.cVal[1];
			macCurrentRxFrame.crcRes.cVal[0]=MACGet();
			macCurrentRxFrame.crcRes.cVal[1]=MACGet();
			//计算CRC校验和
			crcResult=CrcCalc(TxBuffer.cTxBuffer,TxBuffer.cSize);		
			macCurrentRxFrame.rssi=MACGet();
			macCurrentRxFrame.crc=MACGet();
			if(!MACCheckAddress(&macCurrentRxFrame.dstAddress)) 
			{
				macCurrentRxFrame.bReady=0;
				return FALSE;
			}
			//判断自己加上的CRC校验和
			if(crcResult.nVal==macCurrentRxFrame.crcRes.nVal)
			{
				//判断硬件的CRC校验结果
				if(macCurrentRxFrame.crc & 0x80)
				{
					macCurrentRxFrame.bReady=1;
					return TRUE;
				}
			}

			//校验出错，退出
			macCurrentRxFrame.bReady=0;
			return FALSE;			
		}
		
		//若是命令帧或数据帧或信标帧
		if((frameType==MAC_FRAME_DATA) || (frameType==MAC_FRAME_CMD) || (frameType==MAC_FRAME_BEACON))
		{
			//地址模式必须存在，要么是长地址，要么是短地址。若目的地址模式是短地址
			if(macCurrentRxFrame.frameCON.bits.DstAddrMode==2)
			{
				//取出目的网络地址，低字节在前，高字节在后
				macCurrentRxFrame.dstAddress.PANId.cVal[0]=MACGet();
				macCurrentRxFrame.dstAddress.PANId.cVal[1]=MACGet();
				//取出目的地址，低字节在前，高字节在后
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
				//若地址模式是长地址
				//取出目的网络地址，低字节在前，高字节在后
				macCurrentRxFrame.dstAddress.PANId.cVal[0]=MACGet();
				macCurrentRxFrame.dstAddress.PANId.cVal[1]=MACGet();
				//取出目的地址，低字节在前，高字节在后
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
				//若没有目的地址信息，则是信标帧,否则丢弃该数据帧
				if(frameType!=MAC_FRAME_BEACON)
				{
					MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
					macCurrentRxFrame.bReady=0;
					return FALSE;
				}
			}
			else 
			{
				//若是地址模式出错，丢弃该数据帧
				MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
				macCurrentRxFrame.bReady=0;
				return FALSE;	
			}

			//源地址模式可以不存在，此时可能为信标请求指令
			if(macCurrentRxFrame.frameCON.bits.SrcAddrMode)
			{
				//若是源地址和目的地址不在一个网段内，读出源网络地址
				if(!macCurrentRxFrame.frameCON.bits.IntraPAN)
				{
					macCurrentRxFrame.srcAddress.PANId.cVal[0]=MACGet();
					macCurrentRxFrame.srcAddress.PANId.cVal[1]=MACGet();
					cNumber+=2;
					TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.srcAddress.PANId.cVal[0];
					TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.srcAddress.PANId.cVal[1];					
				}

				//判断地址模式
				if(macCurrentRxFrame.frameCON.bits.SrcAddrMode==2)
				{
					//若是短地址模式，取出源地址
					macCurrentRxFrame.srcAddress.AddrMode=MAC_SRC_SHORT_ADDR;
					macCurrentRxFrame.srcAddress.ShortAddr.cVal[0]=MACGet();
					macCurrentRxFrame.srcAddress.ShortAddr.cVal[1]=MACGet();
					cNumber+=2;
					TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.srcAddress.ShortAddr.cVal[0];
					TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.srcAddress.ShortAddr.cVal[1];					
				}
				else if(macCurrentRxFrame.frameCON.bits.SrcAddrMode==3)
				{
					//若是长地址模式，读出长地址
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
					//若是源地址模式出错，丢弃该数据帧
					MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
					macCurrentRxFrame.bReady=0;
					return FALSE;	
				}
				
			}
			else
			{
				//若是源地址模式为空，则看是否是信标请求帧,否则退出
				if(frameType!=MAC_FRAME_CMD)
				{
					MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
					macCurrentRxFrame.bReady=0;
					return FALSE;					
				}	
			}
			CLR_WDT();
			//若是该MAC帧的长度超过上限，则丢弃该数据帧
			if((macCurrentRxFrame.packetSize+4-cNumber)>ConstMacPayloadSize)
			{
				MACDiscardRx(macCurrentRxFrame.packetSize+4-cNumber);
				macCurrentRxFrame.bReady=0;
				return FALSE;	
			}
			
			//负载装入到接收数据帧结构体中
			for(i=0;i<macCurrentRxFrame.packetSize-cNumber;i++)
			{
				//pMsdu指向存储的payload的初始地址
				macCurrentRxFrame.pMsdu[i]=MACGet();
				TxBuffer.cTxBuffer[TxBuffer.cSize++]=macCurrentRxFrame.pMsdu[i];
			}
			//取出自己增加的CRC校验和
			macCurrentRxFrame.crcRes.cVal[0]=MACGet();
			macCurrentRxFrame.crcRes.cVal[1]=MACGet();
			//取出RSSI和CRC结果
			macCurrentRxFrame.rssi=MACGet();
			//macCurrentRxFrame.rssi=PHYGetLinkQuality(macCurrentRxFrame.rssi);
			macCurrentRxFrame.crc=MACGet();
			
			//若是信标帧，不需要地址辨识,否则需要进行地址辨识
			if(frameType!=MAC_FRAME_BEACON)
			{
				if(!MACCheckAddress(&macCurrentRxFrame.dstAddress)) 
				{
					macCurrentRxFrame.bReady=0;
					return FALSE;
				}
			}

			//计算自己加的CRC校验和
			crcResult=CrcCalc(TxBuffer.cTxBuffer,TxBuffer.cSize);
			CLR_WDT();
			//CRC校验结果
			if((macCurrentRxFrame.crcRes.nVal==crcResult.nVal) && (macCurrentRxFrame.crc & 0x80))
			{
				if(macCurrentRxFrame.frameCON.bits.AckRequest)
				{
					//若是需要回复ACk，发送确认帧
					MACSendACK(macCurrentRxFrame.sequenceNumber);
				}
				//修改接收标志位及负载长度
				macCurrentRxFrame.bReady=1;
				macCurrentRxFrame.packetSize=macCurrentRxFrame.packetSize-cNumber;
				LEDBlinkRed();
				//喂狗，防止正常情况进入复位
				CLR_WDT(); 
				return TRUE;
			}
			else
			{
				//CRC校验出错，丢弃该数据帧
				macCurrentRxFrame.bReady=0;
				return FALSE;
			}
		}
		else
		{
			//帧类型出错，丢弃该数据帧
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

//封装MAC帧头
BOOL MACPutHeader(NODE_INFO *pDestAddr, BYTE frameCON)
{
	BYTE IntraPAN;
	BYTE srcAddrMode,dstAddrMode;
	//读取地址模式
	srcAddrMode=macStatus.bits.addrMode; 
	dstAddrMode=(*pDestAddr).AddrMode;
	//是否是在一个网络内
	IntraPAN=frameCON & MAC_INTRA_PAN_YES;
	//清空发送缓冲区
	TxBuffer.cSize=0;
	//写入长度
	TxBuffer.cTxBuffer[TxBuffer.cSize++]=0;
	//写入帧控制域，低字节在前，高字节在后
	TxBuffer.cTxBuffer[TxBuffer.cSize++]=frameCON;
	TxBuffer.cTxBuffer[TxBuffer.cSize++]=srcAddrMode|dstAddrMode;
	//写入MAC层序列码	
    TxBuffer.cTxBuffer[TxBuffer.cSize++]=++macPIB.macDSN;
	CLR_WDT();
	//若目的地址模式为空，则可能为信标帧
	if(dstAddrMode)
	{
		//判断目的地址模式
		if(dstAddrMode==MAC_DST_SHORT_ADDR)
		{
			//短地址模式
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).PANId.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).PANId.cVal[1];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).ShortAddr.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).ShortAddr.cVal[1];
			 //若是广播，ACK强制设置成不允许
			if((*pDestAddr).ShortAddr.nVal==0xFFFF)
				TxBuffer.cTxBuffer[1]&=0xDF;
		}
		else if(dstAddrMode==MAC_DST_LONG_ADDR)
		{
			//长地址模式
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).PANId.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=(*pDestAddr).PANId.cVal[1];
		    memcpy((BYTE *)&TxBuffer.cTxBuffer[TxBuffer.cSize],(BYTE *)&(*pDestAddr).LongAddr.cVal[0],8);
			TxBuffer.cSize+=8;
			//若是广播，ACK强制设置成不允许
			if(((*pDestAddr).LongAddr.byte.dwLsb==0xFFFFFFFF) && ((*pDestAddr).LongAddr.byte.dwMsb==0xFFFFFFFF))
				TxBuffer.cTxBuffer[1]&=0xDF; 
		}
		else
		{
			//如果地址模式不正确，则丢弃该包
			TxBuffer.cSize=0;
			return FALSE;
		}
	}
	
	//短地址模式可以有，也可以没有，例如信标请求帧中，源地址信息不存在
	if(srcAddrMode)
	{
		//若目的地址与源地址不在一个网络内，加入源地址模式
		if(!IntraPAN)
		{
		    TxBuffer.cTxBuffer[TxBuffer.cSize++]=macPIB.macPANId.cVal[0];
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=macPIB.macPANId.cVal[1];
		}

		//若是短地址模式
		if(srcAddrMode==MAC_SRC_SHORT_ADDR)
		{
            switch(Route_flag)
            {
                case 0: //表示目的地址为本身
                case 2: //表示目的地址为下一层节点
                    TxBuffer.cTxBuffer[TxBuffer.cSize++]=macPIB.macShortAddr.cVal[0];
                    TxBuffer.cTxBuffer[TxBuffer.cSize++]=macPIB.macShortAddr.cVal[1];
                    break;
                case 1: //表示式转发下层节点数据，目的地址为协调器
                    TxBuffer.cTxBuffer[TxBuffer.cSize++]=Parameter[route_high];
                    TxBuffer.cTxBuffer[TxBuffer.cSize++]=Parameter[route_low];
                    break;
            }
		}
		else if(srcAddrMode==MAC_SRC_LONG_ADDR)
		{
			//若是长地址模式
		    memcpy((BYTE *)&TxBuffer.cTxBuffer[TxBuffer.cSize],(BYTE *)&macPIB.macLongAddr.cVal[0],sizeof(LONG_ADDR));
			TxBuffer.cSize+=8;
		}
		else
		{
			//若是源地址模式出错，丢弃该帧
			TxBuffer.cSize=0;
			return FALSE;
		}
		
	}

	//修改MAC帧的长度
	TxBuffer.cTxBuffer[0]=TxBuffer.cSize-1; 
	CLR_WDT();
	return TRUE;
}

//封装MAC帧负载部分
void MACPutTxBuffer(BYTE *ptr,BYTE cSize)
{
	WORD i;
	WORD Number;
	//读取帧头的长度
	Number=TxBuffer.cTxBuffer[0];
	//判断帧头封装是否正确
	if(Number+1==TxBuffer.cSize)
	{
		for(i=0;i<cSize;i++)
		{
			TxBuffer.cTxBuffer[TxBuffer.cSize++]=*ptr;
			ptr++;
		}
		//修改长度
		TxBuffer.cTxBuffer[0]=TxBuffer.cSize-1;
		CLR_WDT();
	}
	else
	{
		//若是帧头封装不正确，丢弃本次发送
		TxBuffer.cSize=0;
	}
}

//MAC帧数据发送
BOOL MACTransmitPacket(void)
{
	BYTE cPktSize;
	BYTE cNumber;
	CRC_RESULT crcRes;
	WORD prio;
	//读取长度
	cNumber=TxBuffer.cSize;
	cPktSize=TxBuffer.cTxBuffer[0];	
	//如果长度两者关系不一致，则出错
	if(cNumber!=cPktSize+1)
	{
		TxBuffer.cSize=0;
		return FALSE;
	}
	emDint(&prio);
	//加入CRC校验
	crcRes=CrcCalc(TxBuffer.cTxBuffer,cPktSize+1);
	//写入CRC校验
	MACPutTxBuffer(&crcRes.cVal[0],sizeof(CRC_RESULT));
	//读取长度
	cNumber=TxBuffer.cSize;
	//写入射频
	PHYPutTxBuffer(TxBuffer.cTxBuffer,TxBuffer.cSize);
	//检验写入的是否正确
	cPktSize=PHYGetTxNumber();
	//数据帧结构中的长度不含长度本身,又增加了两个字节的CRC校验和
	if(cPktSize!=cNumber)
	{
   		PHYClearTx();
		emEint(&prio);
		return FALSE;//发送溢出
	}
	//喂狗，防止正常情况进入复位
	CLR_WDT();
	emEint(&prio);
	//放入重发队列
	MACEnqueTxFrame(); 
	//发送缓冲区清空
	TxBuffer.cSize=0;
	//发送
	return MACTransmitByCSMA();
}

//信标帧处理
void MACProcessBeacon(void)
{
	BYTE cSize=0;
	PAN_DESCRIPTOR PANDesc;

	//信标帧没有目的地址信息，所以忽略处理
	//判断FFD的地址模式,协调器回复的超帧进入这个if,因为协调器有源地址模式。
	if(macCurrentRxFrame.srcAddress.AddrMode == MAC_SRC_SHORT_ADDR)
	{
		//短地址模式
		PANDesc.CoordAddrMode=0;
		//网络标识
		PANDesc.CoordPANId.nVal=macCurrentRxFrame.srcAddress.PANId.nVal;
		//短地址
		PANDesc.CoordShortAddr.nVal=macCurrentRxFrame.srcAddress.ShortAddr.nVal;
	}
	else
	{
		//长地址模式
		PANDesc.CoordAddrMode=1;
		//网络标识
		PANDesc.CoordPANId.nVal=macCurrentRxFrame.srcAddress.PANId.nVal;
		//长地址
		memcpy((BYTE *)&PANDesc.CoordLongAddr,(BYTE *)&macCurrentRxFrame.srcAddress.LongAddr,sizeof(LONG_ADDR));		
	}

	//记录超帧描述
	PANDesc.SuperframeSpec.cVal[0]=macCurrentRxFrame.pMsdu[cSize++];
	PANDesc.SuperframeSpec.cVal[1]=macCurrentRxFrame.pMsdu[cSize++];
	CLR_WDT();
	//该位显示协调器的邻居表是否满。
	PANDesc.bits.allowJoin=PANDesc.SuperframeSpec.bits.AssociationPermit;
	
	//修改链路质量
	PANDesc.LinkQuality=PHYGetLinkQuality(macCurrentRxFrame.rssi);
	
	//将协调器加入节点的协调器列表中，可以有多个，目前只需要一个。
	MACRefreshPANRecord(&PANDesc);

	//释放资源
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

//MAC层命令帧处理
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
				//开始设置
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
			
			//macCurrentRxFrame.packetSize这个时候表示的其实是负载的长度，前面已经经过层层拆减，帧头的长度已经去除
			PramaNum=macCurrentRxFrame.packetSize-1;
			if(PramaNum==sizeof(SHORT_ADDR))
			{
				macPIB.macShortAddr.cVal[0]=macCurrentRxFrame.pMsdu[offset++];
				macPIB.macShortAddr.cVal[1]=macCurrentRxFrame.pMsdu[offset++];
				macStatus.bits.addrMode = MAC_SRC_SHORT_ADDR; //有了短地址模式说明可以利用短地址通信了
				PutMACStatus();
				PutMACAddr();
				cPtrTx[cSize++]=0x01;	//1表示地址分配成功
			}	
			else 
			{
				cPtrTx[cSize++]=0x00;   //0表示地址没有分配成功
			}
			cPtrTx[cSize++] = macPIB.CDNum;	
			MACFillSourAddr(&cPtrTx[cSize]); //将本机的子网地址和短地址填充以便发送给协调器
			cSize+=4;

			//又拷贝了两个字节，这个时候这两个字节应该为空，难道是作为结束标志？？
			//memcpy(&cPtrTx[cSize],(BYTE *)&macCurrentRxFrame.pMsdu[offset],PramaNum);
			//cSize+=PramaNum;
		    MACPutHeader(&DestShortAddr,MAC_FRAME_CMD|MAC_INTRA_PAN_YES|MAC_ACK_NO);
			MACPutTxBuffer(cPtrTx,cSize);
			MACTransmitPacket();
			macCurrentRxFrame.bReady=0;
			break;
		case MAC_ASSOC_RESPONSE://节点地址要写入FLASH中
			DeviceType=macPIB.DeviceInfo.bits.DeviceType;
			//if((DeviceType==ZIGBEE_RFD || DeviceType==ZIGBEE_ROUTER) && (!macStatus.bits.isAssociated))
            if((DeviceType==ZIGBEE_RFD || DeviceType==ZIGBEE_ROUTER) )
			{
                BYTE inum;
				//读出分配给自己的地址
				shortAddr.cVal[0]=macCurrentRxFrame.pMsdu[offset++];
				shortAddr.cVal[1]=macCurrentRxFrame.pMsdu[offset++];
				//读出加入网络的状态
				Status=macCurrentRxFrame.pMsdu[offset++];
				//说明加入网络成功，并且给分配了一个短地址
				if((Status==MAC_ASSOCIATION_PAN_SUCCESS) && (shortAddr.nVal!=0xFFFF))
				{
					//从接收帧中取得网络标识
					macPIB.macPANId.nVal=macCurrentRxFrame.srcAddress.PANId.nVal;
					//记录分配的地址
					macPIB.macShortAddr.nVal=shortAddr.nVal;
                    memcpy((BYTE *)&macPIB.macLongAddr,(BYTE *)&macCurrentRxFrame.dstAddress.LongAddr,sizeof(LONG_ADDR));
					for(inum=0;inum<8;inum++)
                    {
                        macPIB.macLongAddr.cVal[inum]=macCurrentRxFrame.pMsdu[offset++];
                    }
                    PANDescriptor.CoordPANId.nVal = macCurrentRxFrame.srcAddress.PANId.nVal;
                    PANDescriptor.CoordShortAddr.nVal = macCurrentRxFrame.srcAddress.ShortAddr.nVal;
                    //存储短地址
					PutMACAddr();
					//修改地址模式	
					macStatus.bits.addrMode=MAC_SRC_SHORT_ADDR;
					//修改入网状态
					macStatus.bits.isAssociated=1;	
					//存储MAC状态
					PutMACStatus(); 
					//保存父节点信息
					PutCoordDescriptor();
					emTxMesg(NET_ADDR_ACK,RealTimeMesg,0,0); 
				}
				else if((Status==MAC_ASSOCIATION_PAN_SUCCESS) && (shortAddr.nVal==0xFFFF))
				{
					//表明自己不需要分配短地址，利用长地址通信
					macPIB.macPANId.nVal=macCurrentRxFrame.srcAddress.PANId.nVal;
					//存储
					//PutMACAddr();
					//修改地址模式	
					macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
					//修改入网标志
					macStatus.bits.isAssociated=1;			
					//存储
					//PutMACStatus(); 
					//保存父节点信息
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
		//收到出网请求
		case MAC_DISASSOC_NOTIFY:
			JoinNwkIndction = 0;//标志位清零
			//初始化网络协调器列表
			MACFormatPANRecord();
			//修改标志位
			macStatus.bits.isAssociated=0;
			//地址模式也回到长地址模式
			macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
			macPIB.macShortAddr.nVal=0;
			macPIB.macPANId.nVal = 0;
			macCurrentRxFrame.bReady=0;
			break;
		default:
			break;
	}
}

//MAC确认帧处理
void MACProcessAck(void)
{
	BYTE dsn;
	dsn=macCurrentRxFrame.sequenceNumber;
	MACRemoveTxFrame(dsn);
	macCurrentRxFrame.bReady=0;
	CLR_WDT();
}

//MAC层状态机
void MACProcessPacket(void)
{
	BYTE cNumber;
	BYTE DeviceType;
	BYTE FrameType;
	BYTE i,j;   //仅作计数用
    BYTE cSize = 0;
	BYTE cPtrTx[40];
	NODE_INFO macAddr;
	//初始化
	cNumber=0; 
	//若是接收到一个MAC帧
	i = MACGetPacket();
	if(i == TRUE)
	{
        //取出MAC帧类型
        FrameType=macCurrentRxFrame.frameCON.bits.FrameType;
        if((0 == Route_flag)||(3 == Route_flag)) //  表明该数据帧是给本节点的或广播的
        {
            switch (FrameType)
            {
                //若是数据帧
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
                //若是信标帧
                case MAC_FRAME_BEACON:
                    MACProcessBeacon();
                    break;
                //若是确认帧
                case MAC_FRAME_ACK:
                    MACProcessAck();
                    break;
                //若是命令帧
                case MAC_FRAME_CMD:
                    MACProcessCommand();
                    break;
                //否则丢弃
                default:
                    macCurrentRxFrame.bReady=0;
                    break;	
            }
        }
        else if((1 == Route_flag)||(2 == Route_flag))        //1代表目的为协调器；2代表节点；3代表广播
        {
            switch(Route_flag)
            {
                case 1:     //目的地址为协调器
                    macAddr.ShortAddr.nVal = macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;;
                    break;
                case 2:     //协调器来的数据，转发给下层节点
                    macAddr.ShortAddr.nVal = ((WORD)Parameter[route_high]<<8&0xff00)|(WORD)Parameter[route_low]&0x00ff;
                    break;
                default:
                    macCurrentRxFrame.bReady=0;
                    return 0;
                    break;
            }
            //路由数据转发
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
            }	//此花括号为数据转发
        }		
	}
	else
	{
  	Nop();
  	Nop();
    } 	
    Route_flag = 0;
	//处理重发部分
	MACRefreshTxFrame();
	//超时处理协调器队列
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
            RESET();//复位
        break;
        case 0x20://第一路继电器线圈通电
             data = data | 0x0020;
             HC595Put(data);
             HC595Out();
        break;        
        case 0x21://第一路继电器线圈断电
             data = data & 0xffdf;
             HC595Put(data);
             HC595Out();
        break;
        case 0x22://第二路继电器线圈通电
             data = data | 0x0010;
             HC595Put(data);
             HC595Out();
        break;
        case 0x23://第二路继电器线圈断电
             data = data & 0xffef;
             HC595Put(data);
             HC595Out();
        break;
        case 0x24://第三路继电器线圈通电
             data = data | 0x0008;
             HC595Put(data);
             HC595Out();
        break;
        case 0x25://第三路继电器线圈断电
             data = data & 0xfff7;
             HC595Put(data);
             HC595Out();
        break;
        case 0x26://第四路继电器线圈通电
             data = data | 0x0004;
             HC595Put(data);
             HC595Out();
        break; 
        case 0x27://第四路继电器线圈断电
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
                
                {//安全控制策略
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

                    Buffer[TACTICE_NUM]=TACTICE_DATA[0];//策略写入缓存区
                    Parameter[TACTICE_NUM]=TACTICE_DATA[0];

                    ErasePage();
                    WritePM(Buffer, SourceAddr);
                    BufferInit();//清空Buffer                 
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
                memcpy(&CurrentSysTime,&ServerTime,sizeof(RTIME));//将服务器时间赋值给最初时刻的系统时间

                emTxMesg(TIME_DATA_REQ,RealTimeMesg,0,0);
            }
            else 
                ErrorFlag = 1;
		break;
        
        case 0x0d:

           emTxMesg(TIME_DATA_REQ,RealTimeMesg,0,0);
        break;
        
        case 0xee:      //接收命令后发送广播包，广播自己所在网络、保存的协调器短地址和自己macPIB短地址
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
//////////        else if((0xbb==Type)||(0x03==Type))//因为这两条指令的长度比较长，需要专门的回复格式
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
//////////        if((Type!=0xbb)&&(Type!=0x03))//因为这两条指令的长度比较长，需要专门的回复格式
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

//重发队列入队
WORD MACEnqueTxFrame(void)
{
	BYTE i;
	BYTE AckRequest;
	//判断是否需要ACK确认
	AckRequest=TxBuffer.cTxBuffer[1] & MAC_ACK_YES;
	if(!AckRequest)
	{
		//若是不需要ACK，则不用存储
		return InValid_Index;
	}

	for(i=0;i<ConstMacTxQueSize;i++)
	{
		if(!TxFrameQue[i].Flags.bits.bInUse)
		{
			memcpy(TxFrameQue[i].cTxBuffer,TxBuffer.cTxBuffer,TxBuffer.cSize);
			//记录发送时间，进行超时判断
			TxFrameQue[i].dwStartTime=GetTicks();
			//重发次数
			TxFrameQue[i].cRetries=0;
			//修改存储标志位
			TxFrameQue[i].Flags.bits.bInUse=1;
			//喂狗，防止正常情况进入复位
			CLR_WDT(); 
			return i;
		}
	}
	return InValid_Index;
}

//删除重发队列内一条记录
WORD MACRemoveTxFrame(BYTE dsn)
{
	BYTE i;
	BYTE SequenceNumber;
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		//取出MAC帧的DSN
		SequenceNumber=TxFrameQue[i].cTxBuffer[3];
		//根据DSN查找MAC帧
		if((SequenceNumber==dsn) && (TxFrameQue[i].Flags.bits.bInUse))
		{
			TxFrameQue[i].Flags.bits.bInUse=0;
			//把帧长度、把MAC层帧头和序列码置成空
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

//查找重发记录
WORD MACSearchTxFrame(BYTE dsn)
{
	BYTE i;
	BYTE SequenceNumber;
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		//取出MAC帧的DSN
		SequenceNumber=TxFrameQue[i].cTxBuffer[3];
		//根据DSN查找MAC帧
		if((SequenceNumber==dsn) && (TxFrameQue[i].Flags.bits.bInUse))
			return i;
	}
	return InValid_Index;
}

//MAC帧重发、超时处理函数
void MACRefreshTxFrame(void)
{
	BYTE i;
	BYTE PacketSize;
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		//若重发次数超过规定值，则丢弃
		if(TxFrameQue[i].Flags.bits.bInUse)
		{
			if(TxFrameQue[i].cRetries>=aMaxFrameRetries)
				TxFrameQue[i].Flags.bits.bInUse=0;
			else if(DiffTicks(TxFrameQue[i].dwStartTime,GetTicks())>macPIB.macAckWaitDuration)
			{
				//否则对顶时间内，没有接收到ACK,重发
				//取出长度
				PacketSize=TxFrameQue[i].cTxBuffer[0];
				//写入射频,已经包含了crc校验
				PHYPutTxBuffer(TxFrameQue[i].cTxBuffer,PacketSize);
				//发送
				MACTransmitByCSMA(); 
				//记录发送时间
				TxFrameQue[i].dwStartTime=GetTicks();
				//重发次数加1
				TxFrameQue[i].cRetries++;
				CLR_WDT(); 
			}
		}
	}
}

//MAC帧重发队列统计
BYTE MACRecordTxFrame(void)
{
	BYTE i;
	BYTE Number=0;
	for(i=0;i<ConstMacTxQueSize;i++)
	{
		//统计重发队列的记录
		if(TxFrameQue[i].Flags.bits.bInUse)
			Number++;
	}
	return Number;
}

/****************************************************
//通过对协调器队列的管理，来选择一个可靠的网络加入
****************************************************/

//格式化协调器列表
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

//增加一个协调器节点
WORD MACAddPANRecord(PAN_DESCRIPTOR *Record)
{
	WORD i;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		if(!CoordQue[i].bits.bInUse)
		{
			CoordQue[i].bits.bInUse=1;
			//接收次数
			CoordQue[i].bits.RxNumber=1;
			//允许加入
			CoordQue[i].bits.allowJoin=1;
			//地址模式
			CoordQue[i].CoordAddrMode=(*Record).CoordAddrMode;
			//其它值默认为0
			CoordQue[i].SecurityUse=0;
			CoordQue[i].SecurityFailure=0;
			CoordQue[i].ACLEntry=0;
			//信道
			CoordQue[i].LogicalChannel=PHYGetChannel();
			//地址
			CoordQue[i].CoordPANId.nVal=(*Record).CoordPANId.nVal;
			CoordQue[i].CoordShortAddr.nVal=(*Record).CoordShortAddr.nVal;
			memcpy((BYTE *)&CoordQue[i].CoordLongAddr,(BYTE *)&((*Record).CoordLongAddr),sizeof(LONG_ADDR));
			//超帧描述
			CoordQue[i].SuperframeSpec.nVal=(*Record).SuperframeSpec.nVal;
			//时间戳
			CoordQue[i].TimeStamp=GetTicks();
			//链路质量
			CoordQue[i].LinkQuality=(*Record).LinkQuality;
			//网络深度
			CoordQue[i].NwkDepth=(*Record).NwkDepth;
			CoordQue[i].CoorCDNum=(*Record).NwkDepth;
			CLR_WDT();
			return i;
		}
	}
	return InValid_Index;
}

//查找协调器节点
WORD MACSearchPANRecord(PAN_DESCRIPTOR *Record)
{
	WORD i;
	BYTE Number;
	for(i=0;i<ConstCoordQueSize;i++)
	{
		if(CoordQue[i].bits.bInUse)
		{
			//若是短地址,0代表短地址，1代表长地址
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
				//若是长地址
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

//根据网络标示查询协调器列表
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

//修改协调器节点的通信次数
void MACRefreshPANRecord(PAN_DESCRIPTOR *Record)
{
	WORD Index;
//	WORD LinkQuality;
//	WORD NwkNumber;
	//查询目前节点的列表中是否已经存在该网络
	Index=MACSearchPANRecord(Record);
	//如果不存在网络，则把协调器加入列表中
	if(Index==InValid_Index)
	{
		//根据链路质量判断是否加入。
		if((*Record).LinkQuality >= RssiLevel)
		{
			MACAddPANRecord(Record);
			JoinNwkIndction = 1;
		}
	}
}

//根据所属的网络，加入到网络中
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

//选择一个最好的网络，选择的条件是链路的质量
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

//统计网络的数量
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

//超时处理
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

//路由器、节点加入网络函数
void MACJoinPAN(void)
{
	WORD Index;
	NODE_INFO Record;
	ASSOC_REQUEST_CAP CapInfo;
	//只要收到了协调器发来的信标帧，则不会再发送信标请求，直接查找自己的记录，自己所属的协调器。
	if(JoinNwkIndction == 1)
	{
		//查找自己所属的协调器
		Index=MACSearchPrioPANRecord(&PANDescriptor);
		if(Index!=InValid_Index)
		{
			CapInfo.cVal=0;
			//不能成为协调器
			CapInfo.bits.EnableCoord=0; 
			//RFD
			CapInfo.bits.DeviceType=MACGetDeviceType(); 
			//供电方式永在 
			CapInfo.bits.PowerSource=1; 
			//空闲处于接收状态
			CapInfo.bits.ReceiveIdle=1;	
			//没有使用安全模式
			CapInfo.bits.EnableSecur=0;
			//要求协调器分配地址 
			CapInfo.bits.AllocAddress=1; 
			//发送入网请求，若是短地址，应该进入这个if
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
			//首先要发送一个信标请求
			MACSendBeaconReq();
		}
	}
}


//节点发送信标请求命令
BOOL MACSendBeaconReq(void)
{
	BYTE cSize=0;
	BYTE cPtrTx[5];
	BYTE addrMode;
	NODE_INFO macAddr;
	//封装MAC帧头
	//目的地址模式为短地址
	macAddr.AddrMode = MAC_DST_SHORT_ADDR;
	//目的网络标识为广播
	macAddr.PANId.nVal=0xFFFF;
	//目的地址为广播
	macAddr.ShortAddr.nVal=0xFFFF;
	//临时存储源地址模式,MAC层初始化的时候默认为长地址模式。
	addrMode=macStatus.bits.addrMode;
	//源地址模式为空，即信标请求帧中不包含源地址信息
	macStatus.bits.addrMode=0;
	MACPutHeader(&macAddr,MAC_FRAME_CMD);
	//恢复地址模式
	macStatus.bits.addrMode=addrMode;
	//封装数据
	cPtrTx[cSize++]=MAC_BEACON_REQUEST;
	MACPutTxBuffer(cPtrTx,cSize);
	CLR_WDT();
	//发送
	return MACTransmitPacket();		
}

/**************************************************************************/
//					节点发送入网请求
//目的地址：网络PAN，目的地址根据信标帧确定
//源地址：源PAN为广播，长地址模式
//挂起设置为0，确认帧为1
/**************************************************************************/

BOOL MACSendAssociationReq(NODE_INFO *macAddr,ASSOC_REQUEST_CAP CapInfo)
{
	BYTE cSize=0;
	BYTE cPtrTx[5];
	SHORT_ADDR PANId;
	//封装MAC帧头
	//临时存储源网络标识
	PANId.nVal=macPIB.macPANId.nVal;
	//源地址网络标示为广播
	macPIB.macPANId.nVal=0xFFFF;
	//目的地址模式根据信标帧来确定
	MACPutHeader(macAddr,MAC_FRAME_CMD); //发送入网请求命令帧
	//恢复源网络标识
	macPIB.macPANId.nVal=PANId.nVal;
	//封装数据
	cPtrTx[cSize++]=MAC_ASSOC_REQUEST;
	cPtrTx[cSize++]=CapInfo.cVal;
	cPtrTx[cSize++]=macPIB.CDNum;
	MACPutTxBuffer(cPtrTx,cSize);
	//发送
	return MACTransmitPacket();		
}

/******************************************************************************/
// 					离开网络命令
//协调器或RFD发起
//目的地址：长地址模式，网络标识为macPANId
//源地址：长地址模式
//挂起为0，ACK设为1，
/******************************************************************************/

BOOL MACSendDisassociationNotify(LONG_ADDR *LongAddr,MAC_DISASSOCIATION_REASON Reason)
{
	BYTE cSize=0;
	BYTE cPtrTx[5];
	BYTE addrMode;
	NODE_INFO macAddr;
	//封装MAC帧头
	//地址模式要用长地址
	macAddr.AddrMode=MAC_DST_LONG_ADDR;
	macAddr.PANId.nVal=0xFFFF;
	memcpy((BYTE *)&macAddr.LongAddr,(BYTE *)LongAddr,sizeof(LONG_ADDR));
	//临时存储地址模式
	addrMode=macStatus.bits.addrMode;
	//源地址模式临时设为长地址
	macStatus.bits.addrMode=MAC_SRC_LONG_ADDR;
	MACPutHeader(&macAddr,MAC_FRAME_CMD|MAC_ACK_YES);
	//恢复源地址模式
	macStatus.bits.addrMode=addrMode;
	//封装数据
	cPtrTx[cSize++]=MAC_DISASSOC_NOTIFY; 
	cPtrTx[cSize++]=Reason; 
	MACPutTxBuffer(cPtrTx,cSize);
	CLR_WDT();
	//发送
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
		//发送数据包；
		memcpy((BYTE *)&DestShortAddr.LongAddr.cVal[0],ptr,8);
		ptr+=8;
		size=size-13;
		frameCON=*ptr++;
		size--;
		MACPutHeader(&DestShortAddr,frameCON);
		MACPutTxBuffer(ptr,size);
		MACTransmitPacket();
	}
	
	//MAC层的任务之一，用于组网。
	i=emWaitMesg(RF_JOIN_NETWORK_REQ,RealTimeMesg,0,0);
	if(i==1)
	{
		DeviceType=macPIB.DeviceInfo.bits.DeviceType;
		//如果设备类型是节点并且没有入网
		if((DeviceType==ZIGBEE_RFD) && (!MACIsNetworkJoined()))
		{
			MACJoinPAN();
		}
	}
	
	i=emWaitMesg(RF_FLUSH_RSPBUFFER_RSP,RealTimeMesg,0,0);
	if(i==1)
	{
		//清除缓冲区
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
