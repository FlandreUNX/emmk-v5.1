/****************************************************************************************************************************************** 
* �ļ�����:	SWM180_pwm.c
* ����˵��:	SWM180��Ƭ����PWM����������
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
#include "SWM180_pwm.h"


/****************************************************************************************************************************************** 
* ��������:	PWM_Init()
* ����˵��:	PWM��ʼ��
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			PWM_InitStructure * initStruct	����PWM����趨ֵ�Ľṹ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_Init(PWM_TypeDef * PWMx, PWM_InitStructure * initStruct)
{
	uint32_t bit_offset = 0;
	
	SYS->CLKEN |= (0x01 << SYS_CLKEN_PWM_Pos);
	
	PWM_Stop(PWMx, 1, 1);	//һЩ�ؼ��Ĵ���ֻ����PWMֹͣʱ����
	
	PWMG->CLKDIV = 0;		//ֻ���ڷ�ƵֵΪ0�����������PWM����ĳ�ʼ��ƽ
	
	PWMx->INIOUT &= ~(PWM_INIOUT_PWMA_Msk | PWM_INIOUT_PWMB_Msk);
	PWMx->INIOUT |= (initStruct->initLevelA << PWM_INIOUT_PWMA_Pos) |
					(initStruct->initLevelB << PWM_INIOUT_PWMB_Pos);
	
	PWMG->CLKDIV = initStruct->clk_div;
	
	PWMx->MODE = initStruct->mode;
	
	PWMx->PERA = initStruct->cycleA;
	PWMx->HIGHA = initStruct->hdutyA;
	PWMx->DZA = initStruct->deadzoneA;
	
	PWMx->PERB = initStruct->cycleB;
	PWMx->HIGHB = initStruct->hdutyB;
	PWMx->DZB = initStruct->deadzoneB;
	
	PWMG->IMSK = 0x00000000;
	
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		bit_offset = 0;
		break;

	case((uint32_t)PWM1):
		bit_offset = 2;
		break;

	case((uint32_t)PWM2):
		bit_offset = 4;
		break;
	
	case((uint32_t)PWM3):
		bit_offset = 6;
		break;
	} 
	
	PWMG->IRAWST = ((0x01 << bit_offset) | (0x01 << (bit_offset+1))  | (0x01 << (bit_offset+12)) | (0x01 << (bit_offset+13)));
	PWMG->IE &= ~((0x01 << bit_offset) | (0x01 << (bit_offset+1))  | (0x01 << (bit_offset+12)) | (0x01 << (bit_offset+13)));
	PWMG->IE |= (initStruct->NCycleAIEn << bit_offset) | (initStruct->NCycleBIEn << (bit_offset+1)) |
				(initStruct->HEndAIEn << (bit_offset+12)) | (initStruct->HEndBIEn << (bit_offset+13));
}

/****************************************************************************************************************************************** 
* ��������:	PWM_Start()
* ����˵��:	����PWM����ʼPWM���
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chA			0 ͨ��A������		1 ͨ��A����
*			uint32_t chB			0 ͨ��B������		1 ͨ��B����
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_Start(PWM_TypeDef * PWMx, uint32_t chA, uint32_t chB)
{
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		PWMG->CHEN |= (chA << PWMG_CHEN_PWM0A_Pos) | (chB << PWMG_CHEN_PWM0B_Pos);
		break;

	case((uint32_t)PWM1):
		PWMG->CHEN |= (chA << PWMG_CHEN_PWM1A_Pos) | (chB << PWMG_CHEN_PWM1B_Pos);
		break;

	case((uint32_t)PWM2):
		PWMG->CHEN |= (chA << PWMG_CHEN_PWM2A_Pos) | (chB << PWMG_CHEN_PWM2B_Pos);
		break;
	
	case((uint32_t)PWM3):
		PWMG->CHEN |= (chA << PWMG_CHEN_PWM3A_Pos) | (chB << PWMG_CHEN_PWM3B_Pos);
		break;
	} 
}

/****************************************************************************************************************************************** 
* ��������:	PWM_Stop()
* ����˵��:	�ر�PWM��ֹͣPWM���
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chA			0 ͨ��A���ر�		1 ͨ��A�ر�
*			uint32_t chB			0 ͨ��B���ر�		1 ͨ��B�ر�
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_Stop(PWM_TypeDef * PWMx, uint32_t chA, uint32_t chB)
{
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		PWMG->CHEN &= ~((chA << PWMG_CHEN_PWM0A_Pos) | (chB << PWMG_CHEN_PWM0B_Pos));
		break;

	case((uint32_t)PWM1):
		PWMG->CHEN &= ~((chA << PWMG_CHEN_PWM1A_Pos) | (chB << PWMG_CHEN_PWM1B_Pos));
		break;

	case((uint32_t)PWM2):
		PWMG->CHEN &= ~((chA << PWMG_CHEN_PWM2A_Pos) | (chB << PWMG_CHEN_PWM2B_Pos));
		break;
	
	case((uint32_t)PWM3):
		PWMG->CHEN &= ~((chA << PWMG_CHEN_PWM3A_Pos) | (chB << PWMG_CHEN_PWM3B_Pos));
		break;
	} 
}

/****************************************************************************************************************************************** 
* ��������:	PWM_SetCycle()
* ����˵��:	��������
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
*			uint16_t cycle			Ҫ�趨������ֵ
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_SetCycle(PWM_TypeDef * PWMx, uint32_t chn, uint16_t cycle)
{
	if(chn == PWM_CH_A)
		PWMx->PERA = cycle;
	else if(chn == PWM_CH_B)
		PWMx->PERB = cycle;
}

/****************************************************************************************************************************************** 
* ��������:	PWM_GetCycle()
* ����˵��:	��ȡ����
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ��ѯ�ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: uint16_t				��ȡ��������ֵ
* ע������: ��
******************************************************************************************************************************************/
uint16_t PWM_GetCycle(PWM_TypeDef * PWMx, uint32_t chn)
{
	uint16_t cycle = 0;
	
	if(chn == PWM_CH_A)
		cycle = PWMx->PERA;
	else if(chn == PWM_CH_B)
		cycle = PWMx->PERB;
	
	return cycle;
}

