/****************************************************************************************************************************************** 
* �ļ�����: SWM180_sdadc.c
* ����˵��:	SWM180��Ƭ����SDADC������
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
#include "SWM180_sdadc.h"

/****************************************************************************************************************************************** 
* ��������: SDADC_Init()
* ����˵��:	Sigma Delta ADCģ��ת������ʼ��
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����Чֵ����SDADC
*			SDADC_InitStructure * initStruct	 ����SDADC����ض�ֵ�Ľṹ��
* ��    ��: ��
* ע������: SDADC��ʱ��Դѡ��λͬADC��ʱ��Դѡ��λ��ͬһλ������ʱע�Ᵽ��һ�£���Ҫ��ͻ
******************************************************************************************************************************************/
void SDADC_Init(SDADC_TypeDef * SDADCx, SDADC_InitStructure * initStruct)
{
	switch((uint32_t)SDADCx)
	{
	case ((uint32_t)SDADC):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_SDADC_Pos);
	
		if(initStruct->clk_src == SDADC_CLKSRC_XTAL_DIV8)
		{
			if((SYS->XTALCR & SYS_XTALCR_EN_Msk) == 0)	// SDADCʹ��XTALʱ��ת������XTAL��û�����Ļ�������֮
			{
				PORT_Init(PORTC, PIN1, PORTC_PIN1_XTAL_IN, 0);
				PORT_Init(PORTC, PIN0, PORTC_PIN0_XTAL_OUT, 0);
				
				SYS->XTALCR = (1 << SYS_XTALCR_EN_Pos);
			}
		}
		
		SYS->CLKSEL &= ~SYS_CLKSEL_ADC_Msk;
		SYS->CLKSEL |= (initStruct->clk_src << SYS_CLKSEL_ADC_Pos);
		break;
	}
	
	SDADCx->CTRL = (initStruct->channels << SDADC_CTRL_CH0SEL_Pos)  |
				   (0 << SDADC_CTRL_RST_Pos)                        |
				   (1 << SDADC_CTRL_BIAS_Pos)                       |
				   (initStruct->Continue << SDADC_CTRL_CONT_Pos)    |
				   (0 << SDADC_CTRL_FAST_Pos)                       |
				   (initStruct->out_cali << SDADC_CTRL_OUTCALI_Pos) |
				   (0 << SDADC_CTRL_LOWCLK_Pos)                     |
				   (initStruct->refp_sel << SDADC_CTRL_REFP_Pos)    |
				   (initStruct->trig_src << SDADC_CTRL_TRIG_Pos)    |
				   (0 << SDADC_CTRL_DMAEN_Pos);
	
	SDADCx->IF = 0x1F;			//����жϱ�־
	SDADCx->IE = (initStruct->EOC_IEn << SDADC_IE_EOC_Pos) 	  |
				 (initStruct->OVF_IEn << SDADC_IE_FFOV_Pos)   |
				 (initStruct->HFULL_IEn << SDADC_IE_FFHF_Pos) |
				 (initStruct->FULL_IEn << SDADC_IE_FFF_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_Config_Set()
* ����˵��:	SDADC֧��3�����ã�6������ͨ����ѡ������һ����Ϊ�Լ������ã��˺��������������õľ�������
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
*			uint32_t CFGx				Ҫ�����ĸ����ã���ȡֵSDADC_CFG_A��SDADC_CFG_B��SDADC_CFG_C
*			uint32_t gian				���棬��ȡֵSDADC_CFG_GAIN_1��SDADC_CFG_GAIN_2��... ...��SDADC_CFG_GAIN_1DIV2
*			uint32_t sigle_end			0 ���ģʽ    1 ����ģʽ
*			uint32_t refm_as_inn		0 ͨ����N�������ΪN�����    1 ������AINREFM������ΪN�����
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_Config_Set(SDADC_TypeDef * SDADCx, uint32_t CFGx, uint32_t gian, uint32_t sigle_end, uint32_t refm_as_inn)
{
	switch(CFGx)
	{
	case SDADC_CFG_A:
		SDADCx->CFGA = (gian << SDADC_CFG_GAIN_Pos)    |
					   (sigle_end << SDADC_CFG_SE_Pos) |
					   (refm_as_inn << SDADC_CFG_REFM_Pos);
		break;
	
	case SDADC_CFG_B:
		SDADCx->CFGB = (gian << SDADC_CFG_GAIN_Pos)    |
					   (sigle_end << SDADC_CFG_SE_Pos) |
					   (refm_as_inn << SDADC_CFG_REFM_Pos);
		break;
	
	case SDADC_CFG_C:
		SDADCx->CFGB = (gian << SDADC_CFG_GAIN_Pos)    |
					   (sigle_end << SDADC_CFG_SE_Pos) |
					   (refm_as_inn << SDADC_CFG_REFM_Pos);
		break;
	}
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_Config_Cali()
* ����˵��:	SDADC֧��3�����ã�6������ͨ����ѡ������һ����Ϊ�Լ������ã��˺��������������õľ�������
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
*			uint32_t CFGx				Ҫ�����ĸ����ã���ȡֵSDADC_CFG_A��SDADC_CFG_B��SDADC_CFG_C
*			uint32_t cali_com			У׼�����ˣ���ȡֵ��SDADC_CALI_COM_GND��SDADC_CALI_COM_VDD_1DIV2��SDADC_CALI_COM_VDD
*			uint32_t cali_fast			����У׼���ɽ�У׼ʱ��������1/3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_Config_Cali(SDADC_TypeDef * SDADCx, uint32_t CFGx, uint32_t cali_com, uint32_t cali_fast)
{
	uint32_t sdadc_CTRL = SDADCx->CTRL;		//����SDADCx->CTRL�Ĵ�����ֵ
	
	SDADCx->CTRL = ((1 << 9) << SDADC_CTRL_CH0SEL_Pos)  		|	//ѡ��У׼ͨ��
				   (0 << SDADC_CTRL_RST_Pos)                   	|
				   (1 << SDADC_CTRL_EN_Pos)						|
				   (1 << SDADC_CTRL_BIAS_Pos)                  	|
				   (0 << SDADC_CTRL_CONT_Pos)    				|
				   (cali_fast << SDADC_CTRL_FAST_Pos)           |
				   (SDADC_OUT_RAW << SDADC_CTRL_OUTCALI_Pos) 	|
				   (0 << SDADC_CTRL_LOWCLK_Pos)                	|
				   (sdadc_CTRL & SDADC_CTRL_REFP_Msk)    		|
				   (cali_com << SDADC_CTRL_CALIN_Pos)			|
				   (SDADC_TRIGSRC_SW << SDADC_CTRL_TRIG_Pos)   	|
				   (0 << SDADC_CTRL_DMAEN_Pos);
	
	SDADCx->CFGS &= ~(3 << (9*2));			//ѡ��ҪУ׼�ĸ�����
	SDADCx->CFGS |= (CFGx << (9*2));
	
	SDADC_Start(SDADC);						//����ת����ִ��У׼
	while((SDADC->STAT & SDADC_STAT_CALEOC_Msk) == 0);
	SDADC->STAT = 1 << SDADC_STAT_CALEOC_Pos;
	
	SDADCx->CTRL = sdadc_CTRL;				//�ָ�SDADCx->CTRL�Ĵ�����ֵ
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_Config_Sel()
* ����˵��:	SDADC֧��3�����ã�6������ͨ����ѡ������һ����Ϊ�Լ������ã��˺�������Ϊͨ��ѡ������
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
*			uint32_t CFGx				Ҫ�����ĸ����ã���ȡֵSDADC_CFG_A��SDADC_CFG_B��SDADC_CFG_C
*			uint32_t channels			ѡ��ʹ������CFGx��ͨ������ȡֵSDADC_CH0��SDADC_CH1��... ... ��SDADC_CH5������ϣ�������λ�����㣩
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_Config_Sel(SDADC_TypeDef * SDADCx, uint32_t CFGx, uint32_t channels)
{
	uint32_t i;
	
	for(i = 0; i < 10; i++)
	{
		if(channels & (1 << i))
		{
			SDADCx->CFGS &= ~(3 << (i*2));
			SDADCx->CFGS |= (CFGx << (i*2));
		}
	}
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_Open()
* ����˵��:	SDADC����������������������Ӳ������SDADCת��
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_Open(SDADC_TypeDef * SDADCx)
{
	SDADCx->CTRL |= (0x01 << SDADC_CTRL_EN_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_Close()
* ����˵��:	SDADC�رգ��޷�������������Ӳ������SDADCת��
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_Close(SDADC_TypeDef * SDADCx)
{
	SDADCx->CTRL &= ~(0x01 << SDADC_CTRL_EN_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_Start()
* ����˵��:	��������ģʽ������SDADCת��
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_Start(SDADC_TypeDef * SDADCx)
{
	SDADCx->START = (1 << SDADC_START_GO_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_Stop()
* ����˵��:	��������ģʽ��ֹͣSDADCת��
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_Stop(SDADC_TypeDef * SDADCx)
{
	SDADCx->START &= ~(0x01 << SDADC_START_GO_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_Read()
* ����˵��:	��ȡת�����
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ��ȡ��SDADC����ȡֵ����SDADC
* ��    ��: int16_t						��ȡ����ת�����
*			uint32_t *chn				ת����������ĸ�ͨ����0 SDADC_CH0    1 SDADC_CH1��... ...��5 SDADC_CH5
* ע������: ��
******************************************************************************************************************************************/
int16_t SDADC_Read(SDADC_TypeDef * SDADCx, uint32_t *chn)
{
	uint32_t data = SDADCx->DATA;
	*chn = (data & SDADC_DATA_CHNUM_Msk) >> SDADC_DATA_CHNUM_Pos;
	
	return (data & SDADC_DATA_VALUE_Msk);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IsEOC()
* ����˵��:	�Ƿ�End Of Conversion
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ��ѯ��SDADC����ȡֵ����SDADC
* ��    ��: uint32_t					1 ��ͨ�����ת��    0 û��ͨ�����ת��
* ע������: ��
******************************************************************************************************************************************/
uint32_t SDADC_IsEOC(SDADC_TypeDef * SDADCx)
{
	return (SDADCx->STAT & SDADC_STAT_EOC_Msk) ? 1 : 0;
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_ChnOpen()
* ����˵��:	SDADCͨ��������ģ��ת�����ڿ�����ͨ�������β���ת��
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
*			uint32_t chns				Ҫ�򿪵�ͨ������ЧֵSDADC_CH0��SDADC_CH1��... ... ��SDADC_CH5������ϣ�������λ�����㣩
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_ChnOpen(SDADC_TypeDef * SDADCx, uint32_t chns)
{
	SDADCx->CTRL |= chns;
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_ChnClose()
* ����˵��:	SDADCͨ���ر�
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
*			uint32_t chns				Ҫ�رյ�ͨ������ЧֵSDADC_CH0��SDADC_CH1��... ... ��SDADC_CH5������ϣ�������λ�����㣩	
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_ChnClose(SDADC_TypeDef * SDADCx, uint32_t chns)
{
	SDADCx->CTRL &= ~chns;
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IntEOCEn()
* ����˵��:	ת������ж�ʹ��
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_IntEOCEn(SDADC_TypeDef * SDADCx)
{
	SDADCx->IE |= (1 << SDADC_IE_EOC_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IntEOCDis()
* ����˵��:	ת������жϽ�ֹ
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_IntEOCDis(SDADC_TypeDef * SDADCx)
{
	SDADCx->IE &= ~(1 << SDADC_IE_EOC_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IntEOCClr()
* ����˵��:	ת������жϱ�־���
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_IntEOCClr(SDADC_TypeDef * SDADCx)
{
	SDADCx->IF = (1 << SDADC_IF_EOC_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IntEOCStat()
* ����˵��:	ת������ж�״̬
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: uint32_t					1 ͨ�������ת��    0 ͨ��δ���ת��
* ע������: ��
******************************************************************************************************************************************/
uint32_t SDADC_IntEOCStat(SDADC_TypeDef * SDADCx)
{	
	return (SDADCx->IF & SDADC_IF_EOC_Msk) ? 1 : 0;
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IntOVFEn()
* ����˵��:	��������ж�ʹ��
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_IntOVFEn(SDADC_TypeDef * SDADCx)
{
	SDADCx->IE |= (1 << SDADC_IE_FFOV_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IntOVFDis()
* ����˵��:	��������жϽ�ֹ
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_IntOVFDis(SDADC_TypeDef * SDADCx)
{	
	SDADCx->IE &= ~(1 << SDADC_IE_FFOV_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IntOVFClr()
* ����˵��:	��������жϱ�־���
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SDADC_IntOVFClr(SDADC_TypeDef * SDADCx)
{
	SDADCx->IF = (1 << SDADC_IF_FFOV_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	SDADC_IntOVFStat()
* ����˵��:	��������ж�״̬
* ��    ��: SDADC_TypeDef * SDADCx		ָ��Ҫ�����õ�SDADC����ȡֵ����SDADC
* ��    ��: uint32_t					1 ��ͨ�����    0 û��ͨ�����
* ע������: ��
******************************************************************************************************************************************/
uint32_t SDADC_IntOVFStat(SDADC_TypeDef * SDADCx)
{	
	return (SDADCx->IF & SDADC_IF_FFOV_Msk) ? 1 : 0;
}
