#include "Mac.H"
#include "SysTask.h"
#include "led.h"
#include "infra.h"
#include "bootloader.h"
#include "timeds.h"
#include "Loc.h"

_CONFIG1(JTAGEN_OFF           // JTAG Diabled
          & GCP_OFF            // Code Protection Disabled
          & GWRP_OFF           // Write Protection Disabled
          & BKBUG_OFF          // Background Debugger Disabled
//          & COE_OFF            // Clip-on Emulation Mode Disabled
          & ICS_PGx3           // ICD Pins Select: EMUC/EMUD share PGC2/PGD2
          & FWDTEN_OFF          // Watchdog Timer Enabled
          & WINDIS_OFF			// Windowed WDT Disabled
		  & FWPSA_PR128			//预分频7位
		  & WDTPS_PS2048);      //后分频1:2048 
 _CONFIG2(IESO_OFF				//Internal External switch over mode
          & FNOSC_FRC          // Oscillator Selection: Fast RC oscillator
          & FCKSM_CSDCMD       // Clock switching and clock monitor: Both disabled
          & OSCIOFNC_ON       // OSCO/RC15 function: RC15
          & POSCMOD_NONE);       // Oscillator Selection: Primary disabled
 
/*
修改日期：2017年1月20日
修改者：李延超

通信速率变为2.4k
这是节点的程序，加入了组网的功能。
节点在底层循环中通过定时器2不断的向协调器发送入网请求，入网主要通过RSSI的值的大小判断加入哪个网络，不会判断有多少网络
当入网成功之后，节点仍然通过定时器2不断向协调器发送数据包告知自己的存在。
同时当收到协调器的出网请求后，会重新开始申请地址，但是上电时获得的长地址不会变。
由于节点上电随机获得地址，在MAC层初始化时需要给节点一个唯一的标示。
*/
/*
组网流程：组网的难题主要在于Flash的操作。
	定时器2用于定时向协调器申请地址信息，以下流程是在节点的角度考虑。
    1. MAC层初始化的时候要获得一个长地址。 
	2. 定时器2时间到，进入MACJoinPAN(void)函数，发送信标命令请求帧，这是一个命令帧。
    3. 协调器回应一个信标帧，广播并让所有节点收到。节点进入MACProcessBeacon()函数，处理接收到的信标帧，目的将协调器加入自己列表，
	   注意关键函数MACRefreshPANRecord(PAN_DESCRIPTOR *Record),决定了加入网络的条件。
    4. 在第3步完成之后，节点等待下一个定时器2的时间，向协调器发送分配地址请求帧，请求短地址。
    5. 收到协调器的响应之后，除了修改MAC层的PIB属性之外，还应该对相关的地址状态等进行存储。
	6. FLASH的操作流程详见FLASHMM.c文件开头处
    7. 读取节点长地址有两种方法，其一是节点以数据帧形式封装长地址到负载中，协调器当做数据帧上传；
 *     其二是节点发送普通数据包，协调器解析数据包后将长地址单独加入到以太网数据包中上传到服务器。
*/

extern BYTE Parameter[PARAMETER_NUM];
extern IO_INFO AD_Value[];
void PutOut(void);
void HardTimer_Init(void);
extern WORD data;//继电器动作标志位
void CircleTask(void)
{
	BYTE i;
    
	if(emCheckHardTimer(1))
	{
		emDelHardTimer(1);
//////////		InOut_Control();    //信息采集，控制逻辑
//////////        PutOut();           //继电器控制
        SysRealTime();      //系统时间计算
		emStartHardTimer(1);
	}
	if(emCheckHardTimer(2))
	{
		emDelHardTimer(2);
		if(!MACIsNetworkJoined())
		{
			emTxMesg(RF_JOIN_NETWORK_REQ,RealTimeMesg,0,0); 
		}
        else
        {}
		emStartHardTimer(2);
	}
    
    if(emCheckHardTimer(3))//心跳包
	{
		emDelHardTimer(3);
		emTxMesg(TEM_SENT_REQ,RealTimeMesg,0,0);
		emStartHardTimer(3);
	}
	
	if(emCheckHardTimer(4))//进行状态转换
	{
		emDelHardTimer(4);
		
		while (RF_TRX_TX == PHYGetTRxState());// 等待不是TX的状态		
		//10分钟左右做一次状态转换，RX->IDLE—>RX
		//在状态转换时候，CC2500会自动做频率校准的。消耗时间大约是800us。
		PHYSetTRxState(RF_TRX_RX);
		PHYSetTRxState(RF_TRX_IDLE);
		PHYSetTRxState(RF_TRX_RX);
					
		emStartHardTimer(4);
	}
        if(emCheckHardTimer(5))			 
	{
		emDelHardTimer(5);
        data=(data|0x0004);
        HC595Put(data);
        HC595Out();
        emStartHardTimer(6);
	}
        if(emCheckHardTimer(6))			 
	{
		emDelHardTimer(6);
        data=(data&0xfffb);
        HC595Put(data);
        HC595Out();
        emStartHardTimer(5);
	}
    if(emCheckHardTimer(7))			 
	{
		emDelHardTimer(7);
//第一路继电器线圈断电
        data=(data&0xfff7);
        HC595Put(data);
        HC595Out();
	}
        if(emCheckHardTimer(8))			 
	{
		emDelHardTimer(8);
        LEDBlinkRed();
        emStartHardTimer(8);
	}

	i=emSearchMesg(RF_FLUSH_RSPBUFFER_REQ);
	if(i!=InvalidMesg)
	{
		emTxMesg(RF_FLUSH_RSPBUFFER_RSP,RealTimeMesg,0,0);
	}
	
	i=emSearchMesg(RF_REV_MESSAGE_NOTIFY);
	if(i!=InvalidMesg)
	{
		emTxMesg(RF_REV_MESSAGE_REQ,RealTimeMesg,0,0);
	}
	
	SchedTask();
}

