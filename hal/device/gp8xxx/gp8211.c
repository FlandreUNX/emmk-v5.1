//
// Created by Fland on 2024/6/22.
//

#include "emmk-config.h"
#include "emmk-driver.h"
#include "./gp8211.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "GP8211"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define OPERATE_ERR_CHECK_RETURN(x) if (x < 0) return -1
#define OPERATE_ERR_TRY(x) if (x < 0) {LOG_W("OPERATE_ERR_TRY"); goto l_try;}

#define OPERATE_CREATE_TRY(x) uint32_t try = x
#define OPERATE_TRY_CHECK() \
    l_try:                        \
    if (try != 0) {         \
        try--;              \
    } else {                \
        LOG_E("OperateFailed");                    \
        return -1; \
    }


#define RESOLUTION_12_BIT                           0x0FFF
#define RESOLUTION_15_BIT                           0x7FFF
#define GP8211_CONFIG_CURRENT_REG                  ((uint8_t) (0x02))
#define DFGP8211_I2C_DEVICEADDR                    ((0xB0) >> 1)   //!< i2c address

#define GP8211_STORE_TIMING_HEAD            0x02  ///< Store function timing start head
#define GP8211_STORE_TIMING_ADDR            0x10  ///< The first address for entering store timing
#define GP8211_STORE_TIMING_CMD1            0x03  ///< The command 1 to enter store timing
#define GP8211_STORE_TIMING_CMD2            0x00  ///< The command 2 to enter store timing
#define GP8211_STORE_TIMING_DELAY           10    ///< Store procedure interval delay time: 10ms, more than 7ms

/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */

/*@{*/

/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */

/*@{*/

/*@}*/

/**
 * @addtogroup Private hal func
 * @note none
 */

/*@{*/

static int32_t regWrite(GP8211_Instance_t *ins, uint8_t reg, uint8_t data) {
    uint8_t buff[2] = {
            reg, data,
    };
    return kdi2c_write(ins->iicIf, DFGP8211_I2C_DEVICEADDR, buff, 2);
}


static int32_t sendData(GP8211_Instance_t *ins, uint16_t data, uint8_t channel) {
    uint8_t buff[5] = {
            0x00,
            (data & 0xff),
            (data >> 8),
            (data & 0xff),
            (data >> 8)
    };
    int32_t rc = -1;
    if (channel == 0) {
        buff[0] = GP8211_CONFIG_CURRENT_REG;
        rc = kdi2c_write(ins->iicIf, DFGP8211_I2C_DEVICEADDR, buff, 3);
    } else if (channel == 1) {
        buff[0] = GP8211_CONFIG_CURRENT_REG << 1;
        rc = kdi2c_write(ins->iicIf, DFGP8211_I2C_DEVICEADDR, buff, 3);
    } else if (channel == 2) {
        buff[0] = GP8211_CONFIG_CURRENT_REG;
        rc = kdi2c_write(ins->iicIf, DFGP8211_I2C_DEVICEADDR, buff, 5);
    }
    return rc;
}

/*@}*/

/**
 * @addtogroup Public Func
 * @note none
 */

/*@{*/

int32_t gp8211_hardInit(GP8211_Instance_t *ins, void *iicIf) {
    ASSERT(ins != NULL);
    ASSERT(iicIf != NULL);

    ins->iicIf = iicIf;
    kdi2c_init(ins->iicIf);
    kdi2c_powerUp(ins->iicIf);

    return 0;
}


int32_t gp8211_hardFinalize(GP8211_Instance_t *ins) {
    ASSERT(ins != NULL);
    kdi2c_powerDown(ins->iicIf);
    kdi2c_finalize(ins->iicIf);

    return 0;
}


int32_t gp8211_softInit(GP8211_Instance_t *ins) {
    ASSERT(ins != NULL);
    ins->config.resolution = RESOLUTION_15_BIT;
    ins->config.range = GP8211_RANGE_5V0;
    return gp8211_setRange(ins, ins->config.range);
}


void gp8211_softFinalize(GP8211_Instance_t *ins) {
    ASSERT(ins != NULL);
}


int32_t gp8211_setRange(GP8211_Instance_t *ins, GP8211_Range_t range) {
    uint8_t data = 0x55;
    OPERATE_CREATE_TRY(3);
OPERATE_TRY_CHECK()
    switch (range) {
        case GP8211_RANGE_5V0:
            OPERATE_ERR_TRY(regWrite(ins, GP8211_CONFIG_CURRENT_REG >> 1, data))
            break;
        case GP8211_RANGE_10V0:
            data = 0x77;
            OPERATE_ERR_TRY(regWrite(ins, GP8211_CONFIG_CURRENT_REG >> 1, data))
            break;
        default:
            break;
    }
    ins->config.range = range;
    return 0;
}


int32_t gp8211_setResolution(GP8211_Instance_t *ins, uint16_t resolution) {
    ASSERT(ins != NULL);
    ins->config.resolution = resolution;
    return 0;
}

uint32_t gp8211_caliRawByVolt(GP8211_Instance_t *ins, float mv) {
    uint16_t mv16;
    if (ins->config.range == GP8211_RANGE_5V0) {
        mv16 = (uint16_t) (mv / 5000.0f * (float) 0x7FFF);
    } else {
        mv16 = (uint16_t) (mv / 10000.0f * (float) 0x7FFF);
    }
    return mv16;
}

float gp8211_getVoltByRaw(GP8211_Instance_t *ins, uint16_t value) {
    float mv = 0;
    if (ins->config.range == GP8211_RANGE_5V0) {
        mv = (float) value / (float) 0x7FFF * 5000.0f;
    } else {
        mv = (float) value / (float) 0x7FFF * 10000.0f;
    }
    return mv;
}

int32_t gp8211_setOutVolt(GP8211_Instance_t *ins, uint16_t voltage, uint8_t channel) {
    ASSERT(ins != NULL);

    if (voltage > ins->config.resolution) {
        voltage = ins->config.resolution;
    }

    OPERATE_CREATE_TRY(3);
OPERATE_TRY_CHECK()
    OPERATE_ERR_TRY(sendData(ins, voltage, channel))
    return 0;
}


int32_t gp8211_setOutVolt_gp8512(GP8211_Instance_t *ins, uint16_t voltage, uint8_t channel) {
    ASSERT(ins != NULL);

    if (voltage > ins->config.resolution) {
        voltage = ins->config.resolution;
    }

    OPERATE_CREATE_TRY(3);
OPERATE_TRY_CHECK()
    OPERATE_ERR_TRY(sendData(ins, voltage, channel))
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
