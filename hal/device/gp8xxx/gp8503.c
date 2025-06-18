//
// Created by Fland on 2024/6/22.
//

#include "emmk-config.h"
#include "emmk-driver.h"
#include "./gp8503.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
static const char *tag = "gp8503";
#define TAG tag

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define I2C_DEVICEADDR                    ((0xB0) >> 1)   //!< i2c address

/*@}*/

/**
 * @addtogroup Public Func
 * @note none
 */

/*@{*/

int32_t gp8503_hardInit(gp8503_Instance_t *ins, void *iicIf) {
    ASSERT(ins != NULL);
    ASSERT(iicIf != NULL);

    ins->kdi2c = iicIf;
    kdi2c_init(ins->kdi2c);
    kdi2c_powerUp(ins->kdi2c);

    return 0;
}


int32_t gp8503_hardFinalize(gp8503_Instance_t *ins) {
    ASSERT(ins != NULL);
    kdi2c_powerDown(ins->kdi2c);
    kdi2c_finalize(ins->kdi2c);

    return 0;
}


int32_t gp8503_softInit(gp8503_Instance_t *ins, gp8503_Range_t range, float vcc) {
    ASSERT(ins != NULL);
    ins->config.range = range;
    ins->config.vcc = vcc;
    return 0;
}


void gp8503_softFinalize(gp8503_Instance_t *ins) {
    ASSERT(ins != NULL);
}

int32_t gp8503_setOutVolt(gp8503_Instance_t *ins, gp8503_Channel_t ch, float mv) {
    ASSERT(ins != NULL);

    if (mv == ins->output[ch].f) {
        return 0;
    }

    uint16_t mv16 = 0;
    if (ins->config.range == GP8503_RANGE_2V5) {
        mv16 = ((uint16_t) (mv / 2500.0f)) * 0xFFF;
    } else {
        mv16 = ((uint16_t) (mv / ins->config.vcc)) * 0xFFF;
    }

    mv16 = mv16 << 4;

    uint8_t data[3] = {
            ch,
            mv16 & 0x00FF,
            (mv16 >> 8) & 0x00FF,
    };
    int32_t rc = kdi2c_write(ins->kdi2c, I2C_DEVICEADDR, data, 3);
    if (rc != 0) {
        LOG_W("setOutVolt, Failed");
        return rc;
    }
    ins->output[ch].f = mv;
    return rc;
}


int32_t gp8503_setOutVolt2(gp8503_Instance_t *ins, float mv0, float mv1) {
    ASSERT(ins != NULL);

    if (mv0 == ins->output[0].f && mv1 == ins->output[1].f) {
        return 0;
    }

    uint16_t mv16[2] = {0};
    if (ins->config.range == GP8503_RANGE_2V5) {
        mv16[0] = (uint16_t) (mv0 / 2500.0f * (float) 0xFFF);
        mv16[1] = (uint16_t) (mv1 / 2500.0f * (float) 0xFFF);
    } else {
        mv16[0] = (uint16_t) (mv0 / ins->config.vcc * (float) 0xFFF);
        mv16[1] = (uint16_t) (mv1 / ins->config.vcc * (float) 0xFFF);
    }

    mv16[0] = mv16[0] << 4;
    mv16[1] = mv16[1] << 4;

    uint8_t data[5] = {
            0x02,
            mv16[0] & 0x00FF,
            (mv16[0] >> 8) & 0x00FF,
            mv16[1] & 0x00FF,
            (mv16[1] >> 8) & 0x00FF,
    };
    int32_t rc = kdi2c_write(ins->kdi2c, I2C_DEVICEADDR, data, 5);
    if (rc != 0) {
        LOG_W("setOutVolt2, Failed");
        return rc;
    }
    ins->output[0].f = mv0;
    ins->output[1].f = mv1;
    return rc;
}

uint32_t gp8503_caliRawByVolt(gp8503_Instance_t *ins, float mv) {
    uint16_t mv16;
    if (ins->config.range == GP8503_RANGE_2V5) {
        mv16 = (uint16_t) (mv / 2500.0f * (float) 0xFFF);
    } else {
        mv16 = (uint16_t) (mv / ins->config.vcc * (float) 0xFFF);
    }
    mv16 = mv16 << 4;
    return mv16;
}

int32_t gp8503_setOutRaw2(gp8503_Instance_t *ins, uint16_t u1, uint16_t u2) {
    ASSERT(ins != NULL);

    if (u1 == ins->output[0].u && u2 == ins->output[1].u) {
        return 0;
    }
    uint8_t data[5] = {
            0x02,
            u1 & 0x00FF,
            (u1 >> 8) & 0x00FF,
            u2 & 0x00FF,
            (u2 >> 8) & 0x00FF,
    };
    int32_t rc = kdi2c_write(ins->kdi2c, I2C_DEVICEADDR, data, 5);
    if (rc != 0) {
        LOG_W("setRaw2, Failed");
        return rc;
    }
    ins->output[0].u = u1;
    ins->output[1].u = u2;
    return rc;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
