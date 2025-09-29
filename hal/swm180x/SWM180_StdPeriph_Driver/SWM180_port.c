/****************************************************************************************************************************************** 
* �ļ�����: SWM180_port.c
* ����˵��:	SWM180��Ƭ���Ķ˿����Ź���ѡ��⺯��
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
#include "SWM180_port.h"


/****************************************************************************************************************************************** 
* ��������: PORT_Init()
* ����˵��:	�˿����Ź���ѡ�񣬿��õĹ��ܼ�"SWM180_port.h"�ļ�
* ��    ��: PORT_TypeDef * PORTx	ָ��PORT�˿ڣ���Чֵ����PORTA��PORTB��PORTC��PORTD��PORTE	
*			uint32_t n		   		ָ��PORT���ţ���Чֵ����PIN0��PIN1��PIN2��... ... PIN14��PIN15
*			uint32_t func	   		ָ���˿�����Ҫ�趨�Ĺ��ܣ����ȡֵ��"SWM180_port.h"�ļ�
*			uint32_t digit_in_en	��������ʹ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void PORT_Init(PORT_TypeDef * PORTx, uint32_t n, uint32_t func, uint32_t digit_in_en)
{
	
	if(func > 99)
	{
		if(n <= PIN5)
		{
			PORTx->FUNMUX0 &= ~(0x1F << (n*5));
			PORTx->FUNMUX0 |= (func-100) << (n*5);
		}
		else if(n <= PIN11)
		{
			PORTx->FUNMUX1 &= ~(0x1F << ((n-6)*5));
			PORTx->FUNMUX1 |= (func-100) << ((n-6)*5);
		}
		else if(n <= PIN15)
		{
			PORTx->FUNMUX2 &= ~(0x1F << ((n-12)*5));
			PORTx->FUNMUX2 |= (func-100) << ((n-12)*5);
		}
	}
	
	switch((uint32_t)PORTx)
	{
		case ((uint32_t)PORTA):
			if(n <= PIN7)
			{
				PORTG->PORTA_SEL1 &= ~(0x03 << (n*2));
				PORTG->PORTA_SEL1 |= (func > 99 ? 1 : func) << (n*2);
			}
			else if(n <= PIN11)
			{
				PORTG->PORTA_SEL2 &= ~(0x03 << ((n-8)*2));
				PORTG->PORTA_SEL2 |= (func > 99 ? 1 : func) << ((n-8)*2);
			}
			else if(n <= PIN15)
			{
				PORTG->PORTA_SEL2 &= ~(0x07 << (8+(n-12)*3));
				PORTG->PORTA_SEL2 |= (func > 99 ? 1 : func) << (8+(n-12)*3);
			}
			break;
			
		case ((uint32_t)PORTB):
			if(n <= PIN15)
			{
				PORTG->PORTB_SEL &= ~(0x03 << (n*2));
				PORTG->PORTB_SEL |= (func > 99 ? 1 : func) << (n*2);
			}
			break;
		
		case ((uint32_t)PORTC):
			if(n <= PIN3)
			{
				PORTG->PORTC_SEL &= ~(0x03 << (n*2));
				PORTG->PORTC_SEL |= (func > 99 ? 1 : func) << (n*2);
			}
			else if(n <= PIN7)
			{
				PORTG->PORTC_SEL &= ~(0x07 << (8+(n-4)*3));
				PORTG->PORTC_SEL |= (func > 99 ? 1 : func) << (8+(n-4)*3);
			}
			break;
		
		case ((uint32_t)PORTD):
			if(n <= PIN7)
			{
				PORTG->PORTD_SEL &= ~(0x03 << (n*2));
				PORTG->PORTD_SEL |= (func > 99 ? 1 : func) << (n*2);
			}
			break;
		
		case ((uint32_t)PORTE):
			if(n <= PIN7)
			{
				PORTG->PORTE_SEL &= ~(0x03 << (n*2));
				PORTG->PORTE_SEL |= (func > 99 ? 1 : func) << (n*2);
			}
			break;
	}
	
	PORTx->INEN &= ~(0x01 << n);
	PORTx->INEN |= (digit_in_en << n);
}
