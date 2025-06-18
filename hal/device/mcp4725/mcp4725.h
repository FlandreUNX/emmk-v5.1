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
 
#ifndef _DEV_DAC_mcp4725H_
#define _DEV_DAC_mcp4725H_

#include <stdint.h>
 
/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

typedef struct {
    void *kdi2c;
    
    uint8_t address;

    uint16_t currentValue;
} mcp4725_Instance_t;

typedef enum {
    MCP4725_POWER_DOWN_MODE_NORMAL = 0,
    MCP4725_POWER_DOWN_MODE_1K = 1,
    MCP4725_POWER_DOWN_MODE_100K = 2,
    MCP4725_POWER_DOWN_MODE_500K = 3,
} mcp4725_PowerDownMode_t;

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
 * @addtogroup Private funcs
 * @note none
 */
 
/*@{*/

#define mcp4725_voltToValue(refMv, mv) ((float) mv / (((float) refMv) / 4096.0f))

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

extern void mcp4725_init(mcp4725_Instance_t *instance, uint8_t address, void *kdi2c);
extern void mcp4725_finalize(mcp4725_Instance_t *instance);

extern int32_t mcp4725_powerUp(mcp4725_Instance_t *instance);
extern int32_t mcp4725_powerDown(mcp4725_Instance_t *instance);

extern int32_t mcp4725_updateWithEeprom(mcp4725_Instance_t *instance, uint16_t dacValue);
extern int32_t mcp4725_updateFast(mcp4725_Instance_t *instance, uint16_t dacValue);

/*@}*/

#endif
