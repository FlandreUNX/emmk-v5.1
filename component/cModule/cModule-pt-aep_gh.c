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
 
#include "global.h"

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "cModule-pt-aep-gh"

#define CMASSERT(err) ASSERT(err)

/*@}*/

#if CONFIG_CMODULE_PROTOCOL_IS_AEP_GH == 1
 
/**
 * @addtogroup ComponentSupport
 * @note none
 */
 
/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)
extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */

/*@{*/

#define DIRECT_WRITE(l)      rilat_directWritePhy(instance, (uint8_t *) &buffer[0], l); // LOG_D("%s", buffer);
#define HEX_WRITE(l)         memset(&buffer[64], 0x00, 64); l = klStr_hex2str((uint8_t *) buffer, l, &buffer[64]); rilat_directWritePhy(instance, (uint8_t *) &buffer[64], l); // LOG_I("HEX-> %s", buffer);

#define PT_HEAD     0x68
#define PT_TVER     0x19
#define PT_VVER     0x15
#define PT_TAIL     0x16

#define PT_HEAD_IX       0
#define PT_HEAD_LX       1

#define PT_TVER_IX       1
#define PT_TVER_LX       1

#define PT_VVER_IX       2
#define PT_VVER_LX       1

#define PT_LEN_IX        3
#define PT_LEN_LX        2

#define PT_MID_IX        5
#define PT_MID_LX        1

#define PT_CTL_IX        6 
#define PT_CTL_LX        1

#define PT_DID_IX        7 
#define PT_DID_LX        8

#define PT_CMD_IX        15
#define PT_CMD_LX        1

#define PT_DAT_IX        16

#define PT_CRC_IX(pl)    (16 + pl)
#define PT_CRC_LX        2

#define PT_TAIL_IX(pl)   (16 + pl + 2)
#define PT_TAIL_LX       1

#define PT_FRAME_BASE_SIZE   19

typedef enum {
    CTL_UPLOAD = 0,
    CTL_DOWNLOAD = KLBIT(7),
    
    CTL_NONE_CONTINUE = 0,
    CTL_HAS_CONTINUE = KLBIT(6),
    
    CTL_FUNC_DATA_REPORT = 0x01,
    CTL_FUNC_DATA_DISTRIBUTE = 0x02,
    CTL_FUNC_DATA_READ = 0x04,
    CTL_FUNC_DATA_WRITE = 0x05,
    CTL_FUNC_DATA_WRITE_READ = 0x08,
} CtlCode_t;

typedef enum {
    RC_SUCCESS = 0,
    RC_FRAME_ERR = 1,
    RC_VERSION_ERR = 2,
    RC_UNVALID_CMD = 3,
    RC_CRC_ERR = 4,
    RC_DATETIME_ERR = 5,
    RC_ARG_ERR = 6,
    RC_DEVICE_ID_ERR = 0x10,
    RC_POLE_ID_ERR = 0x11,
    RC_UNREGISTED = 0x20,
} ResultCode_t;

typedef struct {
    cModule_Instance_t *ins;
    
    void *protocol;
    uint8_t command;
    uint8_t sensorCount;
    bool isContinue: 1;
    
    bool isWriten: 1;
    uint16_t messageId;
    klist_t list;
} PtBufferCollection_t;

/*@}*/

/**
 * @addtogroup Vars
 * @note none
 */

/*@{*/

static uint8_t mCurrentMid = 0;

static klist_t mPtBufferList;

/*@}*/

/**
 * @addtogroup Private
 * @note none
 */

/*@{*/

static uint8_t getU32DecLen(uint32_t dec) {
    uint8_t _num = 0;
    
    do {
        _num++;
        dec /= 10;
    } while (dec);
    return _num;
}

static void dec2bcd_nl(uint32_t dec, uint8_t decNumLen, uint8_t *out) {
    for (int8_t i = decNumLen; i > 0; i--) {
        uint8_t n = dec % 100;
        out[i - 1] = bin2bcd(n);
        dec = dec / 100;
    }
}

static uint32_t getFrameSize(uint32_t dataLen) {
    return dataLen + PT_FRAME_BASE_SIZE;
}


