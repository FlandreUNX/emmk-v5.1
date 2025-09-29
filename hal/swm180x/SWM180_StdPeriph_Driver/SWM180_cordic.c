/****************************************************************************************************************************************** 
* �ļ�����: SWM180_cordic.c
* ����˵��:	SWM180��Ƭ����CORDIC����������
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
#include "SWM180_cordic.h"


/****************************************************************************************************************************************** 
* ��������:	CORDIC_Init()
* ����˵��:	CORDIC��ʼ��
* ��    ��: CORDIC_TypeDef * CORDICx	ָ��Ҫ�����õ�CORDIC����Чֵ����CORDIC
* ��    ��: ��
* ע������: CORDICģ������DIVģ�飬��ΪCORDIC_Arctan()Ҫʹ��Ӳ������ģ��������ֵ
******************************************************************************************************************************************/
void CORDIC_Init(CORDIC_TypeDef * CORDICx)
{
	switch((uint32_t)CORDICx)
	{
	case ((uint32_t)CORDIC):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_CORDIC_Pos);
		break;
	}
	
	DIV_Init(DIV);		//CORDIC_Arctan()Ҫʹ��Ӳ������ģ��������ֵ��������Ҫ��ʼ��Ӳ������ģ��
}