/****************************************************************************************************************************************** 
* ��������:	PWM_SetHDuty()
* ����˵��:	���øߵ�ƽʱ��
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
*			uint16_t hduty			Ҫ�趨�ĸߵ�ƽʱ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_SetHDuty(PWM_TypeDef * PWMx, uint32_t chn, uint16_t hduty)
{
	if(chn == PWM_CH_A)
		PWMx->HIGHA = hduty;
	else if(chn == PWM_CH_B)
		PWMx->HIGHB = hduty;
}

/****************************************************************************************************************************************** 
* ��������:	PWM_GetHDuty()
* ����˵��: ��ȡ�ߵ�ƽʱ��
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ��ѯ�ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: uint16_t				��ȡ���ĸߵ�ƽʱ��
* ע������: ��
******************************************************************************************************************************************/
uint16_t PWM_GetHDuty(PWM_TypeDef * PWMx, uint32_t chn)
{
	uint16_t hduty = 0;
	
	if(chn == PWM_CH_A)
		hduty = PWMx->HIGHA;
	else if(chn == PWM_CH_B)
		hduty = PWMx->HIGHB;
	
	return hduty;
}

/****************************************************************************************************************************************** 
* ��������:	PWM_SetDeadzone()
* ����˵��:	��������ʱ��
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
*			uint8_t deadzone		Ҫ�趨������ʱ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_SetDeadzone(PWM_TypeDef * PWMx, uint32_t chn, uint8_t deadzone)
{
	if(chn == PWM_CH_A)
		PWMx->DZA = deadzone;
	else if(chn == PWM_CH_B)
		PWMx->DZB = deadzone;
}

/****************************************************************************************************************************************** 
* ��������:	PWM_GetDeadzone()
* ����˵��: ��ȡ����ʱ��
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ��ѯ�ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: uint8_t					��ȡ��������ʱ��
* ע������: ��
******************************************************************************************************************************************/
uint8_t  PWM_GetDeadzone(PWM_TypeDef * PWMx, uint32_t chn)
{
	uint8_t deadzone = 0;
	
	if(chn == PWM_CH_A)
		deadzone = PWMx->DZA;
	else if(chn == PWM_CH_B)
		deadzone = PWMx->DZB;
	
	return deadzone;
}