static uint16_t crc16(unsigned char *puchMsg, unsigned int usDataLen) { 
    uint16_t wCRCin = 0x0000; 
    uint16_t wCPoly = 0x1021; 
    uint8_t wChar = 0; 
 
    while (usDataLen--) { 
        wChar = *(puchMsg++); 
        wCRCin ^= (wChar << 8); 
        for (int i = 0; i < 8; i++) { 
            if (wCRCin & 0x8000) 
                wCRCin = (wCRCin << 1) ^ wCPoly; 
            else 
                wCRCin = wCRCin << 1; 
        } 
    } 
    return (wCRCin); 
}


static uint16_t frameGen_base(uint8_t *buf, uint16_t payloadSize, uint8_t ctl) {
    buf[PT_HEAD_IX] = PT_HEAD;
    buf[PT_TVER_IX] = PT_TVER;
    buf[PT_VVER_IX] = PT_VVER;
    uint16_t frameSize = getFrameSize(payloadSize);
    buf[PT_LEN_IX + 0] = (frameSize >> 8) & 0x00FF;
    buf[PT_LEN_IX + 1] = (frameSize >> 0) & 0x00FF;
    buf[PT_MID_IX] = mCurrentMid;
    buf[PT_CTL_IX] = ctl;
    memcpy(&buf[PT_DID_IX], CREQUEST(PT_GET_DEVICE_ID8, {}).ptr, 8);
    uint16_t crc = crc16(&buf[PT_MID_IX], payloadSize + 11);
    buf[PT_CRC_IX(payloadSize) + 0] = (crc >> 8) & 0x00FF;
    buf[PT_CRC_IX(payloadSize) + 1] = (crc >> 0) & 0x00FF;
    buf[PT_TAIL_IX(payloadSize)] = PT_TAIL;
    return frameSize;
}


static uint8_t frameGen_register(cModule_Instance_t *ins, uint8_t *buf, cModule_PtAepGh_DevType_t devType, const char *devCode) {
    ASSERT(buf != NULL);
    
    buf[PT_CMD_IX] = 0xA1;
    uint8_t *p = &buf[PT_DAT_IX];
    
    memcpy(p, ins->modemInfo.imei, 15);
    p += 15;
    
    memcpy(p, ins->modemInfo.iccid, 20);
    p += 20;
    
    // TODO
    memset(p, ' ', 10);
    memcpy(p, "MB26-S", 6);
    p += 10;
    
    // 0=CMCC, 2=CTCC, 3=CNET
    *p = 3;
    p += 1;
    
    // 0=独立式, 1=总线控制, 2=分线控制
    *p = devType;
    p += 1;
    
    memset(p, ' ', 3);
    uint8_t sl = strlen(devCode);
    if (sl == 0) {
        memcpy(p, "GTY", 3);
    } else if (sl <= 3) {
        memcpy(p, devCode, sl);
    }
    p += 3;
    
    // 0=NB, 3=3G, 4=4G, 5=5G
    *p = 0;
    p += 1;
    
    // 主电信息
    *p = 0;
    p += 1;
    *p = 255;
    p += 1;
    
    uint8_t c = getU32DecLen(ins->modemInfo.sei.cellId);
    dec2bcd_nl(ins->modemInfo.sei.cellId, c, &p[6 - c]);
    p += 6;
    
    *p = (ins->modemInfo.sei.rsrp >> 8) & 0x00FF;
    *(p + 1) = (ins->modemInfo.sei.rsrp >> 0) & 0x00FF;
    *(p + 2) = (ins->modemInfo.sei.sinr >> 8) & 0x00FF;
    *(p + 3) = (ins->modemInfo.sei.sinr >> 0) & 0x00FF;
    *(p + 4) = ins->modemInfo.rssi;
    *(p + 5) = -113 + (ins->modemInfo.rssi * 2);
    p += 6;
    
    return (uint32_t) p - (uint32_t) &buf[PT_DAT_IX];
}


static void hexWrite(Rilat_Instance_t *instance, uint8_t *buffer, uint16_t len) {
    uint32_t cur = 0;
    uint8_t *hexBuffer = calloc(1, 129);
    ASSERT(hexBuffer != NULL);
    while (cur < len) {
        uint8_t writeLen = len - cur;
        if (writeLen >= 64) {
            writeLen = 64;
        }
        uint32_t hexLen = klStr_hex2str((uint8_t *) buffer + cur, writeLen, (char *) hexBuffer);
        
        rilat_directWrite(instance, (uint8_t *) hexBuffer, hexLen, 0);
        
        cur += writeLen;
    }
    free(hexBuffer);
}


