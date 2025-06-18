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
#include "./bl0910.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "BL0910"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define REG_COMMAND_WRITE_TAG       0xCA // 0x81
#define REG_COMMAND_READ_TAG        0x35 // 0x82

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

static const float VREF = 1.097f;

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

static int32_t regWrite(BL0910_Instance_t *instance, uint8_t regAddress, uint8_t data[3]) {
    uint8_t ds[6] = {
            instance->ifMode == BL0910_IF_SERIAL ? 0xCA : 0x81,
            regAddress,
            instance->ifMode == BL0910_IF_SERIAL ? data[2] : data[0],
            data[1],
            instance->ifMode == BL0910_IF_SERIAL ? data[0] : data[2],
    };
    uint32_t ck = (ds[1] + ds[2] + ds[3] + ds[4]
                   + (instance->ifMode == BL0910_IF_SPI ? ds[0] : 0)
                  ) & 0xFF;
    ds[5] = (~ck) & 0xFF;

    if (instance->onSpiSelect != NULL) {
        instance->onSpiSelect(instance->ifMode, 0);
    }
    if (instance->onSpiSelect != NULL) {
        instance->onSpiSelect(instance->ifMode, 1);
    }
    instance->onDataTx(instance->ifMode, ds, 6);
    if (instance->onSpiSelect != NULL) {
        instance->onSpiSelect(instance->ifMode, 0);
    }
//    LOG_I("RegWrite-Tx, [%02X, %02X] %02X, %02X, %02X, %02X",
//          ds[0], ds[1], ds[2], ds[3], ds[4], ds[5]);
    return 0;
}


