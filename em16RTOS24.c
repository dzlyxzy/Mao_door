#include <p24FJ64GA002.h>
#include <stdlib.h>
#include "em16RTOS24.h"

unsigned char CurrentTaskId;// ��ǰ���������ID
unsigned char CurrentTaskPrio;//��ǰ������������ȼ�
unsigned char TaskStatusMask = 0; //����״̬���룬ÿ��λ��Ӧһ�����񣬵��������ʱ����ӦλΪ1��
volatile unsigned int CurrentTaskPCL;
volatile unsigned int CurrentTaskPCH;
volatile unsigned int CurrentTaskSPTop;
volatile unsigned int CurrentTaskSPRoot;
volatile unsigned long  mSeconds=0;


TCB pTcb[TaskNum]; // tasknum ���ܳ���maxtasknum
MCB pMcb[TaskNum];
HardTimer pHt[MaxTimer];
LAYMESG pLayMesg[MaxLayMesg];

void emSysInit(void)
{
  int j;
  for(j=0;j<TaskNum;j++)
	{
	pTcb[j].TaskStatus=TASKNULL; //�Ƚ�����������Ϊ�գ��Ա�����Ӧ��
    pTcb[j].TaskPrioty=15;
	}
	pMcb[15].MessageContext=NULLMESG;//��ϵͳ��Ϣ����Ϊ��
  for(j=0;j<MaxLayMesg;j++)
   {
    pLayMesg[j].LayerChangMesg=NULLMESG;
    pLayMesg[j].Msg_Info_Ptr.Ptr=0;
    pLayMesg[j].Msg_Info_Ptr.cSize=0;
    pLayMesg[j].TaskQueue.TaskWaitId=0;
    pLayMesg[j].Flags.cVal=0;
   }
}

void ChangeStatus(unsigned char id)//�����ú��� ����ɾ��
{
   pTcb[id].TaskStatus =READY; 
}

void CurrentTaskWait()
{
	pTcb[CurrentTaskId].TaskStatus = WAIT;
}

void SetTimerTick(unsigned int Tick)//Timer1��ʱ�ӽ���
{
	T1CON=0x00;
	T1CONbits.TCKPS=1;//��Ƶ
	TMR1=0x00;
	PR1 = Tick;//
	IPC0bits.T1IP = 0x07;//���ȼ�7
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;
	T1CONbits.TON = 1;
}

void DisInt()// ��ֹ�жϣ������ٽ���
{
     SRbits.IPL = 7;                 //��ֹ�ж�Ƕ�׺�IPLֻ����д��Ч
          //CORCON��IPL3(ֻ��)��SR��IPL3λ���CPU�ж����ȼ���>=7��ֹ�û��ж�
     IEC0bits.U1RXIE=0;  
     IEC0bits.T1IE = 0;
}

void EnInt()//�����жϣ��˳��ٽ���
{
	SRbits.IPL = 0;
     IEC0bits.U1RXIE=1;  
     IEC0bits.T1IE = 1;
}

void IdleTask()//Ĭ�ϵĿ�������
{
	while(1)
	{
		Nop();
	}
}

void CreateTask(void (* TaskName)(), unsigned char Id, unsigned char Prio, unsigned char stat)
{
	TaskStack * stacktemp;
	unsigned int Idtemp = Id;
	unsigned long tempTaskPC = 0;	
	unsigned int tempPCL = 0;
	unsigned int tempPCH = 0;
    unsigned int TestName;
    TestName=TaskName;
	tempTaskPC = (unsigned long) TestName;
	tempPCL = tempTaskPC;
	tempPCH = tempTaskPC >> 16;
	  
	stacktemp = (TaskStack *) malloc(sizeof(TaskStack));
	if(NULL == stacktemp)
		{
       Idtemp = Id; //�������ſ�������ϵ� ���Ƿ����ɹ�
			 return ; //ע�����ｫ��Ҫ����
		}
			
	stacktemp->TaskPCL = tempPCL;
	stacktemp->TaskPCH = tempPCH;
	stacktemp->TaskPSVPAG = 0;
	stacktemp->TaskRCOUNT = 0;
	stacktemp->TaskCORCON =	0;
	stacktemp->TaskTBLPAG = 0;
	stacktemp->TaskW0 = 0;
	stacktemp->TaskW1 = 0;
	stacktemp->TaskW2 = 0;
	stacktemp->TaskW3 = 0;
	stacktemp->TaskW4 = 0;
	stacktemp->TaskW5 = 0;
	stacktemp->TaskW6 = 0;
	stacktemp->TaskW7 = 0;
	stacktemp->TaskW8 = 0;
	stacktemp->TaskW9 = 0;
	stacktemp->TaskW10 = 0;
	stacktemp->TaskW11 = 0;
	stacktemp->TaskW12 = 0;
	stacktemp->TaskW13 = 0;	
	stacktemp->TaskW14 = 0;
		
	pTcb[Idtemp].TaskSP = (unsigned int) stacktemp;
	pTcb[Idtemp].TaskId = Id;
	pTcb[Idtemp].TaskStatus = stat;
	pTcb[Idtemp].TaskPrioty = Prio;
	pTcb[Idtemp].TaskPCL = tempPCL;
	pTcb[Idtemp].TaskPCH = tempPCH;		 
}