static void directWrite(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
        
        uint8_t *buffer = calloc(1, 384);
        ASSERT(buffer != NULL);
        
        buffer[PT_CMD_IX] = ud->command;
        uint16_t idx = PT_CMD_IX + 1;
        
        cModule_PtAepGh_DataReport_t *d = ud->protocol;
        klDateTime_SampleTm_t tm;
        klDateTime_bktime(ud->ins->aux.timeStamp, &tm);
        
        buffer[idx++] = bin2bcd(tm.year - 2000);
        buffer[idx++] = bin2bcd(tm.mon);
        buffer[idx++] = bin2bcd(tm.day);
        buffer[idx++] = bin2bcd(tm.hour);
        buffer[idx++] = bin2bcd(tm.min);
        buffer[idx++] = bin2bcd(tm.sec);
        
        if (ud->command == 0xA2) {
            buffer[idx++] = (ud->ins->modemInfo.sei.rsrp >> 8) & 0x00FF;
            buffer[idx++] = (ud->ins->modemInfo.sei.rsrp >> 0) & 0x00FF;
            buffer[idx++] = (ud->ins->modemInfo.sei.sinr >> 0) & 0x00FF;
            buffer[idx++] = (ud->ins->modemInfo.sei.sinr >> 0) & 0x00FF;
            buffer[idx++] = ud->ins->modemInfo.rssi;
            buffer[idx++] = -113 + (ud->ins->modemInfo.rssi * 2);
            buffer[idx++] = d[0].controlStatus;  // 控制器状态
            buffer[idx++] = 0;  // 主电信息1
            buffer[idx++] = 255;  // 主电信息2
            buffer[idx++] = CREQUEST(PT_GET_SENSOR_TOTAL_COUNT, {}).u32;
            buffer[idx++] = ud->sensorCount;
            if (ud->sensorCount != 0) {
                for (uint8_t i = 0; i < ud->sensorCount; i++) {
                    buffer[idx++] = d[i].index;
                    buffer[idx++] = d[i].a2.status;
                    buffer[idx++] = d[i].a2.gas;
                    buffer[idx++] = (((uint16_t) (d[i].a2.value * 10)) >> 8) & 0x00FF;
                    buffer[idx++] = (((uint16_t) (d[i].a2.value * 10)) >> 0) & 0x00FF;
                }
            }
        } else if (ud->command == 0xA3) {
            buffer[idx++] = d[0].a3_a4.power;
            buffer[idx++] = d[0].controlStatus;  // 控制器状态
            buffer[idx++] = CREQUEST(PT_GET_SENSOR_TOTAL_COUNT, {}).u32;
            buffer[idx++] = ud->sensorCount;
            if (ud->sensorCount != 0) {
                for (uint8_t i = 0; i < ud->sensorCount; i++) {
                    buffer[idx++] = d[i].index;
                    buffer[idx++] = d[i].a3_a4.status;
                }
            }
        } else if (ud->command == 0xA4) {
            buffer[idx++] = 0xFF;   // 是否触发联动阀控
            buffer[idx++] = CREQUEST(PT_GET_SENSOR_TOTAL_COUNT, {}).u32;
            buffer[idx] = ud->sensorCount;
            if (ud->sensorCount != 0) {
                idx++;
                for (uint8_t i = 0; i < ud->sensorCount; i++) {
                    buffer[idx++] = d[i].index;
                    buffer[idx++] = d[i].a3_a4.alarm;
                    buffer[idx++] = d[i].a3_a4.gas;
                    buffer[idx++] = (((uint16_t) (d[i].a3_a4.value * 10)) >> 8) & 0x00FF;
                    buffer[idx++] = (((uint16_t) (d[i].a3_a4.value * 10)) >> 0) & 0x00FF;
                    buffer[idx++] = (((uint16_t) d[i].a3_a4.temp_x100) >> 8) & 0x00FF;
                    buffer[idx++] = (((uint16_t) d[i].a3_a4.temp_x100) >> 0) & 0x00FF;
                    buffer[idx] = d[i].a3_a4.humi;
                    if (i != ud->sensorCount - 1) {
                        idx++;
                    }
                }
            }
        }
        idx = frameGen_base(buffer, idx - PT_CMD_IX, CTL_UPLOAD | CTL_FUNC_DATA_REPORT | (ud->isContinue ? CTL_HAS_CONTINUE : CTL_NONE_CONTINUE));
        hexWrite(instance, buffer, idx);

        ud->isWriten = true;
        free(buffer);
        break;
    } 
}