/****************************************************************************************************************************************** 
* ��������:	PWM_IntNCycleEn()
* ����˵��: �����ڿ�ʼ�ж�ʹ��
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_IntNCycleEn(PWM_TypeDef * PWMx, uint32_t chn)
{
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		if(chn == PWM_CH_A) PWMG->IE |= (0x01 << PWMG_IE_NEWP0A_Pos);
		else				PWMG->IE |= (0x01 << PWMG_IE_NEWP0B_Pos);
		break;
	
	case((uint32_t)PWM1):
		if(chn == PWM_CH_A) PWMG->IE |= (0x01 << PWMG_IE_NEWP1A_Pos);
		else				PWMG->IE |= (0x01 << PWMG_IE_NEWP1B_Pos);
		break;
	
	case((uint32_t)PWM2):
		if(chn == PWM_CH_A) PWMG->IE |= (0x01 << PWMG_IE_NEWP2A_Pos);
		else				PWMG->IE |= (0x01 << PWMG_IE_NEWP2B_Pos);
		break;
	
	case((uint32_t)PWM3):
		if(chn == PWM_CH_A) PWMG->IE |= (0x01 << PWMG_IE_NEWP3A_Pos);
		else				PWMG->IE |= (0x01 << PWMG_IE_NEWP3B_Pos);
		break;
	} 
}

/****************************************************************************************************************************************** 
* ��������:	PWM_IntNCycleDis()
* ����˵��: �����ڿ�ʼ�жϽ���
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_IntNCycleDis(PWM_TypeDef * PWMx, uint32_t chn)
{
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		if(chn == PWM_CH_A) PWMG->IE &= ~(0x01 << PWMG_IE_NEWP0A_Pos);
		else				PWMG->IE &= ~(0x01 << PWMG_IE_NEWP0B_Pos);
		break;
	
	case((uint32_t)PWM1):
		if(chn == PWM_CH_A) PWMG->IE &= ~(0x01 << PWMG_IE_NEWP1A_Pos);
		else				PWMG->IE &= ~(0x01 << PWMG_IE_NEWP1B_Pos);
		break;
	
	case((uint32_t)PWM2):
		if(chn == PWM_CH_A) PWMG->IE &= ~(0x01 << PWMG_IE_NEWP2A_Pos);
		else				PWMG->IE &= ~(0x01 << PWMG_IE_NEWP2B_Pos);
		break;
	
	case((uint32_t)PWM3):
		if(chn == PWM_CH_A) PWMG->IE &= ~(0x01 << PWMG_IE_NEWP3A_Pos);
		else				PWMG->IE &= ~(0x01 << PWMG_IE_NEWP3B_Pos);
		break;
	} 
}

/****************************************************************************************************************************************** 
* ��������:	PWM_IntNCycleClr()
* ����˵��: �����ڿ�ʼ�жϱ�־���
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_IntNCycleClr(PWM_TypeDef * PWMx, uint32_t chn)
{
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		if(chn == PWM_CH_A) PWMG->IRAWST = (0x01 << PWMG_IRAWST_NEWP0A_Pos);
		else				PWMG->IRAWST = (0x01 << PWMG_IRAWST_NEWP0B_Pos);
		break;
	
	case((uint32_t)PWM1):
		if(chn == PWM_CH_A) PWMG->IRAWST = (0x01 << PWMG_IRAWST_NEWP1A_Pos);
		else				PWMG->IRAWST = (0x01 << PWMG_IRAWST_NEWP1B_Pos);
		break;
	
	case((uint32_t)PWM2):
		if(chn == PWM_CH_A) PWMG->IRAWST = (0x01 << PWMG_IRAWST_NEWP2A_Pos);
		else				PWMG->IRAWST = (0x01 << PWMG_IRAWST_NEWP2B_Pos);
		break;
	
	case((uint32_t)PWM3):
		if(chn == PWM_CH_A) PWMG->IRAWST = (0x01 << PWMG_IRAWST_NEWP3A_Pos);
		else				PWMG->IRAWST = (0x01 << PWMG_IRAWST_NEWP3B_Pos);
		break;
	} 
}

/****************************************************************************************************************************************** 
* ��������:	PWM_IntNCycleStat()
* ����˵��: �����ڿ�ʼ�ж��Ƿ���
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: uint32_t				1 �����ڿ�ʼ�ж��ѷ���    0 �����ڿ�ʼ�ж�δ����
* ע������: ��
******************************************************************************************************************************************/
uint32_t PWM_IntNCycleStat(PWM_TypeDef * PWMx, uint32_t chn)
{
	uint32_t int_stat = 0;
	
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		if(chn == PWM_CH_A) int_stat = (PWMG->IF & PWMG_IF_NEWP0A_Msk);
		else				int_stat = (PWMG->IF & PWMG_IF_NEWP0B_Msk);
		break;

	case((uint32_t)PWM1):
		if(chn == PWM_CH_A) int_stat = (PWMG->IF & PWMG_IF_NEWP1A_Msk);
		else				int_stat = (PWMG->IF & PWMG_IF_NEWP1B_Msk);
		break;

	case((uint32_t)PWM2):
		if(chn == PWM_CH_A) int_stat = (PWMG->IF & PWMG_IF_NEWP2A_Msk);
		else				int_stat = (PWMG->IF & PWMG_IF_NEWP2B_Msk);
		break;
	
	case((uint32_t)PWM3):
		if(chn == PWM_CH_A) int_stat = (PWMG->IF & PWMG_IF_NEWP3A_Msk);
		else				int_stat = (PWMG->IF & PWMG_IF_NEWP3B_Msk);
		break;
	}
	
	return int_stat;
}