static int32_t regRead(BL0910_Instance_t *instance, uint8_t regAddress, uint8_t data[3]) {
    uint8_t ds[8] = {
            instance->ifMode == BL0910_IF_SERIAL ? 0x35 : 0x82,
            regAddress,
            0xFF, 0xFF, 0xFF,
            0xFF,
    };

    if (instance->onSpiSelect != NULL) {
        instance->onSpiSelect(instance->ifMode, 0);
    }
    if (instance->onSpiSelect != NULL) {
        instance->onSpiSelect(instance->ifMode, 1);
    }
    instance->onDataTx(instance->ifMode, ds, 2);
    instance->onDataRx(instance->ifMode, ds + 2, 4);
    if (instance->onSpiSelect != NULL) {
        instance->onSpiSelect(instance->ifMode, 0);
    }
//    LOG_I("RegRead, [%02X, %02X] %02X, %02X, %02X, %02X",
//          ds[0], ds[1], ds[2], ds[3], ds[4], ds[5]);

    uint32_t ck = (ds[1] + ds[2] + ds[3] + ds[4]
                   + (instance->ifMode == BL0910_IF_SPI ? ds[0] : 0)
                  ) & 0xFF;
    ck = (~ck) & 0xFF;
    if (ck != ds[5]) {
        LOG_W("CHECKSUM FAILED, %02X", (uint8_t) (ck & 0x000000FF));
        if (instance->onIfReset != NULL) {
            instance->onIfReset();
        }
        return -1;
    }

    memcpy(data, &ds[2], 3);
    if (instance->ifMode == BL0910_IF_SERIAL) {
        ck = data[0];
        data[0] = data[2];
        data[2] = ck & 0x000000FF;
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
    int32_t regDataU32 = (((int32_t) u8[0]) << 16)
                         | (((int32_t) u8[1]) << 8)
                         | (((int32_t) u8[2]) << 0);
    return regDataU32;
}


static inline uint32_t convertU8toU32(const uint8_t u8[3]) {
    uint32_t regDataU32 = (((int32_t) u8[0]) << 16)
                          | (((int32_t) u8[1]) << 8)
                          | (((int32_t) u8[2]) << 0);
    return regDataU32;
}


static inline void convertU32toU8(uint32_t u32, uint8_t u8[3]) {
    u8[0] = (u32 >> 16) & 0xFF;
    u8[1] = (u32 >> 8) & 0xFF;
    u8[2] = (u32 >> 0) & 0xFF;
}


static inline float convertVoltRaw2VoltV_R(uint32_t raw, uint32_t rf_k, uint32_t rv_k) {
    float v = (((float) raw) * VREF * ((float) rf_k + (float) rv_k))
              / (13162.0f * ((float) rv_k) * 1000);
    return v;
}


static inline float convertAmpRaw2AmpA_R(uint32_t raw, uint32_t rl_mr) {
    float a = (((float) raw) * VREF) / (206000.0f * ((float) rl_mr));
    return a;
}


static inline float convertWattRaw2WattW_R(int32_t raw, uint32_t rl_mr, uint32_t rf_k, uint32_t rv_k) {
    float w = (((float) raw) * (VREF * VREF) * ((float) rf_k + (float) rv_k))
              / (646.6f * ((float) rl_mr) * ((float) rv_k) * 1000.0f);
    return w;
}


static inline float convertCf2Eng_R(uint32_t cf, uint32_t cfdiv, uint32_t rl_mr, uint32_t rf_k, uint32_t rv_k) {
    float eng = (419430.0f * 0.032768f * 16 * (VREF * VREF) * ((float) rf_k + (float) rv_k))
                / (3600000.0f * 650.0f * ((float) rl_mr) * ((float) rv_k) * 1000.0f * ((float) cfdiv));
    return eng * (float) cf;
}


// vrl_r 电压负载电阻(R)
// vrv_k 电压互感器分压电阻(K)
// gain_v 电压通道增益
static inline float convertVoltRaw2VoltV_CT(uint32_t raw, float vrl_r, float vrv_k, float gain_v) {
    float v = (((float) raw) * VREF * (vrv_k)) / (13162.0f * gain_v * vrl_r * 1000);
    return v;
}


// irl_r 电流互感器负载电阻(R)
// ct_rt 电流互感器变比
// gain_i 电流通道增益
static inline float convertAmpRaw2AmpA_CT(uint32_t raw, float irl_r, float ct_rt, float gain_i) {
    float a = (((float) raw) * (VREF)) / (12875.0f * gain_i * (irl_r) * 1000.0f / ct_rt);
    return a;
}


// irl_r 电流互感器负载电阻(R)
// ct_rt 电流互感器变比
// vrl_r 电压负载电阻(R)
// vrv_k 电压互感器分压电阻(K)
// gain_i 电流通道增益
// gain_v 电压通道增益
static inline float convertWattRaw2WattW_CT(int32_t raw,
                                            float irl_r, float ct_rt,
                                            float vrl_r, float vrv_k,
                                            float gain_i, float gain_v) {
    float w = (((float) raw) * (VREF * VREF) * vrv_k)
              / (40.4125f * (irl_r * (1000 / ct_rt)) * gain_i * vrl_r * gain_v * 1000.0f);
    return w;
}


// irl_r 电流互感器负载电阻(R)
// ct_rt 电流互感器变比
// vrl_r 电压负载电阻(R)
// vrv_k 电压互感器分压电阻(K)
static inline float convertCf2Eng_CT(uint32_t cf, uint32_t cfdiv,
                                     float irl_r, float ct_rt,
                                     float vrl_r, float vrv_k,
                                     float gain_i, float gain_v) {
    float kp = (float) (40.4125 * (irl_r * (1000.0f / ct_rt)) * gain_i * vrl_r * gain_v * 1000.0f)
               / ((VREF * VREF) * vrv_k);
    float eng = (4194304.0f * 0.032768f * 16.0f)
                / (3600000.0f * (float) cfdiv * kp);
    return eng * (float) cf;
}


static int32_t regUnlock(BL0910_Instance_t *instance) {
    uint8_t regDataU8[3] = {0};
    regDataU8[0] = 0;
    regDataU8[1] = 0x55;
    regDataU8[2] = 0x55;
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x9E, regDataU8));
    return 0;
}


static int32_t
setChannelGain(BL0910_Instance_t *instance, BL0910_ChannelGainOffset_t offset, BL0910_ChannelGain_t gain) {
    uint8_t regDataU8[3] = {0};
    if (offset >= 100) {
        offset -= 100;
        OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
        OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x61, regDataU8));
        uint32_t regDataU32 = convertU8toU32(regDataU8);
        regDataU32 &= ~(BL0910_CHANNEL_GAIN_MARK << offset);
        regDataU32 |= (gain << offset);
        convertU32toU8(regDataU32, regDataU8);
        OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
        OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x61, regDataU8));
    } else {
        OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
        OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x60, regDataU8));
        uint32_t regDataU32 = convertU8toU32(regDataU8);
        regDataU32 &= ~(BL0910_CHANNEL_GAIN_MARK << offset);
        regDataU32 |= (gain << offset);
        convertU32toU8(regDataU32, regDataU8);
        OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
        OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x60, regDataU8));
    }
    return 0;
}


