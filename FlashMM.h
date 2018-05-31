#ifndef _FLASH_MM_H
#define _FLASH_MM_H

#include "common.h"
#include "flash.h"

/*
Flash�ܹ���Ϊ4��
��һ�飺0X8C00~0X8FFF
�ڶ��飺0X9000~0X93FF�����У�0X9000 = 0X8C00+8*128��һ����8ҳ��һҳ��128���ֽڣ�
�����飺0X9400~0X97FF
���Ŀ飺0X9800~0X9C00
��������
*/

//����洢������ʼ�����ַ����ʼ����ҳ
#define ConstPhyBeginAddr	0x008C00 				//����FLASH����ʼ��ַ


//����洢����ҳ����������ÿҳ��С
#define ConstBlockSize		8       //������С��ÿ�������ҳ��	
#define ConstTotalPhyPages	48	    //�ܹ�ҳ��,6��
#define ConstPageSize		128     //ÿҳ��С�����ֽ�������

//ȷ����ʼ����,����ʵ���Ͼ���ҳ
#define ConstPhyBeginSector	(ConstPhyBeginAddr/ConstPageSize)  //�����ǰ����ֽ�����������
#define ConstDataBeginSector (ConstPhyBeginSector+ConstBlockSize)

#define ConstWordPageSize   (ConstPageSize/2)     //ÿҳ��С�������������㣬������ĺ궨��һ��������
#define ConstDataPageSize	(ConstWordPageSize-4) //��128���ֽ��е����8���ֽ��ó����洢CRCУ���״̬���߼���

//���廵�����Ĵ�С
#define ConstBadSectorSize	(ConstTotalPhyPages-ConstBlockSize)		//���Ϊ����������С��������ĺ궨��һ��������
//��������ҳ�����٣��洢��ҳ��ȥ����������ҳ��
#define ConstDataSectorSize		(ConstTotalPhyPages-ConstBlockSize) //������������С������ĺ궨��һ��������

//����״̬�洢ƫ��

#define ConstStatusOffSet	(ConstWordPageSize-2) //ƫ�����ǰ�����������ģ�������ĺ궨��һ�������ţ�ĿǰΪ62
#define ConstCheckSumOffSet	(ConstWordPageSize-3)	//����洢CRCУ��͵�λ��
#define ConstLogicOffSet	(ConstWordPageSize-1) //ÿҳ�д洢�߼���ַ��ƫ�����������������㣬ĿǰΪ63

//�����
#define MEM_NULL	   0xFFFF //����Ϊ��

//�����ڴ˺����е��õĳ��ȼ��㺯��
#define lengthof(a)		(sizeof(a)/2)		//���㳤��

//����ÿҳ����״̬,״̬���α�Ǩ
#define FREE    0x0F //�����ǿ��е�
#define INVALID 0x0E //��������Ч��
#define INUSE	0x0C //����������ʹ�õ�
#define DIRTY	0x08 //�����ǿ��Բ�����


//��������FLASH��Ϣ
typedef struct _FLASH_INFO
{
	WORD  Version;	   //�汾��
	WORD  EraseNumber; //��������
	WORD  Identifier;  //�û���ʾ
	WORD  BadSector[ConstDataSectorSize/sizeof(WORD)+1];//��Ч������ÿλ����һ������
}FLASH_INFO;

//����ӳ���
typedef struct _MEM_MAP_INFO
{
	WORD PhySector[ConstDataSectorSize]; //��������,�߼������������±����
	WORD FreeNumber;
	WORD DirtyNumber;
}MEM_MAP_INFO;

//�����飬�����߼��飬���Բ�������飬Ҳ���Բ������ݿ�
void FlashEraseMem(WORD LogicBlockNum); 
//��д���������������š�ƫ�ƣ�ƫ���ǰ����ּ���
WORD FlashPutMem(WORD PhySector,WORD OffSet,WORD *Ptr,WORD Number); 
WORD FlashGetMem(WORD PhySector,WORD OffSet,WORD *Ptr,WORD Number); 

//��������������״̬���������������������
void FlashPutStatus(WORD PhySector,WORD Status); 
WORD FlashGetStatus(WORD PhySector); 
void FlashSearchMemStatus(void);

//��������������У��ͣ��������������������
void FlashPutCheckSum(WORD PhySector,WORD CheckSum); 
WORD FlashGetCheckSum(WORD PhySector); 

//�ڹ��������������У���������ҳ����������ֻ�����־��0��ʾ������1��ʾ����
void FlashSetBadSector(WORD PhySector); 

//��ʽ���洢��
void FlashFormatMem(void);		
//����ӳ����һ����¼
WORD AddMemMapRecord(WORD LogicSector,WORD PageNum);
//ɾ��ӳ����һ����¼
WORD RemoveMemMapRecord(WORD LogicSector);
//�޸�ӳ����¼
WORD RefreshMemMapRecord(WORD LogicSector,WORD PageNum);
//�����߼������õ���������
WORD GetMemMapRecord(WORD LogSector);
//��ѯһ��Ϊ���е���������
WORD SearchFreePhySector(void);
//��������������
#define FlashCreateMemInfo()	FlashGetMem(ConstPhyBeginSector,0,(WORD *)&MemInfo,lengthof(FLASH_INFO))
//����ӳ���
void FlashCreateMapInfo(void);
//��ʼ��
void FlashInitSetup(void);
//������Dirty�Ĳ���,�߼����
BOOL FlashCleanMem(WORD nBlkNum);
//�洢������
void FlashManageMem(void);

//ȷ���������ݿ�ĺ���,˼�������ҿ������������������ܷ����Ҫ���������д����������
//�ڶ�������������Щÿҳ��DIRTY����������INUSE��
WORD FlashRefreshMem(void);

//��д���ݣ������߼���������д���ṩ���ϲ�ʹ��
WORD FlashWriteMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number); //д���ݣ������߼�����
WORD FlashReadMem(WORD LogSector,WORD OffSet,WORD *Ptr,WORD Number);  //�����ݣ������߼�����


#endif




