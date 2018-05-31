#include "common.h"


void Delay(WORD t) //延时时间t*2+5us
{
	WORD i;
	for(i=t;i>0;i--)
	{
		Delay_2us();
		ClrWdt();
	}
		
	for(i=1;i>0;i--);
		Nop();
}

/*************************************************
//1、链接指针lnk，1个周期 
//2、 1个周期nop
//3、1个周期， ulnk，释放堆栈
//4、3个周期 return，从函数中返回
**************************************************/

void Delay_2us(void) //1/(8MHZ/2)=0.25us
{
	Nop();
	Nop();
	Nop();
}



