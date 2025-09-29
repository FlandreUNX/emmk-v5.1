/****************************************************************************************************************************************** 
* �ļ�����: SWM180_irqmux.c
* ����˵��:	���ڽ������ж����ӵ��ں��ж�IRQ0--IRQ31
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
#include "SWM180_irqmux.h"


/****************************************************************************************************************************************** 
* ��������: IRQ_Connect()
* ����˵��:	�������ж����ӵ��ں��ж�IRQ0--IRQ31
* ��    ��: uint32_t periph_interrupt	ָ�������жϣ���Чֵ��"SWM180_irqmux.h"
*			uint32_t IRQn		       	Ҫ���ӵ����ں��жϺţ���ЧֵIRQ0_IRQ��IRQ1_IRQ�� ... ... IRQ30_IRQ��IRQ31_IRQ��
*										����IRQ16--IRQ31ÿһ��IRQ�Ͽ����������������жϣ�������Ҫ����λ����IRQ_INT0��IRQ_INT1�����֣�����
*										IRQ30_IRQ | IRQ_INT0 == IRQ30_IRQ�����ԡ���λ��IRQ_INT0����ʡȥ
*			uint32_t priority	       	�ж����ȼ�����Чֵ0��1��2��3����ֵԽС���ȼ�Խ��
* ��    ��: ��
* ע������: ������IRQ_Connect(IRQ0_15_GPIOA1, IRQ6_IRQ, 0)					��GPIOA�˿�����1�ϵ��ж����ӵ�IRQ6�ں��ж�
*
*                 IRQ_Connect(IRQ16_31_UART0, IRQ18_IRQ | IRQ_INT0, 0)		��UART0�ж����ӵ�IRQ18�ں��жϵ��ж�0�ϣ������ɼ򻯳�
*				  IRQ_Connect(IRQ16_31_UART0, IRQ18_IRQ, 0)
*
*				  IRQ_Connect(IRQ16_31_SDADC, IRQ24_IRQ | IRQ_INT1, 0)		��SDADC�ж����ӵ�IRQ24�ں��жϵ��ж�1��
******************************************************************************************************************************************/
void IRQ_Connect(uint32_t periph_interrupt, uint32_t IRQn, uint32_t priority)
{
	uint32_t INT1 = 0;
	__IO uint32_t * IRQ_SRC = &IRQMUX->IRQ0_SRC;
	
	if(IRQn > 0xFF)
	{
		INT1 = 1;
		
		IRQn -= 0x100;
	}
	
	if(IRQn <= IRQ15_IRQ)
	{
		IRQ_SRC = &IRQMUX->IRQ0_SRC + (IRQn - IRQ0_IRQ);
		*IRQ_SRC = periph_interrupt;
	}
	else
	{
		IRQ_SRC = &IRQMUX->IRQ16_SRC + (IRQn - IRQ16_IRQ);
		
		if(INT1 == 1)
		{
			*IRQ_SRC &= ~(0x1F << 5);
			*IRQ_SRC |= (periph_interrupt << 5);
		}
		else
		{
			*IRQ_SRC &= ~(0x1F << 0);
			*IRQ_SRC |= (periph_interrupt << 0);
		}
	}
	
	NVIC_SetPriority((IRQn_Type)IRQn, priority);
	
	NVIC_EnableIRQ((IRQn_Type)IRQn);
}

/****************************************************************************************************************************************** 
* ��������: IRQ_Which()
* ����˵��:	IRQ16--IRQ31ÿһ��IRQ�Ͽ����������������жϣ��˺��������ж����������ж��е���һ���������ж�����
* ��    ��: uint32_t IRQn	IRQ16_IRQ��IRQ17_IRQ�� ... ... IRQ30_IRQ��IRQ31_IRQ
* ��    ��: uint32_t		IRQ_INT0 IRQn��INT0�������ж�    			IRQ_INT1 IRQn��INT1�������ж�
*							IRQ_BOTH IRQn��INT0��INT1���������ж�		IRQ_NONE IRQn��INT0��INT1��δ�����ж�
* ע������: ��
******************************************************************************************************************************************/
uint32_t IRQ_Which(uint32_t IRQn)
{
	uint32_t res = IRQ_NONE;
	__IO uint32_t * IRQ_IF = &IRQMUX->IRQ16_IF + (IRQn - IRQ16_IRQ);
	
	if((*IRQ_IF & 0x01) && (*IRQ_IF & 0x02))
		res = IRQ_BOTH;
	else if(*IRQ_IF & 0x01)
		res = IRQ_INT0;
	else if(*IRQ_IF & 0x02)
		res = IRQ_INT1;
	
	return res;
}
