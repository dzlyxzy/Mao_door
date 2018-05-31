#include <p24FJ64GA002.h>
#include <stdlib.h>
#include "em16RTOS24.h"

unsigned char CurrentTaskId;// 当前运行任务的ID
unsigned char CurrentTaskPrio;//当前运行任务的优先级
unsigned char TaskStatusMask = 0; //任务状态掩码，每个位对应一个任务，当任务就绪时，对应位为1，
volatile unsigned int CurrentTaskPCL;
volatile unsigned int CurrentTaskPCH;
volatile unsigned int CurrentTaskSPTop;
volatile unsigned int CurrentTaskSPRoot;
volatile unsigned long  mSeconds=0;


TCB pTcb[TaskNum]; // tasknum 不能超过maxtasknum
MCB pMcb[TaskNum];
HardTimer pHt[MaxTimer];
LAYMESG pLayMesg[MaxLayMesg];

void emSysInit(void)
{
  int j;
  for(j=0;j<TaskNum;j++)
	{
	pTcb[j].TaskStatus=TASKNULL; //先将所有任务设为空，以备后面应用
    pTcb[j].TaskPrioty=15;
	}
	pMcb[15].MessageContext=NULLMESG;//将系统消息先设为空
  for(j=0;j<MaxLayMesg;j++)
   {
    pLayMesg[j].LayerChangMesg=NULLMESG;
    pLayMesg[j].Msg_Info_Ptr.Ptr=0;
    pLayMesg[j].Msg_Info_Ptr.cSize=0;
    pLayMesg[j].TaskQueue.TaskWaitId=0;
    pLayMesg[j].Flags.cVal=0;
   }
}

void ChangeStatus(unsigned char id)//测试用函数 用完删除
{
   pTcb[id].TaskStatus =READY; 
}

void CurrentTaskWait()
{
	pTcb[CurrentTaskId].TaskStatus = WAIT;
}

void SetTimerTick(unsigned int Tick)//Timer1做时钟节拍
{
	T1CON=0x00;
	T1CONbits.TCKPS=1;//分频
	TMR1=0x00;
	PR1 = Tick;//
	IPC0bits.T1IP = 0x07;//优先级7
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;
	T1CONbits.TON = 1;
}

void DisInt()// 禁止中断，进入临界区
{
     SRbits.IPL = 7;                 //禁止中断嵌套后IPL只读，写无效
          //CORCON中IPL3(只读)与SR中IPL3位组成CPU中断优先级，>=7禁止用户中断
     IEC0bits.U1RXIE=0;  
     IEC0bits.T1IE = 0;
}

void EnInt()//允许中断，退出临界区
{
	SRbits.IPL = 0;
     IEC0bits.U1RXIE=1;  
     IEC0bits.T1IE = 1;
}

void IdleTask()//默认的空闲任务
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
       Idtemp = Id; //这里留着可以设个断点 看是否分配成功
			 return ; //注意这里将来要完善
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
  ///以下为开机首次任务调度 
      
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

unsigned int FindMaxPrioTask() //在任务中调用
{
	unsigned char i = 0;
	unsigned char MaxPrioTaskId = 0;      // 这里可以把这两个临时变量去掉 直接用CurrentId来进入循环判断
	unsigned char MaxPrio = 16;               //优先级越高 值越小
   
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
    	    return 0;//返回值被放到w0
    	 }

    CurrentTaskId = MaxPrioTaskId;
    CurrentTaskPrio = MaxPrio;      
    CurrentTaskSPRoot = (unsigned int)pTcb[CurrentTaskId].TaskSP;
    CurrentTaskSPTop = (unsigned int)pTcb[CurrentTaskId].TaskSP+42;
    CurrentTaskPCL = pTcb[CurrentTaskId].TaskPCL;
    CurrentTaskPCH = pTcb[CurrentTaskId].TaskPCH; 
    return 1;
}

unsigned int IntFindMaxPrioTask() //在中断中调用
{
	unsigned char i = 0;
	unsigned char MaxPrioTaskId = 0;     // 这里可以把这两个临时变量去掉 直接用CurrentId来进入循环判断
	unsigned char MaxPrio = 16;//优先级越高 值越小
   
   MaxPrio = CurrentTaskPrio;
   MaxPrioTaskId = CurrentTaskId;
   // 接下来查找优先级最高的就绪任务
   //查找中断服务程序是否唤醒了比当前任务更高优先级的任务，是，跳转， 否 ，回到当前任务
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
    		return 0;//返回值被放到w0
    	}

    pTcb[CurrentTaskId].TaskStatus = READY; //这里是和任务中调用不同的地方 没有设当前任务为WAIT而是READY
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

