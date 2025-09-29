/****************************************************************************************************************************************** 
* �ļ�����: SWM180_div.c
* ����˵��:	SWM180��Ƭ����DIVӲ����������������
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
#include "SWM180_div.h"


/****************************************************************************************************************************************** 
* ��������:	DIV_Init()
* ����˵��:	Ӳ����������ʼ��
* ��    ��: DIV_TypeDef * DIVx	ָ��Ҫ�����õ�Ӳ������������Чֵ����DIV
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void DIV_Init(DIV_TypeDef * DIVx)
{
	switch((uint32_t)DIVx)
	{
	case ((uint32_t)DIV):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_DIV_Pos);
		break;
	}
}
