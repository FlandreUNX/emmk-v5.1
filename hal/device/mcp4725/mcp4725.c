/*
 * Copyright (C) 2018 Flandreunx@outlook.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "emmk-config.h"
#include "emmk-driver.h"

#include "./mcp4725.h"

/**
 * @addtogroup Private funcs
 * @note none
 */
 
/*@{*/

static int32_t fastWriteOnce(mcp4725_Instance_t *ins, uint16_t dac, mcp4725_PowerDownMode_t pd) {
    uint8_t b[3] = {
        (0x02 << 5) | (pd << 1),
        (((dac << 4) >> 8) & 0xFF),
        (dac << 4) & 0xFF
    };
    return kdi2c_write(ins->kdi2c, ins->address, b, 3);
}


static int32_t dacWithEepromWriteOnce(mcp4725_Instance_t *ins, uint16_t dac, mcp4725_PowerDownMode_t pd) {
    uint8_t b[3] = {
        (0x03 << 5) | (pd << 1),
        (((dac << 4) >> 8) & 0xFF),
        (dac << 4) & 0xFF
    };
    return kdi2c_write(ins->kdi2c, ins->address, b, 3);
}  


static uint16_t eepromReadValue(mcp4725_Instance_t *ins) {
    uint8_t b[3] = {0xFF, 0xFF, 0xFF};
    
    kdi2c_read(ins->kdi2c, ins->address, b, 3);
    
    if (b[1] == 0xFF && b[2] == 0xFF) {
        return 0xFFFF;
    }
    
    return ((((uint16_t) b[1]) << 8) | (((uint16_t) b[2]))) >> 4;
}  
    
/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

void mcp4725_init(mcp4725_Instance_t *instance, uint8_t address, void *kdi2c) {
    ASSERT(instance != NULL);
    ASSERT(kdi2c != NULL);
    
    if (instance->kdi2c != NULL) {
        return;
    }
    
    instance->kdi2c = kdi2c;
    kdi2c_init(kdi2c);
    kdi2c_powerUp(kdi2c);

    instance->address = address;
}


void mcp4725_finalize(mcp4725_Instance_t *instance) {
    ASSERT(instance != NULL);
    if (instance->kdi2c == NULL) {
        return;
    }
    
    kdi2c_powerDown(instance->kdi2c);
    kdi2c_finalize(instance->kdi2c);
    instance->kdi2c = NULL;
}   


int32_t mcp4725_powerUp(mcp4725_Instance_t *instance) {
    ASSERT(instance != NULL);
    
    instance->currentValue = eepromReadValue(instance);
    return fastWriteOnce(instance, instance->currentValue, MCP4725_POWER_DOWN_MODE_NORMAL);
}


int32_t mcp4725_powerDown(mcp4725_Instance_t *instance) {
    ASSERT(instance != NULL);
    
    instance->currentValue = eepromReadValue(instance);
    return fastWriteOnce(instance, instance->currentValue, MCP4725_POWER_DOWN_MODE_100K);
}


int32_t mcp4725_updateWithEeprom(mcp4725_Instance_t *instance, uint16_t dacValue) {
    ASSERT(instance != NULL);
    
    instance->currentValue = dacValue;
    return dacWithEepromWriteOnce(instance, dacValue, MCP4725_POWER_DOWN_MODE_NORMAL);
}


int32_t mcp4725_updateFast(mcp4725_Instance_t *instance, uint16_t dacValue) {
    ASSERT(instance != NULL);
    
    instance->currentValue = dacValue;
    return fastWriteOnce(instance, dacValue, MCP4725_POWER_DOWN_MODE_NORMAL);
}

/*@}*/
