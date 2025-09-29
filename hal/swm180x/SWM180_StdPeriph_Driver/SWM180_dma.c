/****************************************************************************************************************************************** 
* �ļ�����: SWM180_dma.c
* ����˵��:	SWM180��Ƭ����DMA����������
* ����֧��:	http://www.synwit.com.cn/e/tool/gbook/?bid=1
* ע������: �ڲ�ʹ���жϵ�����£�Ҳ���Ե���DMA_CH_INTStat()��ѯ���ݰ����Ƿ���ɣ�������DMA_CH_INTClr()�����ɱ�־
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
#include "SWM180_dma.h"


/****************************************************************************************************************************************** 
* ��������: DMA_CH_Config()
* ����˵��:	DMAͨ������
* ��    ��: uint32_t chn			ָ��Ҫ���õ�ͨ������Чֵ��DMA_CHR_ADC��DMA_CHR_SDADC��DMA_CHR_CAN
*			uint32_t ram_addr		����Ҫ�����˵�RAM�еĵ�ַ�������ֶ���
*			uint32_t num_word		Ҫ���˵�����������ע�⣬��λ���֣������ֽ�
*			uint32_t int_en			�ж�ʹ�ܣ�1 ���ݰ�����ɺ�����ж�    0 ���ݰ�����ɺ󲻲����ж�
* ��    ��: ��
* ע������: �ڴ洢���䣨��Flash��RAM�䣩����������ʹ��DMA_CHM_Config()
******************************************************************************************************************************************/
void DMA_CH_Config(uint32_t chn, uint32_t ram_addr, uint32_t num_word, uint32_t int_en)
{
	DMA->EN = 1;			//ÿ��ͨ�������Լ������Ŀ��ؿ��ƣ������ܿ��ؿ�����һֱ������
	
	DMA_CH_Close(chn);		//����ǰ�ȹرո�ͨ��
	
	switch(chn)
	{
	case DMA_CHR_ADC:
		ADC->CTRL |= (0x01 << ADC_CTRL_DMAEN_Pos) | (0x01 << ADC_CTRL_RES2FF_Pos);
		
		DMA->CH[chn].SRC = 0x5000D094;		//ADC->FFDATA�Ĵ���
		break;
	
	case DMA_CHR_SDADC:
		SDADC->CTRL |= (0x01 << SDADC_CTRL_DMAEN_Pos);
		
		DMA->CH[chn].SRC = 0x50040044;		//SDADC->DATA�Ĵ���
		break;
	
	case DMA_CHR_CAN:
		CAN->CR |= (0x01 << CAN_CR_DMAEN_Pos);
		
		DMA->CH[chn].SRC = 0x50020040;		//CAN->RXFRAME�Ĵ���
		break;
	}
	
	DMA->CH[chn].DST = ram_addr;
	
	DMA->CH[chn].CR = ((num_word*4-1) << DMA_CR_LEN_Pos);
	
	DMA_CH_INTClr(chn);
	if(int_en)	DMA_CH_INTEn(chn);
	else		DMA_CH_INTDis(chn);
}

