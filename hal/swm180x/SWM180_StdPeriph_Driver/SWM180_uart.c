/****************************************************************************************************************************************** 
* �ļ�����:	SWM180_uart.c
* ����˵��:	SWM180��Ƭ����UART���ڹ���������
* ����֧��:	http://www.synwit.com.cn/e/tool/gbook/?bid=1
* ע������: û�б�дLIN������صĺ���
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
#include "SWM180_uart.h"


/****************************************************************************************************************************************** 
* ��������:	UART_Init()
* ����˵��:	UART���ڳ�ʼ��
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
*			UART_InitStructure * initStruct    ����UART��������趨ֵ�Ľṹ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_Init(UART_TypeDef * UARTx, UART_InitStructure * initStruct)
{	
	switch((uint32_t)UARTx)
	{
	case ((uint32_t)UART0):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_UART0_Pos);
		break;
	
	case ((uint32_t)UART1):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_UART1_Pos);
		break;
	
	case ((uint32_t)UART2):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_UART2_Pos);
		break;
	
	case ((uint32_t)UART3):
		SYS->CLKEN |= (0x01 << SYS_CLKEN_UART3_Pos);
		break;
	}
	
	UART_Close(UARTx);	//һЩ�ؼ��Ĵ���ֻ���ڴ��ڹر�ʱ����
	
	UARTx->CTRL |= (0x01 << UART_CTRL_BAUDEN_Pos);
	UARTx->BAUD &= ~UART_BAUD_BAUD_Msk;
	UARTx->BAUD |= ((SystemCoreClock/16/initStruct->Baudrate - 1) << UART_BAUD_BAUD_Pos);
	
	UARTx->FIFO &= ~(UART_FIFO_RXTHR_Msk | UART_FIFO_TXTHR_Msk);
	UARTx->FIFO |= (initStruct->RXThreshold << UART_FIFO_RXTHR_Pos) | 
				   (initStruct->TXThreshold << UART_FIFO_TXTHR_Pos);
	
	UARTx->CTRL &= ~UART_CTRL_TOTIME_Msk;
	UARTx->CTRL |= (initStruct->TimeoutTime << UART_CTRL_TOTIME_Pos);
	
	UARTx->CTRL &= ~(UART_CTRL_RXIE_Msk | UART_CTRL_TXIE_Msk | UART_CTRL_TOIE_Msk);
	UARTx->CTRL |= (initStruct->RXThresholdIEn << UART_CTRL_RXIE_Pos) |
				   (initStruct->TXThresholdIEn << UART_CTRL_TXIE_Pos) |
				   (initStruct->TimeoutIEn << UART_CTRL_TOIE_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	UART_Open()
* ����˵��:	UART���ڴ�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_Open(UART_TypeDef * UARTx)
{
	UARTx->CTRL |= (0x01 << UART_CTRL_EN_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	UART_Close()
* ����˵��:	UART���ڹر�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_Close(UART_TypeDef * UARTx)
{
	UARTx->CTRL &= ~(0x01 << UART_CTRL_EN_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	UART_WriteByte()
* ����˵��:	����һ���ֽ�����
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���ȡֵ����UART0��UART1��UART2��UART3
*			uint8_t data			Ҫ���͵��ֽ�			
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_WriteByte(UART_TypeDef * UARTx, uint8_t data)
{
	UARTx->DATA = data;
}

/****************************************************************************************************************************************** 
* ��������:	UART_ReadByte()
* ����˵��:	��ȡһ���ֽ����ݣ���ָ�������Ƿ�Valid
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���ȡֵ����UART0��UART1��UART2��UART3
*			uint32_t * data			���յ�������
* ��    ��: uint32_t				1 ����Valid    0 ���ݴ���֡����
* ע������: ��
******************************************************************************************************************************************/
uint32_t UART_ReadByte(UART_TypeDef * UARTx, uint32_t * data)
{
	uint32_t reg = UARTx->DATA;
	
	*data = (reg & UART_DATA_DATA_Msk);
	
	return (reg & UART_DATA_VALID_Msk) ? 1 : 0;
}

/****************************************************************************************************************************************** 
* ��������:	UART_IsTXBusy()
* ����˵��:	UART�Ƿ����ڷ�������
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: uint32_t				1 UART���ڷ�������    0 �����ѷ���
* ע������: ��
******************************************************************************************************************************************/
uint32_t UART_IsTXBusy(UART_TypeDef * UARTx)
{
	return (UARTx->CTRL & UART_CTRL_TXIDLE_Msk) ? 0 : 1;
}

/****************************************************************************************************************************************** 
* ��������:	UART_IsRXFIFOEmpty()
* ����˵��:	����FIFO�Ƿ�Ϊ�գ����������˵�����������ݿ��Զ�ȡ
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: uint32_t				1 ����FIFO��    0 ����FIFO�ǿ�
* ע������: ��
******************************************************************************************************************************************/
uint32_t UART_IsRXFIFOEmpty(UART_TypeDef * UARTx)
{
	return (UARTx->CTRL & UART_CTRL_RXNE_Msk) ? 0 : 1;
}

