/****************************************************************************************************************************************** 
* �ļ�����:	SWM180_cmp.c
* ����˵��:	SWM180��Ƭ����CMP�Ƚ�������������
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
#include "SWM180_cmp.h"


/****************************************************************************************************************************************** 
* ��������:	CMP_Init()
* ����˵��:	CMPģ��Ƚ�����ʼ��
* ��    ��: uint32_t CMPx		ָ��Ҫ���õıȽ�������Чֵ����CMP0��CMP1��CMP2
*			uint32_t inp_ex		�Ƚ����������IN+ѡ��1 �ⲿCMPxP����    0 �ڲ�VREF�ο���ѹ
*			uint32_t hys_en		�Ƚ����������ʹ�ܣ�   1 ������������     0 �رճ�������
*			uint32_t int_en		�Ƚ�������仯�ж�ʹ��		 	
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CMP_Init(uint32_t CMPx, uint32_t inp_ex, uint32_t hys_en, uint32_t int_en)
{
	switch(CMPx)
	{
	case CMP0:
		SYS->CMPCR &= ~(SYS_CMPCR_CMP0INP_Msk | SYS_CMPCR_CMP0HYS_Msk);
		SYS->CMPCR |= (inp_ex << SYS_CMPCR_CMP0INP_Pos) | (hys_en << SYS_CMPCR_CMP0HYS_Pos);
		CMP_INTClr(CMP0);
		SYS->CMPSR &= ~SYS_CMPSR_CMP0IE_Msk;
		SYS->CMPSR |= (int_en << SYS_CMPSR_CMP0IE_Pos);
		break;
	
	case CMP1:
		SYS->CMPCR &= ~(SYS_CMPCR_CMP1INP_Msk | SYS_CMPCR_CMP1HYS_Msk);
		SYS->CMPCR |= (inp_ex << SYS_CMPCR_CMP1INP_Pos) | (hys_en << SYS_CMPCR_CMP1HYS_Pos);
		CMP_INTClr(CMP1);
		SYS->CMPSR &= ~SYS_CMPSR_CMP1IE_Msk;
		SYS->CMPSR |= (int_en << SYS_CMPSR_CMP1IE_Pos);
		break;
	
	case CMP2:
		SYS->CMPCR &= ~(SYS_CMPCR_CMP2INP_Msk | SYS_CMPCR_CMP2HYS_Msk);
		SYS->CMPCR |= (inp_ex << SYS_CMPCR_CMP2INP_Pos) | (hys_en << SYS_CMPCR_CMP2HYS_Pos);
		CMP_INTClr(CMP2);
		SYS->CMPSR &= ~SYS_CMPSR_CMP2IE_Msk;
		SYS->CMPSR |= (int_en << SYS_CMPSR_CMP2IE_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������:	CMP_Open()
* ����˵��:	�����Ƚ���
* ��    ��: uint32_t CMPx		ָ��Ҫ���õıȽ�������Чֵ����CMP0��CMP1��CMP2
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CMP_Open(uint32_t CMPx)
{
	switch(CMPx)
	{
	case CMP0:
		SYS->CMPCR |= (0x01 << SYS_CMPCR_CMP0EN_Pos);
		break;
	
	case CMP1:
		SYS->CMPCR |= (0x01 << SYS_CMPCR_CMP1EN_Pos);
		break;
	
	case CMP2:
		SYS->CMPCR |= (0x01 << SYS_CMPCR_CMP2EN_Pos);
		break;
	}		
}

/****************************************************************************************************************************************** 
* ��������:	CMP_Close()
* ����˵��:	�رձȽ���
* ��    ��: uint32_t CMPx		ָ��Ҫ���õıȽ�������Чֵ����CMP0��CMP1��CMP2
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CMP_Close(uint32_t CMPx)
{
	switch(CMPx)
	{
	case CMP0:
		SYS->CMPCR &= ~(0x01 << SYS_CMPCR_CMP0EN_Pos);
		break;
	
	case CMP1:
		SYS->CMPCR &= ~(0x01 << SYS_CMPCR_CMP1EN_Pos);
		break;
	
	case CMP2:
		SYS->CMPCR &= ~(0x01 << SYS_CMPCR_CMP2EN_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������:	CMP_Output()
* ����˵��:	�Ƚ������
* ��    ��: uint32_t CMPx		ָ��Ҫ���õıȽ�������Чֵ����CMP0��CMP1��CMP2
* ��    ��: uint32_t			0 CMPxN��ƽ����CMPxP��ƽ    1 CMPxN��ƽ����CMPxP��ƽ    0xFF ���ú���ʱ��������
* ע������: ��
******************************************************************************************************************************************/
uint32_t CMP_Output(uint32_t CMPx)
{
	switch(CMPx)
	{
	case CMP0:
		return (SYS->CMPSR & SYS_CMPSR_CMP0OUT_Msk) ? 1 : 0;
	
	case CMP1:
		return (SYS->CMPSR & SYS_CMPSR_CMP1OUT_Msk) ? 1 : 0;
	
	case CMP2:
		return (SYS->CMPSR & SYS_CMPSR_CMP2OUT_Msk) ? 1 : 0;
	}
	
	return 0xFF;
}

