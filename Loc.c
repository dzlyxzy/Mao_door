#include "led.h"
#include "mac.h"
#include "common.h"
#include "bootloader.h"
#include "Mac.H"
#include <math.h>


extern BYTE Loc_Buffer[20];
extern WORD distance_Buffer[20];
extern BYTE Loc_i,Loc_j;
extern BYTE Loc_Times;
extern BYTE Type; //系统查询到系统消息TEM_SENT_REQ时，根据全局变量Type进行数据的回复
extern BYTE infrared_correct;

unsigned int register2distance (float reg)
{
//  默认参数 从bootloader.c里的数组加载
//	rssi_offset 71
//	rssi_standard 45
//	rssi_factor 4.0
	float dbm = 0.0;
	float distance = 0.0;
	dbm = reg >= 128 ? (reg-256)/2-Parameter[_rssi_offset] :reg/2-Parameter[_rssi_offset] ;
    WORD mid2;
	distance = (float)pow(10,(-dbm-Parameter[_rssi_standard])/(10*Parameter[_rssi_factor]));
    mid2 = distance * 100;//debug观察寄存器用
	return distance*100;
}

void LocTask(void)
{
	BYTE i;
	BYTE cSize = 0;
	BYTE cPtrTx[50];
	NODE_INFO macAddr;
	
	i=emWaitMesg(LOC_HEART_REQ,RealTimeMesg,0,0);
	if(i==1)
	{
        Delay(0x3000); 
        
	    macAddr.AddrMode = MAC_DST_SHORT_ADDR;
		macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
		macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
		MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);
        
            cPtrTx[cSize++]=0x02;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
            cPtrTx[cSize++]=0x00; 
            cPtrTx[cSize++]=30;   //数据总长度30=0x1E
            cPtrTx[cSize++]=macPIB.macPANId.nVal>>8;
            cPtrTx[cSize++]=macPIB.macPANId.nVal;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal;

            cPtrTx[cSize++]=0x85;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;          

            cPtrTx[cSize++]=0x40;       
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00; 
            cPtrTx[cSize++]=0x00;

            cPtrTx[cSize++]=0x43;       
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00; 
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            
            MACPutTxBuffer(cPtrTx,cSize);	
            MACTransmitPacket();
    }
    i=emWaitMesg(LOC_RSSI_REQ,RealTimeMesg,0,0);
    if(i==1)
	{
            Delay(0x3000);
        
            macAddr.AddrMode = MAC_DST_SHORT_ADDR;
            macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
            macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
            MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);
             
            cPtrTx[cSize++]=0x02;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
            cPtrTx[cSize++]=0x00; 
            cPtrTx[cSize++]=16;   //数据总长度30=0x1E
            
            cPtrTx[cSize++]=macPIB.macPANId.nVal>>8;
            cPtrTx[cSize++]=macPIB.macPANId.nVal;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal; 
          
            cPtrTx[cSize++]=0x40;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=macCurrentRxFrame.rssi;//记录的是锚节点 接收协调器的 RF信号强度
            cPtrTx[cSize++]=macCurrentRxFrame.crc;
            
            MACPutTxBuffer(cPtrTx,cSize);	
            MACTransmitPacket();
    }
    i=emWaitMesg(LOC_DIS_REQ,RealTimeMesg,0,0);
	if(i==1)
	{
            Delay(0x3000);
            
            macAddr.AddrMode = MAC_DST_SHORT_ADDR;
            macAddr.PANId.nVal = PANDescriptor.CoordPANId.nVal;
            macAddr.ShortAddr.nVal = PANDescriptor.CoordShortAddr.nVal;
            MACPutHeader(&macAddr,MAC_FRAME_DATA | MAC_INTRA_PAN_YES | MAC_ACK_NO);
            
            cPtrTx[cSize++]=0x02;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x01;
            cPtrTx[cSize++]=0x00; 
            cPtrTx[cSize++]=37;   
            
            cPtrTx[cSize++]=macPIB.macPANId.nVal>>8;
            cPtrTx[cSize++]=macPIB.macPANId.nVal;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal>>8;
            cPtrTx[cSize++]=macPIB.macShortAddr.nVal;
			
            /*
             * BYTE dSize = cSize - 9;
            while(dSize <= 20) //减去9个字节的头
            {
				cPtrTx[cSize++]=Loc_Buffer[dSize]>>8;
                cPtrTx[cSize++]=Loc_Buffer[dSize++];
			}  
            cPtrTx[9]=0x40;         //按端口号将数据分段,每段7个数据
            cPtrTx[10]=0x01;
            cPtrTx[11]=0x00;    
            
            cPtrTx[16]=0x40;
            cPtrTx[17]=0x02;
            cPtrTx[18]=0x00;
            
            cPtrTx[23]=0x40;
            cPtrTx[24]=0x03;
            cPtrTx[25]=0x00;
            
            cPtrTx[cSize++]=0x40;
            cPtrTx[cSize++]=0x04;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=0x00;
            cPtrTx[cSize++]=infrared_correct;
            
         
            cPtrTx[cSize++]=register2distance(0xc8)>>8;
            cPtrTx[cSize++]=register2distance(0xc8);
            memset(Loc_Buffer,0,30*sizeof(unsigned char));
            memset(distance_Buffer,0,30*sizeof(unsigned int));
            infrared_correct = 0;
             * 
             */

        cPtrTx[cSize++]=0x40;
        cPtrTx[cSize++]=0x01;
        cPtrTx[cSize++]=0x00;
        cPtrTx[cSize++]=0;
        cPtrTx[cSize++]=0;
        cPtrTx[cSize++]=register2distance(Loc_Buffer[0])>>8; // = distance_Buffer[0]>>8;
        cPtrTx[cSize++]=register2distance(Loc_Buffer[0]);

        cPtrTx[cSize++]=0x40;
        cPtrTx[cSize++]=0x02;
        cPtrTx[cSize++]=0x00;
        cPtrTx[cSize++]=0;
        cPtrTx[cSize++]=0;
        cPtrTx[cSize++]=register2distance(Loc_Buffer[1])>>8; // = distance_Buffer[0]>>8;
        cPtrTx[cSize++]=register2distance(Loc_Buffer[1]);

        cPtrTx[cSize++]=0x40;
        cPtrTx[cSize++]=0x03;
        cPtrTx[cSize++]=0x00;
        cPtrTx[cSize++]=0;
        cPtrTx[cSize++]=0;
        cPtrTx[cSize++]=register2distance(Loc_Buffer[2])>>8; // = distance_Buffer[0]>>8;
        cPtrTx[cSize++]=register2distance(Loc_Buffer[2]);


        cPtrTx[cSize++]=0x40;
        cPtrTx[cSize++]=0x04;
        cPtrTx[cSize++]=0x00;
        cPtrTx[cSize++]=0;
        cPtrTx[cSize++]=0;
        cPtrTx[cSize++]=register2distance(Loc_Buffer[3])>>8; // = distance_Buffer[0]>>8;
        cPtrTx[cSize++]=register2distance(Loc_Buffer[3]);
        
        memset(Loc_Buffer,0,20*sizeof(unsigned char));
        memset(distance_Buffer,0,20*sizeof(unsigned int));
        
		MACPutTxBuffer(cPtrTx,cSize);	
		MACTransmitPacket();
	}  
    	
	CurrentTaskWait();
	SchedTask();
}