static int32_t readChannelRMS(BL0910_Instance_t *instance, BL0910_RMSChannel_t channel) {
    uint8_t regAddress = 0x0C + channel;
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, regAddress, regDataU8));
    int32_t regDataU32 = (int32_t) convertU8toU32(regDataU8);
    return regDataU32;
}


static int32_t readChannelFastRMS(BL0910_Instance_t *instance, BL0910_RMSChannel_t channel) {
    uint8_t regAddress = 0x17 + channel;
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, regAddress, regDataU8));
    int32_t regDataU32 = (int32_t) convertU8toU32(regDataU8);
    return regDataU32;
}


static int32_t setFastRmsThreshold(BL0910_Instance_t *instance, uint32_t threshold) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x8B, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    regDataU32 &= ~0x1FFFFF;
    regDataU32 |= threshold;
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x8B, regDataU8));
    return 0;
}


static int32_t readChannelWatt(BL0910_Instance_t *instance, BL0910_WattChannel_t channel) {
    uint8_t regAddress = 0x22 + channel;
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, regAddress, regDataU8));
    int32_t regDataI32 = convertU8toI32(regDataU8);
    regDataI32 = convertS24toS32(regDataI32);
    return regDataI32;
}


static int32_t readChannelCfCnt(BL0910_Instance_t *instance, BL0910_CfCntChannel_t channel) {
    uint8_t regAddress = 0x2F + channel;
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, regAddress, regDataU8));
    int32_t regDataU32 = (int32_t) convertU8toU32(regDataU8);
    return regDataU32;
}


static int32_t readLinePeriod(BL0910_Instance_t *instance) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x4E, regDataU8));
    int32_t regDataU32 = (int32_t) convertU8toU32(regDataU8);
    return regDataU32;
}


static int32_t readCFDIV(BL0910_Instance_t *instance) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0xCE, regDataU8));
    int32_t regDataU32 = (int32_t) convertU8toU32(regDataU8);
    return regDataU32;
}


static int32_t setWattCreep(BL0910_Instance_t *instance, uint16_t wa, uint16_t var) {
    uint8_t regDataU8[3] = {0};
    uint32_t regDataU32 = wa | (var << 12);
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x88, regDataU8));
    return 0;
}


static int32_t setRmsCreep(BL0910_Instance_t *instance, uint16_t rms) {
    uint8_t regDataU8[3] = {0};
    uint32_t regDataU32 = rms;
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x8A, regDataU8));
    return 0;
}


static int32_t setVPKVL(BL0910_Instance_t *instance, uint16_t pklvl) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x8C, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    regDataU32 &= ~0xFFF;
    regDataU32 |= pklvl & 0xFFF;
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x8C, regDataU8));
    return 0;
}


static int32_t setIPKVL(BL0910_Instance_t *instance, uint16_t pklvl) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x8C, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    regDataU32 &= ~(0xFFF << 12);
    regDataU32 |= pklvl & (0xFFF << 12);
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x8C, regDataU8));
    return 0;
}


// BL0910_Status1Offset_t
static uint32_t getStatus1(BL0910_Instance_t *instance) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x96, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    return regDataU32;
}


// BL0910_Status3Offset_t
static uint32_t getStatus3(BL0910_Instance_t *instance) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x56, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    return regDataU32;
}


static int32_t setMode(BL0910_Instance_t *instance, uint8_t is3u6i5u5i) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x98, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    if (is3u6i5u5i) {
        regDataU32 |= 1 << 19;
    } else {
        regDataU32 &= ~(1 << 19);
    }
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x98, regDataU8));
    return 0;
}


static int32_t setControlPriority(BL0910_Instance_t *instance, BL0910_ControlPriority_t pri, uint8_t enable) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x90, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    if (enable) {
        regDataU32 |= pri;
    } else {
        regDataU32 &= ~pri;
    }
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x90, regDataU8));
    return 0;
}


static int32_t getControlPriority(BL0910_Instance_t *instance, BL0910_ControlPriority_t pri) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x90, regDataU8));
    uint32_t regDataU32 = (int32_t) convertU8toU32(regDataU8);
    return regDataU32 & pri;
}


static int32_t setControl(BL0910_Instance_t *instance, BL0910_Control_t con, uint8_t enable) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x90, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    if (enable) {
        regDataU32 |= con;
    } else {
        regDataU32 &= ~con;
    }
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x90, regDataU8));
    return 0;
}


