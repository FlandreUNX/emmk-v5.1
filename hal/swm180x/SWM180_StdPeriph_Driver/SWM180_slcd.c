/****************************************************************************************************************************************** 
* �ļ�����: SWM180_slcd.c
* ����˵��:	SWM180��Ƭ����Segment LCD����������
* ����֧��:	http://www.synwit.com.cn/e/tool/gbook/?bid=1
* ע������:
* �汾����:	V1.0.0		2016��1��30��
* ������¼:  
*
*
*******************************************************************************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH CODING INFORMATION 
* REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME. AS A RESULT, SYNWIT SHALL NOT BE HELD LIABLE 
* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT 
* OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONN-
* -ECTION WITH THEIR PRODUCTS.
*
* COPYRIGHT 2012 Synwit Technology
*******************************************************************************************************************************************/
#include "SWM180.h"
#include "SWM180_slcd.h"


/****************************************************************************************************************************************** 
* ��������:	SLCD_Init()
* ����˵��:	SLCD�ӿڳ�ʼ��
* ��    ��: SLCD_TypeDef * SLCDx	ָ��Ҫ�����õ�SLCD�ӿڣ���Чֵ����SLCD
*			SLCD_InitStructure * initStruct    ����SLCD�ӿ�����趨ֵ�Ľṹ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SLCD_Init(SLCD_TypeDef * SLCDx, SLCD_InitStructure * initStruct)
{
	switch((uint32_t)SLCDx)
	{
	case ((uint32_t)SLCD):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_SLCD_Pos);
		break;
	}
	
	SLCDx->CR = (1 << SLCD_CR_DRIVEN_Pos) 						|
				(1 << SLCD_CR_SCANEN_Pos) 						|
				(1 << SLCD_CR_DISP_Pos)   						|	//�ر���ʾ
				(initStruct->Bias << SLCD_CR_BIAS_Pos) 			|
				(initStruct->Duty << SLCD_CR_DUTY_Pos) 			|
				(initStruct->FrameFreq << SLCD_CR_SCANFRQ_Pos) 	|
				(initStruct->DriveCurr << SLCD_CR_DRIVSEL_Pos) 	|
				(0 << SLCD_CR_KEYSCAN_Pos)					   	|	//����ɨ�蹦�ܹر�
				((SystemCoreClock/1000000) << SLCD_CR_CLKDIV_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SLCD_Open()
* ����˵��:	������ʾ
* ��    ��: SLCD_TypeDef * SLCDx	ָ��Ҫ�����õ�SLCD�ӿڣ���Чֵ����SLCD
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SLCD_Open(SLCD_TypeDef * SLCDx)
{
	SLCDx->CR &= ~SLCD_CR_DISP_Msk;
	SLCDx->CR |= (0 << SLCD_CR_DISP_Pos);	//������ʾ
}

/****************************************************************************************************************************************** 
* ��������:	SLCD_Close()
* ����˵��:	�ر���ʾ
* ��    ��: SLCD_TypeDef * SLCDx	ָ��Ҫ�����õ�SLCD�ӿڣ���Чֵ����SLCD
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SLCD_Close(SLCD_TypeDef * SLCDx)
{
	SLCDx->CR &= ~SLCD_CR_DISP_Msk;
	SLCDx->CR |= (1 << SLCD_CR_DISP_Pos);	//�ر���ʾ
}

/****************************************************************************************************************************************** 
* ��������:	SLCD_Clear()
* ����˵��:	�����ʾ�������ֶζ�����
* ��    ��: SLCD_TypeDef * SLCDx	ָ��Ҫ�����õ�SLCD�ӿڣ���Чֵ����SLCD
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SLCD_Clear(SLCD_TypeDef * SLCDx)
{
	SLCD->DATA[0] = 0x00000000;
	SLCD->DATA[1] = 0x00000000;
	SLCD->DATA[2] = 0x00000000;
	SLCD->DATA[3] = 0x00000000;
}

/****************************************************************************************************************************************** 
* ��������:	SLCD_AllOn()
* ����˵��:	ȫ����ʾ�������ֶζ�����
* ��    ��: SLCD_TypeDef * SLCDx	ָ��Ҫ�����õ�SLCD�ӿڣ���Чֵ����SLCD
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SLCD_AllOn(SLCD_TypeDef * SLCDx)
{
	SLCD->DATA[0] = 0xFFFFFFFF;
	SLCD->DATA[1] = 0xFFFFFFFF;
	SLCD->DATA[2] = 0xFFFFFFFF;
	SLCD->DATA[3] = 0xFFFFFFFF;
}
