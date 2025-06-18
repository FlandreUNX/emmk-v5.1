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

#include "./cs1237.h"

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "cs1237"

/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */

/*@{*/

static inline void delay(uint32_t d) {
    while (d--) {
        __NOP();
    }
}


static inline void setDataPinOut(const cs1237_Instance_t *ins) {
    kdgpio_powerUp(ins->pinData, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
}


static inline void setDataPinIn(const cs1237_Instance_t *ins) {
    kdgpio_powerUp(ins->pinData, KDGPIO_MODE_INPUT, KDGPIO_PULL_NONE);
}


static inline void setDataPinValue(const cs1237_Instance_t *ins, uint8_t i) {
    kdgpio_output(ins->pinData, i);
}


static inline uint32_t getDataPinValue(const cs1237_Instance_t *ins) {
    return kdgpio_input(ins->pinData);
}


static inline void setClkPinValue(const cs1237_Instance_t *ins, const uint8_t i) {
    kdgpio_output(ins->pinClk, i);
}


static inline void oneClk(const cs1237_Instance_t *ins) {
    setClkPinValue(ins, 1);
    delay(2);
    setClkPinValue(ins, 0);
    delay(2);
}


static int32_t readAdcData(const cs1237_Instance_t *ins, int32_t *data) {
    int32_t raw = 0;
    qSTimer_t wait;

    setClkPinValue(ins, 0);
    setDataPinIn(ins);

    qSTimer_Set(&wait, 200);
    while (getDataPinValue(ins) && !qSTimer_Expired(&wait)) {
    }
    if (qSTimer_Expired(&wait)) {
        return -1;
    }

    for (uint8_t i = 0; i < 24; i++) {
        setClkPinValue(ins, 1);
        delay(2);

        raw <<= 1;

        if (getDataPinValue(ins)) {
            raw++;
        }

        setClkPinValue(ins, 0);
        delay(2);
    }

    for (uint8_t i = 0; i < 3; i++) {
        oneClk(ins);
    }

    setDataPinOut(ins);
    setDataPinValue(ins, 1);

    if (raw & 0x800000) {
        raw -= 0x1000000;
    }

    *data = raw;

    return 0;
}


static int32_t writeReg(const cs1237_Instance_t *ins, const uint8_t writeData) {
    uint8_t _data = 0x80;
    qSTimer_t wait;

    setClkPinValue(ins, 0);
    setDataPinIn(ins);

    qSTimer_Set(&wait, 200);
    while (getDataPinValue(ins) && !qSTimer_Expired(&wait)) {
    }
    if (qSTimer_Expired(&wait)) {
        return -1;
    }

    for (uint8_t i = 0; i < 29; i++) {
        oneClk(ins);
    }

    setDataPinOut(ins);

    setClkPinValue(ins, 1);
    delay(3);
    setDataPinValue(ins, 1);
    setClkPinValue(ins, 0);
    delay(3);
    setClkPinValue(ins, 1);
    delay(3);
    setDataPinValue(ins, 1);
    setClkPinValue(ins, 0);
    delay(3);
    setClkPinValue(ins, 1);
    delay(3);
    setDataPinValue(ins, 0);
    setClkPinValue(ins, 0);
    delay(3);
    setClkPinValue(ins, 1);
    delay(3);
    setDataPinValue(ins, 0);
    setClkPinValue(ins, 0);
    delay(3);
    setClkPinValue(ins, 1);
    delay(3);
    setDataPinValue(ins, 1);
    setClkPinValue(ins, 0);
    delay(3);
    setClkPinValue(ins, 1);
    delay(3);
    setDataPinValue(ins, 0);
    setClkPinValue(ins, 0);
    delay(3);
    setClkPinValue(ins, 1);
    delay(3);
    setDataPinValue(ins, 1);
    setClkPinValue(ins, 0);
    delay(3);
    setClkPinValue(ins, 1);
    delay(3);
    setDataPinValue(ins, 0);
    setClkPinValue(ins, 0);
    delay(3);

    oneClk(ins);

    for (uint8_t i = 0; i < 8; i++) {
        setClkPinValue(ins, 1);
        delay(2);

        if ((writeData & _data) != 0) {
            setDataPinValue(ins, 1);
        } else {
            setDataPinValue(ins, 0);
        }

        setClkPinValue(ins, 0);
        delay(2);

        _data >>= 1;
    }

    oneClk(ins);

    return 0;
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

void cs1237_initBus(cs1237_Instance_t *ins, const void *pinData, const void *pinClk) {
    ASSERT(ins != NULL);
    ASSERT(pinData != NULL);
    ASSERT(pinClk != NULL);

    ins->pinData = (void *) pinData;
    ins->pinClk = (void *) pinClk;

    kdgpio_init(ins->pinData);
    kdgpio_powerUp(ins->pinData, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);

    kdgpio_init(ins->pinClk);
    kdgpio_powerUp(ins->pinClk, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
    kdgpio_output(ins->pinClk, 0);
}


void cs1237_finalizeBus(const cs1237_Instance_t *ins) {
    ASSERT(ins != NULL);
    ASSERT(ins->pinData != NULL);
    ASSERT(ins->pinClk != NULL);

    kdgpio_powerUp(ins->pinData, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
    kdgpio_output(ins->pinData, 1);
    kdgpio_powerDown(ins->pinData);
    kdgpio_finalize(ins->pinData);

    kdgpio_powerUp(ins->pinClk, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
    kdgpio_output(ins->pinClk, 1);
    qSTimer_t wait;
    qSTimer_Set(&wait, 1);
    while (!qSTimer_Expired(&wait)) {
    }
    kdgpio_output(ins->pinClk, 0);
    kdgpio_powerDown(ins->pinClk);
    kdgpio_finalize(ins->pinClk);
    qSTimer_Set(&wait, 1);
    while (!qSTimer_Expired(&wait)) {
    }
}


int32_t cs1237_initSoft(cs1237_Instance_t *ins) {
    ins->adcRaw = 0;
    ins->regStatus = 0;

    ins->regValue._ = 0;
    ins->regValue.channel = CS1237_CHANNEL_A;
    ins->regValue.pga = CS1237_PGA_1;
    ins->regValue.refOut = CS1237_REF_OUT_ON;
    ins->regValue.speed = CS1237_SPEED_10HZ;
    writeReg(ins, ins->regValue._);

    return 0;
}


void cs1237_finalizeSoft(cs1237_Instance_t *ins) {
}


int32_t cs1237_read(cs1237_Instance_t *ins) {
    return readAdcData(ins, &ins->adcRaw);
}


int32_t cs1237_setPga(const cs1237_Instance_t *ins, cs1237_Pga_t pga) {
    cs1237_RegValue_t newValue = {
        ._ = ins->regValue._,
    };
    newValue.pga = pga;

    return writeReg(ins, newValue._);
}


int32_t cs1237_setSpeed(const cs1237_Instance_t *ins, const cs1237_Speed_t speed) {
    cs1237_RegValue_t newValue = {
        ._ = ins->regValue._,
    };
    newValue.speed = speed;

    return writeReg(ins, newValue._);
}


inline uint8_t cs1237_getPga(const cs1237_Instance_t *ins) {
    switch (ins->regValue.pga) {
        case CS1237_PGA_1:
            return 1;
        case CS1237_PGA_2:
            return 2;
        case CS1237_PGA_64:
            return 64;
        case CS1237_PGA_128:
            return 128;
        default: return 0;
    }
}


inline int32_t cs1237_getRaw(const cs1237_Instance_t *ins) {
    return ins->adcRaw;
}


inline cs1237_RegValue_t cs1237_getRegValue(const cs1237_Instance_t *ins) {
    return ins->regValue;
}

/*@}*/

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef DBG_SECTION_NAME

/*@}*/