/****************************************************************************************************************************************** 
* ��������:	UART_IsTXFIFOFull()
* ����˵��:	����FIFO�Ƿ�Ϊ���������������Լ���������д������
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: uint32_t				1 ����FIFO��    0 ����FIFO����
* ע������: ��
******************************************************************************************************************************************/
uint32_t UART_IsTXFIFOFull(UART_TypeDef * UARTx)
{
	return (UARTx->CTRL & UART_CTRL_TXF_Msk) ? 1 : 0;
}

/****************************************************************************************************************************************** 
* ��������:	UART_SetBaudrate()
* ����˵��:	���ò�����
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
*			uint32_t baudrate		Ҫ���õĲ�����
* ��    ��: ��
* ע������: ��Ҫ�ڴ��ڹ���ʱ���Ĳ����ʣ�ʹ�ô˺���ǰ���ȵ���UART_Close()�رմ���
******************************************************************************************************************************************/
void UART_SetBaudrate(UART_TypeDef * UARTx, uint32_t baudrate)
{
	UARTx->BAUD &= ~UART_BAUD_BAUD_Msk;
	UARTx->BAUD |= ((SystemCoreClock/16/baudrate - 1) << UART_BAUD_BAUD_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	UART_GetBaudrate()
* ����˵��:	��ѯ������
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: uint32_t				��ǰ������
* ע������: ��
******************************************************************************************************************************************/
uint32_t UART_GetBaudrate(UART_TypeDef * UARTx)
{
	return (UARTx->BAUD & UART_BAUD_BAUD_Msk);
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTRXThresholdEn()
* ����˵��:	��RX FIFO�����ݸ��� >= RXThresholdʱ �����ж�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_INTRXThresholdEn(UART_TypeDef * UARTx)
{
	UARTx->CTRL |= (0x01 << UART_CTRL_RXIE_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTRXThresholdDis()
* ����˵��:	��RX FIFO�����ݸ��� >= RXThresholdʱ �������ж�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_INTRXThresholdDis(UART_TypeDef * UARTx)
{
	UARTx->CTRL &= ~(0x01 << UART_CTRL_RXIE_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTRXThresholdStat()
* ����˵��:	�Ƿ�RX FIFO�����ݸ��� >= RXThreshold
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: uint32_t				1 RX FIFO�����ݸ��� >= RXThreshold		0 RX FIFO�����ݸ��� < RXThreshold
* ע������: RXIF = RXTHRF & RXIE
******************************************************************************************************************************************/
uint32_t UART_INTRXThresholdStat(UART_TypeDef * UARTx)
{
	return (UARTx->BAUD & UART_BAUD_RXIF_Msk) ? 1 : 0;
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTTXThresholdEn()
* ����˵��:	��TX FIFO�����ݸ��� <= TXThresholdʱ �����ж�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_INTTXThresholdEn(UART_TypeDef * UARTx)
{
	UARTx->CTRL |= (0x01 << UART_CTRL_TXIE_Pos);	
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTTXThresholdDis()
* ����˵��:	��TX FIFO�����ݸ��� <= TXThresholdʱ �������ж�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_INTTXThresholdDis(UART_TypeDef * UARTx)
{
	UARTx->CTRL &= ~(0x01 << UART_CTRL_TXIE_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTTXThresholdStat()
* ����˵��:	�Ƿ�TX FIFO�����ݸ��� <= TXThreshold
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: uint32_t				1 TX FIFO�����ݸ��� <= TXThreshold		0 TX FIFO�����ݸ��� > TXThreshold
* ע������: TXIF = TXTHRF & TXIE
******************************************************************************************************************************************/
uint32_t UART_INTTXThresholdStat(UART_TypeDef * UARTx)
{
	return (UARTx->BAUD & UART_BAUD_TXIF_Msk) ? 1 : 0;
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTTimeoutEn()
* ����˵��:	���շ�����ʱʱ �����ж�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_INTTimeoutEn(UART_TypeDef * UARTx)
{
	UARTx->CTRL |= (0x01 << UART_CTRL_TOIE_Pos);	
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTTimeoutDis()
* ����˵��:	���շ�����ʱʱ �������ж�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void UART_INTTimeoutDis(UART_TypeDef * UARTx)
{
	UARTx->CTRL &= ~(0x01 << UART_CTRL_TOIE_Pos);
}

/****************************************************************************************************************************************** 
* ��������:	UART_INTTimeoutStat()
* ����˵��:	�Ƿ����˽��ճ�ʱ�������� TimeoutTime/(Baudrate/10) ��û����RX���Ͻ��յ�����ʱ�����ж�
* ��    ��: UART_TypeDef * UARTx	ָ��Ҫ�����õ�UART���ڣ���Чֵ����UART0��UART1��UART2��UART3
* ��    ��: uint32_t				1 �����˽��ճ�ʱ		0 δ�������ճ�ʱ
* ע������: ��
******************************************************************************************************************************************/
uint32_t UART_INTTimeoutStat(UART_TypeDef * UARTx)
{
	return (UARTx->BAUD & UART_BAUD_TOIF_Msk) ? 1 : 0;
}

