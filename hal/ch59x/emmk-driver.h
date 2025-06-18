//
// Created by unx on 2023/4/29.
//

#ifndef GET_STARTED_EMMK_DRIVER_H
#define GET_STARTED_EMMK_DRIVER_H

#include <stdint.h>

/**
 * @addtogroup Verison
 * @note none
 */

/*@{*/

#define DEVICE_VERSION            "CH59X"
#define DEVICE_MAJOR_VERSION      "0"
#define DEVICE_SECONDARY_VERSION  "1"

/*@}*/

#include "CH59x_common.h"
#include "HAL.h"
#include "CH592SFR.h"
#include "CONFIG.h"

#include "core_riscv.h"

#include "./driver/kdmisc.h"
#include "./driver/kdgpio.h"
#include "./driver/kduart.h"
#include "./driver/kdimtd.h"
//#include "./driver/kdi2c.h"

/**
 * @addtogroup Instance method
 * @note none
 */

/*@{*/

#define AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes) \
__attribute__((section("noncacheable"))) __attribute__((aligned(alignbytes))) var

#define R32_GPIO_PD_DRV(x)   (*(__IO uint32_t *) (x + GPIO_PD_DRV))
#define R32_GPIO_PU(x)       (*(__IO uint32_t *) (x + GPIO_PU))
#define R32_GPIO_DIR(x)      (*(__IO uint32_t *) (x + GPIO_DIR))
#define R32_GPIO_OUT(x)      (*(__IO uint32_t *) (x + GPIO_OUT))
#define R32_GPIO_CLR(x)      (*(__IO uint32_t *) (x + GPIO_CLR))

#define R8_UART_MCR(x) (*(__IO uint8_t *) (x + UART_MCR))
#define R8_UART_FCR(x) (*(__IO uint8_t *) (x + UART_FCR))
#define R8_UART_LCR(x) (*(__IO uint8_t *) (x + UART_LCR))
#define R8_UART_LSR(x) (*(__IO uint8_t *) (x + UART_LSR))
#define R8_UART_RFC(x) (*(__IO uint8_t *) (x + UART_RFC))
#define R8_UART_IER(x) (*(__IO uint8_t *) (x + UART_IER))
#define R8_UART_DIV(x) (*(__IO uint8_t *) (x + UART_DIV))
#define R8_UART_IIR(x) (*(__IO uint8_t *) (x + UART_IIR))
#define R8_UART_THR(x) (*(__IO uint8_t *) (x + UART_THR))
#define R8_UART_TFC(x) (*(__IO uint8_t *) (x + UART_TFC))
#define R8_UART_RBR(x) (*(__IO uint8_t *) (x + UART_RBR))
#define R16_UART_DL(x) (*(__IO uint16_t *) (x + UART_DLL))

#define R32_TMR_CONTROL(x)      (*(__IO uint32_t *) (x + TMR_CTRL_MOD))
#define R8_TMR_CTRL_MOD(x)      (*(__IO uint8_t *) (x + TMR_CTRL_MOD))
#define R8_TMR_INTER_EN(x)      (*(__IO uint8_t *) (x + TMR_INTER_EN))
#define R8_TMR_INT_FLAG(x)      (*(__IO uint8_t *) (x + TMR_INT_FLAG))
#define R8_TMR_FIFO_COUNT(x)      (*(__IO uint8_t *) (x + TMR_FIFO_COUNT))
#define R32_TMR_COUNT(x)      (*(__IO uint32_t *) (x + TMR_COUNT))
#define R32_TMR_CNT_END(x)      (*(__IO uint32_t *) (x + TMR_CNT_END))
#define R32_TMR_FIFO(x)      (*(__IO uint32_t *) (x + TMR_FIFO))

extern void gpio_config(uint32_t b, uint32_t pin, GPIOModeTypeDef mode);

/*@}*/

/**
 * @addtogroup BaseFunc
 * @note none
 */

/*@{*/

extern void kdpwr_reboot(void);
extern void kdpwr_shutDown(void);
extern void kdpwr_deepSleep(void);
extern void kdpwr_sysInit(void);

extern int32_t emmkDriver_initRefsCountUp(int8_t *initRefs);
extern int32_t emmkDriver_initRefsCountDown(int8_t *initRefs);

/*@}*/

#endif //GET_STARTED_EMMK_DRIVER_H
