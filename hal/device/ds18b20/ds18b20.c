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

#include "./ds18b20.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "DS18B20"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define TRANSMIT_TIMEOUT_MS    ((uint32_t) 10)

#define DS_PRECISION          0x7F  // 精度(12位)
#define SKIP_ROM              0xCC  // 跳过ROM
#define SEARCH_ROM            0xF0  // 搜索ROM
#define READ_ROM              0x33  // 读ROM
#define MATCH_ROM             0x55  // 匹配ROM
#define ALARM_ROM             0xEC  // 告警ROM
#define START_CONVERT    	  0x44  // 开始温度转换，在温度转换期间总线上输出0，转换结束后输出1
#define READ_SCRATCHPAD  	  0xBE  // 读暂存器的9个字节
#define WRITE_SCRATCHPAD 	  0x4E  // 写暂存器的温度告警TH和TL
#define COPY_SCRATCHPAD  	  0x48  // 将暂存器的温度告警复制到EEPROM，在复制期间总线上输出0，复制完后输出1
#define REACALL_EEPROM   	  0xB8  // 将EEPROM的温度告警复制到暂存器中，复制期间输出0，复制完成后输出1
#define READ_POWER       	  0xB4  // 读电源的供电方式：0为寄生电源供电；1为外部电源供电

/*@}*/

/**
 * @addtogroup Private one-wire funcs
 * @note none
 */

/*@{*/

static void owWriteBit(DS18B20_UartHandler_t *h, uint8_t bit) {
    uint8_t dummy = 0;
    qSTimer_t wait;
    
    h->callBack->onFlush(h);
    
    if (bit) {
        bit = 0xFF;
    } else {
        bit = 0x00;
    }
    h->callBack->onWriteBit(h, bit);
    
    qSTimer_Set(&wait, TRANSMIT_TIMEOUT_MS);
    while (!qSTimer_Expired(&wait) && !h->callBack->onHasRecv(h)) {
        h->callBack->onBlock(h);
    }
    
    h->callBack->onReadBit(h, &dummy);
}


static void owReadBit(DS18B20_UartHandler_t *h, uint8_t *bit) {
    uint8_t dummy = 0xFF;
    qSTimer_t wait;
    
    h->callBack->onFlush(h);
    h->callBack->onWriteBit(h, dummy);
    
    qSTimer_Set(&wait, TRANSMIT_TIMEOUT_MS);
    while (!qSTimer_Expired(&wait) && !h->callBack->onHasRecv(h)) {
        h->callBack->onBlock(h);
    }
    h->callBack->onReadBit(h, &dummy);
    
    if (dummy == 0xFF) {
        *bit = 0x01;
    } else {
        *bit = 0;
    }
}


static void owWriteByte(DS18B20_UartHandler_t *h, uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        owWriteBit(h, (byte >> i) & 0x01);
    }
}


static void owReadByte(DS18B20_UartHandler_t *h, uint8_t *byte) {
    uint8_t _recvByte = 0;
    
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t bit = 0;
        
        owReadBit(h, &bit);
        _recvByte |= bit << i;
    }
    
    *byte = _recvByte;
}


static int32_t owReset(DS18B20_UartHandler_t *h) {
    uint8_t dummy = 0xF0;
    qSTimer_t wait;
    
    h->callBack->onFlush(h);
    h->callBack->onSetBaudRate(h, 9600);
    h->callBack->onWriteBit(h, dummy);
    
    qSTimer_Set(&wait, TRANSMIT_TIMEOUT_MS);
    while (!qSTimer_Expired(&wait) && !h->callBack->onHasRecv(h)) {
        h->callBack->onBlock(h);
    }
    h->callBack->onReadBit(h, &dummy);
    
    h->callBack->onSetBaudRate(h, 115200);
    h->callBack->onFlush(h);
    
    if (dummy >= 0x90 && dummy <= 0xE0) {
        return 0;
    } else {
        return -1;
    }
}

/*@}*/

/**
 * @addtogroup Private ds18b20 func
 * @note none
 */

/*@{*/

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

int32_t ds18b20_init(DS18B20_UartHandler_t *h) {
    ASSERT(h != NULL);
    ASSERT(h->callBack != NULL);
    
    h->temp = 0;
    
    h->callBack->onInit(h);
    
    owReset(h);
    owWriteByte(h, SKIP_ROM);
    owWriteByte(h, WRITE_SCRATCHPAD);
    owWriteByte(h, DS_PRECISION);
    
    owReset(h);
    owWriteByte(h, SKIP_ROM);
    owWriteByte(h, COPY_SCRATCHPAD);
    
    h->flag.isStart = 0;
    h->flag.isReady = 1;
    
    return 0;
}


void ds18b20_finalize(DS18B20_UartHandler_t *h) {
    ASSERT(h != NULL);
    owReset(h);
    
    h->flag.isStart = 0;
    h->flag.isReady = 0;
    
    h->callBack->onFinalize(h);
}


uint8_t ds18b20_isCommunicateFault(DS18B20_UartHandler_t *h) {
    ASSERT(h != NULL);
    return h->flag.isReady ? 0 : 1;
}


uint8_t ds18b20_isReady(DS18B20_UartHandler_t *h) {
    ASSERT(h != NULL);
    return h->flag.isReady && qSTimer_Expired(&h->waiter) ? 1 : 0;
}


uint8_t ds18b20_isStart(DS18B20_UartHandler_t *h) {
    ASSERT(h != NULL);
    return h->flag.isStart;
}


int32_t ds18b20_start(DS18B20_UartHandler_t *h) {
    ASSERT(h != NULL);
    if (owReset(h) != 0) {
        h->flag.isReady = 0;
        return -1;
    }
    h->flag.isReady = 1;
    
    owWriteByte(h, 0xCC);
    owWriteByte(h, 0x44);
    
    h->flag.isStart = 1;
    
    qSTimer_Set(&h->waiter, 50);
    return 0;
}


int32_t ds18b20_read(DS18B20_UartHandler_t *h) {
    ASSERT(h != NULL);
    uint8_t tempL, tempH;
    int16_t tempData;
    
    h->flag.isStart = 0;
    
    if (owReset(h) != 0) {
        h->flag.isReady = 0;
        return -1;
    }
    h->flag.isReady = 1;
    
    owWriteByte(h, 0xCC);
    owWriteByte(h, 0xBE);
    owReadByte(h, &tempL);
    owReadByte(h, &tempH);
    tempData = tempH;
    tempData <<= 8;
    tempData |= (uint16_t) tempL;
    
    float temp = ((float) tempData * 0.0625f);
    if (temp >= 84.9f) {
        h->flag.isReady = 0;
        return -1;
    } else {
        h->temp = temp;
    }
    
    return 0;
}


float ds18b20_getCurrentTemp(DS18B20_UartHandler_t *h) {
    ASSERT(h != NULL);
    return h->temp;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
