#include "spi.h"
WORD data=0x0000;
void SPIPut(BYTE cByte) //����һ���ֽ�
{
    BYTE i;         
    SCLK=0;                  // ��ʼΪ��
    for(i=0;i<8;i++)         // ÿ�η���һλ
    {
        if(cByte&0x80)       // ����λ��1����0
            SO=1;            // ���Ϊ�ߣ�������ߵ�ƽ
        else
            SO=0;            // ���Ϊ0��������͵�ƽ
        SCLK=1;              // ʱ���ź�Ϊ��
        Nop();               // ��ʱ�ȴ�
        SCLK=0;            	 // ʱ���ź�Ϊ��
        cByte=cByte<<1;      // ��λ��������һλ����
    }
} 

BYTE SPIGet(void) //����һ���ֽ�
{
    BYTE i;         					// ѭ������
    BYTE cVal;    						// ����ֵ
    cVal=0;
    SCLK=0;                 	// ȷ��ʱ���ź�Ϊ��
    for(i=0;i<8;i++)         	// ÿ�η���һλ����
    {
        cVal=cVal<<1;       	// ��λ
        SCLK=1;               // ʱ���ź�Ϊ��
        if (SI==1)            // �������Ϊ�ߵ�ƽ���ǵ͵�ƽ
            cVal |= 0x01;     // ���Ϊ�ߵ�ƽ��������Ӧλ��1
        else
            cVal &= 0xFE;    	//���Ϊ�͵�ƽ����Ӧλ��0
        SCLK = 0;             // ʱ���ź�����
    }
	
    return cVal;
}

void HC595Put(WORD cByte) //д��һ����
{
    BYTE temp;
    temp = CS;
    CS=1;
    BYTE i;         
    for(i=0;i<16;i++)         // ÿ�η���һλ
    {
        if(cByte&0x8000)       // ����λ��1����0
            DS=1;            // ���Ϊ�ߣ�������ߵ�ƽ
        else
            DS=0;            // ���Ϊ0��������͵�ƽ
        SO=0;
        Nop();               // ��ʱ�ȴ� 
        Nop(); 
        SO=1;
        cByte=cByte<<1;      // ��λ��������һλ����
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

