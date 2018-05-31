
#ifndef _RTOS_H
#define _RTOS_H
#include <stdio.h>
#define true 0x01
#define false 0x00
#define MaxTaskNum        256
#define TaskNum               16 //���������
#define MaxTimer               16
#define MaxLayMesg      30  //������Ϣ����
#define HARD_TIMER_NULL 7
#define HARD_TIMER_WORK 6

#define RUNNING			  00			//����״̬
#define READY			  01	
#define WAIT		              02	   		//����Ϣ���¼�
#define DELAY			  03			//׼ȷ��ʱ
#define TERMINATED  	  04			//��ʱ�������״̬
#define FINISHED		        05
#define SUPEND			  06			//���ж�ʱ
#define TASKNULL		15	/*��ʾ������û����*/

#define NULLMESG         0
#define InvalidMesg		0xFF
//#define NULLTASK         0

#define RealTimeMesg    1
#define NRealTimeMesg   0

#define RecACKMesg  0x20  //���յ�ACK��Ϣ��


typedef struct TaskSoftStack
 { 	 
 	  unsigned int  TaskPCL;
 	  unsigned int  TaskPCH;
 	  unsigned int TaskPSVPAG;
 	  unsigned int TaskRCOUNT;
 	  unsigned int TaskCORCON;
 	  unsigned int TaskTBLPAG;
 	  
 	  unsigned int TaskW0;
 	  unsigned int TaskW1;
 	  unsigned int TaskW2;
 	  unsigned int TaskW3;
 	  unsigned int TaskW4;
 	  unsigned int TaskW5;
 	  unsigned int TaskW6;
 	  unsigned int TaskW7;
 	  unsigned int TaskW8;
 	  unsigned int TaskW9;
 	  unsigned int TaskW10;
 	  unsigned int TaskW11;
 	  unsigned int TaskW12;
 	  unsigned int TaskW13;	
 	  unsigned int TaskW14; 
  }TaskStack;

typedef struct TaskControlBlock
{
	TaskStack * TaskSP;
      unsigned char TaskId;   	
	unsigned char TaskStatus;
	unsigned char TaskPrioty;	
      unsigned int TaskPCL;
      unsigned int TaskPCH;  
}TCB;
   
typedef struct SystemMessage                 //ϵͳ��Ϣ
{
	unsigned char TxMessageTaskId;
	unsigned char MessageContext;
}MCB;

typedef struct TaskforMsgQueue
{
    unsigned char TaskWaitId;
	unsigned char TaskPrioty;
}TaskMsgQueue;

typedef struct _MSG_INFO_PTR
{
	unsigned char *Ptr;
	unsigned char cSize;
}MSG_INFO_PTR;

typedef struct SysLayerMesg      //�㽻����Ϣ
{
    unsigned char LayerChangMesg;
    TaskMsgQueue TaskQueue;
    MSG_INFO_PTR Msg_Info_Ptr;
    union
     {
       unsigned char cVal;
       struct
         {
           unsigned char bInUse:1;
           unsigned char bRealTime:1;
           unsigned char bWaitMesg:1;
           unsigned char bMesgNum:4;
           unsigned char bRealMesg:1;
         }bits;
     }Flags;
    
}LAYMESG;

typedef struct{						
	unsigned char Ht_value:5;					
	unsigned char Flg:3;				
	} emHardtStatus;

typedef struct{
	union{
		emHardtStatus IDtimer;
		unsigned char bIDtimer;
	}htstatus;
	unsigned int Lt_value;
	unsigned long temp;
}HardTimer;


void CreateTask(void   (* TaskName)(), unsigned char Id,  unsigned char Prio,unsigned char stat);
void StartTask();			//��ʼ�����񻷾����������������ĵ�һ�ε���
void IdleTask();			//Ĭ�ϵĿ�������
void EnInt();				 //�˳��ٽ���
void DisInt();				//�����ٽ���
unsigned int FindMaxPrioTask();
extern void SchedTask();		//������ĩβ����
void CurrentTaskWait();		//g����ǰ����
unsigned char TxMessage(unsigned char RxMesgTaskId,unsigned char message);
unsigned char RxMessage(unsigned char message);
void SetTimerTick(unsigned int Tick); 		//Tick Ϊʱ������

void ChangeStatus(unsigned char id);			//�����ú���

unsigned char emSetHardTimer(unsigned char tid, unsigned int value);
unsigned char emStartHardTimer(unsigned char tid);
unsigned char emCheckHardTimer(unsigned char tid);
void emDelHardTimer(unsigned char tid);
void emTxSysMesg(unsigned char message);
unsigned char emRxSysMesg(void);
void emDelSysMesg(void);
void emSysInit(void);
int emWaitMesg(unsigned char message,unsigned char state,unsigned char **ptr,unsigned char *size);
unsigned char emTxMesg(unsigned char message,unsigned char state,unsigned char *ptr,unsigned char size);
unsigned char emSearchMesg(unsigned char message);
unsigned char emRxMesg(unsigned char Index,unsigned char state,unsigned char *ptr,unsigned char size);
void emDelMesg(unsigned char message);
#endif