void StartTask()
{
  unsigned char i = 0;
  unsigned char MaxPrioTaskId = 0;
  unsigned char MaxPrio = 16;
  DisInt();
  for(i=0; i<TaskNum; i++)
  {
  	if ( READY == pTcb[i].TaskStatus && pTcb[i].TaskPrioty < MaxPrio )
  		{
  			MaxPrio = pTcb[i].TaskPrioty;
  			MaxPrioTaskId = i;
  		}
  }
  ///����Ϊ�����״�������� 
      
     CurrentTaskId = MaxPrioTaskId;
     CurrentTaskPrio = MaxPrio;
     CurrentTaskSPRoot = (unsigned int) pTcb[CurrentTaskId].TaskSP;
     CurrentTaskSPTop = (unsigned int) pTcb[CurrentTaskId].TaskSP+42;//0x2C;
     CurrentTaskPCL = pTcb[CurrentTaskId].TaskPCL;
     CurrentTaskPCH = pTcb[CurrentTaskId].TaskPCH;
     
     EnInt();
     asm volatile ("push _CurrentTaskPCL");
     asm volatile ("push _CurrentTaskPCH");
     asm volatile ("return"); 
}

unsigned int FindMaxPrioTask() //�������е���
{
	unsigned char i = 0;
	unsigned char MaxPrioTaskId = 0;      // ������԰���������ʱ����ȥ�� ֱ����CurrentId������ѭ���ж�
	unsigned char MaxPrio = 16;               //���ȼ�Խ�� ֵԽС
   
   	for(i=0; i<TaskNum; i++)
     {
    	if ( READY == pTcb[i].TaskStatus && pTcb[i].TaskPrioty < MaxPrio )
  	     {
  			MaxPrio = pTcb[i].TaskPrioty;
  			MaxPrioTaskId = i;
  	     }
     }
    if(CurrentTaskId == MaxPrioTaskId)
    	 {
    	    return 0;//����ֵ���ŵ�w0
    	 }

    CurrentTaskId = MaxPrioTaskId;
    CurrentTaskPrio = MaxPrio;      
    CurrentTaskSPRoot = (unsigned int)pTcb[CurrentTaskId].TaskSP;
    CurrentTaskSPTop = (unsigned int)pTcb[CurrentTaskId].TaskSP+42;
    CurrentTaskPCL = pTcb[CurrentTaskId].TaskPCL;
    CurrentTaskPCH = pTcb[CurrentTaskId].TaskPCH; 
    return 1;
}

unsigned int IntFindMaxPrioTask() //���ж��е���
{
	unsigned char i = 0;
	unsigned char MaxPrioTaskId = 0;     // ������԰���������ʱ����ȥ�� ֱ����CurrentId������ѭ���ж�
	unsigned char MaxPrio = 16;//���ȼ�Խ�� ֵԽС
   
   MaxPrio = CurrentTaskPrio;
   MaxPrioTaskId = CurrentTaskId;
   // �������������ȼ���ߵľ�������
   //�����жϷ�������Ƿ����˱ȵ�ǰ����������ȼ��������ǣ���ת�� �� ���ص���ǰ����
	for(i=0; i<TaskNum; i++)
     {
     	if ( READY == pTcb[i].TaskStatus && pTcb[i].TaskPrioty < MaxPrio )
  		{
  			MaxPrio = pTcb[i].TaskPrioty;
  			MaxPrioTaskId = i;
  		}
     }
   if(CurrentTaskId == MaxPrioTaskId)
    	{
    		return 0;//����ֵ���ŵ�w0
    	}

    pTcb[CurrentTaskId].TaskStatus = READY; //�����Ǻ������е��ò�ͬ�ĵط� û���赱ǰ����ΪWAIT����READY
    CurrentTaskId = MaxPrioTaskId;
    CurrentTaskPrio = MaxPrio;   
    CurrentTaskSPRoot = (unsigned int)pTcb[CurrentTaskId].TaskSP;
    CurrentTaskSPTop = (unsigned int)pTcb[CurrentTaskId].TaskSP+42;
    CurrentTaskPCL = pTcb[CurrentTaskId].TaskPCL;
    CurrentTaskPCH = pTcb[CurrentTaskId].TaskPCH; 
    return 1;
}


