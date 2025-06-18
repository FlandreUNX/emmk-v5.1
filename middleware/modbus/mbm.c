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

#include "./mbm.h"

#if MBM_ENABLE == 1

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef DBG_SECTION_NAME
#undef TAG
#define DBG_SECTION_NAME  "MBM"
#define TAG DBG_SECTION_NAME

/*@}*/

/**
 * @addtogroup variables
 * @note none
 */

/*@{*/

static void threadMbmProcesser(void *arg);

/*@}*/

/**
 * @addtogroup private get/set functions
 * @note none
 */

/*@{*/

inline void __mbm_setDest(MBM_t *d, uint8_t dest) {
    d->destAddr = dest;
}


inline uint8_t *__mbm_getTxPDU(MBM_t *d) {
    return &d->txBuf[MODBUS_SL_PDU_PDU_OFFSET];
}


inline void __mbm_setTxPDULen(MBM_t *d, uint8_t len) {
    d->txPDULen = len;
}


inline uint8_t *__mbm_getRxPDU(MBM_t *d) {
    return d->rxPDUFrameCur;
}

/*@}*/

/**
 * @addtogroup private functions
 * @note none
 */

/*@{*/

static MBM_Error_t assemblyRTU(MBM_t *instance, MBM_RequestHandler_t *req) {
    MBM_FUNC_SECTION_EXTEND;
    for (MBM_FuncHandler_t *func = (MBM_FuncHandler_t *) MBM_FUNC_SECTION_START;
         func != (MBM_FuncHandler_t *) MBM_FUNC_SECTION_END;
         func++) {
        if (req->opts.funcCode == 0xFF) {
            continue;
        }

        if (req->opts.funcCode == func->funcCode) {
            return func->preHandler(instance, req);
        }
    }

    // No func found, error request
    return MBM_ERR_ILLEGAL_REQUEST;
}


static MBM_Error_t sendFrame(MBM_t *instance, bool sendAccess) {
    instance->txBufCur = &instance->txBuf[MODBUS_SL_PDU_ADDR_OFFSET];

    //
    // frame len = dest_address(1) + PDU(n) + CRC16(2)
    //

    // Address, total data length = 0 + 1
    instance->txBufCur[MODBUS_SL_PDU_ADDR_OFFSET] = instance->destAddr;
    instance->txDataLen = 1;

    // PDU data length
    instance->txDataLen += instance->txPDULen;

    // CRC16, "little endian", [0] = crc16_l [1] = crc16_h
    uint16_t crc = klCRC16_modbus(instance->txBufCur, instance->txDataLen);
    instance->txBufCur[instance->txDataLen++] = (uint8_t) (crc & 0x00FF);
    instance->txBufCur[instance->txDataLen++] = (uint8_t) (crc >> 8);

    // Clear rx buf
    instance->rxPDUFrameCur = NULL;
    instance->rxDataLen = 0;
    instance->rxPDULen = 0;

    if (!sendAccess) {
        return MBM_ERR_OK;
    }

    // Start send
    return instance->interface->write(instance, instance->txBuf, instance->txDataLen, (uint32_t) -1) == instance->
           txDataLen
               ? MBM_ERR_OK
               : MBM_ERR_SENT;
}


static MBM_Error_t checkRcvFrame(MBM_t *instance) {
    if (instance->rxDataLen < MODBUS_SL_PDU_SIZE_MIN
        || klCRC16_modbus(instance->rxBuf, instance->rxDataLen) != 0) {
        return MBM_ERR_ILLEGAL_DATA;
    }

    instance->rcvAddr = instance->rxBuf[MODBUS_SL_PDU_ADDR_OFFSET];
    if (instance->rcvAddr != instance->destAddr) {
        return MBM_ERR_ILLEGAL_DATA;
    }

    instance->rxPDULen = instance->rxDataLen
                         - MODBUS_SL_PDU_PDU_OFFSET
                         - MODBUS_SL_PDU_CRC_SIZE;

    instance->rxPDUFrameCur = &instance->rxBuf[MODBUS_SL_PDU_PDU_OFFSET];

    return MBM_ERR_OK;
}


