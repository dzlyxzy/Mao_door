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
		  & FWPSA_PR128			//Ԥ��Ƶ7λ
		  & WDTPS_PS2048);      //���Ƶ1:2048 
 _CONFIG2(IESO_OFF				//Internal External switch over mode
          & FNOSC_FRC          // Oscillator Selection: Fast RC oscillator
          & FCKSM_CSDCMD       // Clock switching and clock monitor: Both disabled
          & OSCIOFNC_ON       // OSCO/RC15 function: RC15
          & POSCMOD_NONE);       // Oscillator Selection: Primary disabled
 
/*
�޸����ڣ�2017��1��20��
�޸��ߣ����ӳ�

ͨ�����ʱ�Ϊ2.4k
���ǽڵ�ĳ��򣬼����������Ĺ��ܡ�
�ڵ��ڵײ�ѭ����ͨ����ʱ��2���ϵ���Э����������������������Ҫͨ��RSSI��ֵ�Ĵ�С�жϼ����ĸ����磬�����ж��ж�������
�������ɹ�֮�󣬽ڵ���Ȼͨ����ʱ��2������Э�����������ݰ���֪�Լ��Ĵ��ڡ�
ͬʱ���յ�Э�����ĳ�������󣬻����¿�ʼ�����ַ�������ϵ�ʱ��õĳ���ַ����䡣
���ڽڵ��ϵ������õ�ַ����MAC���ʼ��ʱ��Ҫ���ڵ�һ��Ψһ�ı�ʾ��
*/
/*
�������̣�������������Ҫ����Flash�Ĳ�����
	��ʱ��2���ڶ�ʱ��Э���������ַ��Ϣ�������������ڽڵ�ĽǶȿ��ǡ�
    1. MAC���ʼ����ʱ��Ҫ���һ������ַ�� 
	2. ��ʱ��2ʱ�䵽������MACJoinPAN(void)�����������ű���������֡������һ������֡��
    3. Э������Ӧһ���ű�֡���㲥�������нڵ��յ����ڵ����MACProcessBeacon()������������յ����ű�֡��Ŀ�Ľ�Э���������Լ��б�
	   ע��ؼ�����MACRefreshPANRecord(PAN_DESCRIPTOR *Record),�����˼��������������
    4. �ڵ�3�����֮�󣬽ڵ�ȴ���һ����ʱ��2��ʱ�䣬��Э�������ͷ����ַ����֡������̵�ַ��
    5. �յ�Э��������Ӧ֮�󣬳����޸�MAC���PIB����֮�⣬��Ӧ�ö���صĵ�ַ״̬�Ƚ��д洢��
	6. FLASH�Ĳ����������FLASHMM.c�ļ���ͷ��
    7. ��ȡ�ڵ㳤��ַ�����ַ�������һ�ǽڵ�������֡��ʽ��װ����ַ�������У�Э������������֡�ϴ���
 *     ����ǽڵ㷢����ͨ���ݰ���Э�����������ݰ��󽫳���ַ�������뵽��̫�����ݰ����ϴ�����������
*/

extern BYTE Parameter[PARAMETER_NUM];
extern IO_INFO AD_Value[];
void PutOut(void);
void HardTimer_Init(void);
extern WORD data;//�̵���������־λ
void CircleTask(void)
{
	BYTE i;
    
	if(emCheckHardTimer(1))
	{
		emDelHardTimer(1);
//////////		InOut_Control();    //��Ϣ�ɼ��������߼�
//////////        PutOut();           //�̵�������
        SysRealTime();      //ϵͳʱ�����
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
    
    if(emCheckHardTimer(3))//������
	{
		emDelHardTimer(3);
		emTxMesg(TEM_SENT_REQ,RealTimeMesg,0,0);
		emStartHardTimer(3);
	}
	
	if(emCheckHardTimer(4))//����״̬ת��
	{
		emDelHardTimer(4);
		
		while (RF_TRX_TX == PHYGetTRxState());// �ȴ�����TX��״̬		
		//10����������һ��״̬ת����RX->IDLE��>RX
		//��״̬ת��ʱ��CC2500���Զ���Ƶ��У׼�ġ�����ʱ���Լ��800us��
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
//��һ·�̵�����Ȧ�ϵ�
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
	//�˿����ֻ�
	AD1PCFG=0xFFFF; //��ģ������ĳ�����I/O
	//Ӳ����ʼ��
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
    
	//�ⲿ�ж�1�������жϣ���ʼ��
//////	RPINR0bits.INT1R=10; //GDO0 ����ʱ��������
//////	LATBbits.LATB2=1; //LNA
//////	LATBbits.LATB15=0; //PA
    //�ⲿ�ж�1����ݮ�ɿ��Ź�����ʼ��
    TRISBbits.TRISB0=1;
    RPINR0bits.INT1R=0; //��ݮ�ɿ��Ź�
    
	IFS1bits.INT1IF=0;
	IEC1bits.INT1IE=1;//�ر������ж�
	IPC5bits.INT1IP=7;//������ȼ�Ϊ7�����εݼ�6ֱ��1
	INTCON2bits.INT1EP=1;//1Ϊ�½��أ�0Ϊ������
	
    

    //�ⲿ�ж�2����ݮ�ɿ����жϣ���ʼ��
    TRISBbits.TRISB1=1;
    RPINR1bits.INT2R=1;//ԭ���������ж�ӳ��RP10���ǵ��滻
	IFS1bits.INT2IF=0;
	IEC1bits.INT2IE=1;
	IPC7bits.INT2IP=6;//������ȼ�Ϊ7�����εݼ�6ֱ��1
	INTCON2bits.INT2EP=0;//1Ϊ�½��أ�0Ϊ������
    
	//��FLASH�ĸ�ʽ��
	//FlashFormatMem();
    
	//Э��ջ��ʼ��
	emSysInit();
//////    RFInitSetup();      //������ͨ������
//////	PHYInitSetup();  //������ͨ������
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
    emSetHardTimer(1,40);//��֪ѭ�����
  	emSetHardTimer(2,100);
    emSetHardTimer(3,Parameter[update]*6000);  //�¶ȸ�֪��ʱ�ϴ�ʱ������                  //������ 10min
  	emSetHardTimer(4,30000);//640s����һ��״̬ת������ֹ����ƫ��
    emSetHardTimer(5,1000);//��ݮ��ÿ��һ���͵�ƽ��PIC��������ͨ�ţ����2����û��⵽��PIC20Sǿ�ƶϵ硣
    emSetHardTimer(6,200);//����բʱ����2s;
    emSetHardTimer(7,50);//������0.5s�Զ��ָ��̵���
  	//start the timer, discard the returning value
    emSetHardTimer(8,500);//������
    
  	emStartHardTimer(1);
    emStartHardTimer(5);//������ݮ�ɿ��Ź�
    emStartHardTimer(8);
//////  	emStartHardTimer(2);
//////	    emStartHardTimer(3);
//////  	emStartHardTimer(4);  
}