/****************************************************************************************************************************************** 
* ��������:	PWM_IntHEndEn()
* ����˵��: �ߵ�ƽ�����ж�ʹ��
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_IntHEndEn(PWM_TypeDef * PWMx, uint32_t chn)
{
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		if(chn == PWM_CH_A) PWMG->IE |= (0x01 << PWMG_IE_HEND0A_Pos);
		else				PWMG->IE |= (0x01 << PWMG_IE_HEND0B_Pos);
		break;
	
	case((uint32_t)PWM1):
		if(chn == PWM_CH_A) PWMG->IE |= (0x01 << PWMG_IE_HEND1A_Pos);
		else				PWMG->IE |= (0x01 << PWMG_IE_HEND1B_Pos);
		break;
	
	case((uint32_t)PWM2):
		if(chn == PWM_CH_A) PWMG->IE |= (0x01 << PWMG_IE_HEND2A_Pos);
		else				PWMG->IE |= (0x01 << PWMG_IE_HEND2B_Pos);
		break;
	
	case((uint32_t)PWM3):
		if(chn == PWM_CH_A) PWMG->IE |= (0x01 << PWMG_IE_HEND3A_Pos);
		else				PWMG->IE |= (0x01 << PWMG_IE_HEND3B_Pos);
		break;
	} 
}

/****************************************************************************************************************************************** 
* ��������:	PWM_IntHEndDis()
* ����˵��: �ߵ�ƽ�����жϽ���
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_IntHEndDis(PWM_TypeDef * PWMx, uint32_t chn)
{
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		if(chn == PWM_CH_A) PWMG->IE &= ~(0x01 << PWMG_IE_HEND0A_Pos);
		else				PWMG->IE &= ~(0x01 << PWMG_IE_HEND0B_Pos);
		break;
	
	case((uint32_t)PWM1):
		if(chn == PWM_CH_A) PWMG->IE &= ~(0x01 << PWMG_IE_HEND1A_Pos);
		else				PWMG->IE &= ~(0x01 << PWMG_IE_HEND1B_Pos);
		break;
	
	case((uint32_t)PWM2):
		if(chn == PWM_CH_A) PWMG->IE &= ~(0x01 << PWMG_IE_HEND2A_Pos);
		else				PWMG->IE &= ~(0x01 << PWMG_IE_HEND2B_Pos);
		break;
	
	case((uint32_t)PWM3):
		if(chn == PWM_CH_A) PWMG->IE &= ~(0x01 << PWMG_IE_HEND3A_Pos);
		else				PWMG->IE &= ~(0x01 << PWMG_IE_HEND3B_Pos);
		break;
	} 
}

/****************************************************************************************************************************************** 
* ��������:	PWM_IntHEndClr()
* ����˵��: �ߵ�ƽ�����жϱ�־���
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PWM_IntHEndClr(PWM_TypeDef * PWMx, uint32_t chn)
{
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		if(chn == PWM_CH_A) PWMG->IRAWST = (0x01 << PWMG_IRAWST_HEND0A_Pos);
		else				PWMG->IRAWST = (0x01 << PWMG_IRAWST_HEND0B_Pos);
		break;
	
	case((uint32_t)PWM1):
		if(chn == PWM_CH_A) PWMG->IRAWST = (0x01 << PWMG_IRAWST_HEND1A_Pos);
		else				PWMG->IRAWST = (0x01 << PWMG_IRAWST_HEND1B_Pos);
		break;
	
	case((uint32_t)PWM2):
		if(chn == PWM_CH_A) PWMG->IRAWST = (0x01 << PWMG_IRAWST_HEND2A_Pos);
		else				PWMG->IRAWST = (0x01 << PWMG_IRAWST_HEND2B_Pos);
		break;
	
	case((uint32_t)PWM3):
		if(chn == PWM_CH_A) PWMG->IRAWST = (0x01 << PWMG_IRAWST_HEND3A_Pos);
		else				PWMG->IRAWST = (0x01 << PWMG_IRAWST_HEND3B_Pos);
		break;
	} 
}

/****************************************************************************************************************************************** 
* ��������:	PWM_IntHEndStat()
* ����˵��: �ߵ�ƽ�����ж��Ƿ���
* ��    ��: PWM_TypeDef * PWMx		ָ��Ҫ�����õ�PWM����Чֵ����PWM0��PWM1��PWM2��PWM3
*			uint32_t chn			ѡ��Ҫ�����ĸ�ͨ������Чֵ��PWM_CH_A��PWM_CH_B
* ��    ��: uint32_t				1 �ߵ�ƽ�����ж��ѷ���    0 �ߵ�ƽ�����ж�δ����
* ע������: ��
******************************************************************************************************************************************/
uint32_t PWM_IntHEndStat(PWM_TypeDef * PWMx, uint32_t chn)
{
	uint32_t int_stat = 0;
	
	switch((uint32_t)PWMx)
	{
	case((uint32_t)PWM0):
		if(chn == PWM_CH_A) int_stat = (PWMG->IF & PWMG_IF_HEND0A_Msk);
		else				int_stat = (PWMG->IF & PWMG_IF_HEND0B_Msk);
		break;

	case((uint32_t)PWM1):
		if(chn == PWM_CH_A) int_stat = (PWMG->IF & PWMG_IF_HEND1A_Msk);
		else				int_stat = (PWMG->IF & PWMG_IF_HEND1B_Msk);
		break;

	case((uint32_t)PWM2):
		if(chn == PWM_CH_A) int_stat = (PWMG->IF & PWMG_IF_HEND2A_Msk);
		else				int_stat = (PWMG->IF & PWMG_IF_HEND2B_Msk);
		break;
	
	case((uint32_t)PWM3):
		if(chn == PWM_CH_A) int_stat = (PWMG->IF & PWMG_IF_HEND3A_Msk);
		else				int_stat = (PWMG->IF & PWMG_IF_HEND3B_Msk);
		break;
	}
	
	return int_stat;
}
