#include "SysTask.h"

BYTE RFSysError=0;

void SYSTask(void)
{
	BYTE i;
	i=emWaitMesg(RF_OVERFLOW_SYS_EROR,RealTimeMesg,0,0);
	if(i==1)
	{
		RFSysError++;
		if(RFSysError==2)
		{
			MACFlushTxFrame();
			RFSysError=0;
		}
	}
	CurrentTaskWait();
	SchedTask();
}
