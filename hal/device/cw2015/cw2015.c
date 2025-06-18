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

#include "./cw2015.h"

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

#define CW2015_ADDRESS     0x62

#define REG_VERSION     0x00
#define REG_VCELL       0x02
#define REG_SOC         0x04
#define REG_RRT_ALERT   0x06
#define REG_CONFIG      0x08
#define REG_MODE        0x0A
#define REG_BATINFO     0x10
 
#define SIZE_BATINFO    64 

#define MODE_SLEEP_MASK     (0xC0)
#define MODE_SLEEP          (0xC0)
#define MODE_NORMAL         (0x00)
#define MODE_QUICK_START    (0x30)
#define MODE_RESTART        (0x0F)

#define CONFIG_UPDATE_FLG   (0x02)
#define ATHD_MASK           (0xF8)

/*@}*/

/**
 * @addtogroup Local variables
 * @note none
 */
 
/*@{*/

#if CONFIG_DRIVER_USING_CONST == 0
static kdI2C_t *gDevCw2015Interface = NULL;
#else
extern const kdi2c_t * const gDevCw2015Interface;
#endif
static uint8_t mIsInterfaceInit = 0;

uint16_t mCw2015BatVolt = 0;

/*@}*/

/**
 * @addtogroup Local func
 * @note none
 */
 
/*@{*/

static inline int32_t reg_read(uint8_t reg, uint8_t *data) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regRead((void *) gDevCw2015Interface, CW2015_ADDRESS, reg, 1, data, 1);
}


static inline int32_t reg_write(uint8_t reg, uint8_t data) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regWrite((void *) gDevCw2015Interface, CW2015_ADDRESS, reg, 1, &data, 1);
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

void cw2015_initBus(void *devIf) {
    if (mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 1;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevCw2015Interface != NULL) {
        return -1;
    }
#endif
        
#if CONFIG_DRIVER_USING_CONST == 0
    gDevCw2015Interface = devIf;
    ASSERT(gDevCw2015Interface != NULL);
#endif
    kdi2c_init((void *) gDevCw2015Interface);
    kdi2c_powerUp((void *) gDevCw2015Interface);
}


int32_t cw2015_initSoft(void) {
    if (reg_write(REG_CONFIG, 0) != 0) {
        return -1;
    }
    
    if (reg_write(REG_MODE, MODE_QUICK_START) != 0) {
        return -1;
    }

    return cw2015_poll();
}


void cw2015_finalizeSoft(void) {
    if (reg_write(REG_MODE, MODE_SLEEP) != 0) {
        return;
    }
}


void cw2015_finalizeBus(void) {
    if (!mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 0;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevCw2015Interface == NULL) {
        return -1;
    }
#endif

    kdi2c_powerDown((void *) gDevCw2015Interface);
    kdi2c_finalize((void *) gDevCw2015Interface);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevCw2015Interface = NULL;
#endif
}


uint16_t cw2015_getMVCell(void) {
    uint8_t data;
    int16_t vl = 0;
    uint32_t mvcell = 0;
    
    if (reg_read(REG_VCELL, &data) != 0) {
        return 0;
    }
    
    vl = ((uint16_t) data) << 8;
    
    if (reg_read(REG_VCELL + 1, &data) != 0) {
        return 0;
    }
    
    vl += ((uint16_t) data);
    
    mvcell = vl * 312 / 1024;

    return mvcell;
}


int32_t cw2015_getSOC(void) {
    uint8_t data[2];
    
    int16_t vl = 0;
    int32_t soc;

    if (reg_read(REG_SOC, &data[0]) != 0) {
        return -1;
    }
    
    if (reg_read(REG_SOC + 1, &data[1]) != 0) {
        return -1;
    }
    
    int32_t sitemp = 0xFF00;
    
    vl = ((uint16_t) data[0]) << 8;
    vl += ((uint16_t) data[1]);
    
    if (sitemp >= 0xFF00) {
        sitemp = vl;
    } else {
        if ((abs(sitemp - vl) <= 0xFF) && (vl > 0xFF) && (vl < 0x6300)) {
            vl = sitemp;
        } else {
            if ((vl == 0) && (sitemp >= 0x0200)) {
                 vl = sitemp;
            } else {
                sitemp = vl;
                
                if ((vl <= 0xFF) && (vl > 0)) {
                    sitemp = 0;
                    vl = 0x0100;
                }
                
                if (vl >= 0x6300) {
                    sitemp = 0x6380;
                }
            }
        }
    }
    
    soc = vl >> 8;
    
    return soc;
}


int32_t cw2015_getRRT(void) {
	uint8_t data[2];
	uint16_t minute;

    if (reg_read(REG_RRT_ALERT, &data[0]) != 0) {
        return -1;
    }
    
    minute = ((uint16_t) data[0]) << 8;
    
    if (reg_read(REG_RRT_ALERT + 1, &data[1]) != 0) {
        return -1;
    }
    
    minute += ((uint16_t) data[0]);

	return minute;
}


int32_t cw2015_poll(void) {
    mCw2015BatVolt = cw2015_getMVCell();
    if (mCw2015BatVolt > 5000) {
        return -1;
    }
    
    return 0;
}


uint16_t cw2015_getVolt(void) {
    return mCw2015BatVolt;
}

/*@}*/
