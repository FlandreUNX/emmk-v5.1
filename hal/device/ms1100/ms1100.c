//
// Created by Fland on 2024/6/22.
//

#include "emmk-config.h"
#include "emmk-driver.h"
#include "./ms1100.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "MS1100"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define OPERATE_ERR_CHECK_RETURN(x) if (x < 0) return -1
#define OPERATE_ERR_TRY(x) if (x < 0) {LOG_W("OperateRetry"); goto l_try;}

#define OPERATE_CREATE_TRY(x) uint32_t try = x
#define OPERATE_TRY_CHECK() \
    l_try:                        \
    if (try != 0) {         \
        try--;              \
    } else {                \
        LOG_E("OperateFailed");                    \
        return -1; \
    }

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

static int32_t regWrite(MS1100_Instance_t *ins, uint8_t data) {
    return kdi2c_write(ins->iicIf, ins->addr, &data, 1);
}


static int32_t regRead(MS1100_Instance_t *ins, uint8_t data[3]) {
    return kdi2c_read(ins->iicIf, ins->addr, data, 3);
}

/*@}*/

/**
 * @addtogroup Public Func
 * @note none
 */

/*@{*/

int32_t ms1100_init(MS1100_Instance_t *ins, void *iicIf, uint8_t addr) {
    ASSERT(ins != NULL);
    ASSERT(iicIf != NULL);
    ins->addr = addr;
    ins->iicIf = iicIf;
    kdi2c_init(ins->iicIf);
    kdi2c_powerUp(ins->iicIf);
    
    OPERATE_CREATE_TRY(3);
    OPERATE_TRY_CHECK();
    OPERATE_ERR_TRY(regRead(ins, ins->config.d8));
    return 0;
}


int32_t ms1100_finalize(MS1100_Instance_t *ins) {
    ASSERT(ins != NULL);
    kdi2c_powerDown(ins->iicIf);
    kdi2c_finalize(ins->iicIf);
    return 0;
}


void ms1100_setPga(MS1100_Instance_t *ins, MS1100_Pga_t pga) {
    ASSERT(ins != NULL);
    ins->config.pga = pga;
}


void ms1100_setDr(MS1100_Instance_t *ins, MS1100_Dr_t dr) {
    ASSERT(ins != NULL);
    ins->config.dr = dr;
}


void ms1100_setOnceMode(MS1100_Instance_t *ins, bool s) {
    ASSERT(ins != NULL);
    ins->config.onceMode = s;
}


int32_t ms1100_updateConfig(MS1100_Instance_t *ins) {
    ASSERT(ins != NULL);
    uint8_t cfg = ins->config.d8[2];
    OPERATE_CREATE_TRY(3);
    OPERATE_TRY_CHECK();
    OPERATE_ERR_TRY(regWrite(ins, cfg));
    OPERATE_ERR_TRY(regRead(ins, ins->config.d8));
    
    LOG_I("CFG[%02X], REG[%02X,%02X,%02X]", cfg, ins->config.d8[0], ins->config.d8[1], ins->config.d8[2]);
    
    if (cfg != (ins->config.d8[2] & 0x7F)) {
        LOG_W("Cfg(%02X) notEqual!", cfg);
        return -1;
    }
    return 0;
}


uint8_t ms1100_isDataReady(MS1100_Instance_t *ins) {
    return ins->config.stDrdy;
}


int32_t ms1100_read(MS1100_Instance_t *ins) {
    ASSERT(ins != NULL);
    OPERATE_CREATE_TRY(3);
OPERATE_TRY_CHECK();
    OPERATE_ERR_TRY(regRead(ins, ins->config.d8));
    return 0;
}


int16_t ms1100_getAd(MS1100_Instance_t *ins) {
    uint16_t ui16 = ((((uint16_t) ins->config.d8[0]) << 8) & 0xFF00)
                  | ((((uint16_t) ins->config.d8[1])) & 0x00FF);
    return *((int16_t *) &ui16);
}


float ms1100_getVolt(MS1100_Instance_t *ins, int16_t ad) {
    uint32_t b;
    switch (ins->config.dr) {
        case MS1100_DR_240SPS_12B: {
            b = 2048;
            if (ad & KLBIT(11)) {
                ad &= ~(int16_t) KLBIT(11);
                ad |= (int16_t) KLBIT(15);
            }
            break;
        }
        case MS1100_DR_60SPS_14B: {
            b = 8192;
            if (ad & KLBIT(13)) {
                ad &= ~(int16_t) KLBIT(13);
                ad |= (int16_t) KLBIT(15);
            }
            break;
        }
        case MS1100_DR_30SPS_15B: {
            b = 16384;
            if (ad & KLBIT(14)) {
                ad &= ~(int16_t) KLBIT(14);
                ad |= (int16_t) KLBIT(15);
            }
            break;
        }
        case MS1100_DR_15SPS_16B: {
            b = 32768;
            break;
        }
    }
//    float v = ((float) ad) / ((float) (b >> ins->config.pga)) * ((float) (2048));
    float v = ((float) ad) * (((float) (2048 >> ins->config.pga)) / (float) b);
    return v;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
