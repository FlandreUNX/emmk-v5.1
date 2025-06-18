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

#include "./bh1721.h"

/**
 * @addtogroup ProgramProfile
 * @note none
 */
 
/*@{*/

#define CONFIG_DRIVER_USING_CONST        (1)

/*@}*/
 
/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

#define ADDRESS    0x23

typedef enum {
	BH_POWER_DOWN = 0,
	BH_POWER_ON = 0x01,
	BH_AUTO_MEASURE = 0x10,
	BH_H_MEASURE = 0x12,
	BH_L_MEASURE = 0x13,
} StateMode_t;

/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */
 
/*@{*/

#if CONFIG_DRIVER_USING_CONST == 0
static kdI2C_t *gDevBh1721Interface = NULL;
#else
extern const kdi2c_t * const gDevBh1721Interface;
#endif
static uint8_t mIsInterfaceInit = 0;

static qSTimer_t mCyclePollTmcd;
static uint8_t mIsCycleWaiting = 0;

/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */
 
/*@{*/

static int32_t _writeData(uint8_t value) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    if (kdi2c_write((void *) gDevBh1721Interface, ADDRESS, &value, 1) != 0) {
        return -1;
    }

    return 0;
}

static int32_t _readData(uint16_t *data) {
    uint8_t rd[2];

    if (!mIsInterfaceInit) {
        return -1;
    }
    
    if (kdi2c_read((void *) gDevBh1721Interface, ADDRESS, rd, 2) != 0) {
        return -1;
    }
    
    *data = ((uint16_t) rd[0]) << 8 | (((uint16_t) rd[1]) & 0x00FF);
    
    return 0;
}

static inline int32_t _disableMeasure(void) {
    return _writeData(BH_POWER_DOWN);
}

static int32_t _enableMeasure(StateMode_t mode) {
    if (mIsCycleWaiting) {
        if (qSTimer_Expired(&mCyclePollTmcd)) {
            mIsCycleWaiting = 0;
            return 0;
        }
        
        return 1;
    }
    
    if (_writeData(BH_POWER_ON) != 0) {
        goto l_exit;
    }
    
    if (_writeData(mode) != 0) {
        goto l_exit;
    }

    if (mode == BH_H_MEASURE) {
        qSTimer_Set(&mCyclePollTmcd, 210);
    } else if (mode == BH_L_MEASURE) {
        qSTimer_Set(&mCyclePollTmcd, 16);
    } else {
        qSTimer_Set(&mCyclePollTmcd, 210 + 16);
    }
    
    mIsCycleWaiting = 1;
    return 1;
    
l_exit:
    mIsCycleWaiting = 0;
    _disableMeasure();
    return -1;
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

void bh1271_initBus(void *devIf) {
    if (mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 1;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevBh1721Interface != NULL) {
        return -1;
    }
#endif
    
#if CONFIG_DRIVER_USING_CONST == 0
    gDevBh1721Interface = KDINSTANCE(I2C, path);
    ASSERT(gDevBh1721Interface != NULL);
#endif
    kdi2c_init((void *) gDevBh1721Interface);
    kdi2c_powerUp((void *) gDevBh1721Interface);
}

int32_t bh1721_initSoft(void) {
    mIsCycleWaiting = 0;
    
    if (_disableMeasure() < 0) {
        kdi2c_powerDown((void *) gDevBh1721Interface);
        kdi2c_finalize((void *) gDevBh1721Interface);
        return -1;
    }
    
    return 0;
}

void bh1721_finalizeBus(void) {
    if (!mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 0;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevBh1721Interface == NULL) {
        return;
    }
#endif
    
    kdi2c_powerDown((void *) gDevBh1721Interface);
    kdi2c_finalize((void *) gDevBh1721Interface);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevBh1721Interface = NULL;
#endif
}

void bh1721_finalizeSoft(void) {
    _disableMeasure();
}

int32_t bh1721_lux(uint16_t *lux) {
    uint16_t rv;
    int32_t rc = 0;
    
    rc = _enableMeasure(BH_AUTO_MEASURE);
    if (rc == 1) {
        return 1;
    } else if (rc < 0) {
        _disableMeasure();
        return -1;
    }
    
    if (_readData(&rv) != 0) {
        _disableMeasure();
        return -1;
    }
    
    _disableMeasure();
    
    if (lux != NULL) {
        uint32_t u32 = rv;
        u32 = (u32 * 10) / 12;
        u32 = u32 * 139 / 13;
        
        *lux = (float) u32 / 1.2f;
    }
    
    return 0;
}

/*@}*/
