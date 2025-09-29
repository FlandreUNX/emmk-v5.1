/****************************************************************************************************************************************** 
* �ļ�����: SWM180_cache.c
* ����˵��:	SWM180��Ƭ��CACHE������������
* ����֧��:	http://www.synwit.com.cn/e/tool/gbook/?bid=1
* ע������:
* �汾����: V1.0.0		2016��1��30��
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
#include "SWM180_cache.h"

/****************************************************************************************************************************************** 
* ��������:	CACHE_Init()
* ����˵��:	CHCHE��ʼ��
* ��    ��: CACHE_InitStructure * initStruct	����CACHE����趨ֵ�Ľṹ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CACHE_Init(CACHE_InitStructure * initStruct)
{
	CACHE->CR = (1 << CACHE_CR_RST_Pos) |
				(initStruct->Alg << CACHE_CR_ALG_Pos);
	
	CACHE->LOCKTHR = initStruct->Threshold;
	
	CACHE->IF = 0x07;	//����жϱ�־
	CACHE->IE = (1                    << CACHE_IE_EN_Pos)      |
				(initStruct->ResetIEn << CACHE_IE_RESET_Pos)   |
				(0                    << CACHE_IE_INVALID_Pos) |
				(0                    << CACHE_IE_PREFETCH_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	CACHE_Reset()
* ����˵��:	CHCHE��λ�����CHCHE���б�־������
* ��    ��: ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void CACHE_Reset(void)
{
	CACHE->CR |= (1 << CACHE_CR_RST_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	CACHE_Prefetch()
* ����˵��:	CHCHEԤȡ����
* ��    ��: uint32_t addr		��Flash��addr��ʼ��128�ֽڴ���Ԥȡ��Cache�в�����
* ��    ��: uint32_t			0 Ԥȡ����ɹ�    1 Ԥȡʧ��    2 ����ʧ��    3 δ֪����
* ע������: ��
******************************************************************************************************************************************/
uint32_t CACHE_Prefetch(uint32_t addr)
{
	uint32_t status;
	
	CACHE->PREFCR = (1 << CACHE_PREFCR_EN_Pos) |
					(addr << CACHE_PREFCR_ADDR_Pos);
	
	while((CACHE->PREFSR & CACHE_PREFSR_FINISH_Msk) == 0);
	
	if((CACHE->PREFSR & CACHE_PREFSR_LOCKSUCC_Msk) != 0)  status = 0;
	else if((CACHE->PREFSR & CACHE_PREFSR_FAIL_Msk) != 0) status = 1;
	else if((CACHE->PREFSR & CACHE_PREFSR_SUCC_Msk) != 0) status = 2;
	else                                                  status = 3;
	
	CACHE->PREFSR = CACHE_PREFSR_SUCC_Msk | CACHE_PREFSR_FAIL_Msk | CACHE_PREFSR_LOCKSUCC_Msk;	//д1�����־
	
	return status;
}

/****************************************************************************************************************************************** 
* ��������:	CACHE_PrefetchSlotNumber()
* ����˵��:	��ȡԤȡ�������������Slot�ı��
* ��    ��: ��
* ��    ��: uint32_t			������Slot��
* ע������: ��
******************************************************************************************************************************************/
uint32_t CACHE_PrefetchSlotNumber(void)
{
	return (CACHE->PREFSR & CACHE_PREFSR_SLOTNUM_Msk) >> CACHE_PREFSR_SLOTNUM_Pos;
}

/****************************************************************************************************************************************** 
* ��������:	CACHE_Invalid()
* ����˵��:	��������Cache�еĴ���ʧЧ���Ӷ��ͷ�Cache Slot
* ��    ��: uint32_t addr		��Flash��addr��ʼ��128�ֽڴ�����Cache�еĿ���ʧЧ���Ӷ��ͷ�Cache Slot
* ��    ��: uint32_t			0 ʧЧ�ɹ�����Slot���Ϊ��Ч    1 ʧЧʧ�ܣ�ָ�����벻��Cache��    2 δ֪����
* ע������: ��
******************************************************************************************************************************************/
uint32_t CACHE_Invalid(uint32_t addr)
{
	CACHE->INVALID = (1 << CACHE_INVALID_EN_Pos) |
					 (addr << CACHE_INVALID_ADDR_Pos);
	
	while((CACHE->INVALIDSR & CACHE_INVALIDSR_FINISH_Msk) == 0);
	
	if((CACHE->INVALIDSR & CACHE_INVALIDSR_SUCC_Msk) != 0)      return 0;
	else if((CACHE->INVALIDSR & CACHE_INVALIDSR_FAIL_Msk) != 0) return 1;
	else                                                        return 2;
}

/****************************************************************************************************************************************** 
* ��������:	CACHE_InvalidSlotNumber()
* ����˵��:	��ȡʧЧ�����ͷŵ�Slot�ı��
* ��    ��: ��
* ��    ��: uint32_t			ʧЧ��Slot��
* ע������: ��
******************************************************************************************************************************************/
uint32_t CACHE_InvalidSlotNumber(void)
{
	return (CACHE->INVALIDSR & CACHE_INVALIDSR_SLOTNUM_Msk) >> CACHE_INVALIDSR_SLOTNUM_Pos;
}
