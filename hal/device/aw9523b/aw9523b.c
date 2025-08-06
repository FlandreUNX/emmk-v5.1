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

#include "./aw9523b.h"
 
/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "AW9523B"
#undef TAG
#define TAG  DBG_SECTION_NAME

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

#define BASE_ADDRESS_B7   0x58  

#define REG_ADDR_P0_INPUT              0x00
#define REG_ADDR_P1_INPUT              0x01
#define REG_ADDR_P0_OUTPUT             0x02
#define REG_ADDR_P1_OUTPUT             0x03
#define REG_ADDR_P0_DIR                0x04
#define REG_ADDR_P1_DIR                0x05
#define REG_ADDR_ID                    0x10
#define REG_ADDR_GALBAL1               0x11
#define REG_ADDR_RESET                 0x7F

#define REG_GALBAL1_GPOMD              (1u << 4u)

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

static inline int32_t readReg(aw9523b_Instance_t *instnace, uint8_t regAddr, uint8_t *data, uint8_t len) {
    return kdi2c_regRead(((kdi2c_t *) instnace->i2cIf), 
        instnace->address, 
        regAddr, 1, 
        data, len);
}


static inline int32_t writeReg(aw9523b_Instance_t *instnace, uint8_t regAddr, uint8_t *data, uint8_t len) {
    return kdi2c_regWrite(((kdi2c_t *) instnace->i2cIf), 
        instnace->address, 
        regAddr, 1, 
        data, len);
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

void aw9523b_initBus(aw9523b_Instance_t *instnace, void *ifPath) {
    ASSERT(instnace != NULL);
    ASSERT(ifPath != NULL);

    instnace->i2cIf = ifPath;
    kdi2c_init(((kdi2c_t *) instnace->i2cIf));
    kdi2c_powerUp(((kdi2c_t *) instnace->i2cIf));
}


int32_t aw9523b_initSoft(aw9523b_Instance_t *instnace, aw9523b_B7AddressPin_t addressSet, uint16_t defDir, uint16_t defOut) {
    ASSERT(instnace != NULL);

    instnace->address = BASE_ADDRESS_B7 | addressSet;

    uint8_t data = 0;
    if (readReg(instnace, REG_ADDR_ID, &data, 1) != 0) {
        return -1;
    }
    if (data != 0x23) {
        return -1;
    }
    
    if (readReg(instnace, REG_ADDR_GALBAL1, &data, 1) != 0) {
        return -1;
    }
    data |= REG_GALBAL1_GPOMD;
    if (writeReg(instnace, REG_ADDR_GALBAL1, &data, 1) != 0) {
        return -1;
    }
    
    instnace->portDir.u16 = defDir;
    if (writeReg(instnace, REG_ADDR_P0_DIR, &instnace->portDir.u8[0], 1) != 0) {
        return -1;
    }
    if (writeReg(instnace, REG_ADDR_P1_DIR, &instnace->portDir.u8[1], 1) != 0) {
        return -1;
    }
    
    instnace->outputData.u16 = defOut;
    if (writeReg(instnace, REG_ADDR_P0_OUTPUT, &instnace->outputData.u8[0], 1) != 0) {
        return -1;
    }
    if (writeReg(instnace, REG_ADDR_P1_OUTPUT, &instnace->outputData.u8[1], 1) != 0) {
        return -1;
    }

    return 0;
}


void aw9523b_finalizeBus(aw9523b_Instance_t *instnace) {
    ASSERT(instnace != NULL);
    
    if (instnace->i2cIf == NULL) {
        return;
    }
    kdi2c_powerDown(((kdi2c_t *) instnace->i2cIf));
    kdi2c_finalize(((kdi2c_t *) instnace->i2cIf));
    instnace->i2cIf = NULL;
}


void aw9523b_finalizeSoft(aw9523b_Instance_t *instnace) {
}


int32_t aw9523b_softReset(aw9523b_Instance_t *instnace) {
    ASSERT(instnace != NULL);
    
    uint8_t data = 0;
    
    if (writeReg(instnace, REG_ADDR_RESET, &data, 1) != 0) {
        return -1;
    }
    
    return 0;
}



int32_t aw9523b_setPinDir(aw9523b_Instance_t *instnace, aw9523b_Pin_t pin, aw9523b_Dir_t dir) {
    ASSERT(instnace->i2cIf != NULL);

    if (dir) {
        instnace->portDir.u16 |= pin;
    } else {
        instnace->portDir.u16 &= ~pin;
    }
    
    if (writeReg(instnace, REG_ADDR_P0_DIR, &instnace->portDir.u8[0], 1) != 0) {
        return -1;
    }
    if (writeReg(instnace, REG_ADDR_P1_DIR, &instnace->portDir.u8[1], 1) != 0) {
        return -1;
    }
    
    return 0;
}


int32_t aw9523b_setPinPortDir(aw9523b_Instance_t *instnace, uint16_t pinPort) {
    ASSERT(instnace->i2cIf != NULL);

    uint16_t diff = instnace->portDir.u16;
    
    instnace->portDir.u16 = pinPort;
        
    if (diff != instnace->portDir.u16) {
        if (writeReg(instnace, REG_ADDR_P0_DIR, &instnace->portDir.u8[0], 1) != 0) {
            return -1;
        }
        if (writeReg(instnace, REG_ADDR_P1_DIR, &instnace->portDir.u8[1], 1) != 0) {
            return -1;
        }
    }
    
    return 0;
}


int32_t aw9523b_updateInput(aw9523b_Instance_t *instnace) {
    ASSERT(instnace->i2cIf != NULL);
    
    uint8_t data[2] = {0};
    
    if (readReg(instnace, REG_ADDR_P0_INPUT, &data[0], 1) != 0) {
        return -1;
    }
    if (readReg(instnace, REG_ADDR_P1_INPUT, &data[1], 1) != 0) {
        return -1;
    }
    
    memcpy(&instnace->inputData.u16, data, 2);
    
    return 0;
}


int32_t aw9523b_updateOutput(aw9523b_Instance_t *instnace) {
    ASSERT(instnace->i2cIf != NULL);

    if (writeReg(instnace, REG_ADDR_P0_OUTPUT, &instnace->outputData.u8[0], 1) != 0) {
        return -1;
    }
    if (writeReg(instnace, REG_ADDR_P1_OUTPUT, &instnace->outputData.u8[1], 1) != 0) {
        return -1;
    }

    return 0;
}


uint16_t aw9523b_readPin(aw9523b_Instance_t *instnace, aw9523b_Pin_t pin) {
    ASSERT(instnace->i2cIf != NULL);
    
    return instnace->inputData.u16 & pin;
}


uint16_t aw9523b_readPinPort(aw9523b_Instance_t *instnace) {
    ASSERT(instnace->i2cIf != NULL);
    
    return instnace->inputData.u16;
}


int32_t aw9523b_setPin(aw9523b_Instance_t *instnace, aw9523b_Pin_t pin, uint8_t vl) {
    ASSERT(instnace->i2cIf != NULL);

    if (vl) {
        instnace->outputData.u16 |= pin;
    } else {
        instnace->outputData.u16 &= ~pin;
    }

    return 0;
}


int32_t aw9523b_setPinPort(aw9523b_Instance_t *instnace, uint16_t pinPort) {
    ASSERT(instnace->i2cIf != NULL);

    instnace->outputData.u16 = pinPort;

    return 0;
}

/*@}*/

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#undef TAG

/*@}*/
