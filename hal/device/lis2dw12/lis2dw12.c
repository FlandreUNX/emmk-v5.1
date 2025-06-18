/*
 * Copyright (C) 2020 Flandreunx@outlook.com
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

#include "./lis2dw12.h"
#include "./lis2dw12_reg.h"

/**
 * @addtogroup ProgramProfile
 * @note none
 */
 
/*@{*/

#define CONFIG_DRIVER_USING_CONST        (1)

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define ADDRESS     (0x32 >> 1)

/*@}*/

/**
 * @addtogroup Local variables
 * @note none
 */
 
/*@{*/

#if CONFIG_DRIVER_USING_CONST == 0
static kdI2C_t *gDevLis2dw12Interface = NULL;
#else
extern const kdi2c_t * const gDevLis2dw12Interface;
#endif
static uint8_t mIsInterfaceInit = 0;

static int32_t _regRead(void *h, uint8_t reg, uint8_t *data, uint16_t length);
static int32_t _regWrite(void *h, uint8_t reg, const uint8_t *data, uint16_t length);
static const stmdev_ctx_t mDevCtx = {
    .write_reg = _regWrite, 
    .read_reg = _regRead,
};

/*@}*/

/**
 * @addtogroup Local func
 * @note none
 */
 
/*@{*/

static int32_t _regRead(void *h, uint8_t reg, uint8_t *data, uint16_t length) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regRead((void *) gDevLis2dw12Interface, ADDRESS, reg, 1, (uint8_t *) data, length);
}

static int32_t _regWrite(void *h, uint8_t reg, const uint8_t *data, uint16_t length) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regWrite((void *) gDevLis2dw12Interface, ADDRESS, reg, 1, (uint8_t *) data, length); 
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

void lis2dw12_initBus(void *devIf) {
    if (mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 1;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevLis2dw12Interface != NULL) {
        return -1;
    }
#endif
        
#if CONFIG_DRIVER_USING_CONST == 0
    gDevLis2dw12Interface = devIf;
    ASSERT(gDevLis2dw12Interface != NULL);
#endif
    kdi2c_init((void *) gDevLis2dw12Interface);
    kdi2c_powerUp((void *) gDevLis2dw12Interface);
}

int32_t lis2dw12_initSoft(void) {
    uint8_t whoamI;
    int32_t rc = lis2dw12_device_id_get((stmdev_ctx_t *) &mDevCtx, &whoamI);
    if (rc != 0 || whoamI != LIS2DW12_ID) {
        return -1;
    }
    
    return 0;
}

void lis2dw12_finalizeBus(void) {
    if (!mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 0;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevLis2dw12Interface == NULL) {
        return -1;
    }
#endif
    
    kdi2c_powerDown((void *) gDevLis2dw12Interface);
    kdi2c_finalize((void *) gDevLis2dw12Interface);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevLis2dw12Interface = NULL;
#endif
}

void lis2dw12_finalizeSoft(void) {

}

stmdev_ctx_t* lis2dw12_getCtx(void) {
    return (stmdev_ctx_t *) &mDevCtx;
}

/*@}*/