/****************************************************************************************************************************************** 
* ��������: DMA_CHM_Config()
* ����˵��:	DMAͨ�����ã����ڴ洢���䣨��Flash��RAM�䣩��������
* ��    ��: uint32_t chn			ָ��Ҫ���õ�ͨ������Чֵ��DMA_CHW_FLASH��DMA_CHR_FLASH
*			uint32_t src_addr		��DMA_CHW_FLASHͨ����Դ��ַ��RAM�е�ַ���ֶ��룻    ��DMA_CHR_FLASHͨ����Դ��ַ��Flash�е�ַ���ֶ���
*			uint32_t dst_addr		��DMA_CHW_FLASHͨ����Ŀ��ֱ����Flash�е�ַ���ֶ��룻��DMA_CHR_FLASHͨ����Ŀ���ַ��RAM�е�ַ���ֶ���
*			uint32_t num_word		Ҫ���˵�����������ע�⣬��λ���֣������ֽ�
*			uint32_t int_en			�ж�ʹ�ܣ�1 ���ݰ�����ɺ�����ж�    0 ���ݰ�����ɺ󲻲����ж�
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void DMA_CHM_Config(uint32_t chn, uint32_t src_addr, uint32_t dst_addr, uint32_t num_word, uint32_t int_en)
{
	DMA->EN = 1;			//ÿ��ͨ�������Լ������Ŀ��ؿ��ƣ������ܿ��ؿ�����һֱ������
	
	DMA_CH_Close(chn);		//����ǰ�ȹرո�ͨ��
	
	DMA->CH[chn].SRC = src_addr;
	DMA->CH[chn].DST = dst_addr;
	
	DMA->CH[chn].CR &= ~DMA_CR_LEN_Msk;
	DMA->CH[chn].CR |= ((num_word*4-1) << DMA_CR_LEN_Pos);
	
	DMA_CH_INTClr(chn);
	if(int_en)	DMA_CH_INTEn(chn);
	else		DMA_CH_INTDis(chn);
}

/****************************************************************************************************************************************** 
* ��������: DMA_CH_Open()
* ����˵��:	DMAͨ����
* ��    ��: uint32_t chn			ָ��Ҫ���õ�ͨ������Чֵ��DMA_CHW_FLASH��DMA_CHR_FLASH��DMA_CHR_ADC��DMA_CHR_SDADC��DMA_CHR_CAN
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void DMA_CH_Open(uint32_t chn)
{
	switch(chn)
	{
	case DMA_CHW_FLASH:
		DMA->CH[chn].CR |= (0x01 << DMA_CR_WEN_Pos);
		break;
	
	case DMA_CHR_FLASH:
	case DMA_CHR_ADC:		
	case DMA_CHR_SDADC:		
	case DMA_CHR_CAN:
		DMA->CH[chn].CR |= (0x01 << DMA_CR_REN_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������: DMA_CH_Close()
* ����˵��:	DMAͨ���ر�
* ��    ��: uint32_t chn			ָ��Ҫ���õ�ͨ������Чֵ��DMA_CHW_FLASH��DMA_CHR_FLASH��DMA_CHR_ADC��DMA_CHR_SDADC��DMA_CHR_CAN
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void DMA_CH_Close(uint32_t chn)
{
	switch(chn)
	{
	case DMA_CHW_FLASH:
		DMA->CH[chn].CR &= ~(0x01 << DMA_CR_WEN_Pos);
		break;
	
	case DMA_CHR_FLASH:
	case DMA_CHR_ADC:		
	case DMA_CHR_SDADC:		
	case DMA_CHR_CAN:
		DMA->CH[chn].CR &= ~(0x01 << DMA_CR_REN_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������: DMA_CH_INTEn()
* ����˵��:	DMA�ж�ʹ�ܣ����ݰ�����ɺ󴥷��ж�
* ��    ��: uint32_t chn			ָ��Ҫ���õ�ͨ������Чֵ��DMA_CHW_FLASH��DMA_CHR_FLASH��DMA_CHR_ADC��DMA_CHR_SDADC��DMA_CHR_CAN
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void DMA_CH_INTEn(uint32_t chn)
{
	switch(chn)
	{
	case DMA_CHW_FLASH:
		DMA->IE |=  (1 << DMA_IE_WFLASH_Pos);
		DMA->IM &= ~(1 << DMA_IM_WFLASH_Pos);
		break;
	
	case DMA_CHR_FLASH:
		DMA->IE |=  (1 << DMA_IE_RFLASH_Pos);
		DMA->IM &= ~(1 << DMA_IM_RFLASH_Pos);
		break;
	
	case DMA_CHR_ADC:
		DMA->IE |=  (1 << DMA_IE_ADC_Pos);
		DMA->IM &= ~(1 << DMA_IM_ADC_Pos);
		break;
	
	case DMA_CHR_SDADC:
		DMA->IE |=  (1 << DMA_IE_SDADC_Pos);
		DMA->IM &= ~(1 << DMA_IM_SDADC_Pos);
		break;
	
	case DMA_CHR_CAN:
		DMA->IE |=  (1 << DMA_IE_CAN_Pos);
		DMA->IM &= ~(1 << DMA_IM_CAN_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������: DMA_CH_INTDis()
* ����˵��:	DMA�жϽ�ֹ�����ݰ�����ɺ󲻴����ж�
* ��    ��: uint32_t chn			ָ��Ҫ���õ�ͨ������Чֵ��DMA_CHW_FLASH��DMA_CHR_FLASH��DMA_CHR_ADC��DMA_CHR_SDADC��DMA_CHR_CAN
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void DMA_CH_INTDis(uint32_t chn)
{
	switch(chn)
	{
	case DMA_CHW_FLASH:
		DMA->IM |=  (1 << DMA_IM_WFLASH_Pos);
		DMA->IE &= ~(1 << DMA_IE_WFLASH_Pos);
		break;
	
	case DMA_CHR_FLASH:
		DMA->IM |=  (1 << DMA_IM_RFLASH_Pos);
		DMA->IE &= ~(1 << DMA_IE_RFLASH_Pos);
		break;
	
	case DMA_CHR_ADC:
		DMA->IM |=  (1 << DMA_IM_ADC_Pos);
		DMA->IE &= ~(1 << DMA_IE_ADC_Pos);
		break;
	
	case DMA_CHR_SDADC:
		DMA->IM |=  (1 << DMA_IM_SDADC_Pos);
		DMA->IE &= ~(1 << DMA_IE_SDADC_Pos);
		break;
	
	case DMA_CHR_CAN:
		DMA->IM |=  (1 << DMA_IM_CAN_Pos);
		DMA->IE &= ~(1 << DMA_IE_CAN_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������: DMA_CH_INTClr()
* ����˵��:	DMA�жϱ�־���
* ��    ��: uint32_t chn			ָ��Ҫ���õ�ͨ������Чֵ��DMA_CHW_FLASH��DMA_CHR_FLASH��DMA_CHR_ADC��DMA_CHR_SDADC��DMA_CHR_CAN
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void DMA_CH_INTClr(uint32_t chn)
{
	switch(chn)
	{
	case DMA_CHW_FLASH:
		DMA->IF = (1 << DMA_IF_WFLASH_Pos);
		break;
	
	case DMA_CHR_FLASH:
		DMA->IF = (1 << DMA_IF_RFLASH_Pos);
		break;
	
	case DMA_CHR_ADC:
		DMA->IF = (1 << DMA_IF_ADC_Pos);
		break;
	
	case DMA_CHR_SDADC:
		DMA->IF = (1 << DMA_IF_SDADC_Pos);
		break;
	
	case DMA_CHR_CAN:
		DMA->IF = (1 << DMA_IF_CAN_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������: DMA_CH_INTStat()
* ����˵��:	DMA�ж�״̬��ѯ
* ��    ��: uint32_t chn			ָ��Ҫ���õ�ͨ������Чֵ��DMA_CHW_FLASH��DMA_CHR_FLASH��DMA_CHR_ADC��DMA_CHR_SDADC��DMA_CHR_CAN
* ��    ��: uint32_t				1 ���ݰ������    0 ���ݰ���δ���
* ע������: ��
******************************************************************************************************************************************/
uint32_t DMA_CH_INTStat(uint32_t chn)
{
	uint32_t stat = 0;
	
	switch(chn)
	{
	case DMA_CHW_FLASH:
		stat = (DMA->IF & DMA_IF_WFLASH_Msk) ? 1 : 0;
		break;
	
	case DMA_CHR_FLASH:
		stat = (DMA->IF & DMA_IF_RFLASH_Msk) ? 1 : 0;
		break;
	
	case DMA_CHR_ADC:
		stat = (DMA->IF & DMA_IF_ADC_Msk)    ? 1 : 0;
		break;
	
	case DMA_CHR_SDADC:
		stat = (DMA->IF & DMA_IF_SDADC_Msk)  ? 1 : 0;
		break;
	
	case DMA_CHR_CAN:
		stat = (DMA->IF & DMA_IF_CAN_Msk)    ? 1 : 0;
		break;
	}
	
	return stat;
}