static uint32_t readControl(BL0910_Instance_t *instance, BL0910_Control_t con) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x90, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    return regDataU32 & con;
}


static int32_t cleanEng(BL0910_Instance_t *instance) {
    uint8_t regDataU8[3] = {0};
    regDataU8[0] = 0;
    regDataU8[1] = 0x1F;
    regDataU8[2] = 0xFF;
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x9D, regDataU8));
    return 0;
}


static int32_t softReset(BL0910_Instance_t *instance) {
    uint32_t regDataU32 = 0x5A5A5A;
    uint8_t regDataU8[3] = {0};
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x9F, regDataU8));
    return 0;
}


static int32_t readTps1(BL0910_Instance_t *instance) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x5E, regDataU8));
    int32_t regDataU32 = (int32_t) convertU8toU32(regDataU8);
    return regDataU32;
}


static int32_t setModeAddSel(BL0910_Instance_t *instance, uint8_t mode) {
    uint8_t regDataU8[3] = {0};
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regRead(instance, 0x98, regDataU8));
    uint32_t regDataU32 = convertU8toU32(regDataU8);
    if (mode) {
        regDataU32 |= KLBIT(8);
    } else {
        regDataU32 &= ~KLBIT(8);
    }
    convertU32toU8(regDataU32, regDataU8);
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(regWrite(instance, 0x98, regDataU8));
    return 0;
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

int32_t bl0910_initSoft(BL0910_Instance_t *instance) {
    assert(instance != NULL);

    int32_t rc = 0;
    OPERATE_CREATE_TRY(3);
    OPERATE_TRY_CHECK()

    OPERATE_ERR_TRY(setControlPriority(instance, BL0910_CONTROL_PRIORITY_ALL, 1))
    OPERATE_ERR_TRY(setControl(instance, BL0910_CONTROL_ALL, 0))

    for (uint8_t i = 0; i < instance->channelArg.voltGainCount; i++) {
        OPERATE_ERR_TRY(setChannelGain(instance,
                                       instance->channelArg.voltGain[i],
                                       instance->channelArg.gain_v))
    }
    for (uint8_t i = 0; i < instance->channelArg.AmpGainCount; i++) {
        OPERATE_ERR_TRY(setChannelGain(instance,
                                       instance->channelArg.ampGain[i],
                                       instance->channelArg.gain_i))
    }

    return rc;
}


void bl0910_finalizeSoft(BL0910_Instance_t *instance) {
    assert(instance != NULL);
}

/*@}*/

/**
 * @addtogroup Base
 * @note none
 */

/*@{*/

int32_t bl0910_getControlPriorty(BL0910_Instance_t *instance, BL0910_ControlPriority_t mark) {
    assert(instance != NULL);

    int32_t val = getControlPriority(instance, mark);
    OPERATE_ERR_CHECK_RETURN(val);

    return val;
}


int32_t bl0910_softReset(BL0910_Instance_t *instance) {
    assert(instance != NULL);
    OPERATE_CREATE_TRY(3);
    OPERATE_TRY_CHECK()
    OPERATE_ERR_TRY(softReset(instance))
    return 0;
}


int32_t bl0910_setModeAc3(BL0910_Instance_t *instance, uint8_t e) {
    assert(instance != NULL);
    OPERATE_CREATE_TRY(3);
    OPERATE_TRY_CHECK()
    OPERATE_ERR_TRY(setMode(instance, e ? 1 : 0))
//    OPERATE_ERR_CHECK_RETURN(setModeAddSel(instance, 1));
    return 0;
}


inline int32_t bl0910_setChannelControl(BL0910_Instance_t *instance, uint32_t channel, uint8_t enable) {
    assert(instance != NULL);
    OPERATE_CREATE_TRY(3);
    OPERATE_TRY_CHECK()
    OPERATE_ERR_TRY(setControl(instance, channel, enable))
    return 0;
}


inline int32_t bl0910_readChannelControl(BL0910_Instance_t *instance, uint32_t channel) {
    assert(instance != NULL);

    return readControl(instance, channel);
}


float bl0910_measureTempInternal(BL0910_Instance_t *instance) {
    assert(instance != NULL);

    int32_t val = readTps1(instance);
    OPERATE_ERR_CHECK_RETURN(val);

    float t = ((float) val - 64) * 12.5f / 59.0f - 40.0f;
    return t;
}


