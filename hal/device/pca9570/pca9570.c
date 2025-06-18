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

#include "./pca9570.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define ADDRESS     ((0b01001000) >> 1)

/*@}*/

/**
 * @addtogroup Local variables
 * @note none
 */
 
/*@{*/


/*@}*/

/**
 * @addtogroup Local func
 * @note none
 */
 
/*@{*/

static inline int32_t regRead(void *kd2ic, uint8_t *data) {
    if (kd2ic == NULL) {
        return -1;
    }
    return kdi2c_read((void *) kd2ic, ADDRESS, data, 1);
}

static inline int32_t regWrite(void *kd2ic, uint8_t data) {
    if (kd2ic == NULL) {
        return -1;
    }
    return kdi2c_write((void *) kd2ic, ADDRESS, &data, 1); 
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

void pca9570_initBus(Pca9570_Instance_t *ins, void *devIf) {
    ASSERT(ins != NULL);
    ASSERT(devIf != NULL);
    
    ins->driver = devIf;
    ASSERT(ins->driver != NULL);
    kdi2c_init((void *) ins->driver);
    kdi2c_powerUp((void *) ins->driver);
}

int32_t pca9570_initSoft(Pca9570_Instance_t *ins) {
    return 0;
}

void pca9570_finalizeSoft(Pca9570_Instance_t *ins) {

}

void pca9570_finalizeBus(Pca9570_Instance_t *ins) {
    ASSERT(ins != NULL);
    
    if (ins->driver == NULL) {
        return;
    }

    kdi2c_powerDown((void *) ins->driver);
    kdi2c_finalize((void *) ins->driver);
}

int32_t pca9570_writePin(Pca9570_Instance_t *ins, uint8_t pin, bool v) {
    ASSERT(ins != NULL);
    if (pin >= 4) {
        return -1;
    }
    
    uint8_t data = 0;
    if (regRead(ins->driver, &data) != 0) {
        return -1;
    }
    if (v) {
        data |= KLBIT(pin);
    } else {
        data &= ~KLBIT(pin);
    }
    if (regWrite(ins->driver, data) != 0) {
        return -1;
    }
    return 0;
}

int32_t pca9570_writePort(Pca9570_Instance_t *ins, uint8_t portData) {
    ASSERT(ins != NULL);
    if (regWrite(ins->driver, portData) != 0) {
        return -1;
    }
    return 0;
}

int32_t pca9570_readPin(Pca9570_Instance_t *ins, uint8_t pin) {
    ASSERT(ins != NULL);
    if (pin >= 4) {
        return -1;
    }
    
    uint8_t data = 0;
    if (regRead(ins->driver, &data) != 0) {
        return -1;
    }
    if (data & KLBIT(pin)) {
        return 1;
    } else {
        return 0;
    }
}

int32_t pca9570_readPort(Pca9570_Instance_t *ins) {

    ASSERT(ins != NULL);

    uint8_t data = 0;
    if (regRead(ins->driver, &data) != 0) {
        return -1;
    }
    return data;
}

/*@}*/