static MB_Exception_t handleFunc(MBM_t *instance, MBM_ResponseHandler_t *rep) {
    uint8_t funcCode = instance->rxPDUFrameCur[MODBUS_PDU_FUNC_OFFSET];

    // If receive frame has exception. The receive function code highest bit is 1
    if (funcCode >> 7) {
        // [0] = funcCode
        // [1] = exception_code
        return (MB_Exception_t) instance->rxPDUFrameCur[MODBUS_PDU_DATA_OFFSET];
    }

    MBM_FUNC_SECTION_EXTEND;
    MBM_FuncHandler_t *func = (MBM_FuncHandler_t *) MBM_FUNC_SECTION_START;
    for (; func != (MBM_FuncHandler_t *) MBM_FUNC_SECTION_END; func++) {
        if (func->funcCode == funcCode) {
            return func->lastHandler(instance, rep);
        }
    }

    return MB_EX_ILLEGAL_FUNCTION;
}

/*@}*/

/**
 * @addtogroup functions
 * @note none
 */

/*@{*/

void mbm_init(MBM_t *instance, MB_Interface_t *interface, void *userData) {
    ASSERT(instance != NULL);
    ASSERT(interface != NULL);

    instance->interface = interface;
    instance->userData = userData;
}


void mbm_finalize(MBM_t *instance) {
    ASSERT(instance != NULL);

    instance->interface = NULL;
}


MB_Exception_t mbm_input(MBM_t *instance, const uint8_t *data, uint16_t dataLength,
                      MBM_ResponseHandler_t *rep) {
    ASSERT(instance != NULL);
    ASSERT(rep != NULL);

    if (data == 0 || dataLength == 0) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }
    memcpy(instance->rxBuf, data, dataLength);
    instance->rxDataLen = dataLength;
    if (checkRcvFrame(instance) != MBM_ERR_OK) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }
    if (handleFunc(instance, rep) != MB_EX_NONE) {
        return MB_EX_ILLEGAL_DATA_VALUE;
    }
    rep->success = true;
    return MB_EX_NONE;
}


MBM_Error_t mbm_create(MBM_t *instance, MBM_RequestHandler_t *req, uint8_t *data, uint16_t *dataLength) {
    ASSERT(req != NULL);

    MBM_Error_t err = assemblyRTU(instance, req);
    if (err != MBM_ERR_OK) {
        return err;
    }
    err = sendFrame(instance, false);
    if (data != NULL) {
        memcpy(data, instance->txBuf, instance->txDataLen);
    }
    if (dataLength != NULL) {
        *dataLength = instance->txDataLen;
    }
    return err;
}


