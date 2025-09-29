/****************************************************************************************************************************************** 
* ΄¼þû³ƺ	system_SWM180.c
* ¹¦Ĝ˵÷:	SWM180µ¥Ƭ»úµĊ±֓ɨփ
* ¼¼ʵ֧³ֺ	http://www.synwit.com.cn/e/tool/gbook/?bid=1
* עҢʂϮ:
* °汾ȕƚ: V1.0.0		2016Ī1Ԃ30ȕ
* ɽ¼¶¼ǂ¼:
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
#include <stdint.h>
#include "emmk-v5.1/hal/swm180x/SWM180_StdPeriph_Driver/SWM180.h"


/******************************************************************************************************************************************
 * ϵͳʱ֓ɨ¶¨
 *****************************************************************************************************************************************/
#define SYS_CLK_24MHz	0	 	//0 Ě²¿¸߆µ24MHz RCձµ´Ʒ
#define SYS_CLK_6MHz	1		//1 Ě²¿¸߆µ 6MHz RCձµ´Ʒ
#define SYS_CLK_48MHz	2		//2 Ě²¿¸߆µ48MHz RCձµ´Ʒ
#define SYS_CLK_12MHz	3		//3 Ě²¿¸߆µ12MHz RCձµ´Ʒ
#define SYS_CLK_32KHz	4		//4 Ě²¿µ͆µ32KHz RCձµ´Ʒ
#define SYS_CLK_XTAL	5		//5 ͢²¿XTAL¾§̥ձµ´Ʒ£¨2-30MHz£©

#define SYS_CLK   	SYS_CLK_48MHz


#define __HSI		(24000000UL)		//¸ߋلڲ¿ʱ֓
#define __LSI		(   32000UL)		//µ͋لڲ¿ʱ֓
#define __HSE		(24000000UL)		//¸ߋٍⲿʱ֓



uint32_t SystemCoreClock  = __HSI;   				//System Clock Frequency (Core Clock)
uint32_t CyclesPerUs      = (__HSI / 1000000); 		//Cycles per micro second


/******************************************************************************************************************************************
* º¯ʽû³ƺ
* ¹¦Ĝ˵÷: This function is used to update the variable SystemCoreClock and must be called whenever the core clock is changed
* ʤ    ȫ:
* ʤ    ³ö:
* עҢʂϮ:
******************************************************************************************************************************************/
void SystemCoreClockUpdate(void)
{
	if(SYS->CLKSEL & SYS_CLKSEL_SYS_Msk)			//SYS_CLK  <= HFCK
	{
		if(SYS->CLKSEL & SYS_CLKSEL_HFCK_Msk)			//HFCK <= HRC/4
		{
			if(SYS->HRCCR & SYS_HRCCR_DBL_Msk)				//HRC = 48MHz
			{
				SystemCoreClock = __HSI*2/4;
			}
			else											//HRC = 24MHz
			{
				SystemCoreClock =  __HSI/4;
			}
		}
		else											//HFCK <= HRC
		{
			if(SYS->HRCCR & SYS_HRCCR_DBL_Msk)				//HRC = 48MHz
			{
				SystemCoreClock = __HSI*2;
			}
			else											//HRC = 24MHz
			{
				SystemCoreClock = __HSI;
			}
		}
	}
	else											//SYS_CLK  <= LFCK
	{
		if(SYS->CLKSEL & SYS_CLKSEL_LFCK_Msk)			//LFCK <= XTAL
		{
			SystemCoreClock = __HSE;
		}
		else											//LFCK <= LRC
		{
			SystemCoreClock = __LSI;
		}
	}
}