/****************************************************************************************************************************************** 
* ��������:	CMP_SetVRef()
* ����˵��:	�����ڲ��ο���ѹ
* ��    ��: uint32_t volt		�ڲ��ο���ѹ����Чֵ��CMP_VREF_0V30��CMP_VREF_0V45��... ... ��CMP_VREF_2V55
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CMP_SetVRef(uint32_t volt)
{
	SYS->CMPCR &= ~SYS_CMPCR_VREF_Msk;
	SYS->CMPCR |= (volt << SYS_CMPCR_VREF_Pos);
}


/****************************************************************************************************************************************** 
* ��������:	CMP_INTEn()
* ����˵��:	�Ƚ�������仯�ж�ʹ��
* ��    ��: uint32_t CMPx		ָ��Ҫ���õıȽ�������Чֵ����CMP0��CMP1��CMP2
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CMP_INTEn(uint32_t CMPx)
{
	switch(CMPx)
	{
	case CMP0:
		SYS->CMPSR |= (0x01 << SYS_CMPSR_CMP0IE_Pos);
		break;
	
	case CMP1:
		SYS->CMPSR |= (0x01 << SYS_CMPSR_CMP1IE_Pos);
		break;
	
	case CMP2:
		SYS->CMPSR |= (0x01 << SYS_CMPSR_CMP2IE_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������:	CMP_INTDis()
* ����˵��:	�Ƚ�������仯�жϽ�ֹ
* ��    ��: uint32_t CMPx		ָ��Ҫ���õıȽ�������Чֵ����CMP0��CMP1��CMP2
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CMP_INTDis(uint32_t CMPx)
{
	switch(CMPx)
	{
	case CMP0:
		SYS->CMPSR &= ~(0x01 << SYS_CMPSR_CMP0IE_Pos);
		break;
	
	case CMP1:
		SYS->CMPSR &= ~(0x01 << SYS_CMPSR_CMP1IE_Pos);
		break;
	
	case CMP2:
		SYS->CMPSR &= ~(0x01 << SYS_CMPSR_CMP2IE_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������:	CMP_INTClr()
* ����˵��:	�Ƚ�������仯�жϱ�־���
* ��    ��: uint32_t CMPx		ָ��Ҫ���õıȽ�������Чֵ����CMP0��CMP1��CMP2
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CMP_INTClr(uint32_t CMPx)
{
	switch(CMPx)
	{
	case CMP0:
		SYS->CMPSR = (SYS->CMPSR & 0xFF) | (0x01 << SYS_CMPSR_CMP0IF_Pos);
		break;
	
	case CMP1:
		SYS->CMPSR = (SYS->CMPSR & 0xFF) | (0x01 << SYS_CMPSR_CMP1IF_Pos);
		break;
	
	case CMP2:
		SYS->CMPSR = (SYS->CMPSR & 0xFF) | (0x01 << SYS_CMPSR_CMP2IF_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������:	CMP_INTStat()
* ����˵��:	�Ƚ�������仯�ж�״̬
* ��    ��: uint32_t CMPx		ָ��Ҫ���õıȽ�������Чֵ����CMP0��CMP1��CMP2
* ��    ��: uint32_t			1 �Ƚ�������б仯    0 �Ƚ�������ޱ仯    0xFF ���ú���ʱ��������
* ע������: ��
******************************************************************************************************************************************/
uint32_t CMP_INTStat(uint32_t CMPx)
{
	switch(CMPx)
	{
	case CMP0:
		return (SYS->CMPSR & SYS_CMPSR_CMP0IF_Msk);
	
	case CMP1:
		return (SYS->CMPSR & SYS_CMPSR_CMP1IF_Msk);
	
	case CMP2:
		return (SYS->CMPSR & SYS_CMPSR_CMP2IF_Msk);
	}
	
	return 0xFF;
}
