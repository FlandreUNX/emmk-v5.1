//
// Created by Fland on 2024/6/22.
//

#include "emmk-config.h"
#include "emmk-driver.h"
#include "./dac8562.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG "dac8562"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

typedef enum {
    WRITE_INPUT = 0b00000000,
    SFT_LDAC = 0b00001000,
    WRITE_INPUT_UPDATE_ALL = 0b00010000,
    WRITE_INPUT_UPDATE_N = 0b00011000,
    POWER_UP_DOWN = 0b00100000,
    SOFT_RESET = 0b00101000,
    SET_LDAC = 0b00110000,
    INTERNAL_REF_CTL = 0b00111000,
} Command_t;

typedef enum {
    ADDR_CHN_A = 0b00000000,
    ADDR_CHN_B = 0b00000001,
    ADDR_GAIN = 0b00000010,
    ADDR_CHN_AB = 0b00000011,
} Address_t;

/// @Command_t INTERNAL_REF_CTL
#define CMD_DATA_IREF_ENABLE     0x0001
#define CMD_DATA_IREF_DISABLE    0x0000

/// @Command_t WRITE_INPUT
/// @dac8562_Gain_t

/// @Command_t POWER_UP_DOWN
/// @dac8562_PowerCtl_t

/// @Command_t SET_LDAC
#define CMD_DATA_SET_LDAC_A1_B1   0x0000
#define CMD_DATA_SET_LDAC_A0_B1   0x0001
#define CMD_DATA_SET_LDAC_A1_B0   0x0002
#define CMD_DATA_SET_LDAC_A0_B0   0x0003

/*@}*/

/**
 * @addtogroup Private Func
 * @note none
 */

/*@{*/

static void writeCommand(dac8562_Instance_t *ins, Command_t cmd, Address_t addr, uint16_t data) {
    uint8_t buf[3] = {
            cmd | addr,
            (data >> 8) & 0x00FF,
            data & 0x00FF
    };
    kdspi_sendData(ins->kdspi, buf, 3, -1);
}

/*@}*/

/**
 * @addtogroup Public Func
 * @note none
 */

/*@{*/

int32_t dac8562_hardInit(dac8562_Instance_t *ins, void *kdspi) {
    ASSERT(ins != NULL);
    ASSERT(kdspi != NULL);

    ins->kdspi = kdspi;
    kdspi_init(ins->kdspi);
    kdspi_powerUp(ins->kdspi);

    return 0;
}

int32_t dac8562_hardFinalize(dac8562_Instance_t *ins) {
    ASSERT(ins != NULL);
    kdspi_powerDown(ins->kdspi);
    kdspi_finalize(ins->kdspi);

    return 0;
}

int32_t dac8562_softInit(dac8562_Instance_t *ins, bool iVrefEnable, dac8562_Gain_t gain, uint16_t vref) {
    ASSERT(ins != NULL);
    writeCommand(ins, SOFT_RESET, 0, 0x0001);
    writeCommand(ins, POWER_UP_DOWN, 0, DAC8562_POWER_CTL_UP_AB);
    if (iVrefEnable) {
        writeCommand(ins, INTERNAL_REF_CTL, 0, CMD_DATA_IREF_ENABLE);
    } else {
        writeCommand(ins, INTERNAL_REF_CTL, 0, CMD_DATA_IREF_DISABLE);
    }
    ins->config.refMv = vref;
    writeCommand(ins, WRITE_INPUT, ADDR_GAIN, gain);
    writeCommand(ins, SET_LDAC, 0, CMD_DATA_SET_LDAC_A0_B0);
    return 0;
}

void dac8562_softFinalize(dac8562_Instance_t *ins, dac8562_PowerCtl_t ctl) {
}

void dac8562_powerControl(dac8562_Instance_t *ins, dac8562_PowerCtl_t ctl) {
    ASSERT(ins != NULL);
    if (ctl != DAC8562_POWER_CTL_IGNORE) {
        writeCommand(ins, POWER_UP_DOWN, 0, ctl);
    }
}

uint32_t dac8562_caliRawByVolt(dac8562_Instance_t *ins, float mv) {
    uint16_t mv16 = (uint16_t) (((mv / (float) ins->config.refMv)) * (float) 0xFFFF);
    return mv16;
}

int32_t dac8562_setOutVolt(dac8562_Instance_t *ins, dac8562_Channel_t chn, float mv) {
    ASSERT(ins != NULL);
    if (chn == DAC8562_CHN_AB) {
        return -1;
    }
    if (mv == ins->output[chn].f) {
        return 0;
    }

    ins->output[chn].f = mv;
    writeCommand(ins, WRITE_INPUT, chn, dac8562_caliRawByVolt(ins, mv));
    return 0;
}

int32_t dac8562_setOutVolt2(dac8562_Instance_t *ins, float mv) {
    ASSERT(ins != NULL);

    ins->output[0].f = mv;
    ins->output[1].f = mv;
    writeCommand(ins, WRITE_INPUT, ADDR_CHN_AB, dac8562_caliRawByVolt(ins, mv));
    return 0;
}

int32_t dac8562_setOutRaw(dac8562_Instance_t *ins, dac8562_Channel_t chn, uint16_t u) {
    ASSERT(ins != NULL);
    if (chn == DAC8562_CHN_AB) {
        return -1;
    }
    if (u == ins->output[chn].u) {
        return 0;
    }

    ins->output[chn].u = u;
    writeCommand(ins, WRITE_INPUT, chn, u);
    return 0;
}

int32_t dac8562_setOutRaw2(dac8562_Instance_t *ins, uint16_t u) {
    ASSERT(ins != NULL);
    ins->output[0].u = u;
    ins->output[1].u = u;
    writeCommand(ins, WRITE_INPUT, ADDR_CHN_AB, u);
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
