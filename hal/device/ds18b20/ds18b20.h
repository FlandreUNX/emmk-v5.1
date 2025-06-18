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

#ifndef _DS18B20_H_
#define _DS18B20_H_

#include <stdint.h>

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

typedef struct DS18B20_UartHandler DS18B20_UartHandler_t;

typedef struct {
    void (*onInit)(void *handler);
    void (*onFinalize)(void *handler);
    void (*onBlock)(void *handler);
    void (*onWriteBit)(void *handler, uint8_t byte);
    void (*onReadBit)(void *handler, uint8_t *byte);
    uint8_t (*onHasRecv)(void *handler);
    void (*onFlush)(void *handler);
    void (*onSetBaudRate)(void *handler, uint32_t bd);
} DS18B20_CallBack_t;

struct DS18B20_UartHandler {
    const DS18B20_CallBack_t *callBack;
    float temp;
    qSTimer_t waiter;
    struct {
        uint8_t isStart: 1;
        uint8_t isReady: 1;
    } flag;
};

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



/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

extern int32_t ds18b20_init(DS18B20_UartHandler_t *h);
extern void ds18b20_finalize(DS18B20_UartHandler_t *h);

extern uint8_t ds18b20_isCommunicateFault(DS18B20_UartHandler_t *h);
extern uint8_t ds18b20_isReady(DS18B20_UartHandler_t *h);
extern uint8_t ds18b20_isStart(DS18B20_UartHandler_t *h);

extern int32_t ds18b20_start(DS18B20_UartHandler_t *h);
extern int32_t ds18b20_read(DS18B20_UartHandler_t *h);

extern float ds18b20_getCurrentTemp(DS18B20_UartHandler_t *h);

/*@}*/

#endif