unsigned char emSetHardTimer(unsigned char tid, unsigned int value)//设置定时器
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
		if(ttv<tv)//没有到设定的定时值,返回0
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
	for(i=0;i<15;i++)//将所有任务都变成READY，以便于所有任务都能接收此消息，前提是RTOS相当于任务15
	{
		if((pTcb[i].TaskStatus>1)&&(pTcb[i].TaskStatus<15))
			pTcb[i].TaskStatus=READY;
	}
	EnInt();
}

unsigned char emRxSysMesg(void)//用此函数接收系统消息
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

int emWaitMesg(unsigned char message,unsigned char state,unsigned char **ptr,unsigned char *size)//如果消息指向了某处，即消息结构体中的ptr值不为0，则将size字节的数据读取到形参ptr中。
{
    unsigned char i;
//	DisInt();
    for(i=0;i<MaxLayMesg;i++)
      {  
        //for messges in use
         if(pLayMesg[i].Flags.bits.bInUse==1&&pLayMesg[i].LayerChangMesg==message)
            {  
             if(pLayMesg[i].Flags.bits.bRealMesg)  //真正产生了消息了
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
                if(pLayMesg[i].Flags.bits.bRealTime!=1)  //不是实时信息，则当消息数为0时，要除去该消息；如果消息数不为0，则不除去该消息
                 {   
                   pLayMesg[i].Flags.bits.bMesgNum--; //已经处理了一个该消息
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
                      pLayMesg[i].Flags.bits.bWaitMesg=0;  //目前如果只能是每种消息只能是一个任务发送的话，就可以这样做，但是如果是每种消息可以由多个任务发送的话
                                                        //就要做相应的处理了，这里需要注意！
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
                   return 1;   //返回值为1，说明等到了消息，则进行相应的操作  
                }   
              else     //没有产生消息
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
             pLayMesg[i].Flags.bits.bRealMesg=0; //并不是真正的产生该消息了，而是创建了该消息。
             pLayMesg[i].Flags.bits.bMesgNum=0;
             pLayMesg[i].Msg_Info_Ptr.Ptr=0;
             pLayMesg[i].Msg_Info_Ptr.cSize=0;
            
             return -1;  //返回值为-1，说明没有等到消息，但是已经发送命令，待下次收到消息后即可执行相应的操作了。
           } 
     }

     return 0;   //没有空间才存储等待message的任务了，需要通知该任务，该任务等待消息失败了，不会被
                 //消息唤醒了，除非重新发送消息。
}

unsigned char emTxMesg(unsigned char message,unsigned char state,unsigned char *ptr,unsigned char size)//如果发送的消息带有数据，则数据存在于形参ptr中，此时，需要将该数据传递到消息结构体中，以便等待消息函数或者发送消息函数来读取。
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
          if(pLayMesg[i].Flags.bits.bRealTime)//如果是实时消息
             {
              pLayMesg[i].Flags.bits.bMesgNum=1;
               if(pLayMesg[i].Flags.bits.bWaitMesg==1)                
                  {
                   RxLayTaskId=pLayMesg[i].TaskQueue.TaskWaitId;
                   pTcb[RxLayTaskId].TaskStatus = READY;
                   return 1;
                  }
                else
                  return 0;  //虽然是实时性消息，但是没有任务在等待该实时性消息，所以返回错误。
             } 
           else  //如果是非实时性消息
             {
               pLayMesg[i].Flags.bits.bMesgNum++;
               if(pLayMesg[i].Flags.bits.bWaitMesg==1)
                 {
                   RxLayTaskId=pLayMesg[i].TaskQueue.TaskWaitId;
                   pTcb[RxLayTaskId].TaskStatus = READY;
                 }
                return 1;
             }   //else
         }  //如果消息存在 
      }
      if(state==1)
       {
        return 0;  //说明虽然是实时性消息，但是没有任务在等待该消息。
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
             if(pLayMesg[i].Flags.bits.bRealMesg)  //真正产生了消息了
              {
	              return i;
              } 
         }   
      }  
     return InvalidMesg;   //搜索的消息不存在
}

unsigned char emRxMesg(unsigned char Index,unsigned char state,unsigned char *ptr,unsigned char size)
{
	if((pLayMesg[Index].Flags.bits.bInUse==1)&&(pLayMesg[Index].Flags.bits.bRealMesg==1))
	{
		if(pLayMesg[Index].Flags.bits.bRealTime!=1)  //不是实时信息，则当消息数为0时，要除去该消息；如果消息数不为0，则不除去该消息
		{
			pLayMesg[Index].Flags.bits.bMesgNum--; //已经处理了一个该消息
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