unsigned char TxMessage(unsigned char RxMesgTaskId,unsigned char message)
{    
     if(pMcb[RxMesgTaskId].MessageContext!=NULLMESG)
         return 0;
	 //pMcb[RxMesgTaskId].TxMessageTaskId = CurrentTaskId;
	 pMcb[RxMesgTaskId].MessageContext = message;
	 pTcb[RxMesgTaskId].TaskStatus = READY;
	    return  1;
}


unsigned char RxMessage(unsigned char message)
{
	if(message == pMcb[CurrentTaskId].MessageContext)
     {
	    pMcb[CurrentTaskId].MessageContext=NULLMESG;
        return 1;  
     }
	 else
	 	 return 0;
}

unsigned char emSetHardTimer(unsigned char tid, unsigned int value)//���ö�ʱ��
{
	pHt[tid].htstatus.IDtimer.Flg=HARD_TIMER_NULL;
	pHt[tid].Lt_value=value;
	return 1;
}

unsigned char emStartHardTimer(unsigned char tid)
{
	if(pHt[tid].htstatus.IDtimer.Flg!=HARD_TIMER_NULL)
		return 0;
	pHt[tid].htstatus.IDtimer.Flg=HARD_TIMER_WORK;
	pHt[tid].temp=mSeconds;
	return 1;
}

unsigned char emCheckHardTimer(unsigned char tid)
{
	unsigned long tv,ttv;

    if(pHt[tid].htstatus.IDtimer.Flg!=HARD_TIMER_WORK)
       return 0;
	tv=pHt[tid].Lt_value;
	if(mSeconds<pHt[tid].temp)//mSecond overflow
	{
		ttv=4294967295-pHt[tid].temp+mSeconds;	//4294967296
		if(ttv<tv)//û�е��趨�Ķ�ʱֵ,����0
			return 0;
		if(ttv>tv)
		{
			pHt[tid].temp=mSeconds;
			return 5;//pHt[tid].htstatus.IDtimer.Flg=5;
		}
		else 
		{		
			pHt[tid].temp=mSeconds;
			return 1;
		}
	}
	else
	{
		ttv=mSeconds-pHt[tid].temp;
		if(ttv>tv)
		{
			pHt[tid].temp=mSeconds;
			return 5;
		}
		 if(ttv<tv)
			return 0;
		 else 
		{
			pHt[tid].temp=mSeconds;
			return 1;
		}
	}

}

void emDelHardTimer(unsigned char tid)
{
	pHt[tid].htstatus.IDtimer.Flg=HARD_TIMER_NULL;
}


void emTxSysMesg(unsigned char message)
{
	unsigned char i;
	DisInt();
	pMcb[15].MessageContext=message;
	for(i=0;i<15;i++)//���������񶼱��READY���Ա������������ܽ��մ���Ϣ��ǰ����RTOS�൱������15
	{
		if((pTcb[i].TaskStatus>1)&&(pTcb[i].TaskStatus<15))
			pTcb[i].TaskStatus=READY;
	}
	EnInt();
}

unsigned char emRxSysMesg(void)//�ô˺�������ϵͳ��Ϣ
{
	unsigned char i;
	if(pMcb[15].MessageContext==NULLMESG)
		return 0;
	i=pMcb[15].MessageContext;
	return i;
}
void emDelSysMesg(void)
{
    if(pMcb[15].MessageContext!=NULLMESG)
           pMcb[15].MessageContext=NULLMESG;
}

