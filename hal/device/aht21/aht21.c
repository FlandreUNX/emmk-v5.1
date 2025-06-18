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

#include "./aht21.h"

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

#define DEFAULT_ADDRESS                     0x38

/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */
 
/*@{*/

#if CONFIG_DRIVER_USING_CONST == 0
static kdI2C_t *gDevAht21Interface = NULL;
#else
extern const kdi2c_t * const gDevAht21Interface;
#endif
static uint8_t mIsInterfaceInit = 0;

static qSTimer_t mCyclePollTmcd;
static uint8_t mIsCycleWaiting = 0;

float mAht21CurrentT = 0;
float mAht21CurrentRH = 0;

/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */
 
/*@{*/

static int32_t startMeasure(void) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    
    if (mIsCycleWaiting) {
        if (qSTimer_Expired(&mCyclePollTmcd)) {
            mIsCycleWaiting = 0;
            return 0;
        }
        return 1;
    }
    
    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    int32_t rc = kdi2c_write((void *) gDevAht21Interface, DEFAULT_ADDRESS, cmd, 3);
    if (rc < 0) {
        return -1;
    }
    
    mIsCycleWaiting = 1;
    qSTimer_Set(&mCyclePollTmcd, 100);
    return 0;
}

static int32_t tryRead(void) {
    uint8_t result[6] = {0};
    int32_t rc = -1;
    uint32_t u32 = 0;
    
    if (!mIsInterfaceInit) {
        return -1;
    }
    
    rc = kdi2c_read((void *) gDevAht21Interface, DEFAULT_ADDRESS, result, 1);
    if (rc < 0) {
        return -1;
    }
    if (result[0] & 0x80) {
        return 1;
    }
    
    rc = kdi2c_read((void *) gDevAht21Interface, DEFAULT_ADDRESS, result, 6);
    if (rc < 0) {
        return -1;
    }
    
    u32 = (u32 | result[1]) << 8;
	u32 = (u32 | result[2]) << 8;
	u32 = (u32 | result[3]);
	u32 = u32 >> 4;
    mAht21CurrentRH = (((float) u32) * 100.0f * 10.0f / 1024.0f / 1024.0f) / 10.0f;
    
    u32 = 0;
	u32 = (u32 | result[3]) << 8;
	u32 = (u32 | result[4]) << 8;
	u32 = (u32 | result[5]);
	u32 = u32 & 0xfffff;
    mAht21CurrentT = (((float) u32) * 200.0f * 10.0f / 1024.0f / 1024.0f - 500.0f) / 10.0f;
    
    return 0;
}

static int32_t tryReadStatus(void) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    
    uint8_t st = 0;
    int32_t rc = kdi2c_read((void *) gDevAht21Interface, DEFAULT_ADDRESS, &st, 1);
    if (rc < 0) {
        return -1;
    }
    return st;
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

void aht21_initBus(void *devIf) {
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevBh1721Interface != NULL) {
        return -1;
    }
#endif
    
#if CONFIG_DRIVER_USING_CONST == 0
    gDevAht21Interface = devIf;
    ASSERT(gDevAht21Interface != NULL);
#endif
    kdi2c_init((void *) gDevAht21Interface);
    kdi2c_powerUp((void *) gDevAht21Interface);
    
    mIsInterfaceInit = 1;
}

int32_t aht21_initSoft(void) {
    if (tryReadStatus() < 0) {
        return -1;
    }
    
    return 0;
}

void aht21_finalizeBus(void) {
    if (!mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 0;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevAht21Interface == NULL) {
        return;
    }
#endif
    
    kdi2c_powerDown((void *) gDevAht21Interface);
    kdi2c_finalize((void *) gDevAht21Interface);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevAht21Interface = NULL;
#endif
}

void aht21_finalizeSoft(void) {

}

int32_t aht21_measure(void) {
    int32_t rc = -1;
    
    rc = startMeasure();
    if (rc == 1)  {
        return 1;
    } else if (rc < 0) {
        return rc;
    }
    
    rc = tryRead();
    if (rc != 0) {
        return rc;
    }
    
    return 0;
}

float aht21_getTempC(void) {
    return mAht21CurrentT;
}

float aht21_getHumi(void) {
    return mAht21CurrentRH;
}

/*@}*/
