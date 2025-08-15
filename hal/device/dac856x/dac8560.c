//
// Created by Fland on 2024/6/22.
//

#include "emmk-config.h"
#include "emmk-driver.h"
#include "./dac8560.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG "dac8560"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define COMMAND_INTERNAL_REF_CTL_DISABLE     0
#define COMMAND_INTERNAL_REF_CTL_ENABLE1     1
#define COMMAND_INTERNAL_REF_CTL_ENABLE2     2

static const uint8_t COMMAND[3][3] = {
    {0b01001000, 0b00000100, 0b00000001},
    {0b01001100, 0b00000100, 0b00000000},
    {0b01001001, 0b00000100, 0b00000001},
};



/*@}*/

/**
 * @addtogroup Private Func
 * @note none
 */

/*@{*/

static void writeCommand(dac8560_Instance_t *ins, uint8_t cmd) {
    kdspi_sendData(ins->kdspi, (uint8_t *) &COMMAND[cmd][0], 3, -1);
}


static void writeData(dac8560_Instance_t *ins, uint8_t pd, uint16_t data) {
    uint8_t buf[3] = {
        pd, data >> 8, data & 0xFF
    };
    kdspi_sendData(ins->kdspi, buf, 3, -1);
}

/*@}*/

/**
 * @addtogroup Public Func
 * @note none
 */

/*@{*/

int32_t dac8560_hardInit(dac8560_Instance_t *ins, void *kdspi) {
    ASSERT(ins != NULL);
    ASSERT(kdspi != NULL);

    ins->kdspi = kdspi;
    kdspi_init(ins->kdspi);
    kdspi_powerUp(ins->kdspi);

    return 0;
}

int32_t dac8560_hardFinalize(dac8560_Instance_t *ins) {
    ASSERT(ins != NULL);
    kdspi_powerDown(ins->kdspi);
    kdspi_finalize(ins->kdspi);

    return 0;
}

int32_t dac8560_softInit(dac8560_Instance_t *ins, bool iVrefEnable, uint16_t vref) {
    ASSERT(ins != NULL);
    if (iVrefEnable) {
        writeCommand(ins, COMMAND_INTERNAL_REF_CTL_ENABLE1);
        writeCommand(ins, COMMAND_INTERNAL_REF_CTL_ENABLE2);
    } else {
        writeCommand(ins, COMMAND_INTERNAL_REF_CTL_DISABLE);
    }
    ins->config.refMv = vref;
    return 0;
}

void dac8560_softFinalize(dac8560_Instance_t *ins) {

}

uint32_t dac8560_caliRawByVolt(dac8560_Instance_t *ins, float mv) {
    uint16_t mv16 = (uint16_t) (((mv / (float) ins->config.refMv)) * (float) 0xFFFF);
    return mv16;
}

int32_t dac8560_setOutVolt(dac8560_Instance_t *ins, dac8560_PowerCtl_t pd, float mv) {
    writeData(ins, pd, dac8560_caliRawByVolt(ins, mv));
    return 0;
}


int32_t dac8560_setOutRaw(dac8560_Instance_t *ins, dac8560_PowerCtl_t pd, uint16_t u) {
    writeData(ins, pd, u);
    return 0;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
