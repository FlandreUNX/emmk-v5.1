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

#if CONFIG_CMODULE_PROTOCOL_IS_CAAEP == 1

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "cModule-pt-caaep"

#define CMASSERT(err) ASSERT(err)

/*@}*/

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
 * @addtogroup ProtocolPrivate
 * @note none
 */

/*@{*/

static int32_t protocolSetupTransmitPack(cModule_Instance_t *ins, uint8_t *data, uint16_t dataLength, uint32_t waitConfirmSec, uint32_t messageId) {
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    _cModule_packMutexLock(ins, true);
    
    cModule_ProtocolGenMessage_t *msg = calloc(1, sizeof(cModule_ProtocolGenMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) data;
    msg->payloadLength = dataLength;
    
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolGenMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynData = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = 0;
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


static void protocolResponseNoArg(cModule_Instance_t *ins, uint16_t mid, uint8_t cmd, int8_t result) {
    uint8_t *sendBuffer = calloc(1, 8);
    if (sendBuffer == NULL) {
        return;
    }
    
    sendBuffer[0] = 0x02;
    sendBuffer[1] = (mid >> 8) & 0x00FF;
    sendBuffer[2] = (mid) & 0x00FF;
    sendBuffer[3] = 0x00;
    sendBuffer[4] = 0x00;
    sendBuffer[5] = 0x00;
    sendBuffer[6] = cmd;
    sendBuffer[7] = result;
    
    protocolSetupTransmitPack(ins, sendBuffer, 8, 0, ++(*_cModule_getGenMsgId()));
}


static void protocolResponseArg(cModule_Instance_t *ins, uint16_t mid, uint8_t cmd, int8_t result, uint16_t arg) {
    uint8_t *sendBuffer = calloc(1, 8);
    if (sendBuffer == NULL) {
        return;
    }
    
    sendBuffer[0] = 0x02;
    sendBuffer[1] = (mid >> 8) & 0x00FF;
    sendBuffer[2] = (mid) & 0x00FF;
    sendBuffer[3] = 0x00;
    sendBuffer[4] = (arg >> 8) & 0x00FF;
    sendBuffer[5] = (arg) & 0x00FF;
    sendBuffer[6] = cmd;
    sendBuffer[7] = result;
    
    protocolSetupTransmitPack(ins, sendBuffer, 8, 0, ++*_cModule_getGenMsgId());
}


static int32_t ptJwReqestProcess(cModule_Instance_t *ins, uint8_t *data, uint16_t dataLength) {
    if (data[0] != 0x01) {
        return -1;
    }
    
    uint16_t mid = (((uint16_t) data[1]) << 8) | (((uint16_t) data[2]));
    uint16_t args = (((uint16_t) data[3]) << 8) | (((uint16_t) data[4]));
    uint8_t cmd = data[5];
    switch (cmd) {
        /// @SetZero
        case 0x07: {
            // int8_t result = CREQUEST(PT_CAAEP_ON_SET_ZERO, {}).i32;
            // protocolResponseNoArg(mid, cmd, result);
        } break;
        
        /// @SetSpan
        case 0x08: {
            // int8_t result = CREQUEST(PT_CAAEP_ON_SET_SPAN, {.u32 = args & 0x00FF}).i32;
            // protocolResponseNoArg(mid, cmd, result);
        } break;
    }
    
    return 0;
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_protocolTransmit(cModule_Instance_t *ins, ProtocolCAAEP_t *protocol) {
    if (ins->state.currentState >= CMODULE_STATE_FAILED_RES) {
        return -1;
    } else if (ins->state.currentState < CMODULE_STATE_DISCON) {
        return 1;
    }
    
    int32_t rc = -1;
    uint8_t *bufferHex = calloc(1, CAAEP_PAYLOAD_LENGTH + 1);
    if (bufferHex == NULL) {
        return -1;
    }
    
    bufferHex[CAAEP_PAYLOAD_PREFIX_POS] = 0x00;
    memcpy(&bufferHex[CAAEP_PAYLOAD_IMEI_POS], cModule_getModuleUId(ins), 15);
    memcpy(&bufferHex[CAAEP_PAYLOAD_IMSI_POS], cModule_getModuleComId(ins), 15);
    
    bufferHex[CAAEP_PAYLOAD_SEQ_POS] = (uint8_t) ((*_cModule_getGenMsgId() >> 8) & 0x00FF);
    bufferHex[CAAEP_PAYLOAD_SEQ_POS + 1] = (uint8_t) (*_cModule_getGenMsgId() & 0x00FF);
    (*_cModule_getGenMsgId())++;
    {
        int16_t dbm = cModule_getRssi(ins);
        bufferHex[CAAEP_PAYLOAD_RSSI_POS] = (uint8_t) ((dbm >> 8) & 0x00FF);
        bufferHex[CAAEP_PAYLOAD_RSSI_POS + 1] = (uint8_t) (dbm & 0x00FF);
    }
    bufferHex[CAAEP_PAYLOAD_BAT_POS] = protocol->system.battery;
    bufferHex[CAAEP_PAYLOAD_DEV_TYPE_POS] = 0x00;
    bufferHex[CAAEP_PAYLOAD_DEV_TYPE_POS + 1] = 0x01;
    bufferHex[CAAEP_PAYLOAD_CMD_POS] = protocol->gas.state;
    bufferHex[CAAEP_PAYLOAD_LEL_POS] = (protocol->gas.value) >> 8;
    bufferHex[CAAEP_PAYLOAD_LEL_POS + 1] = (protocol->gas.value) & 0x00FF;
    bufferHex[CAAEP_PAYLOAD_MINDEX1_POS] = protocol->reserve.r1;
    bufferHex[CAAEP_PAYLOAD_MINDEX2_POS] = protocol->reserve.r2;
    
    if (protocolSetupTransmitPack(ins, bufferHex, CAAEP_PAYLOAD_LENGTH, 0, *_cModule_getGenMsgId()) != 0) {
        free(bufferHex);
        return -1;
    }
    return 0;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

int32_t _cModule_onProtocolTransmited(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return 0;
}


int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return ptJwReqestProcess(ins, info->payload, info->payloadLength);
}

/*@}*/

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME

/*@}*/

#endif