static void switchToRC24MHz(void);
static void switchToRC6MHz(void);
static void switchToRC48MHz(void);
static void switchToRC12MHz(void);
static void switchToRC32KHz(void);
static void switchToXTAL(void);
/******************************************************************************************************************************************
* º¯ʽû³ƺ
* ¹¦Ĝ˵÷: The necessary initializaiton of systerm
* ʤ    ȫ:
* ʤ    ³ö:
* עҢʂϮ:
******************************************************************************************************************************************/
void SystemInit(void)
{
	uint32_t i;

	SYS->CLKEN |= (1 << SYS_CLKEN_OSC_Pos);

	switch(SYS_CLK)
	{
		case SYS_CLK_24MHz:			//0 Ě²¿¸߆µ24MHz RCձµ´Ʒ
			if(SYS->CLKSEL & SYS_CLKSEL_SYS_Msk)	//µ±ǰʱ֓ʇ¸߆µRC£¬О¸ĸ߆µRCʱ֓ƵʱШҪψǐµ½һ¸öΈ¶¨ʱ֓Դ
			{
				switchToRC32KHz();
			}
			switchToRC24MHz();
			break;

		case SYS_CLK_6MHz:			//1 Ě²¿¸߆µ 6MHz RCձµ´Ʒ
			if(SYS->CLKSEL & SYS_CLKSEL_SYS_Msk)	//µ±ǰʱ֓ʇ¸߆µRC£¬О¸ĸ߆µRCʱ֓ƵʱШҪψǐµ½һ¸öΈ¶¨ʱ֓Դ
			{
				switchToRC32KHz();
			}
			switchToRC6MHz();
			break;

		case SYS_CLK_48MHz:			//2 Ě²¿¸߆µ48MHz RCձµ´Ʒ
			if(SYS->CLKSEL & SYS_CLKSEL_SYS_Msk)	//µ±ǰʱ֓ʇ¸߆µRC£¬О¸ĸ߆µRCʱ֓ƵʱШҪψǐµ½һ¸öΈ¶¨ʱ֓Դ
			{
				switchToRC32KHz();
			}
			switchToRC48MHz();
			break;

		case SYS_CLK_12MHz:			//3 Ě²¿¸߆µ12MHz RCձµ´Ʒ
			if(SYS->CLKSEL & SYS_CLKSEL_SYS_Msk)	//µ±ǰʱ֓ʇ¸߆µRC£¬О¸ĸ߆µRCʱ֓ƵʱШҪψǐµ½һ¸öΈ¶¨ʱ֓Դ
			{
				switchToRC32KHz();
			}
			switchToRC12MHz();
			break;

		case SYS_CLK_32KHz:			//4 Ě²¿µ͆µ32KHz RCձµ´Ʒ
			if((SYS->CLKSEL & SYS_CLKSEL_SYS_Msk) == 0)
			{
				switchToRC24MHz();
			}
			switchToRC32KHz();
			break;

		case SYS_CLK_XTAL:			//5 ͢²¿XTAL¾§̥ձµ´Ʒ£¨2-30MHz£©
			if((SYS->CLKSEL & SYS_CLKSEL_SYS_Msk) == 0)
			{
				switchToRC24MHz();
			}
			switchToXTAL();
			break;
	}

	for(i = 0;i <10000;i++);		//µȴýʱ֓Έ¶¨¡£¡£¡£
	
	SystemCoreClockUpdate();
}

static void switchToRC24MHz(void)
{
	SYS->HRCCR = (1 << SYS_HRCCR_EN_Pos) |
				 (0 << SYS_HRCCR_DBL_Pos);		//HRC = 24MHz
	
	SYS->CLKSEL &= ~SYS_CLKSEL_HFCK_Msk;		//HFCK  <=  HRC
	SYS->CLKSEL |= (1 << SYS_CLKSEL_SYS_Pos);	//SYS_CLK  <= HFCK
}

static void switchToRC6MHz(void)
{
	SYS->HRCCR = (1 << SYS_HRCCR_EN_Pos) |
				 (0 << SYS_HRCCR_DBL_Pos);		//HRC = 24MHz
	
	SYS->CLKSEL |= (1 << SYS_CLKSEL_HFCK_Pos);	//HFCK  <=  HRC/4
	SYS->CLKSEL |= (1 << SYS_CLKSEL_SYS_Pos);	//SYS_CLK  <= HFCK
}

static void switchToRC48MHz(void)
{
	SYS->HRCCR = (1 << SYS_HRCCR_EN_Pos) |
				 (1 << SYS_HRCCR_DBL_Pos);		//HRC = 48MHz		
	
	SYS->CLKSEL &= ~SYS_CLKSEL_HFCK_Msk;		//HFCK  <=  HRC
	SYS->CLKSEL |= (1 << SYS_CLKSEL_SYS_Pos);	//SYS_CLK  <= HFCK
}

static void switchToRC12MHz(void)
{
	SYS->HRCCR = (1 << SYS_HRCCR_EN_Pos) |
				 (1 << SYS_HRCCR_DBL_Pos);		//HRC = 48MHz
	
	SYS->CLKSEL |= (1 << SYS_CLKSEL_HFCK_Pos);	//HFCK  <=  HRC/4
	SYS->CLKSEL |= (1 << SYS_CLKSEL_SYS_Pos);	//SYS_CLK  <= HFCK
}

static void switchToRC32KHz(void)
{
	SYS->LRCCR = (1 << SYS_LRCCR_EN_Pos);
	
	SYS->CLKSEL &= ~(1 << SYS_CLKSEL_LFCK_Pos);	//LFCK  <=  LRC
	SYS->CLKSEL &= ~(1 << SYS_CLKSEL_SYS_Pos);	//SYS_CLK  <= LFCK
}

static void switchToXTAL(void)
{
	uint32_t i;
	
	PORT_Init(PORTC, PIN1, PORTC_PIN1_XTAL_IN, 0);
	PORT_Init(PORTC, PIN0, PORTC_PIN0_XTAL_OUT, 0);
	
	SYS->XTALCR = (1 << SYS_XTALCR_EN_Pos);
	
	for(i = 0; i < 22118; i++);
	
	SYS->CLKSEL |= (1 << SYS_CLKSEL_LFCK_Pos);	//LFCK  <=  XTAL
	SYS->CLKSEL &= ~(1 << SYS_CLKSEL_SYS_Pos);	//SYS_CLK  <= LFCK
}
