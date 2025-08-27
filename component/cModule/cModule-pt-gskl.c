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
#define DBG_SECTION_NAME  "gskl"

#define CMASSERT(err) ASSERT(err)

/*@}*/

#if CONFIG_CMODULE_PROTOCOL_IS_GSKL_TCP == 1

#include "mbedtls/md.h"
#include "mbedtls/aes.h"

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

#define PT_HEAD     0x68
#define PT_TVER     0x53
#define PT_VVER     0x02
#define PT_TAIL     0x16

#define PT_HEAD_IX       0
#define PT_HEAD_LX       1

#define PT_TVER_IX       (PT_HEAD_IX + PT_HEAD_LX)
#define PT_TVER_LX       1

#define PT_VVER_IX       (PT_TVER_IX + PT_TVER_LX)
#define PT_VVER_LX       1

#define PT_LEN_IX        (PT_VVER_IX + PT_VVER_LX)
#define PT_LEN_LX        2

#define PT_ADDR_IX        (PT_LEN_IX + PT_LEN_LX)
#define PT_ADDR_LX        17

#define PT_MID_IX        (PT_ADDR_IX + PT_ADDR_LX)
#define PT_MID_LX        1

#define PT_CTL_IX        (PT_MID_IX + PT_MID_LX)
#define PT_CTL_LX        2

#define PT_DID_IX        (PT_CTL_IX + PT_CTL_LX)
#define PT_DID_LX        2

#define PT_KV_IX        (PT_DID_IX + PT_DID_LX)
#define PT_KV_LX        1

#define PT_DATA_IX        (PT_KV_IX + PT_KV_LX)

#define PT_CRC_IX(pl)    (PT_DATA_IX + pl)
#define PT_CRC_LX        2

#define PT_TAIL_IX(pl)   (PT_DATA_IX + pl + 2)
#define PT_TAIL_LX       1

#define PT_FRAME_BASE_SIZE          (PT_DATA_IX + 3)
#define PT_FRAME_DAT_MAC_SIZE       (32)

typedef enum {
    CTL_UPLOAD = 0,
    CTL_DOWNLOAD = KLBIT(15),
    
    CTL_NONE_CONTINUE = 0,
    CTL_HAS_CONTINUE = KLBIT(14),
    
    CTL_KEY_ST_DEFUALT = KLBIT(0),
    CTL_KEY_ST_RELEASE = KLBIT(13),
    
    CTL_ENCRYPT_NONE = KLBIT(0),
    CTL_ENCRYPT_SET = KLBIT(12),
    
    CTL_VERFY_NONE = KLBIT(0),
    CTL_VERFY_MAC = KLBIT(11),
    
    CTL_FUNC_DATA_REPORT = 0x01,
    CTL_FUNC_DATA_DISTRIBUTE = 0x02,
    CTL_FUNC_DATA_READ_ADDR = 0x03,
    CTL_FUNC_DATA_READ = 0x04,
    CTL_FUNC_DATA_WRITE = 0x05,
    CTL_FUNC_DATA_WRITE_READ = 0x06,
    CTL_FUNC_KEY_RESET = 0x07,
    CTL_FUNC_KEY_UPDATE = 0x08,
} CtlCode_t;

