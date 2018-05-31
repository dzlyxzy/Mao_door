#include "Interface.h"

/*********************************************************
�� �� ����crcCalc
��    �ܣ�CRC16λУ��
˵    ����add() calc()Ϊ�Ӻ���
�� �� ֵ�����ֽ�crch  ���ֽ�crcl      
***********************************************************/
CRC_RESULT CrcCalc(const BYTE *pBuf,WORD nSize)
{
	WORD CrcReg,nTemp;
	CRC_RESULT Result;
	WORD i,j;
	CrcReg=0xFFFF;
	for(i=0;i<nSize;i++)
	{
		CrcReg^=pBuf[i];
		for(j=0;j<8;j++)
		{
			nTemp=CrcReg&0x0001;
			CrcReg=CrcReg>>1;
			if(nTemp==0x0001)
				CrcReg^=0xA001;
		}	
	}
	Result.nVal=CrcReg;
	return Result;	
}

//ָ������
WORD CalcExponent(BYTE num,BYTE exp)
{
	WORD i,Result;;
	Result=1;
	for(i=0;i<exp;i++)
	{
		Result=Result*num;
	}
	return Result;
}

//���ֵ
/*WORD GetRandom(WORD rnd)
{
	BYTE bStr;
	WORD nVal;
	bStr=rnd&0x0F;
	nVal=bStr;
	bStr=(rnd>>8)&0x0F;
	nVal<<=4;
	nVal|=bStr;
	nVal=nVal*(rnd&0x000F);
	return nVal;
}*/
