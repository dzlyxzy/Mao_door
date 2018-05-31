#include "spi.h"
WORD data=0x0000;
void SPIPut(BYTE cByte) //发送一个字节
{
    BYTE i;         
    SCLK=0;                  // 开始为低
    for(i=0;i<8;i++)         // 每次发送一位
    {
        if(cByte&0x80)       // 检测该位是1还是0
            SO=1;            // 如果为高，则输出高电平
        else
            SO=0;            // 如果为0，则输出低电平
        SCLK=1;              // 时钟信号为高
        Nop();               // 延时等待
        SCLK=0;            	 // 时钟信号为低
        cByte=cByte<<1;      // 移位，发送下一位数据
    }
} 

BYTE SPIGet(void) //接收一个字节
{
    BYTE i;         					// 循环变量
    BYTE cVal;    						// 返回值
    cVal=0;
    SCLK=0;                 	// 确保时钟信号为低
    for(i=0;i<8;i++)         	// 每次发送一位数据
    {
        cVal=cVal<<1;       	// 移位
        SCLK=1;               // 时钟信号为高
        if (SI==1)            // 检测总线为高电平还是低电平
            cVal |= 0x01;     // 如果为高电平，进行相应位置1
        else
            cVal &= 0xFE;    	//如果为低电平，相应位置0
        SCLK = 0;             // 时钟信号拉低
    }
	
    return cVal;
}

void HC595Put(WORD cByte) //写入一个字
{
    BYTE temp;
    temp = CS;
    CS=1;
    BYTE i;         
    for(i=0;i<16;i++)         // 每次发送一位
    {
        if(cByte&0x8000)       // 检测该位是1还是0
            DS=1;            // 如果为高，则移入高电平
        else
            DS=0;            // 如果为0，则移入低电平
        SO=0;
        Nop();               // 延时等待 
        Nop(); 
        SO=1;
        cByte=cByte<<1;      // 移位，发送下一位数据
    } 
    CS = temp;
} 

void HC595Out(void)
{
    HCK=0; 
    Nop(); 
    Nop(); 
    HCK=1; 
}

