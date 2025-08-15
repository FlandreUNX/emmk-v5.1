//
// Created by Fland on 24-10-23.
//

#include "ads1115.h"

#include "emmk-config.h"
#include "emmk-driver.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
static const char *tag = "ads1115";
#define TAG     tag

/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */

/*@{*/


static inline int32_t readReg(ads1115_Instance_t *ins, uint8_t regAddr, uint8_t *data) {
    if (kdi2c_write(ins->i2cIf, ins->address, &regAddr, 1) < 0) {
        return -1;
    }
    return kdi2c_read(ins->i2cIf, ins->address, data, 2);
}


static inline int32_t writeReg(ads1115_Instance_t *ins, uint8_t regAddr, uint8_t *data) {
    uint8_t d[3] = {
            regAddr, data[0], data[1]
    };
    return kdi2c_write(ins->i2cIf, ins->address, d, 3);
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

void ads1115_initBus(ads1115_Instance_t *ins, void *kdi2c) {
    ASSERT(ins != NULL);
    ASSERT(kdi2c != NULL);
    
    if (ins->i2cIf != NULL) {
        return;
    }
    ins->i2cIf = kdi2c;
    kdi2c_init(((kdi2c_t *) ins->i2cIf));
    kdi2c_powerUp(((kdi2c_t *) ins->i2cIf));
}

int32_t ads1115_initSoft(ads1115_Instance_t *ins) {
    ASSERT(ins != NULL);
    
    int32_t rc;
    uint8_t data[2] = {0};
    
    if (readReg(ins, 0x01, data) != 0) {
        rc = -1;
        goto l_errorExit;
    }
    ins->reg.config.u16 = ((((uint16_t) data[0]) << 8) & 0xFF00) | (((uint16_t) data[1]) & 0x00FF);
    
    LOG_I("softInit, Success, addr=%02X", ins->address);
    return 0;
    
    l_errorExit:
    LOG_E("softInit, Failed, rc=%ld, addr=%02X", rc, ins->address);
    return rc;
}

void ads1115_finalizeBus(ads1115_Instance_t *ins) {
    ASSERT(ins != NULL);
    
    if (ins->i2cIf == NULL) {
        return;
    }
    kdi2c_powerDown(((kdi2c_t *) ins->i2cIf));
    kdi2c_finalize(((kdi2c_t *) ins->i2cIf));
    ins->i2cIf = NULL;
}

void ads1115_finalizeSoft(ads1115_Instance_t *ins) {
}

int32_t ads1115_updateConfig(ads1115_Instance_t *ins) {
    ASSERT(ins != NULL);
    
    ads1115_Config_t config;
    config.u16 = ins->reg.config.u16;
    uint8_t data[2] = {
            config.u16 >> 8,
            config.u16 & 0x00FF
    };
    if (writeReg(ins, 0x01, data) != 0) {
        LOG_W("updateConfig, Failed");
        return -1;
    }
    ins->reg.config.u16 = config.u16;
    return 0;
}

int32_t ads1115_reloadConfig(ads1115_Instance_t *ins) {
    ASSERT(ins != NULL);
    
    uint8_t data[2] = {0};
    if (readReg(ins, 0x01, data) != 0) {
        LOG_W("reloadConfig, Failed");
        return -1;
    }
    ins->reg.config.u16 = ((((uint16_t) data[0]) << 8) & 0xFF00) | (((uint16_t) data[1]) & 0x00FF);
    return 0;
}

void ads1115_getConfig(ads1115_Instance_t *ins, ads1115_Config_t *config) {
    ASSERT(ins != NULL);
    ASSERT(config != NULL);
    config->u16 = ins->reg.config.u16;
}

void ads1115_setConfig(ads1115_Instance_t *ins, ads1115_Config_t *config) {
    ASSERT(ins != NULL);
    ASSERT(config != NULL);
    ins->reg.config.u16 = config->u16;
}

int32_t ads1115_convertOnceShot(ads1115_Instance_t *ins) {
    ASSERT(ins != NULL);
    
    ads1115_Config_t config;
    config.u16 = ins->reg.config.u16;
    config.os = 1;
    uint8_t data[2] = {
            config.u16 >> 8,
            config.u16 & 0x00FF
    };
    if (writeReg(ins, 0x01, data) != 0) {
        LOG_W("setConvertOnceShot, Failed");
        return -1;
    }
    ins->reg.config.u16 = config.u16;
    return 0;
}

int32_t ads1115_reloadData(ads1115_Instance_t *ins) {
    ASSERT(ins != NULL);
    
    uint8_t data[2] = {0};
    if (readReg(ins, 0x00, data) != 0) {
        LOG_W("reloadData, Failed");
        return -1;
    }
    ins->reg.data = (((uint16_t) data[0]) << 8 & 0xFF00) | (((uint16_t) data[1]) & 0x00FF);
    return 0;
}

int16_t ads1115_getRaw(ads1115_Instance_t *ins) {
    ASSERT(ins != NULL);
    return *((int16_t *) (&ins->reg.data));
}

float ads1115_getVolt(ads1115_Instance_t *ins) {
    static const uint16_t PGA_SET[] = {
            6144, 4096, 2048, 1024, 512, 256, 256, 256
    };
    float mv = (float) ads1115_getRaw(ins) / 32768.0f * (float) PGA_SET[ins->reg.config.pga];
    return mv;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