int emWaitMesg(unsigned char message,unsigned char state,unsigned char **ptr,unsigned char *size)//�����Ϣָ����ĳ��������Ϣ�ṹ���е�ptrֵ��Ϊ0����size�ֽڵ����ݶ�ȡ���β�ptr�С�
{
    unsigned char i;
//	DisInt();
    for(i=0;i<MaxLayMesg;i++)
      {  
        //for messges in use
         if(pLayMesg[i].Flags.bits.bInUse==1&&pLayMesg[i].LayerChangMesg==message)
            {  
             if(pLayMesg[i].Flags.bits.bRealMesg)  //������������Ϣ��
              {  
                if(pLayMesg[i].Msg_Info_Ptr.cSize!=0)
                   {
                     *ptr=pLayMesg[i].Msg_Info_Ptr.Ptr;
                     *size=pLayMesg[i].Msg_Info_Ptr.cSize;
                   }
                else 
                   {
                    ptr=0;
                    size=0;
                   } 
                if(pLayMesg[i].Flags.bits.bRealTime!=1)  //����ʵʱ��Ϣ������Ϣ��Ϊ0ʱ��Ҫ��ȥ����Ϣ�������Ϣ����Ϊ0���򲻳�ȥ����Ϣ
                 {   
                   pLayMesg[i].Flags.bits.bMesgNum--; //�Ѿ�������һ������Ϣ
                   if(pLayMesg[i].Flags.bits.bMesgNum==0)
                    {
                      pLayMesg[i].LayerChangMesg=NULLMESG;
                      pLayMesg[i].TaskQueue.TaskWaitId=0;
                      pLayMesg[i].Msg_Info_Ptr.Ptr=0;
                      pLayMesg[i].Msg_Info_Ptr.cSize=0;
                      pLayMesg[i].Flags.cVal=0;
                    } 
                   else
                    {
                      pLayMesg[i].Flags.bits.bWaitMesg=0;  //Ŀǰ���ֻ����ÿ����Ϣֻ����һ�������͵Ļ����Ϳ��������������������ÿ����Ϣ�����ɶ�������͵Ļ�
                                                        //��Ҫ����Ӧ�Ĵ����ˣ�������Ҫע�⣡
                      pLayMesg[i].Flags.bits.bRealTime=0;
                      pLayMesg[i].TaskQueue.TaskWaitId=0;
                      pLayMesg[i].Msg_Info_Ptr.Ptr=0;
                      pLayMesg[i].Msg_Info_Ptr.cSize=0;
                      pLayMesg[i].Flags.bits.bRealMesg=1;
                     }
                 }  

                else  
                   pLayMesg[i].Flags.bits.bRealMesg=0;
                   pLayMesg[i].Msg_Info_Ptr.Ptr=0;
                   pLayMesg[i].Msg_Info_Ptr.cSize=0;
                   return 1;   //����ֵΪ1��˵���ȵ�����Ϣ���������Ӧ�Ĳ���  
                }   
              else     //û�в�����Ϣ
                 return -1;    
              } 
       }  
     for(i=0;i<MaxLayMesg;i++)
     {
       //for messages not using
        if(pLayMesg[i].Flags.bits.bInUse==0)
           {
             if(state==RealTimeMesg)
               pLayMesg[i].Flags.bits.bRealTime=1;
              else
               pLayMesg[i].Flags.bits.bRealTime=0;

             pLayMesg[i].LayerChangMesg=message;
             pLayMesg[i].TaskQueue.TaskWaitId=CurrentTaskId;
             pLayMesg[i].TaskQueue.TaskPrioty=CurrentTaskPrio;
             pLayMesg[i].Flags.bits.bInUse=1;
             pLayMesg[i].Flags.bits.bWaitMesg=1;
             pLayMesg[i].Flags.bits.bRealMesg=0; //�����������Ĳ�������Ϣ�ˣ����Ǵ����˸���Ϣ��
             pLayMesg[i].Flags.bits.bMesgNum=0;
             pLayMesg[i].Msg_Info_Ptr.Ptr=0;
             pLayMesg[i].Msg_Info_Ptr.cSize=0;
            
             return -1;  //����ֵΪ-1��˵��û�еȵ���Ϣ�������Ѿ�����������´��յ���Ϣ�󼴿�ִ����Ӧ�Ĳ����ˡ�
           } 
     }

     return 0;   //û�пռ�Ŵ洢�ȴ�message�������ˣ���Ҫ֪ͨ�����񣬸�����ȴ���Ϣʧ���ˣ����ᱻ
                 //��Ϣ�����ˣ��������·�����Ϣ��
}