static uint32_t directGetPayloadLength(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
        
        uint16_t len = 0;
        
        if (ud->command == 0xA2) {
            len = 17 + 5 * ud->sensorCount;
        } else if (ud->command == 0xA3) {
            len = 10 + 2 * ud->sensorCount;
        } else if (ud->command == 0xA4) {
            len = 9 + 8 * ud->sensorCount;
        }
        len = getFrameSize(len);
        return len;
    } 
    
    return 0;
}


static void directWriteResponse(Rilat_Instance_t *instance, uint32_t packMsgId, bool isSuccess, bool isForce) {
    PtBufferCollection_t *ud = NULL;
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        ud = klist_entry(pos, PtBufferCollection_t, list);
        if (isForce) {
            klist_delete(&ud->list);
            free(ud->protocol);
            free(ud);
            ud = NULL;
            continue;
        } 
        if (packMsgId != ud->messageId) {
            continue;
        }
        klist_delete(&ud->list);
        break;
    }
    if (ud != NULL) {
        uint16_t messageId = ud->messageId;
        free(ud->protocol);
        free(ud);
        CREQUEST(PT_ON_TRANSMIT_SUCCESS, {.u16[0] = messageId, .u16[1] = isSuccess});
    }
}

static int32_t requestReport(cModule_Instance_t *ins, uint8_t cmd, uint8_t rc) {
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    _cModule_packMutexLock(ins, true);
    
    uint8_t *buf = calloc(1, 72 + PT_FRAME_BASE_SIZE);
    ASSERT(buf != NULL);
    buf[PT_CMD_IX] = cmd;
    buf[PT_DAT_IX + 0] = rc;

    uint16_t payloadSize = frameGen_base(buf, 1, CTL_UPLOAD | CTL_NONE_CONTINUE | CTL_FUNC_DATA_REPORT);
    mCurrentMid++;
    
    cModule_ProtocolGenMessage_t *msg = calloc(1, sizeof(cModule_ProtocolGenMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) buf;
    msg->payloadLength = payloadSize;
    
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolGenMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynData = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = (*_cModule_getGenMsgId())++;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    
    return pack->aux.gen.msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

void cModule_ptAepGh_ntpUpdate(cModule_Instance_t *ins) {
    ASSERT(ins != NULL);
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return;
    }
    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = 0;
    pack->payload = NULL;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_NTP_UPDATE;
    pack->aux.gen.msgId = (*_cModule_getGenMsgId())++;
    pack->aux.gen.sendRetryCount = 2;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
}

int32_t cModule_ptAepGh_report(cModule_Instance_t *ins, void *p, uint8_t cmd, uint32_t count, bool isContinue) {
    ASSERT(ins != NULL);
    
    if (CREQUEST(PT_GET_REGISTERED_STATE, {}).u32 == CMODULE_PTAEPGH_REG_ST_UNKNOW) {
        LOG_I("DeviceUnRegistered, Skip");
        return 0;
    }

    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    _cModule_packMutexLock(ins, true);
    
    cModule_ProtocolGenMessage_t *msg = calloc(1, sizeof(cModule_ProtocolGenMessage_t));
    ASSERT(msg != NULL);
    msg->payload = NULL;
    msg->payloadLength = 0;
    msg->writer.isWritenSuccess = false;
    msg->writer.onDirectWrite = directWrite;
    msg->writer.onDorectGetLength = directGetPayloadLength;
    msg->writer.onDirectWriteResponse = directWriteResponse;

    PtBufferCollection_t *ud = calloc(1, sizeof(PtBufferCollection_t));
    ASSERT(ud != NULL);
    klist_init(&ud->list);
    ud->ins = ins;
    ud->protocol = p;
    ud->sensorCount = count;
    ud->messageId = (*_cModule_getGenMsgId())++;
    ud->isContinue = isContinue;
    ud->command = cmd;
    klist_addTail(&mPtBufferList, &ud->list);
    
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolGenMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynData = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = ud->messageId;
    pack->aux.gen.sendRetryCount = 2;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    
    return pack->aux.gen.msgId;
}


int32_t cModule_ptAepGh_regsiter(cModule_Instance_t *ins, cModule_PtAepGh_DevType_t devType, const char *devCode) {
    if (CREQUEST(PT_GET_REGISTERED_STATE, {}).u32 == CMODULE_PTAEPGH_REG_ST_RELEASE) {
        LOG_I("DeviceRegistered");
        return 0;
    }
    
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    _cModule_packMutexLock(ins, true);
    
    uint8_t *buf = calloc(1, 72 + PT_FRAME_BASE_SIZE);
    ASSERT(buf != NULL);
    uint16_t payloadSize = frameGen_register(ins, buf, devType, devCode);
    payloadSize = frameGen_base(buf, payloadSize, CTL_UPLOAD | CTL_NONE_CONTINUE | CTL_FUNC_DATA_REPORT);
    mCurrentMid++;
    
    cModule_ProtocolGenMessage_t *msg = calloc(1, sizeof(cModule_ProtocolGenMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) buf;
    msg->payloadLength = payloadSize;
    
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolGenMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynData = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = (*_cModule_getGenMsgId())++;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    
    return pack->aux.gen.msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    uint8_t *p = &((uint8_t *) info->payload)[PT_HEAD_IX];
    if (*p != PT_HEAD) {
        return -1;
    }
    if (p[info->payloadLength - 1] != PT_TAIL) {
        return -1;
    }
    uint16_t payloadSize = (((uint16_t) p[PT_LEN_IX + 0]) << 8) | (((uint16_t) p[PT_LEN_IX + 1]));
    uint16_t crc = (((uint16_t) p[info->payloadLength - 3]) << 8) | (((uint16_t) p[info->payloadLength - 2]));
    if (crc != crc16(&p[PT_MID_IX], payloadSize - 19 + 11)) {
        return -1;
    }
    uint8_t cmd = p[PT_CMD_IX];
    uint8_t *dat = &p[PT_DAT_IX];
    if (cmd == 0xA1) { // Register
        if (dat[0] == RC_SUCCESS) {
            uint32_t activeState = dat[1];
            if (activeState == 0) {
                LOG_W("CMD=A1, UnActived");
            } else {
                LOG_I("CMD=A1, Actived, %d", activeState);
                CREQUEST(PT_SET_DEVICE_ID8, {.ptr = &dat[2]}, activeState);
                CREQUEST(PT_SET_REPORT_OPTIONS, {.ptr = &dat[10]});
            }
        } else {
            LOG_W("A1, ERR=%d", dat[0]);
            CREQUEST(PT_ON_REG_FAILED, {});
        }
    } else if (cmd == 0x30) { // UploadOption
        LOG_I("CMD=30, UploadOption");
        CREQUEST(PT_SET_REPORT_OPTIONS, {.ptr = &dat[0]});
        requestReport(ins, cmd, RC_SUCCESS);
    } else if (cmd == 0x33) { // DiffOption
        LOG_I("CMD=33, DiffOption");
        cModule_PtAepGh_DiffOption_t diff;
        diff.enable = dat[0];
        diff.gasType = dat[1];
        diff.sec = dat[2];
        diff.diff = (((uint16_t) dat[3]) << 8) & 0xFF00;
        diff.diff |= (((uint16_t) dat[4])) & 0x00FF;
        CREQUEST(PT_SET_DIFF_OPTION, {.ptr = &diff});
        requestReport(ins, cmd, RC_SUCCESS);
    } else if (cmd == 0x32) { // AlarmOption
        LOG_I("CMD=32, AlarmOption");
        cModule_PtAepGh_AlarmThresholdOption_t th;
        th.gasType = dat[0];
        th.low = (((uint16_t) dat[1]) << 8) & 0xFF00;
        th.low |= (((uint16_t) dat[2])) & 0x00FF;
        th.high = (((uint16_t) dat[3]) << 8) & 0xFF00;
        th.high |= (((uint16_t) dat[4])) & 0x00FF;
        CREQUEST(PT_SET_ALARM_THRESHOLD_OPTION, {.ptr = &th});
        requestReport(ins, cmd, RC_SUCCESS);
    } else if (cmd == 0xA5) { // Operate1
        LOG_I("CMD=A5, Alarm1");
       
        CREQUEST(PT_ON_SAMPLE_ACTION, {.u32 = dat[0]});
        requestReport(ins, cmd, RC_SUCCESS);
    }
    
    return -1;
}

/*@}*/

/**
 * @addtogroup Public
 * @note none
 */
 
/*@{*/

void cModule_protocol_init(void) {
    klist_init(&mPtBufferList);
}

/*@}*/

#endif

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME

/*@}*/
