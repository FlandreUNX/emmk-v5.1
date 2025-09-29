/****************************************************************************************************************************************** 
* �ļ�����:	SWM180_flash.c
* ����˵��:	ʹ��оƬ��IAP���ܽ�Ƭ��Flashģ���EEPROM���������ݣ�����󲻶�ʧ
* ����֧��:	http://www.synwit.com.cn/e/tool/gbook/?bid=1
* ע������:
* �汾����: V1.0.0		2016��1��30��
* ������¼: 
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
#include "SWM180_flash.h"

typedef void (*IAPFunc)(uint32_t faddr, uint32_t raddr, uint32_t cnt, uint32_t cmd);
IAPFunc IAPfunc = (IAPFunc)0x1000601;

/****************************************************************************************************************************************** 
* ��������:	FLASH_Erase()
* ����˵��:	FLASH����������ÿ������4K�ֽ�
* ��    ��: uint32_t addr		Ҫ���������ĵ�ַ������4K���룬��addr%4096 == 0
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void FLASH_Erase(uint32_t addr)
{
	__disable_irq();
	
	IAPfunc(addr, 0, 0, 0x51);
	
	__enable_irq();
	
	CACHE_Invalid(addr);
}

/****************************************************************************************************************************************** 
* ��������:	FLASH_Write()
* ����˵��:	FLASH����д��
* ��    ��: uint32_t addr		����Ҫд�뵽Flash�еĵ�ַ���ֶ���
*			uint32_t buff[]		Ҫд��Flash�е�����
*			uint32_t cnt		Ҫд�����ݵĸ���������Ϊ��λ
* ��    ��: ��
* ע������: Ҫд������ݱ���ȫ����ͬһҳ�ڣ���addr/256 == (addr+cnt*4)/256
******************************************************************************************************************************************/
void FLASH_Write(uint32_t addr, uint32_t buff[], uint32_t cnt)
{	
	__disable_irq();
	
	IAPfunc(addr, (uint32_t)buff, cnt*4, 0x52);
	
	__enable_irq();
	
	CACHE_Invalid(addr);
}

/****************************************************************************************************************************************** 
* ��������:	FLASH_Read()
* ����˵��:	FLASH���ݶ�ȡ
* ��    ��: uint32_t addr		Ҫ��ȡ��������Flash�еĵ�ַ���ֶ���
*			uint32_t buff[]		��ȡ�������ݴ���buffָ����ڴ�
*			uint32_t cnt		Ҫ��ȡ�����ݵĸ���������Ϊ��λ
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void FLASH_Read(uint32_t addr, uint32_t buff[], uint32_t cnt)
{	
	uint32_t i;
	
	while(FLASH->STAT & FLASH_STAT_BUSY_Msk);
	
	FLASH->CR = (1 << FLASH_CR_FFCLR_Pos);			//Clear FIFO
	
	FLASH->CR = (FLASH_CMD_READ_DATA << FLASH_CR_CMD_Pos) |
				(0 << FLASH_CR_LEN_Pos);			//������
	FLASH->ADDR = addr;
	FLASH->START = 1;
	
	for(i = 0; i < cnt; i++)
	{
		while(FLASH->STAT & FLASH_STAT_FE_Msk);		//FIFO Empty
		
		buff[i] = FLASH->DATA;
	}
	
	FLASH->START = 0;
	
	FLASH->CR = (1 << FLASH_CR_FFCLR_Pos);			//Clear FIFO
	FLASH->CR = 0;
}



__attribute__((section("PlaceInRAM")))
void AppToIsp(void)
{
	__disable_irq();       //���һ��Ҫ��, ��ֹ�жϴ��DMA�Ŀ���	
	*(volatile unsigned int *)0x40000900 = 0x00; //4k mask = 0,�л���Isp��(rom code�Ѿ�����λ��1)
/********************************** Set Cache And Sram Switch *******************************/ 
//	DRAM_SIZE = 0x01; //��ֹcache�����������sramΪ16/16	//֮����ȷ��	
/********************************** Copy Code From Flash To Sram *******************************/	
	DMA->EN = 0x01;			//ÿ��ͨ�������Լ������Ŀ��ؿ��ƣ������ܿ��ؿ�����һֱ������				
	DMA->CH[DMA_CHR_FLASH].CR &= ~(0x01<<DMA_CR_REN_Pos);		//����ǰ�ȹرո�ͨ��	
	DMA->CH[DMA_CHR_FLASH].SRC = 0x400;  //FLASH_ADDR
	DMA->CH[DMA_CHR_FLASH].DST = 0x00;   //SRAM_ADDR	
	DMA->CH[DMA_CHR_FLASH].CR &= ~DMA_CR_LEN_Msk;				
	DMA->CH[DMA_CHR_FLASH].CR |= ((0xC00-1)<<DMA_CR_LEN_Pos);  //copy 3K from Isp to App
	DMA->CH[DMA_CHR_FLASH].CR |= (0x01<<DMA_CR_REN_Pos);	
	while((DMA->CH[DMA_CHR_FLASH].CR&DMA_CR_REN_Msk)==0x01);		
	while(FLASH->STAT&(0x01<<16));                 //�ȴ�FlashCtrl���ڿ���״̬
	*(volatile unsigned int *)0x40000900 = 0x01; //4k mask = 1,�л���App��(rom code�Ѿ�����λ��1)		
//	__enable_irq();	//����ִ��������λ���Բ�ʹ�����ж�
/********************************** System Reset(Address(0x00), Sram Code area) ****************/		
	SCB->AIRCR = ((0x5FA<<SCB_AIRCR_VECTKEY_Pos)|SCB_AIRCR_SYSRESETREQ_Msk); //ϵͳ������λ
	__DSB();                                    /* Ensure completion of memory access */              
	while(1);		
}
