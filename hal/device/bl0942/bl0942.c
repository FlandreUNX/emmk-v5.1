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
#include "./bl0942.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "BL0942"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define OPERATE_ERR_CHECK_RETURN(x) if (x < 0) return -1
#define OPERATE_ERR_TRY(x) if (x < 0) {LOG_W("OperateRetry"); goto l_try;}

#define OPERATE_CREATE_TRY(x) uint32_t try = x
#define OPERATE_TRY_CHECK() \
    l_try:                        \
    if (try != 0) {         \
        try--;              \
    } else {                \
        LOG_E("OperateFailed");                    \
        return -1; \
    }

/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */

/*@{*/

static const float VREF = 1.218f;

/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */

/*@{*/

/*@}*/

/**
 * @addtogroup HalFunc
 * @note none
 */

/*@{*/

static void reset(BL0942_Instance_t *instance) {
    instance->onReset();
}


static int32_t regWrite(BL0942_Instance_t *instance, uint8_t regAddress, uint8_t data[3]) {
    uint8_t ds[6] = {
            0xA8,
            regAddress,
            data[0], data[1], data[2],
            0x00
    };

    uint8_t ck = 0;;
    for (uint8_t i = 0; i < 5; i++) {
        ck += ds[i];
    }
    ck = ~(ck);
    ds[5] = ck;

    instance->onDataTx(ds, 6);
    return 0;
}


static int32_t regRead(BL0942_Instance_t *instance, uint8_t regAddress, uint8_t data[3]) {
    uint8_t ds[26] = {
            0x58,
            regAddress,
            0xFF, 0xFF, 0xFF,
            0xFF,
    };
    instance->onDataTx(ds, 2);
    instance->onDataRx(&ds[2], 4);

    uint8_t ck = 0;;
    for (uint8_t i = 0; i < 5; i++) {
        ck += ds[i];
    }
    ck = ~(ck);

    if (ck != ds[5]) {
        LOG_W("CHECKSUM FAILED, %02X", (uint8_t) (ck & 0x000000FF));
        reset(instance);
        return -1;
    }

    memcpy(data, &ds[9], 3);
    return 0;
}


static int32_t regReadAll(BL0942_Instance_t *instance, uint8_t data[23]) {
    static const uint8_t ds[2] = {0x58, 0xAA};

    instance->onDataTx((void *) ds, 2);
    instance->onDataRx(data, 23);

    if (data[0] != 0x55) {
        LOG_W("HEAD NOT FOUND, %02X", (uint8_t) (data[0]));
        reset(instance);
        return -1;
    }

    uint8_t ck = ds[0];
    for (uint8_t i = 0; i < 22; i++) {
        ck += data[i];
    }
    ck = ~(ck);

    if (ck != data[22]) {
        LOG_W("CHECKSUM FAILED, %02X", (uint8_t) (ck & 0x000000FF));
        reset(instance);
        return -1;
    }

    return 0;
}

/*@}*/

/**
 * @addtogroup PrivateFunc
 * @note none
 */

/*@{*/

static inline int32_t convertS24toS32(int32_t s24) {
    if ((s24 & 0x800000) == 0x800000) {
        s24 |= 0xFF000000;
    }
    return s24;
}


static inline int32_t convertU8toI32(const uint8_t u8[3]) {
    int32_t regDataU32 = (((int32_t) u8[2]) << 16)
                         | (((int32_t) u8[1]) << 8)
                         | (((int32_t) u8[0]) << 0);
    return regDataU32;
}


static inline uint32_t convertU8toU32(const uint8_t u8[3]) {
    uint32_t regDataU32 = (((int32_t) u8[2]) << 16)
                          | (((int32_t) u8[1]) << 8)
                          | (((int32_t) u8[0]) << 0);
    return regDataU32;
}


static inline void convertU32toU8(uint32_t u32, uint8_t u8[3]) {
    u8[2] = (u32 >> 16) & 0xFF;
    u8[1] = (u32 >> 8) & 0xFF;
    u8[0] = (u32 >> 0) & 0xFF;
}


static float convertVolt_V_RES(int32_t raw, uint32_t r2, uint32_t r1) {
    float v = (((float) raw) * VREF * ((float) r2 + (float) r1)) / (73989.0f * (float) r1 * 1000);
    return v;
}


static float convertAmp_A_RES(int32_t raw, float rl) {
    float a = (((float) raw) * VREF) / (305978.0f * rl * 1000);
    return a;
}


static float convertWatt_W_RES(int32_t raw, uint32_t r2, uint32_t r1, float rl) {
    float w = (((float) raw) * VREF * VREF * ((float) r2 + (float) r1)) / (3537.0f * rl * (float) r1 * 1000 * 1000);
    return w;
}


static double convertEng_kwh_RES(uint32_t cf, uint32_t r2, uint32_t r1, float rl) {
    double eng = ((1638.4 * 256.0f * VREF * VREF * ((float) r2 + (float) r1)) / (3600000.0f * 3537.0f * rl * (float) r1 * 1000 * 1000));
    return eng * (float) cf;
}


/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

int32_t bl0942_initSoft(BL0942_Instance_t *instance) {
    return bl0942_read(instance, NULL);
}


void bl0942_finalizeSoft(BL0942_Instance_t *instance) {
}

/*@}*/

/**
 * @addtogroup Base
 * @note none
 */

/*@{*/

int32_t bl0942_read(BL0942_Instance_t *instance, BL0942_Data_t *data) {
    ASSERT(instance);

    int32_t rc = 0;
    OPERATE_CREATE_TRY(3);
    OPERATE_TRY_CHECK();

    uint8_t dataAll[23] = {0};
    OPERATE_ERR_TRY(regReadAll(instance, dataAll));

    if (data != NULL) {
        int32_t regI = convertS24toS32(convertU8toI32(&dataAll[1]));
        int32_t regV = convertS24toS32(convertU8toI32(&dataAll[4]));
        int32_t regW = convertS24toS32(convertU8toI32(&dataAll[10]));
        uint32_t regCf = convertU8toU32(&dataAll[13]);
        uint8_t regSt = dataAll[19];

        data->v = convertVolt_V_RES(regV, instance->channelArg.r2, instance->channelArg.r1);
        data->a = convertAmp_A_RES(regI, instance->channelArg.rl);
        data->w = convertWatt_W_RES(regW, instance->channelArg.r2, instance->channelArg.r1, instance->channelArg.rl);
        data->e = (float) convertEng_kwh_RES(regCf, instance->channelArg.r2, instance->channelArg.r1, instance->channelArg.rl);
        data->st = regSt;
    }

    return rc;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