float bl0910_readChannelVoltCt(BL0910_Instance_t *instance, BL0910_RMSChannel_t channel) {
    assert(instance != NULL);
    float v;
    int32_t val;

    val = readChannelRMS(instance, channel);
    OPERATE_ERR_CHECK_RETURN(val);

    v = convertVoltRaw2VoltV_CT(val,
                                instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    return v;
}


float bl0910_readChannelAmpCt(BL0910_Instance_t *instance, BL0910_RMSChannel_t channel) {
    assert(instance != NULL);
    float v;
    int32_t val;

    val = readChannelRMS(instance, channel);
    OPERATE_ERR_CHECK_RETURN(val);

    v = convertAmpRaw2AmpA_CT(val,
                              instance->channelArg.irl_r, instance->channelArg.ct_rt,
                              BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );

    return v;
}


float bl0910_readChannelWattCt(BL0910_Instance_t *instance, BL0910_WattChannel_t channel) {
    assert(instance != NULL);
    float v;
    int32_t val;

    val = readChannelWatt(instance, channel);

    v = convertWattRaw2WattW_CT(val,
                                instance->channelArg.irl_r, instance->channelArg.ct_rt, instance->channelArg.vrl_r,
                                instance->channelArg.vrv_k,
                                BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    return v;
}


float bl0910_readChannelEngCt(BL0910_Instance_t *instance, BL0910_CfCntChannel_t channel) {
    assert(instance != NULL);
    float v;
    int32_t val;

    val = readCFDIV(instance);
    OPERATE_ERR_CHECK_RETURN(val);
    uint32_t cfdiv = val;

    val = readChannelCfCnt(instance, channel);
    OPERATE_ERR_CHECK_RETURN(val);

    v = convertCf2Eng_CT(val, cfdiv,
                         instance->channelArg.irl_r, instance->channelArg.ct_rt, instance->channelArg.vrl_r,
                         instance->channelArg.vrv_k,
                         BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                         BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    return v;
}


inline int32_t bl0910_cleanEng(BL0910_Instance_t *instance) {
    assert(instance != NULL);
    int32_t val;
    val = cleanEng(instance);
    OPERATE_ERR_CHECK_RETURN(val);
    return val;
}

/*@}*/

/**
 * @addtogroup 1U10I
 * @note none
 */

/*@{*/

int32_t bl0910_1u10iRead(BL0910_Instance_t *instance, BL0910_1U10IData_t *d) {
    assert(instance != NULL);
    assert(d != NULL);

    uint32_t u32;
    int32_t val;

    // Volt
    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_11);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_u = val;

    d->vl_u = convertVoltRaw2VoltV_CT(d->raw_u,
                                      instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                      BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    // Amp
    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_2);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[0] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_3);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[1] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_4);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[2] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_1);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[3] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_5);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[4] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_6);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[5] = val;

    d->vl_ix[0] = convertAmpRaw2AmpA_CT(d->raw_ix[0],
                                        instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                        BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[1] = convertAmpRaw2AmpA_CT(d->raw_ix[1],
                                        instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                        BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[2] = convertAmpRaw2AmpA_CT(d->raw_ix[2],
                                        instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                        BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[3] = convertAmpRaw2AmpA_CT(d->raw_ix[3],
                                        instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                        BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[4] = convertAmpRaw2AmpA_CT(d->raw_ix[4],
                                        instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                        BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[5] = convertAmpRaw2AmpA_CT(d->raw_ix[5],
                                        instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                        BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );

    // Watt
    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_2);
    d->raw_watt[0] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_3);
    d->raw_watt[1] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_4);
    d->raw_watt[2] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_1);
    d->raw_watt[3] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_5);
    d->raw_watt[4] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_6);
    d->raw_watt[5] = val;

    d->vl_watt[0] = convertWattRaw2WattW_CT(d->raw_watt[0],
                                            instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                            instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[1] = convertWattRaw2WattW_CT(d->raw_watt[1],
                                            instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                            instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[2] = convertWattRaw2WattW_CT(d->raw_watt[2],
                                            instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                            instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[3] = convertWattRaw2WattW_CT(d->raw_watt[3],
                                            instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                            instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[4] = convertWattRaw2WattW_CT(d->raw_watt[4],
                                            instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                            instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[5] = convertWattRaw2WattW_CT(d->raw_watt[5],
                                            instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                            instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                            BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    // Eng
    val = readCFDIV(instance);
    OPERATE_ERR_CHECK_RETURN(val);
    uint32_t cfdiv = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_2);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[0] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_3);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[1] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_4);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[2] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_1);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[3] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_5);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[4] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_6);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[5] = val;

    d->vl_eng[0] = convertCf2Eng_CT(d->raw_cf[0], cfdiv,
                                    instance->channelArg.irl_r, instance->channelArg.ct_rt, instance->channelArg.vrl_r,
                                    instance->channelArg.vrv_k,
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[1] = convertCf2Eng_CT(d->raw_cf[1], cfdiv,
                                    instance->channelArg.irl_r, instance->channelArg.ct_rt, instance->channelArg.vrl_r,
                                    instance->channelArg.vrv_k,
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[2] = convertCf2Eng_CT(d->raw_cf[2], cfdiv,
                                    instance->channelArg.irl_r, instance->channelArg.ct_rt, instance->channelArg.vrl_r,
                                    instance->channelArg.vrv_k,
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[3] = convertCf2Eng_CT(d->raw_cf[3], cfdiv,
                                    instance->channelArg.irl_r, instance->channelArg.ct_rt, instance->channelArg.vrl_r,
                                    instance->channelArg.vrv_k,
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[4] = convertCf2Eng_CT(d->raw_cf[4], cfdiv,
                                    instance->channelArg.irl_r, instance->channelArg.ct_rt, instance->channelArg.vrl_r,
                                    instance->channelArg.vrv_k,
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[5] = convertCf2Eng_CT(d->raw_cf[5], cfdiv,
                                    instance->channelArg.irl_r, instance->channelArg.ct_rt, instance->channelArg.vrl_r,
                                    instance->channelArg.vrv_k,
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                    BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    val = (int32_t) getStatus1(instance);
    u32 = val;
    d->ipk.i1 = u32 & BL0910_ST1_OFFSET_PK2 ? 1 : 0;
    d->ipk.i2 = u32 & BL0910_ST1_OFFSET_PK3 ? 1 : 0;
    d->ipk.i3 = u32 & BL0910_ST1_OFFSET_PK4 ? 1 : 0;
    d->ipk.i4 = u32 & BL0910_ST1_OFFSET_PK1 ? 1 : 0;
    d->ipk.i5 = u32 & BL0910_ST1_OFFSET_PK5 ? 1 : 0;
    d->ipk.i6 = u32 & BL0910_ST1_OFFSET_PK6 ? 1 : 0;
    d->vpk.any = u32 & BL0910_ST1_OFFSET_PKV;

    val = (int32_t) readControl(instance, BL0910_CONTROL_3U6I_I1 | BL0910_CONTROL_3U6I_I2);
    u32 = val;
    d->control.i1 = u32 & BL0910_CONTROL_2 ? 1 : 0;
    d->control.i2 = u32 & BL0910_CONTROL_3 ? 1 : 0;
    d->control.i3 = u32 & BL0910_CONTROL_4 ? 1 : 0;
    d->control.i4 = u32 & BL0910_CONTROL_7 ? 1 : 0;
    d->control.i5 = u32 & BL0910_CONTROL_8 ? 1 : 0;
    d->control.i6 = u32 & BL0910_CONTROL_9 ? 1 : 0;

    val = (int32_t) getStatus3(instance);
    u32 = val;
    d->leak.i1 = u32 & BL0910_ST3_OFFSET_2 ? 1 : 0;
    d->leak.i2 = u32 & BL0910_ST3_OFFSET_3 ? 1 : 0;
    d->leak.i3 = u32 & BL0910_ST3_OFFSET_4 ? 1 : 0;
    d->leak.i4 = u32 & BL0910_ST3_OFFSET_1 ? 1 : 0;
    d->leak.i5 = u32 & BL0910_ST3_OFFSET_5 ? 1 : 0;
    d->leak.i6 = u32 & BL0910_ST3_OFFSET_6 ? 1 : 0;

    return 0;
}


int32_t bl0910_1u10iControl(BL0910_Instance_t *instance, BL0910_Control_t index, uint8_t enable) {
    assert(instance != NULL);

    OPERATE_ERR_CHECK_RETURN(setControl(instance, index, enable ? 1 : 0));
    return 0;
}

/*@}*/

/**
 * @addtogroup 3U6I
 * @note none
 */

/*@{*/

int32_t bl0910_3u6iConfig(BL0910_Instance_t *instance) {
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setMode(instance, 1));

    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setControlPriority(instance, BL0910_CONTROL_PRIORITY_ALL, 1));

    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setControl(instance, BL0910_CONTROL_ALL, 0));

    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN2_OFFSET_10, instance->channelArg.gain_v));
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN2_OFFSET_9, instance->channelArg.gain_v));
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN2_OFFSET_8, instance->channelArg.gain_v));

    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN1_OFFSET_2, instance->channelArg.gain_i));
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN1_OFFSET_3, instance->channelArg.gain_i));
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN1_OFFSET_4, instance->channelArg.gain_i));

    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN1_OFFSET_1, instance->channelArg.gain_i));
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN1_OFFSET_5, instance->channelArg.gain_i));
    OPERATE_ERR_CHECK_RETURN(regUnlock(instance));
    OPERATE_ERR_CHECK_RETURN(setChannelGain(instance, BL0910_CHANNEL_GAIN2_OFFSET_6, instance->channelArg.gain_i));
    return 0;
}