MBM_Error_t mbm_request(MBM_t *instance,
                        MBM_RequestHandler_t *req, MBM_ResponseHandler_t *rep,
                        uint32_t timeout,
                        uint8_t isSyncRecv) {
    ASSERT(instance != NULL);
    ASSERT(req != NULL);
    ASSERT(rep != NULL);

    MBM_Error_t err;

    if (isSyncRecv) {
        if (!instance->sync.flag.isWaiting) {
            return MBM_ERR_ILLEGAL_REQUEST;
        }

        instance->rxDataLen = instance->interface->read(instance, instance->rxBuf, MODBUS_SL_PDU_SIZE_MAX, 0);
        if (instance->rxDataLen <= 0) {
            if (qSTimer_Expired(&instance->sync.tmcd)) {
                err = MBM_ERR_RESPONSE_TIMEOUT;

                goto _l_sync_exit;
            } else {
                return MBM_ERR_OK;
            }
        }

        instance->interface->afterRead(instance);

        // Process receive frame
        err = checkRcvFrame(instance);
        if (err != MBM_ERR_OK) {
            goto _l_sync_exit;
        }

        // Func handle
        rep->excep = handleFunc(instance, rep);

        err = MBM_ERR_OK;

    _l_sync_exit:
        instance->sync.flag.isWaiting = 0;
        instance->sync.flag.isCompleted = 1;

        if (err != MBM_ERR_OK) {
            rep->success = 0;
            rep->err = err;
        } else {
            rep->success = 1;
            rep->err = MBM_ERR_OK;
        }

        instance->interface->close(instance);

        return err;
    } else if (timeout & 0x80000000) {
        instance->interface->open(instance, 0);

    _l_async_retry:
        instance->sync.flag.isWaiting = 1;
        instance->sync.flag.isCompleted = 0;

        instance->interface->flush(instance);

        // Combine pdu frame
        err = assemblyRTU(instance, req);
        if (err != MBM_ERR_OK) {
            goto _l_async_exit;
        }

        // Send frame
        instance->interface->beforeWrite(instance);
        err = sendFrame(instance, true);
        instance->interface->afterWrite(instance);
        if (err != MBM_ERR_OK) {
            goto _l_async_exit;
        }

        // Wait receive complete or reponse timeout
        instance->interface->beforeRead(instance);
        instance->rxDataLen = instance->interface->read(instance, instance->rxBuf, MODBUS_SL_PDU_SIZE_MAX,
                                                        timeout & 0x7FFFFFFF);
        instance->interface->afterRead(instance);
        if (instance->rxDataLen <= 0) {
            err = MBM_ERR_RESPONSE_TIMEOUT;
            goto _l_async_exit;
        }

        // Process receive frame
        err = checkRcvFrame(instance);
        if (err != MBM_ERR_OK) {
            goto _l_async_exit;
        }

        // Func handle
        rep->excep = handleFunc(instance, rep);

        err = MBM_ERR_OK;

    _l_async_exit:
        instance->sync.flag.isWaiting = 0;
        instance->sync.flag.isCompleted = 1;

        if (err != MBM_ERR_OK) {
            if (req->opts.retryCount != 0 && req->_.retry < req->opts.retryCount) {
                req->_.retry++;
                goto _l_async_retry;
            }

            req->_.retry = 0;
            rep->success = 0;
            rep->err = err;
        } else {
            rep->success = 1;
            rep->err = MBM_ERR_OK;
        }

        instance->interface->close(instance);

        return err;
    } else {
        instance->sync.flag.isWaiting = 1;
        instance->sync.flag.isCompleted = 0;

        instance->interface->open(instance, 0);

    _l_syncWrite_retry:
        instance->interface->flush(instance);

        // Combine pdu frame
        err = assemblyRTU(instance, req);
        if (err != MBM_ERR_OK) {
            goto l_syncWrite_exit;
        }

        // Send frame
        instance->interface->beforeWrite(instance);
        err = sendFrame(instance, true);
        instance->interface->afterWrite(instance);
        if (err != MBM_ERR_OK) {
            goto l_syncWrite_exit;
        }

        qSTimer_Set(&instance->sync.tmcd, timeout);

        // Wait receive complete or reponse timeout
        instance->interface->beforeRead(instance);
        return MBM_ERR_OK;

    l_syncWrite_exit:
        instance->sync.flag.isWaiting = 0;
        instance->sync.flag.isCompleted = 1;

        if (err != MBM_ERR_OK) {
            if (req->opts.retryCount != 0 && req->_.retry < req->opts.retryCount) {
                req->_.retry++;
                goto _l_syncWrite_retry;
            }

            req->_.retry = 0;
            rep->success = 0;
            rep->err = err;
        } else {
            rep->success = 1;
            rep->err = MBM_ERR_OK;
        }

        instance->interface->close(instance);

        return err;
    }
}


uint8_t mbm_isBusy(MBM_t *instance) {
    return instance->sync.flag.isWaiting;
}


uint8_t mbm_isOperateCompleted(MBM_t *instance) {
    return instance->sync.flag.isCompleted;
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

#endif