void main(void)
{
	WORD prio;
	emDint(&prio);
	//端口数字化
	AD1PCFG=0xFFFF; //把模拟输入改成数字I/O
	//硬件初始化
    BufferInit();

	//UnlockREG();
	//LockREG();

	//ADInit();
	DeviceInitSetup();

	FlashInitSetup();
    LEDInitSetup();
	InitTicks();

	SPIInit();
//////    SPIInitCC2500off();
    
	//外部中断1（无线中断）初始化
//////	RPINR0bits.INT1R=10; //GDO0 接收时输入引脚
//////	LATBbits.LATB2=1; //LNA
//////	LATBbits.LATB15=0; //PA
    //外部中断1（树莓派看门狗）初始化
    TRISBbits.TRISB0=1;
    RPINR0bits.INT1R=0; //树莓派看门狗
    
	IFS1bits.INT1IF=0;
	IEC1bits.INT1IE=1;//关闭无线中断
	IPC5bits.INT1IP=7;//最高优先级为7，依次递减6直至1
	INTCON2bits.INT1EP=1;//1为下降沿，0为上升沿
	
    

    //外部中断2（树莓派开门中断）初始化
    TRISBbits.TRISB1=1;
    RPINR1bits.INT2R=1;//原来的无线中断映射RP10，记得替换
	IFS1bits.INT2IF=0;
	IEC1bits.INT2IE=1;
	IPC7bits.INT2IP=6;//最高优先级为7，依次递减6直至1
	INTCON2bits.INT2EP=0;//1为下降沿，0为上升沿
    
	//将FLASH的格式化
	//FlashFormatMem();
    
	//协议栈初始化
	emSysInit();
//////    RFInitSetup();      //可设置通信速率
//////	PHYInitSetup();  //可设置通信速率
//////	MACInitSetup();
	LEDBlinkRed();
	LEDBlinkYellow();
	CurSysTimeInit();
    
    HC595Put(0x00);
    HC595Out();
	emEint(&prio);
  
    GetParameters();
    HardTimer_Init();    
  	CreateTask(SYSTask,0,0,READY);	
//////  	CreateTask(MACTask,9,9,READY);
//////  	CreateTask(TemTask,6,6,READY);
//////    CreateTask(LocTask,7,7,READY);
//////  	CreateTask(InfraTask,5,5,READY);
  	CreateTask(CircleTask,15,15,READY);

  	StartTask();
}
void HardTimer_Init(void)
{
    emSetHardTimer(1,40);//感知循环检测
  	emSetHardTimer(2,100);
    emSetHardTimer(3,Parameter[update]*6000);  //温度感知定时上传时间设置                  //测试用 10min
  	emSetHardTimer(4,30000);//640s进行一次状态转换，防止晶振偏离
    emSetHardTimer(5,1000);//树莓派每秒一个低电平与PIC进行心跳通信；如果2分钟没检测到则PIC20S强制断电。
    emSetHardTimer(6,200);//开合闸时间间隔2s;
    emSetHardTimer(7,50);//开锁后，0.5s自动恢复继电器
  	//start the timer, discard the returning value
    emSetHardTimer(8,500);//心跳灯
    
  	emStartHardTimer(1);
    emStartHardTimer(5);//开启树莓派看门狗
    emStartHardTimer(8);
//////  	emStartHardTimer(2);
//////	    emStartHardTimer(3);
//////  	emStartHardTimer(4);  
}