unsigned char emTxMesg(unsigned char message,unsigned char state,unsigned char *ptr,unsigned char size)//������͵���Ϣ�������ݣ������ݴ������β�ptr�У���ʱ����Ҫ�������ݴ��ݵ���Ϣ�ṹ���У��Ա�ȴ���Ϣ�������߷�����Ϣ��������ȡ��
{
   	unsigned char i;
    unsigned char RxLayTaskId;
	//DisInt();
    for(i=0;i<MaxLayMesg;i++)
      {
       if(pLayMesg[i].Flags.bits.bInUse==1&&pLayMesg[i].LayerChangMesg==message)
        {
          if(size!=0)
            {
              pLayMesg[i].Msg_Info_Ptr.Ptr=ptr;
              pLayMesg[i].Msg_Info_Ptr.cSize=size;
            }
          pLayMesg[i].Flags.bits.bRealMesg=1;
          if(pLayMesg[i].Flags.bits.bRealTime)//�����ʵʱ��Ϣ
             {
              pLayMesg[i].Flags.bits.bMesgNum=1;
               if(pLayMesg[i].Flags.bits.bWaitMesg==1)                
                  {
                   RxLayTaskId=pLayMesg[i].TaskQueue.TaskWaitId;
                   pTcb[RxLayTaskId].TaskStatus = READY;
                   return 1;
                  }
                else
                  return 0;  //��Ȼ��ʵʱ����Ϣ������û�������ڵȴ���ʵʱ����Ϣ�����Է��ش���
             } 
           else  //����Ƿ�ʵʱ����Ϣ
             {
               pLayMesg[i].Flags.bits.bMesgNum++;
               if(pLayMesg[i].Flags.bits.bWaitMesg==1)
                 {
                   RxLayTaskId=pLayMesg[i].TaskQueue.TaskWaitId;
                   pTcb[RxLayTaskId].TaskStatus = READY;
                 }
                return 1;
             }   //else
         }  //�����Ϣ���� 
      }
      if(state==1)
       {
        return 0;  //˵����Ȼ��ʵʱ����Ϣ������û�������ڵȴ�����Ϣ��
       }
     for(i=0;i<MaxLayMesg;i++)
      {
        if(pLayMesg[i].Flags.bits.bInUse==0)
           {
            pLayMesg[i].Flags.bits.bInUse=1;
            pLayMesg[i].LayerChangMesg=message;
            pLayMesg[i].Flags.bits.bMesgNum=0;
            pLayMesg[i].Flags.bits.bMesgNum++;
            pLayMesg[i].Flags.bits.bRealMesg=1;
             if(!size)
            {
              pLayMesg[i].Msg_Info_Ptr.Ptr=ptr;
              pLayMesg[i].Msg_Info_Ptr.cSize=size;
            }
            return 1;
           } 
      }
     //EnInt();
	 return 0;
}

unsigned char emSearchMesg(unsigned char message)
{
    unsigned char i;
    for(i=0;i<MaxLayMesg;i++)
      {  
         if(pLayMesg[i].Flags.bits.bInUse==1&&pLayMesg[i].LayerChangMesg==message)
         {  
             if(pLayMesg[i].Flags.bits.bRealMesg)  //������������Ϣ��
              {
	              return i;
              } 
         }   
      }  
     return InvalidMesg;   //��������Ϣ������
}

unsigned char emRxMesg(unsigned char Index,unsigned char state,unsigned char *ptr,unsigned char size)
{
	if((pLayMesg[Index].Flags.bits.bInUse==1)&&(pLayMesg[Index].Flags.bits.bRealMesg==1))
	{
		if(pLayMesg[Index].Flags.bits.bRealTime!=1)  //����ʵʱ��Ϣ������Ϣ��Ϊ0ʱ��Ҫ��ȥ����Ϣ�������Ϣ����Ϊ0���򲻳�ȥ����Ϣ
		{
			pLayMesg[Index].Flags.bits.bMesgNum--; //�Ѿ�������һ������Ϣ
			if(pLayMesg[Index].Flags.bits.bMesgNum==0)
			{
				pLayMesg[Index].LayerChangMesg=NULLMESG;
				pLayMesg[Index].TaskQueue.TaskWaitId=0;
				pLayMesg[Index].Msg_Info_Ptr.cSize=0;
				pLayMesg[Index].Flags.cVal=0;
			}
			else
			{
				pLayMesg[Index].Flags.bits.bRealMesg=1;
				pLayMesg[Index].Flags.bits.bWaitMesg=0;
				pLayMesg[Index].Flags.bits.bRealTime=0;
				pLayMesg[Index].TaskQueue.TaskWaitId=0;
				pLayMesg[Index].Msg_Info_Ptr.Ptr=0;
				pLayMesg[Index].Msg_Info_Ptr.cSize=0;
			}
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}


void emDelMesg(unsigned char message)
{
   unsigned char i;
   for(i=0;i<MaxLayMesg;i++)
  {
	  if(pLayMesg[i].Flags.bits.bInUse==1&&pLayMesg[i].LayerChangMesg==message)
      {
	      pLayMesg[i].LayerChangMesg=NULLMESG;
          pLayMesg[i].TaskQueue.TaskWaitId=0;
          pLayMesg[i].Msg_Info_Ptr.Ptr=0;
          pLayMesg[i].Msg_Info_Ptr.cSize=0;
          pLayMesg[i].Flags.cVal=0;
       }       
  } 
}