int32_t bl0910_3u6iRead(BL0910_Instance_t *instance, BL0910_3U6IData_t *d) {
    assert(instance != NULL);
    assert(d != NULL);

    uint32_t u32;
    int32_t val;

    // Volt
    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_10);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ua = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_9);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ub = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_8);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_uc = val;

    d->vl_ua = convertVoltRaw2VoltV_CT(d->raw_ua,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_ub = convertVoltRaw2VoltV_CT(d->raw_ub,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_uc = convertVoltRaw2VoltV_CT(d->raw_uc,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    // Amp
    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_2);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[0][0] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_3);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[0][1] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_4);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[0][2] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_1);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[1][0] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_5);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[1][1] = val;

    val = readChannelRMS(instance, BL0910_RMS_CHANNEL_6);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_ix[1][2] = val;

    d->vl_ix[0][0] = convertAmpRaw2AmpA_CT(d->raw_ix[0][0],
                                           instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                           BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[0][1] = convertAmpRaw2AmpA_CT(d->raw_ix[0][1],
                                           instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                           BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[0][2] = convertAmpRaw2AmpA_CT(d->raw_ix[0][2],
                                           instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                           BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]);
    d->vl_ix[1][0] = convertAmpRaw2AmpA_CT(d->raw_ix[1][0],
                                           instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                           BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[1][1] = convertAmpRaw2AmpA_CT(d->raw_ix[1][1],
                                           instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                           BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );
    d->vl_ix[1][2] = convertAmpRaw2AmpA_CT(d->raw_ix[1][2],
                                           instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                           BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i]
    );

    // Watt
    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_2);
    d->raw_watt[0][0] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_3);
    d->raw_watt[0][1] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_4);
    d->raw_watt[0][2] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_1);
    d->raw_watt[1][0] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_5);
    d->raw_watt[1][1] = val;

    val = readChannelWatt(instance, BL0910_WATT_CHANNEL_6);
    d->raw_watt[1][2] = val;

    d->vl_watt[0][0] = convertWattRaw2WattW_CT(d->raw_watt[0][0],
                                               instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                               instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[0][1] = convertWattRaw2WattW_CT(d->raw_watt[0][1],
                                               instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                               instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[0][2] = convertWattRaw2WattW_CT(d->raw_watt[0][2],
                                               instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                               instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[1][0] = convertWattRaw2WattW_CT(d->raw_watt[1][0],
                                               instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                               instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[1][1] = convertWattRaw2WattW_CT(d->raw_watt[1][1],
                                               instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                               instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_watt[1][2] = convertWattRaw2WattW_CT(d->raw_watt[1][2],
                                               instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                               instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                               BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    // Eng
    val = readCFDIV(instance);
    OPERATE_ERR_CHECK_RETURN(val);
    uint32_t cfdiv = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_2);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[0][0] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_3);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[0][1] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_4);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[0][2] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_1);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[1][0] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_5);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[1][1] = val;

    val = readChannelCfCnt(instance, BL0910_CFCNT_CHANNEL_6);
    OPERATE_ERR_CHECK_RETURN(val);
    d->raw_cf[1][2] = val;

    d->vl_eng[0][0] = convertCf2Eng_CT(d->raw_cf[0][0], cfdiv,
                                       instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[0][1] = convertCf2Eng_CT(d->raw_cf[0][1], cfdiv,
                                       instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[0][2] = convertCf2Eng_CT(d->raw_cf[0][2], cfdiv,
                                       instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[1][0] = convertCf2Eng_CT(d->raw_cf[1][0], cfdiv,
                                       instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[1][1] = convertCf2Eng_CT(d->raw_cf[1][1], cfdiv,
                                       instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );
    d->vl_eng[1][2] = convertCf2Eng_CT(d->raw_cf[1][2], cfdiv,
                                       instance->channelArg.irl_r, instance->channelArg.ct_rt,
                                       instance->channelArg.vrl_r, instance->channelArg.vrv_k,
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_i],
                                       BL0910_CHANNEL_GAIN_X[instance->channelArg.gain_v]
    );

    val = (int32_t) getStatus1(instance);
    u32 = val;
    d->ipk.i1a = u32 & BL0910_ST1_OFFSET_PK2 ? 1 : 0;
    d->ipk.i1b = u32 & BL0910_ST1_OFFSET_PK3 ? 1 : 0;
    d->ipk.i1c = u32 & BL0910_ST1_OFFSET_PK4 ? 1 : 0;
    d->ipk.i2a = u32 & BL0910_ST1_OFFSET_PK1 ? 1 : 0;
    d->ipk.i2b = u32 & BL0910_ST1_OFFSET_PK5 ? 1 : 0;
    d->ipk.i2c = u32 & BL0910_ST1_OFFSET_PK6 ? 1 : 0;
    d->ipk.i1_any = (d->ipk.i1a || d->ipk.i1b || d->ipk.i1c) ? 1 : 0;
    d->ipk.i2_any = (d->ipk.i2a || d->ipk.i2b || d->ipk.i2c) ? 1 : 0;
    d->vpk.a = u32 & BL0910_ST1_OFFSET_PK10 ? 1 : 0;
    d->vpk.b = u32 & BL0910_ST1_OFFSET_PK9 ? 1 : 0;
    d->vpk.c = u32 & BL0910_ST1_OFFSET_PK8 ? 1 : 0;
    d->vpk.any = (d->vpk.a || d->vpk.b || d->vpk.c) ? 1 : 0;

    val = (int32_t) readControl(instance, BL0910_CONTROL_3U6I_I1 | BL0910_CONTROL_3U6I_I2);
    u32 = val;
    d->control.i1a = u32 & BL0910_CONTROL_2 ? 1 : 0;
    d->control.i1b = u32 & BL0910_CONTROL_3 ? 1 : 0;
    d->control.i1c = u32 & BL0910_CONTROL_4 ? 1 : 0;
    d->control.i2a = u32 & BL0910_CONTROL_7 ? 1 : 0;
    d->control.i2b = u32 & BL0910_CONTROL_8 ? 1 : 0;
    d->control.i2c = u32 & BL0910_CONTROL_9 ? 1 : 0;
    d->control.i1_all = ((u32 & BL0910_CONTROL_3U6I_I1) == BL0910_CONTROL_3U6I_I1) ? 1 : 0;
    d->control.i2_all = ((u32 & BL0910_CONTROL_3U6I_I2) == BL0910_CONTROL_3U6I_I2) ? 1 : 0;

    val = (int32_t) getStatus3(instance);
    u32 = val;
    d->leak.i1a = u32 & BL0910_ST3_OFFSET_2 ? 1 : 0;
    d->leak.i1b = u32 & BL0910_ST3_OFFSET_3 ? 1 : 0;
    d->leak.i1c = u32 & BL0910_ST3_OFFSET_4 ? 1 : 0;
    d->leak.i2a = u32 & BL0910_ST3_OFFSET_1 ? 1 : 0;
    d->leak.i2b = u32 & BL0910_ST3_OFFSET_5 ? 1 : 0;
    d->leak.i2c = u32 & BL0910_ST3_OFFSET_6 ? 1 : 0;
    d->leak.i1_any = (d->leak.i1a || d->leak.i1b || d->leak.i1c) ? 1 : 0;
    d->leak.i2_any = (d->leak.i2a || d->leak.i2b || d->leak.i2c) ? 1 : 0;

    return 0;
}


int32_t bl0910_3u6iControl(BL0910_Instance_t *instance, BL0910_Control_t index, uint8_t enable) {
    assert(instance != NULL);
    assert(index <= 1);

    OPERATE_ERR_CHECK_RETURN(setControl(instance, index, enable ? 1 : 0));
    return 0;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
