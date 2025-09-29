/****************************************************************************************************************************************** 
* �ļ�����:	SWM180_i2c.c
* ����˵��:	SWM180��Ƭ����I2C���нӿڹ���������
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
#include "SWM180_i2c.h"


/****************************************************************************************************************************************** 
* ��������:	I2C_Init()
* ����˵��:	I2C��ʼ��
* ��    ��: I2C_TypeDef * I2Cx		ָ��Ҫ�����õ�I2C����Чֵ����I2C0��I2C1
*			I2C_InitStructure * initStruct	����I2C����趨ֵ�Ľṹ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void I2C_Init(I2C_TypeDef * I2Cx, I2C_InitStructure * initStruct)
{
	switch((uint32_t)I2Cx)
	{
	case ((uint32_t)I2C0):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_I2C0_Pos);
		break;
	
	case ((uint32_t)I2C1):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_I2C1_Pos);
		break;
	}
	
	I2C_Close(I2Cx);	//һЩ�ؼ��Ĵ���ֻ����I2C�ر�ʱ����
	
	if(initStruct->Master == 1)
	{
		I2Cx->SLVCR &= ~I2C_SLVCR_SLAVE_Msk;
		I2Cx->SLVCR |= (0 << I2C_SLVCR_SLAVE_Pos);
		
		I2Cx->CLKDIV = SystemCoreClock/5/initStruct->MstClk - 1;
		
		I2Cx->MSTCMD = (I2Cx->MSTCMD & (~I2C_MSTCMD_IF_Msk)) | (1 << I2C_MSTCMD_IF_Pos);	//ʹ���ж�֮ǰ������жϱ�־
		I2Cx->CTRL &= ~I2C_CTRL_MSTIE_Msk;
		I2Cx->CTRL |= (initStruct->MstIEn << I2C_CTRL_MSTIE_Pos);
	}
	else
	{
		I2Cx->SLVCR &= ~I2C_SLVCR_SLAVE_Msk;
		I2Cx->SLVCR |= (1 << I2C_SLVCR_SLAVE_Pos);
		
		I2Cx->SLVCR &= ~(I2C_SLVCR_ADDR7b_Msk | I2C_SLVCR_ADDR_Msk);
		I2Cx->SLVCR |= (1 << I2C_SLVCR_ACK_Pos) |
					   (initStruct->Addr7b << I2C_SLVCR_ADDR7b_Pos) |
					   (initStruct->SlvAddr << I2C_SLVCR_ADDR_Pos);
		
		I2Cx->SLVIF = I2C_SLVIF_RXEND_Msk | I2C_SLVIF_TXEND_Msk | I2C_SLVIF_STADET_Msk | I2C_SLVIF_STODET_Msk;	//���жϱ�־
		I2Cx->SLVCR &= ~(I2C_SLVCR_IE_RXEND_Msk | I2C_SLVCR_IE_TXEND_Msk | I2C_SLVCR_IE_STADET_Msk | I2C_SLVCR_IE_STODET_Msk |
						 I2C_SLVCR_IE_RDREQ_Msk | I2C_SLVCR_IE_WRREQ_Msk);
		I2Cx->SLVCR |= (initStruct->SlvRxEndIEn << I2C_SLVCR_IE_RXEND_Pos) |
					   (initStruct->SlvTxEndIEn << I2C_SLVCR_IE_TXEND_Pos) |
					   (initStruct->SlvSTADetIEn << I2C_SLVCR_IE_STADET_Pos) |
					   (initStruct->SlvSTODetIEn << I2C_SLVCR_IE_STODET_Pos) |
					   (initStruct->SlvRdReqIEn << I2C_SLVCR_IE_RDREQ_Pos)  |
					   (initStruct->SlvWrReqIEn << I2C_SLVCR_IE_WRREQ_Pos);
	}
}

/****************************************************************************************************************************************** 
* ��������:	I2C_Open()
* ����˵��:	I2C�򿪣������շ�
* ��    ��: I2C_TypeDef * I2Cx		ָ��Ҫ�����õ�I2C����Чֵ����I2C0��I2C1
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void I2C_Open(I2C_TypeDef * I2Cx)
{
	I2Cx->CTRL |= (0x01 << I2C_CTRL_EN_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	I2C_Close()
* ����˵��:	I2C�رգ���ֹ�շ�
* ��    ��: I2C_TypeDef * I2Cx		ָ��Ҫ�����õ�I2C����Чֵ����I2C0��I2C1
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void I2C_Close(I2C_TypeDef * I2Cx)
{
	I2Cx->CTRL &= ~I2C_CTRL_EN_Msk;
}