typedef struct {
    cModule_Instance_t *ins;
    
    struct {
        uint16_t id;
        void *obj;
        uint8_t count;
    } obj;
    struct {
        uint16_t dataLen;
    } pack;
    
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

static klist_t mPtBufferList;

static uint8_t mCurrentMid = 0;

/*@}*/

/**
 * @addtogroup ProtocolPrivate
 * @note none
 */

/*@{*/

static void createDevId(uint8_t *devIdRaw, uint8_t *devIdOut) {
    uint8_t tn = 0;
    for (uint8_t i = 0; i < 7; i++) {
        tn = (devIdRaw[i * 2 + 0] - '0') * 10 + (devIdRaw[i * 2 + 1] - '0');
        devIdOut[i] = bin2bcd(tn);
    }
}

static int32_t pkcs7_padding(uint8_t *buffer, size_t dataLen) {
	uint8_t paddingNum = 16 - (dataLen % 16);
	memset(&buffer[dataLen], paddingNum, paddingNum);
	return dataLen + paddingNum;
}

static int32_t pkcs7_paddingLength(size_t dataLen) {
    uint8_t paddingNum = 16 - (dataLen % 16);
    return dataLen + paddingNum;
}

static void decrypt(uint8_t *encryptData, uint16_t encryptDataLen, uint8_t *outData) {
    uint8_t *key = calloc(1, 16 + 1);
    ASSERT(key != NULL);
    klStr_hexStr2hex(CREQUEST(PT_GET_ENCRYPT_KEY, {}).ptr, 32, (char *) key);
        
    mbedtls_aes_context *aes = calloc(1, sizeof(mbedtls_aes_context));
    ASSERT(aes != NULL);
    mbedtls_aes_init(aes);
    ASSERT(mbedtls_aes_setkey_dec(aes, key, 128) == 0);
    for (uint8_t i = 0; i < encryptDataLen / 16; i++) {
        ASSERT(mbedtls_aes_crypt_ecb(aes, MBEDTLS_AES_DECRYPT, &encryptData[i * 16], &outData[i * 16]) == 0);
    }
    mbedtls_aes_free(aes);
    free(aes);
        
    free(key);
}

static void encrypt_withHmacSha256(uint8_t *devId, uint8_t *rawData, size_t rawDataLen, uint8_t *outData) {
    uint16_t paddingDataLen = pkcs7_paddingLength(rawDataLen);
    uint8_t *paddingData = calloc(1, paddingDataLen + 1);
    ASSERT(paddingData != NULL);
    pkcs7_padding(paddingData, rawDataLen);
    memcpy(paddingData, rawData, rawDataLen);

    uint8_t *randCodePadding = calloc(1, pkcs7_paddingLength(16) + 1);
    ASSERT(randCodePadding != NULL);
    pkcs7_padding(randCodePadding, 16);
    createDevId(devId, randCodePadding + 9);
        
    uint8_t *key = calloc(1, 16 + 1);
    ASSERT(key != NULL);
    klStr_hexStr2hex(CREQUEST(PT_GET_ENCRYPT_KEY, {}).ptr, 32, (char *) key);
    uint8_t *mdKey = calloc(1, 16 + 1);
    ASSERT(mdKey != NULL);
        
    mbedtls_aes_context *aes = calloc(1, sizeof(mbedtls_aes_context));
    ASSERT(aes != NULL);
    mbedtls_aes_init(aes);
    ASSERT(mbedtls_aes_setkey_enc(aes, key, 128) == 0);
    for (uint8_t i = 0; i < paddingDataLen / 16; i++) {
        ASSERT(mbedtls_aes_crypt_ecb(aes, MBEDTLS_AES_ENCRYPT, &paddingData[i * 16], &outData[i * 16]) == 0);
    }
    mbedtls_aes_free(aes);
    free(paddingData);
    
    mbedtls_aes_init(aes);
    ASSERT(mbedtls_aes_setkey_enc(aes, key, 128) == 0);
    ASSERT(mbedtls_aes_crypt_ecb(aes, MBEDTLS_AES_ENCRYPT, randCodePadding, mdKey) == 0);
    mbedtls_aes_free(aes);

    free(aes);
    free(key);

    const mbedtls_md_info_t *mdInfo;
    mbedtls_md_context_t *md = calloc(1, sizeof(mbedtls_md_context_t));
    ASSERT(md != NULL);
    uint8_t *mdResult = calloc(1, 32 + 1);
    ASSERT(mdResult != NULL);
    mbedtls_md_init(md);
    mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_setup(md, mdInfo, 1);
    mbedtls_md_hmac_starts(md, mdKey, 16);
    mbedtls_md_hmac_update(md, randCodePadding, 16);
    mbedtls_md_hmac_update(md, outData, paddingDataLen);
    mbedtls_md_hmac_finish(md, mdResult);
    mbedtls_md_free(md);
    memcpy(&outData[paddingDataLen], mdResult, 32);
    free(mdResult);
    free(md);
        
    free(mdKey);
    free(randCodePadding);
}

void crc16_invertUint8(unsigned char *dBuf, unsigned char *srcBuf) {
    int i;
    unsigned char tmp[4];
    tmp[0] = 0;
    for (i = 0; i < 8; i++) {
        if (srcBuf[0] & (1 << i))
            tmp[0] |= 1 << (7 - i);
    }
    dBuf[0] = tmp[0];
}

void crc16_invertUint16(unsigned short *dBuf, unsigned short *srcBuf) {
    int i;
    unsigned short tmp[4];
    tmp[0] = 0;
    for (i = 0; i < 16; i++) {
        if (srcBuf[0] & (1 << i))
            tmp[0] |= 1 << (15 - i);
    }
    dBuf[0] = tmp[0];
}

static uint16_t crc16(unsigned char *puchMsg, unsigned int usDataLen) { 
    unsigned short wCRCin = 0xFFFF;
    unsigned short wCPoly = 0x8005;
    unsigned char wChar = 0;
    while (usDataLen--) {
        wChar = *(puchMsg++);
        crc16_invertUint8(&wChar, &wChar);
        wCRCin ^= (wChar << 8);
        for (int i = 0; i < 8; i++) {
            if (wCRCin & 0x8000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    crc16_invertUint16(&wCRCin, &wCRCin);
    return (wCRCin); 
}

static uint16_t frameGen_base(uint8_t *devId, uint8_t *buf, uint16_t payloadSize, CtlCode_t ctl, uint16_t objId) {
    buf[PT_HEAD_IX] = PT_HEAD;
    buf[PT_TVER_IX] = PT_TVER;
    buf[PT_VVER_IX] = PT_VVER;
    
    uint16_t frameSize = PT_FRAME_BASE_SIZE + payloadSize;
    buf[PT_LEN_IX + 0] = (frameSize >> 8) & 0x00FF;
    buf[PT_LEN_IX + 1] = (frameSize >> 0) & 0x00FF;
    
    memset(&buf[PT_ADDR_IX + 0], 0x00, 17);
    buf[PT_ADDR_IX + 0] = 0x14;
    createDevId(devId, &buf[PT_ADDR_IX + 10]);
    
    buf[PT_MID_IX] = mCurrentMid++;
        
    buf[PT_CTL_IX + 0] = (ctl >> 8) & 0x00FF;
    buf[PT_CTL_IX + 1] = (ctl >> 0) & 0x00FF;
        
    buf[PT_DID_IX + 0] = (objId >> 8) & 0x00FF;
    buf[PT_DID_IX + 1] = (objId >> 0) & 0x00FF;
        
    buf[PT_KV_IX] = 0;

    uint16_t crc = crc16(buf, frameSize - 3);
    buf[PT_CRC_IX(payloadSize) + 0] = (crc >> 8) & 0x00FF;
    buf[PT_CRC_IX(payloadSize) + 1] = (crc >> 0) & 0x00FF;
    buf[PT_TAIL_IX(payloadSize)] = PT_TAIL;
    return frameSize;
}

static uint32_t dataGen_report(cModule_Instance_t *ins, Protocol_Gskl_9031_t *data, uint8_t count, uint8_t *out) {
    if (out == NULL || data == NULL) {
        return 17 * count;
    }
    volatile uint16_t idx = 0;

    for (uint8_t i = 0; i < count; i++) {
        klDateTime_SampleTm_t tm;
        klDateTime_bktime(data->ts, &tm);
        out[idx++] = data->channel;
        out[idx++] = (data->status >> 8) & 0x00FF;
        out[idx++] = (data->status >> 0) & 0x00FF;
        out[idx++] = (data->lelAvg_x100 >> 8) & 0x00FF;
        out[idx++] = (data->lelAvg_x100 >> 0) & 0x00FF;
        out[idx++] = (data->lelMax_x100 >> 8) & 0x00FF;
        out[idx++] = (data->lelMax_x100 >> 0) & 0x00FF;
        out[idx++] = (data->lelMin_x100 >> 8) & 0x00FF;
        out[idx++] = (data->lelMin_x100 >> 0) & 0x00FF;
        out[idx++] = (data->bvatVolt_x100 >> 8) & 0x00FF;
        out[idx++] = (data->bvatVolt_x100 >> 0) & 0x00FF;
        out[idx++] = bin2bcd(tm.year - 2000);
        out[idx++] = bin2bcd(tm.mon);
        out[idx++] = bin2bcd(tm.day);
        out[idx++] = bin2bcd(tm.hour);
        out[idx++] = bin2bcd(tm.min);
        out[idx++] = bin2bcd(tm.sec);
        
        LOG_I("[pt-gskl] SensorGened, %d", i);
    }
    
    return 17 * count;
}

static uint32_t dataGen_register(cModule_Instance_t *ins, uint8_t *data) {
    if (data == NULL) {
        return 76;
    }
    
    volatile uint8_t idx = 0;
    
    klDateTime_SampleTm_t tm;
    klDateTime_bktime(ins->aux.timeStamp + (SECS_PER_HOUR * 8), &tm);
        
    // 从站时钟
    data[idx++] = bin2bcd(tm.year - 2000);
    data[idx++] = bin2bcd(tm.mon);
    data[idx++] = bin2bcd(tm.day);
    data[idx++] = bin2bcd(tm.hour);
    data[idx++] = bin2bcd(tm.min);
    data[idx++] = bin2bcd(tm.sec);
    
    // 通信制式
    data[idx++] = 4; // 1电信NB，2移动NB，3联通NB，4电信4G，5移动4G，6联通4G
    // SIM卡类型
    data[idx++] = 0; // 0贴片卡, 1插拔卡, 2eSIM, 3其它
    
    memcpy(&data[idx], ins->modemInfo.imei, 15);
    idx += 15;
    memcpy(&data[idx], ins->modemInfo.iccid, 20);
    idx += 20;
    memcpy(&data[idx], ins->modemInfo.imsi, 15);
    idx += 15;
    
    // CSQ
    data[idx++] = ins->modemInfo.rssi;
    // ECL
    data[idx++] = 0;
    // RSRP
    data[idx++] = 0;
    data[idx++] = 0;
    // RSSI
    data[idx++] = 0;
    data[idx++] = 0;
    // SNR
    data[idx++] = 0;
    data[idx++] = 0;
    // CELLID
    data[idx++] = 0;
    data[idx++] = 0;
    data[idx++] = 0;
    data[idx++] = 0;
    // PCI
    data[idx++] = 0;
    data[idx++] = 0;
    // EARFCN
    data[idx++] = 0;
    data[idx++] = 0;
    // 版本号
    data[idx++] = 0x01;
    data[idx++] = 0x10;

    return 76;
}

static int32_t ack(cModule_Instance_t *ins, Protocol_Gskl_ObjId_t id, uint16_t rc) {
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    _cModule_packMutexLock(ins, true);
    
    uint8_t *sendBuffer = NULL;
    uint16_t sendBufferSize = 0;
    
    if (id == GSKL_OBJID_SET_KEY 
        || id == GSKL_OBJID_SET_OPTS
        || id == GSKL_OBJID_SET_ADDR
        || id == GSKL_OBJID_RESET_KEY) {
        uint8_t rawDataLen = 2;
        uint8_t *rawData = calloc(1, rawDataLen + 1);
        ASSERT(rawData != NULL);
        rawData[0] = rc >> 8;
        rawData[1] = rc & 0x00FF;
        
        uint16_t crptyDataLen = PT_FRAME_DAT_MAC_SIZE + pkcs7_paddingLength(rawDataLen);
        sendBufferSize = crptyDataLen + PT_FRAME_BASE_SIZE;
        sendBuffer = calloc(1, sendBufferSize + 1);
        ASSERT(sendBuffer != NULL);
        
        encrypt_withHmacSha256(CREQUEST(PT_GET_DEVSN, {}).ptr, rawData, rawDataLen, &sendBuffer[PT_DATA_IX]);
        frameGen_base(CREQUEST(PT_GET_DEVSN, {}).ptr, sendBuffer, crptyDataLen, 
            CTL_UPLOAD | CTL_NONE_CONTINUE | (CREQUEST(PT_IS_ENCRYPT_KEY, {}).u32 ? CTL_KEY_ST_RELEASE : CTL_KEY_ST_DEFUALT) 
                | CTL_ENCRYPT_SET | CTL_VERFY_MAC 
                | (id == GSKL_OBJID_RESET_KEY ? CTL_FUNC_KEY_RESET : (id == GSKL_OBJID_SET_KEY ? CTL_FUNC_KEY_UPDATE : CTL_FUNC_DATA_REPORT)),
            id);
        
        free(rawData);
    } else if (id == GSKL_OBJID_GET_ADDR) {
        sendBufferSize = 66 + PT_FRAME_BASE_SIZE;
        sendBuffer = calloc(1, sendBufferSize + 1);
        ASSERT(sendBuffer != NULL);
        uint8_t *data = &sendBuffer[PT_DATA_IX];
        Protocol_Gskl_NetTarget_t *nt = calloc(1, sizeof(Protocol_Gskl_NetTarget_t));
        ASSERT(nt != NULL);
        CREQUEST(PT_GET_HOST_TARGET, {.ptr = nt});
        data[0] = nt->target1;
        memcpy(&data[1], nt->host1, 30);
        data[31] = (nt->port1 >> 8) & 0x00FF;
        data[32] = (nt->port1 >> 0) & 0x00FF;
        data[33] = nt->target2;
        memcpy(&data[34], nt->host2, 30);
        data[64] = (nt->port2 >> 8) & 0x00FF;
        data[65] = (nt->port2 >> 0) & 0x00FF;
        free(nt);
        
        frameGen_base(CREQUEST(PT_GET_DEVSN, {}).ptr, sendBuffer, 66, 
                CTL_UPLOAD | CTL_NONE_CONTINUE | (CREQUEST(PT_IS_ENCRYPT_KEY, {}).u32 ? CTL_KEY_ST_RELEASE : CTL_KEY_ST_DEFUALT) 
                | CTL_ENCRYPT_NONE | CTL_VERFY_NONE | CTL_FUNC_DATA_REPORT,
            GSKL_OBJID_GET_ADDR);
    } else if (id == GSKL_OBJID_GET_OPTS) {
        sendBufferSize = 23 + PT_FRAME_BASE_SIZE;
        sendBuffer = calloc(1, sendBufferSize + 1);
        ASSERT(sendBuffer != NULL);
        
        sendBufferSize = 23 + PT_FRAME_BASE_SIZE;
        sendBuffer = calloc(1, sendBufferSize + 1);
        ASSERT(sendBuffer != NULL);
        uint8_t *data = &sendBuffer[PT_DATA_IX];
        Protocol_Gskl_Timing_t *timing = calloc(1, sizeof(Protocol_Gskl_Timing_t));
        ASSERT(timing != NULL);
        CREQUEST(PT_GET_TIMING, {.ptr = timing});
        Protocol_Gskl_Threshold_t *th = calloc(1, sizeof(Protocol_Gskl_Threshold_t));
        CREQUEST(PT_GET_ALARM_TH, {.ptr = th});
        ASSERT(th != NULL);
        th->ll = th->ll * 100;
        th->l = th->l * 100;
        th->h = th->h * 100;
        data[0] = bin2bcd(timing->uploadTime[0]);
        data[1] = bin2bcd(timing->uploadTime[1]);
        data[2] = timing->discreteTime;
        data[3] = (timing->idleMeasureInterval >> 8) & 0x00FF;
        data[4] = (timing->idleMeasureInterval >> 0) & 0x00FF;
        data[5] = (timing->idleReportInterval >> 8) & 0x00FF;
        data[6] = (timing->idleReportInterval >> 0) & 0x00FF;
        data[7] = (timing->preAlarmMeasureInterval >> 8) & 0x00FF;
        data[8] = (timing->preAlarmMeasureInterval >> 0) & 0x00FF;
        data[9] = (timing->preAlarmReportInterval >> 8) & 0x00FF;
        data[10] = (timing->preAlarmReportInterval >> 0) & 0x00FF;
        data[11] = (timing->alarmMeasureInterval >> 8) & 0x00FF;
        data[12] = (timing->alarmMeasureInterval >> 0) & 0x00FF;
        data[13] = (timing->alarmReportInterval >> 8) & 0x00FF;
        data[14] = (timing->alarmReportInterval >> 0) & 0x00FF;
        data[15] = ((uint16_t) th->ll >> 8) & 0x00FF;
        data[16] = ((uint16_t) th->ll >> 0) & 0x00FF;
        data[17] = ((uint16_t) th->l >> 8) & 0x00FF;
        data[18] = ((uint16_t) th->l >> 0) & 0x00FF;
        data[19] = ((uint16_t) th->h >> 8) & 0x00FF;
        data[20] = ((uint16_t) th->h >> 0) & 0x00FF;
        data[21] = (timing->heartBeat >> 8) & 0x00FF;
        data[22] = (timing->heartBeat >> 0) & 0x00FF;
        free(timing);
        free(th);
        frameGen_base(CREQUEST(PT_GET_DEVSN, {}).ptr, sendBuffer, 23, 
                CTL_UPLOAD | CTL_NONE_CONTINUE | (CREQUEST(PT_IS_ENCRYPT_KEY, {}).u32 ? CTL_KEY_ST_RELEASE : CTL_KEY_ST_DEFUALT) 
                | CTL_ENCRYPT_NONE | CTL_VERFY_NONE | CTL_FUNC_DATA_REPORT,
            GSKL_OBJID_GET_OPTS);
    }
    
    cModule_ProtocolTcpIpMessage_t *msg = calloc(1, sizeof(cModule_ProtocolTcpIpMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) sendBuffer;
    msg->payloadLength = sendBufferSize;
    msg->dStream = false;
    msg->timeout = 10000;

    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolTcpIpMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynTcpIpData = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = mCurrentMid;
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

static void fill_threshold(Protocol_Gskl_Threshold_t *th, uint8_t *data) {
    th->ll = (float) ((((uint16_t) data[15]) << 8) | ((uint16_t) data[16])) * 0.01;
    th->l = (float) ((((uint16_t) data[17]) << 8) | ((uint16_t) data[18])) * 0.01;
    th->h = (float) ((((uint16_t) data[19]) << 8) | ((uint16_t) data[20])) * 0.01;
}

static void fill_timing(Protocol_Gskl_Timing_t *timing, uint8_t *data) {
    uint16_t resultCode = 0;
    
    timing->uploadTime[0] = bcd2bin(data[0]);
    timing->uploadTime[1] = bcd2bin(data[1]);
    timing->discreteTime = data[2];
    
    resultCode = (((uint16_t) data[3]) << 8) | ((uint16_t) data[4]);
    if (resultCode != 0) {
        timing->idleMeasureInterval = resultCode;
    }
    
    resultCode = (((uint16_t) data[5]) << 8) | ((uint16_t) data[6]);
    if (resultCode != 0) {
        timing->idleReportInterval = resultCode;
    }
    
    resultCode = (((uint16_t) data[7]) << 8) | ((uint16_t) data[8]);
    if (resultCode != 0) {
        timing->preAlarmMeasureInterval = resultCode;
    }
    
    resultCode = (((uint16_t) data[9]) << 8) | ((uint16_t) data[10]);
    if (resultCode != 0) {  
        timing->preAlarmReportInterval = resultCode;
    }
    
    resultCode = (((uint16_t) data[11]) << 8) | ((uint16_t) data[12]);
    if (resultCode != 0) {
        timing->alarmMeasureInterval = resultCode;
    }
    
    resultCode = (((uint16_t) data[13]) << 8) | ((uint16_t) data[14]);
    if (resultCode != 0) {
        timing->alarmReportInterval = resultCode;
    }
    
    resultCode = (((uint16_t) data[21]) << 8) | ((uint16_t) data[22]);
    if (resultCode != 0) {
        timing->heartBeat = resultCode;
    }
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
        
        uint8_t *sendBuffer = calloc(1, ud->pack.dataLen + PT_FRAME_BASE_SIZE + 1);
        ASSERT(sendBuffer != NULL);
        
        if (ud->obj.id == GSKL_OBJID_REGISTER) {
            uint8_t rawDataLen = dataGen_register(NULL, NULL);
            uint8_t *rawData = calloc(1, rawDataLen + 1);
            ASSERT(rawData != NULL);
            dataGen_register(ud->ins, rawData);
            
            encrypt_withHmacSha256(CREQUEST(PT_GET_DEVSN, {}).ptr, rawData, rawDataLen, &sendBuffer[PT_DATA_IX]);
          
            frameGen_base(CREQUEST(PT_GET_DEVSN, {}).ptr, sendBuffer, ud->pack.dataLen, 
                CTL_UPLOAD | CTL_NONE_CONTINUE | CTL_KEY_ST_DEFUALT | CTL_ENCRYPT_SET | CTL_VERFY_MAC | CTL_FUNC_DATA_REPORT,
                ud->obj.id);
            
            hexWrite(instance, sendBuffer, ud->pack.dataLen + PT_FRAME_BASE_SIZE);
            
            free(rawData);
        } else if (ud->obj.id == GSKL_OBJID_REPORT_SELF || ud->obj.id == GSKL_OBJID_REPORT_ALARM) {
            uint8_t rawDataLen = dataGen_report(NULL, NULL, ud->obj.count, NULL);
            uint8_t *rawData = calloc(1, rawDataLen + 1);
            ASSERT(rawData != NULL);
            dataGen_report(ud->ins, ud->obj.obj, ud->obj.count, rawData);
            
            encrypt_withHmacSha256(CREQUEST(PT_GET_DEVSN, {}).ptr, rawData, rawDataLen, &sendBuffer[PT_DATA_IX]);
          
            frameGen_base(CREQUEST(PT_GET_DEVSN, {}).ptr, sendBuffer, ud->pack.dataLen, 
                CTL_UPLOAD | CTL_NONE_CONTINUE 
                    | (CREQUEST(PT_IS_ENCRYPT_KEY, {}).u32 ? CTL_KEY_ST_RELEASE : CTL_KEY_ST_DEFUALT) 
                    | CTL_ENCRYPT_SET | CTL_VERFY_MAC | CTL_FUNC_DATA_REPORT,
                ud->obj.id);
            
            hexWrite(instance, sendBuffer, ud->pack.dataLen + PT_FRAME_BASE_SIZE);
            
            free(rawData);
        }
        
        free(sendBuffer);

        ud->isWriten = true;
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
        if (ud->obj.id == GSKL_OBJID_REGISTER) {
            len += pkcs7_paddingLength(dataGen_register(NULL, NULL));
            len += PT_FRAME_DAT_MAC_SIZE;
            ud->pack.dataLen = len;
            len += PT_FRAME_BASE_SIZE;
        } else if (ud->obj.id == GSKL_OBJID_REPORT_SELF || ud->obj.id == GSKL_OBJID_REPORT_ALARM) {
            len += pkcs7_paddingLength(dataGen_report(NULL, NULL, ud->obj.count, NULL));
            len += PT_FRAME_DAT_MAC_SIZE;
            ud->pack.dataLen = len;
            len += PT_FRAME_BASE_SIZE;
        }
        
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
            if (ud->obj.id == GSKL_OBJID_REPORT_SELF || ud->obj.id == GSKL_OBJID_REPORT_ALARM) {
                free(ud->obj.obj);
            }
            free(ud);
            ud = NULL;
            continue;
        }
        
        if (!ud->isWriten) {
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
        if (ud->obj.id == GSKL_OBJID_REPORT_SELF || ud->obj.id == GSKL_OBJID_REPORT_ALARM) {
            free(ud->obj.obj);
        }
        free(ud);
        CREQUEST(PT_ON_TRANSMIT_SUCCESS, {.u16[0] = ud->messageId, .u16[1] = isSuccess});
    }
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_protocolTransmit_gskl(cModule_Instance_t *ins, Protocol_Gskl_ObjId_t objId, void *obj, uint32_t objCount) {
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    int32_t rc = -1;
    cModule_ProtocolTcpIpMessage_t *msg = calloc(1, sizeof(cModule_ProtocolTcpIpMessage_t));
    ASSERT(msg != NULL);
    
    msg->payload = NULL;
    msg->payloadLength = 0;
    
    msg->writer.onDirectWrite = directWrite;
    msg->writer.onDirectGetLength = directGetPayloadLength;
    msg->writer.onDirectWriteResponse = directWriteResponse;
    
    msg->host = NULL;
    msg->hostContant = true;
    msg->port = 0;
    
    msg->dStream = false;
    msg->timeout = 10000;
    
    PtBufferCollection_t *ud = calloc(1, sizeof(PtBufferCollection_t));
    ASSERT(ud != NULL);
    klist_init(&ud->list);
    ud->ins = ins;
    ud->obj.count = objCount;
    ud->obj.obj = obj;
    ud->obj.id = objId;
    ud->messageId = (*_cModule_getGenMsgId())++;
    klist_addTail(&mPtBufferList, &ud->list);
    
    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolTcpIpMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynTcpIpData = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = ud->messageId;
    pack->aux.gen.sendRetryCount = 0;
    pack->aux.gen.sendRetryCountReload = 0;
    pack->aux.gen.sendFailedAfterReboot = 0;
    pack->aux.gen.sendRetrySec = 0;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    pack->aux.gen.sendBeforeDelaySec = 3;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    
    LOG_I("[pt-gskl] report, %04X", objId);
    
    return pack->aux.gen.msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

void _cModule_onProtocolTcpConnectAlloc(cModule_Instance_t *ins, 
        char **borkerIp) {
}

void _cModule_onProtocolTcpConnect(cModule_Instance_t *ins, 
        char **borkerIp, uint16_t *port) {
    *borkerIp = CREQUEST(PT_GET_IP, {}).ptr;
    *port = CREQUEST(PT_GET_PORT, {}).u32;
}
        
void _cModule_onProtocolTcpConnectFree(cModule_Instance_t *ins, 
        char **borkerIp) {
}
        
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
    if (crc != crc16(&p[PT_HEAD_IX], payloadSize - 3)) {
        return -1;
    }
    uint16_t did = (((uint16_t) p[PT_DID_IX + 0]) << 8) | (((uint16_t) p[PT_DID_IX + 1]));
    uint16_t encryptDataLen;
    uint8_t *decrptyData = NULL;
    if (did != GSKL_OBJID_GET_ADDR && did != GSKL_OBJID_GET_OPTS) {
        encryptDataLen = payloadSize - PT_FRAME_BASE_SIZE - PT_FRAME_DAT_MAC_SIZE;
        decrptyData = calloc(1, encryptDataLen + 1);
        ASSERT(decrptyData != NULL);
        decrypt(&p[PT_DATA_IX], encryptDataLen, decrptyData);
    }
    
    if (did == GSKL_OBJID_REGISTER) {
        uint16_t resultCode = (((uint16_t) decrptyData[0]) << 8) | ((uint16_t) decrptyData[1]);
        if (resultCode == 0x0000 || resultCode == 0x0101 || resultCode == 0x0111) {
            klDateTime_SampleTm_t *tm = calloc(1, sizeof(klDateTime_SampleTm_t));
            ASSERT(tm != NULL);
            tm->year = 2000 + bcd2bin(decrptyData[2]);
            tm->mon = bcd2bin(decrptyData[3]);
            tm->day = bcd2bin(decrptyData[4]);
            tm->hour = bcd2bin(decrptyData[5]);
            tm->min = bcd2bin(decrptyData[6]);
            tm->sec = bcd2bin(decrptyData[7]);
            CREQUEST(PT_ON_UPDATE_TS, {.u32 = klDateTime_mktime(tm)});
            free(tm);
            
            Protocol_Gskl_Timing_t *timing = calloc(1, sizeof(Protocol_Gskl_Timing_t));
            ASSERT(timing != NULL);
            fill_timing(timing, &decrptyData[8]);
            CREQUEST(PT_ON_UPDATE_TIMING, {.ptr = timing});
            free(timing);
            
            Protocol_Gskl_Threshold_t *th = calloc(1, sizeof(Protocol_Gskl_Threshold_t));
            ASSERT(th != NULL);
            fill_threshold(th, &decrptyData[8]);
            CREQUEST(PT_ON_UPDATE_ALARM_TH, {.ptr = th});
            free(th);
            
            CREQUEST(PT_ON_REGISTER_COMPLETED, {.u32 = true});
        } else {
            CREQUEST(PT_ON_REGISTER_COMPLETED, {.u32 = false});
        }
        LOG_I("ReportDid=%04X, %04X", did, resultCode);
    } else if (did == GSKL_OBJID_SET_KEY) {
        CREQUEST(PT_ON_KEY_SET, {.ptr = decrptyData});
        ack(ins, GSKL_OBJID_SET_KEY, 0);
    } else if (did == GSKL_OBJID_RESET_KEY) {
        klDateTime_SampleTm_t *tm = calloc(1, sizeof(klDateTime_SampleTm_t));
        ASSERT(tm != NULL);
        tm->year = 2000 + bcd2bin(decrptyData[2]);
        tm->mon = bcd2bin(decrptyData[3]);
        tm->day = bcd2bin(decrptyData[4]);
        tm->hour = bcd2bin(decrptyData[5]);
        tm->min = bcd2bin(decrptyData[6]);
        tm->sec = bcd2bin(decrptyData[7]);
        uint32_t ts = klDateTime_mktime(tm);
        free(tm);
        if (ts - (cModule_getTimeStamp(CONFIG_CMODULE_INSTANCE()) + 8 * SECS_PER_HOUR) < SECS_PER_DAY) {
            ack(ins, GSKL_OBJID_RESET_KEY, 0);
            CREQUEST(PT_ON_KEY_RESET, {});
        }
    } else if (did == GSKL_OBJID_SET_ADDR) {
        Protocol_Gskl_NetTarget_t *nt = calloc(1, sizeof(Protocol_Gskl_NetTarget_t));
        ASSERT(nt != NULL);
        nt->target1 = decrptyData[0];
        memcpy(nt->host1, &decrptyData[1], 30);
        nt->port1 = (((uint16_t) decrptyData[31]) << 8) | ((uint16_t) decrptyData[32]);
        nt->target2 = decrptyData[33];
        memcpy(nt->host2, &decrptyData[34], 30);
        nt->port2 = (((uint16_t) decrptyData[64]) << 8) | ((uint16_t) decrptyData[65]);
        CREQUEST(PT_ON_UPDATE_HOST_TARGET, {.ptr = nt});
        free(nt);
        ack(ins, GSKL_OBJID_SET_ADDR, 0);
    } else if (did == GSKL_OBJID_GET_ADDR) {
        ack(ins, GSKL_OBJID_GET_ADDR, 0);
    } else if (did == GSKL_OBJID_SET_OPTS) {
        Protocol_Gskl_Timing_t *timing = calloc(1, sizeof(Protocol_Gskl_Timing_t));
        ASSERT(timing != NULL);
        fill_timing(timing, decrptyData);
        CREQUEST(PT_ON_UPDATE_TIMING, {.ptr = timing});
        free(timing);
        
        Protocol_Gskl_Threshold_t *th = calloc(1, sizeof(Protocol_Gskl_Threshold_t));
        ASSERT(th != NULL);
        fill_threshold(th, decrptyData);
        CREQUEST(PT_ON_UPDATE_ALARM_TH, {.ptr = th});
        free(th);
        
        ack(ins, GSKL_OBJID_SET_OPTS, 0);
    } else if (did == GSKL_OBJID_GET_OPTS) {
        ack(ins, GSKL_OBJID_GET_OPTS, 0);
    } else if (did == GSKL_OBJID_REPORT_SELF || did == GSKL_OBJID_REPORT_ALARM) {
        uint16_t resultCode = (((uint16_t) decrptyData[0]) << 8) | ((uint16_t) decrptyData[1]);
        if (resultCode == 0x0000 || resultCode == 0x0101) {
            LOG_I("ReportDid=%04X, Success", did);
        } else {
            LOG_I("ReportDid=%04X, Failed, %04X", did, resultCode);
        }
    }
    
    free(decrptyData);
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

bool cModule_protocol_isCaching(void) {
    return klist_empty(&mPtBufferList) ? false : true;
}    

const char *cModule_protocol_getDefaultKey(void) {
    static const char *DEFAULT_KEY = "696F74676476312E3077306634743879";
    return DEFAULT_KEY;
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
