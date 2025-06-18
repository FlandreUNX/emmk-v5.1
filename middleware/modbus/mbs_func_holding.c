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

#include "./mbs.h"

#if MBS_ENABLE && MBS_ENABLE_FUNC_HOLDING_REG == 1

/**
 * @addtogroup Private variables
 * @note none
 */

/*@{*/

static void _onReadHoldingHandle(MBS_t *mbs);
MBS_FUNC_DEFINE(ReadHolding, MODBUS_FUNC_READ_HOLDING_REGISTER, _onReadHoldingHandle);

static void _onWriteOneHoldingHandle(MBS_t *mbs);
MBS_FUNC_DEFINE(WriteOneHolding, MODBUS_FUNC_WRITE_REGISTER, _onWriteOneHoldingHandle);

static void _onWriteMultiHoldingHandle(MBS_t *mbs);
MBS_FUNC_DEFINE(WriteMultiHolding, MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS, _onWriteMultiHoldingHandle);

static void onReadWriteHoldingHandler(MBS_t *mbs);
MBS_FUNC_DEFINE(ReadWriteMultiHolding, MODBUS_FUNC_READWRITE_MULTIPLE_REGISTERS, onReadWriteHoldingHandler);

/*@}*/

/**
 * @addtogroup Private func
 * @note none
 */

/*@{*/

#define READ_HOLDING_REQUEST_ERROR_FUNC_OFFSET         (0)
#define READ_HOLDING_REQUEST_ERROR_EXCEPTION_OFFSET    (1)

static void _onErrorResponse(MBS_t *mbs, uint8_t code, MB_Exception_t exception) {
    ////
    //// Response
    //// Response PDU = errorCode + exceptionCode
    ////
    mbs->txPayloadCursor[READ_HOLDING_REQUEST_ERROR_FUNC_OFFSET] = code;
    mbs->txPayloadCursor[READ_HOLDING_REQUEST_ERROR_EXCEPTION_OFFSET] = exception;

    ////
    //// Response PDU length = errorCode + exceptionCode
    ////
    mbs->txPayloadLength = 2;
}

/*@}*/

/**
 * @addtogroup Read holding
 * @note none
 */

/*@{*/

#define READ_HOLDING_REQUEST_FUNC_OFFSET    (0)
#define READ_HOLDING_REQUEST_ADDR_OFFSET    (1)
#define READ_HOLDING_REQUEST_COUNT_OFFSET   (3)

#define READ_HOLDING_RESPONSE_FUNC_OFFSET    (0)
#define READ_HOLDING_RESPONSE_COUNT_OFFSET   (1)
#define READ_HOLDING_RESPONSE_DATA_OFFSET    (2)

#define READ_HOLDING_RESPONSE_ERROR_CODE     0x83

static void _onReadHoldingHandle(MBS_t *mbs) {
    uint8_t *ptr;
    uint16_t address;
    uint16_t registerCount;

    address = ((uint16_t) mbs->rxPayloadCursor[READ_HOLDING_REQUEST_ADDR_OFFSET]) << 8;
    address |= ((uint16_t) mbs->rxPayloadCursor[READ_HOLDING_REQUEST_ADDR_OFFSET + 1]) & 0x00FF;

    registerCount = ((uint16_t) mbs->rxPayloadCursor[READ_HOLDING_REQUEST_COUNT_OFFSET]) << 8;
    registerCount |= ((uint16_t) mbs->rxPayloadCursor[READ_HOLDING_REQUEST_COUNT_OFFSET + 1]) & 0x00FF;

    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        if (mbs->regMap.holdRegMapSize < registerCount) {
            _onErrorResponse(mbs, READ_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }
    }

    mbs->txPayloadCursor[READ_HOLDING_RESPONSE_FUNC_OFFSET] = MODBUS_FUNC_READ_HOLDING_REGISTER;
    mbs->txPayloadCursor[READ_HOLDING_RESPONSE_COUNT_OFFSET] = registerCount * 2;
    mbs->txPayloadLength = 1 + 1 + registerCount * 2;
    if (mbs->txPayloadLength + MODBUS_SL_PDU_SIZE_MIN > mbs->txBufferSize) {
        _onErrorResponse(mbs, READ_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

    ptr = &mbs->txPayloadCursor[READ_HOLDING_RESPONSE_DATA_OFFSET];

    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        for (uint16_t i = 0; i < registerCount; i++) {
            MBS_Register_t *handler = _mbs_findHandlerByTypeAndAddress(MBS_REGSITER_TYPE_HOLDING, address + i);
            if (handler == NULL) {
                _onErrorResponse(mbs, READ_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }

            *ptr++ = *handler->data >> 8;
            *ptr++ = *handler->data & 0x00FF;

            if (handler->va->onRequested != NULL) {
                handler->va->onRequested(MBS_REGSITER_HANDLER_REQUEST_TYPE_HOST_READ, handler);
            }
        }
    } else if (mbs->sync.flag.mode == MBS_MODE_WORK) {
        if (mbs->callback == NULL || mbs->callback->onDirectOperate == NULL) {
            _onErrorResponse(mbs, READ_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }

        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[READ_HOLDING_REQUEST_FUNC_OFFSET],
                                       MB_DIRECT_IO_READ_START,
                                       address, registerCount, NULL);

        uint16_t regData;
        for (uint16_t i = 0; i < registerCount; i++) {
            int32_t rc = mbs->callback->onDirectOperate(mbs,
                                                        mbs->address,
                                                        mbs->rxPayloadCursor[READ_HOLDING_REQUEST_FUNC_OFFSET],
                                                        MB_DIRECT_IO_READ,
                                                        address, i,
                                                        &regData);
            if (rc != 0) {
                _onErrorResponse(mbs, READ_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }
            *ptr++ = regData >> 8;
            *ptr++ = regData & 0x00FF;
        }

        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[READ_HOLDING_REQUEST_FUNC_OFFSET],
                                       MB_DIRECT_IO_READ_END,
                                       address, registerCount, NULL);
    }
}

/*@}*/

/**
 * @addtogroup Write one holding
 * @note none
 */

/*@{*/

#define WRITE_ONE_HOLDING_REQUEST_FUNC_OFFSET    (0)
#define WRITE_ONE_HOLDING_REQUEST_ADDR_OFFSET    (1)
#define WRITE_ONE_HOLDING_REQUEST_DATA_OFFSET    (3)

#define WRITE_ONE_HOLDING_RESPONSE_FUNC_OFFSET    (0)
#define WRITE_ONE_HOLDING_RESPONSE_ADDR_OFFSET    (1)
#define WRITE_ONE_HOLDING_RESPONSE_DATA_OFFSET    (3)

#define WRITE_ONE_HOLDING_RESPONSE_ERROR_CODE     0x86

static void _onWriteOneHoldingHandle(MBS_t *mbs) {
    uint16_t address;
    uint16_t dataToWrite;

    address = ((uint16_t) mbs->rxPayloadCursor[WRITE_ONE_HOLDING_REQUEST_ADDR_OFFSET]) << 8;
    address |= ((uint16_t) mbs->rxPayloadCursor[WRITE_ONE_HOLDING_REQUEST_ADDR_OFFSET + 1]) & 0x00FF;

    dataToWrite = ((uint16_t) mbs->rxPayloadCursor[WRITE_ONE_HOLDING_REQUEST_DATA_OFFSET]) << 8;
    dataToWrite |= ((uint16_t) mbs->rxPayloadCursor[WRITE_ONE_HOLDING_REQUEST_DATA_OFFSET + 1]) & 0x00FF;

    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        MBS_Register_t *handler = _mbs_findHandlerByTypeAndAddress(MBS_REGSITER_TYPE_HOLDING, address);
        if (handler == NULL) {
            _onErrorResponse(mbs, WRITE_ONE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }

        if (handler->va->onWriteAuth != NULL) {
            if (handler->va->onWriteAuth(handler, &dataToWrite) == 0) {
                _onErrorResponse(mbs, WRITE_ONE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_VALUE);
                return;
            }
        }

        if (handler->va->onValueCheckAuth != NULL) {
            if (handler->va->onValueCheckAuth(handler, &dataToWrite) == 0) {
                _onErrorResponse(mbs, WRITE_ONE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_VALUE);
                return;
            }
        }

        *handler->data = dataToWrite;

        if (handler->va->onRequested != NULL) {
            handler->va->onRequested(MBS_REGSITER_HANDLER_REQUEST_TYPE_HOST_WRITE, handler);
        }
    } else if (mbs->sync.flag.mode == MBS_MODE_WORK) {
        if (mbs->callback == NULL || mbs->callback->onDirectOperate == NULL) {
            _onErrorResponse(mbs, WRITE_ONE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }

        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[WRITE_ONE_HOLDING_REQUEST_FUNC_OFFSET],
                                       MB_DIRECT_IO_WRITE_START,
                                       address, 1, NULL);

        int32_t rc = mbs->callback->onDirectOperate(mbs,
                                                    mbs->address,
                                                    mbs->rxPayloadCursor[WRITE_ONE_HOLDING_REQUEST_FUNC_OFFSET],
                                                    MB_DIRECT_IO_WRITE,
                                                    address, 0,
                                                    &dataToWrite);

        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[WRITE_ONE_HOLDING_REQUEST_FUNC_OFFSET],
                                       MB_DIRECT_IO_WRITE_END,
                                       address, 1, NULL);
        if (rc != 0) {
            _onErrorResponse(mbs, WRITE_ONE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }
    }

    mbs->txPayloadCursor[WRITE_ONE_HOLDING_RESPONSE_FUNC_OFFSET] = MODBUS_FUNC_WRITE_REGISTER;
    mbs->txPayloadCursor[WRITE_ONE_HOLDING_RESPONSE_ADDR_OFFSET] = mbs->rxPayloadCursor[
        WRITE_ONE_HOLDING_REQUEST_ADDR_OFFSET];
    mbs->txPayloadCursor[WRITE_ONE_HOLDING_RESPONSE_ADDR_OFFSET + 1] = mbs->rxPayloadCursor[
        WRITE_ONE_HOLDING_REQUEST_ADDR_OFFSET + 1];
    mbs->txPayloadCursor[WRITE_ONE_HOLDING_REQUEST_DATA_OFFSET] = mbs->rxPayloadCursor[
        WRITE_ONE_HOLDING_REQUEST_DATA_OFFSET];
    mbs->txPayloadCursor[WRITE_ONE_HOLDING_REQUEST_DATA_OFFSET + 1] = mbs->rxPayloadCursor[
        WRITE_ONE_HOLDING_REQUEST_DATA_OFFSET + 1];
    mbs->txPayloadLength = 1 + 2 + 2;
}

/*@}*/

/**
 * @addtogroup Write multi holding
 * @note none
 */

/*@{*/

#define WRITE_MULTI_HOLDING_REQUEST_FUNC_OFFSET          (0)
#define WRITE_MULTI_HOLDING_REQUEST_START_ADDR_OFFSET    (1)
#define WRITE_MULTI_HOLDING_REQUEST_COUNT_OFFSET         (3)
#define WRITE_MULTI_HOLDING_REQUEST_BYTE_COUNT_OFFSET    (5)
#define WRITE_MULTI_HOLDING_REQUEST_DATA_OFFSET          (6)

#define WRITE_MULTI_HOLDING_RESPONSE_FUNC_OFFSET         (0)
#define WRITE_MULTI_HOLDING_RESPONSE_ADDR_OFFSET         (1)
#define WRITE_MULTI_HOLDING_RESPONSE_COUNT_OFFSET        (3)

#define WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE       0x80

static void _onWriteMultiHoldingHandle(MBS_t *mbs) {
    uint8_t *rxPtr;
    uint16_t rxData;
    uint16_t startAddress;
    uint16_t regCount;
    uint8_t byteCount;

    startAddress = ((uint16_t) mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_START_ADDR_OFFSET]) << 8;
    startAddress |= ((uint16_t) mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_START_ADDR_OFFSET + 1]) & 0x00FF;

    regCount = ((uint16_t) mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_COUNT_OFFSET]) << 8;
    regCount |= ((uint16_t) mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_COUNT_OFFSET + 1]) & 0x00FF;

    byteCount = mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_BYTE_COUNT_OFFSET];

    rxPtr = &mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_DATA_OFFSET];

    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        if (mbs->regMap.holdRegMapSize < regCount) {
            _onErrorResponse(mbs, WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }

        for (uint16_t i = 0; i < regCount; i++) {
            MBS_Register_t *handler = _mbs_findHandlerByTypeAndAddress(MBS_REGSITER_TYPE_HOLDING, startAddress + i);
            if (handler == NULL) {
                _onErrorResponse(mbs, WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }

            rxData = ((uint16_t) rxPtr[i * 2]) << 8 | (((uint16_t) rxPtr[i * 2 + 1]) & 0x00FF);

            if (handler->va->onWriteAuth != NULL) {
                if (handler->va->onWriteAuth(handler, &rxData) == 0) {
                    _onErrorResponse(mbs, WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                    return;
                }
            }

            if (handler->va->onValueCheckAuth != NULL) {
                if (handler->va->onValueCheckAuth(handler, &rxData) == 0) {
                    _onErrorResponse(mbs, WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_VALUE);
                    return;
                }
            }

            *handler->data = rxData;

            if (handler->va->onRequested != NULL) {
                handler->va->onRequested(MBS_REGSITER_HANDLER_REQUEST_TYPE_HOST_WRITE, handler);
            }
        }
    } else if (mbs->sync.flag.mode == MBS_MODE_WORK) {
        if (mbs->callback == NULL || mbs->callback->onDirectOperate == NULL) {
            _onErrorResponse(mbs, WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }

        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_FUNC_OFFSET],
                                       MB_DIRECT_IO_WRITE_START,
                                       startAddress, regCount, NULL);

        for (uint16_t i = 0; i < regCount; i++) {
            rxData = ((uint16_t) rxPtr[i * 2]) << 8 | (((uint16_t) rxPtr[i * 2 + 1]) & 0x00FF);

            int32_t rc = mbs->callback->onDirectOperate(mbs,
                                                        mbs->address,
                                                        mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_FUNC_OFFSET],
                                                        MB_DIRECT_IO_WRITE,
                                                        startAddress, i,
                                                        &rxData);
            if (rc != 0) {
                _onErrorResponse(mbs, WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }
        }

        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[WRITE_MULTI_HOLDING_REQUEST_FUNC_OFFSET],
                                       MB_DIRECT_IO_WRITE_END,
                                       startAddress, regCount, NULL);
    }

    mbs->txPayloadCursor[WRITE_MULTI_HOLDING_RESPONSE_FUNC_OFFSET] = MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS;
    mbs->txPayloadCursor[WRITE_MULTI_HOLDING_RESPONSE_ADDR_OFFSET] = mbs->rxPayloadCursor[
        WRITE_MULTI_HOLDING_REQUEST_START_ADDR_OFFSET];
    mbs->txPayloadCursor[WRITE_MULTI_HOLDING_RESPONSE_ADDR_OFFSET + 1] = mbs->rxPayloadCursor[
        WRITE_MULTI_HOLDING_REQUEST_START_ADDR_OFFSET + 1];
    mbs->txPayloadCursor[WRITE_MULTI_HOLDING_RESPONSE_COUNT_OFFSET] = mbs->rxPayloadCursor[
        WRITE_MULTI_HOLDING_REQUEST_COUNT_OFFSET];
    mbs->txPayloadCursor[WRITE_MULTI_HOLDING_RESPONSE_COUNT_OFFSET + 1] = mbs->rxPayloadCursor[
        WRITE_MULTI_HOLDING_REQUEST_COUNT_OFFSET + 1];
    mbs->txPayloadLength = 1 + 2 + 2;
}

/*@}*/

/**
 * @addtogroup Read Write
 * @note none
 */

/*@{*/

#define READ_WRITE_HOLDING_REQUEST_FUNC_OFFSET          (0)

#define READ_WRITE_HOLDING_REQUEST_WRITE_START_ADDR_OFFSET    (5)
#define READ_WRITE_HOLDING_REQUEST_WRITE_COUNT_OFFSET         (7)
#define READ_WRITE_HOLDING_REQUEST_WRITE_BYTE_COUNT_OFFSET    (9)
#define READ_WRITE_HOLDING_REQUEST_WRITE_DATA_OFFSET          (10)

#define READ_WRITE_HOLDING_REQUEST_READ_START_ADDR_OFFSET    (1)
#define READ_WRITE_HOLDING_REQUEST_READ_COUNT_OFFSET         (3)

#define READ_WRITE_HOLDING_RESPONSE_FUNC_OFFSET    (0)
#define READ_WRITE_HOLDING_RESPONSE_COUNT_OFFSET   (1)
#define READ_WRITE_HOLDING_RESPONSE_DATA_OFFSET    (2)

#define READ_WRITE_HOLDING_RESPONSE_ERROR_CODE     0x83

static void onReadWriteHoldingHandler(MBS_t *mbs) {
    uint16_t data;
    uint16_t writeStartAddress = (((uint16_t) mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_WRITE_START_ADDR_OFFSET]) << 8)
                            | (((uint16_t) mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_WRITE_START_ADDR_OFFSET + 1])
                               & 0x00FF);
    uint16_t writeRegCount = (((uint16_t) mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_WRITE_COUNT_OFFSET]) << 8)
                        | (((uint16_t) mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_WRITE_COUNT_OFFSET + 1]) &
                           0x00FF);
    uint8_t writeByteCount = mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_WRITE_BYTE_COUNT_OFFSET];
    uint8_t *dataPtr = &mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_WRITE_DATA_OFFSET];
    uint16_t readStartAddress = (((uint16_t) mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_READ_START_ADDR_OFFSET]) << 8)
                            | (((uint16_t) mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_READ_START_ADDR_OFFSET + 1])
                               & 0x00FF);
    uint16_t readRegCount = (((uint16_t) mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_READ_COUNT_OFFSET]) << 8)
                        | (((uint16_t) mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_READ_COUNT_OFFSET + 1]) &
                           0x00FF);

    /// Write
    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        if (mbs->regMap.holdRegMapSize < writeRegCount) {
            _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }
        for (uint16_t i = 0; i < writeRegCount; i++) {
            MBS_Register_t *handler = _mbs_findHandlerByTypeAndAddress(MBS_REGSITER_TYPE_HOLDING, writeStartAddress + i);
            if (handler == NULL) {
                _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }
            data = ((uint16_t) dataPtr[i * 2]) << 8 | (((uint16_t) dataPtr[i * 2 + 1]) & 0x00FF);
            if (handler->va->onWriteAuth != NULL) {
                if (handler->va->onWriteAuth(handler, &data) == 0) {
                    _onErrorResponse(mbs, WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                    return;
                }
            }
            if (handler->va->onValueCheckAuth != NULL) {
                if (handler->va->onValueCheckAuth(handler, &data) == 0) {
                    _onErrorResponse(mbs, WRITE_MULTI_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_VALUE);
                    return;
                }
            }
            *handler->data = data;
            if (handler->va->onRequested != NULL) {
                handler->va->onRequested(MBS_REGSITER_HANDLER_REQUEST_TYPE_HOST_WRITE, handler);
            }
        }
    } else if (mbs->sync.flag.mode == MBS_MODE_WORK) {
        if (mbs->callback == NULL || mbs->callback->onDirectOperate == NULL) {
            _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }
        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_FUNC_OFFSET],
                                       MB_DIRECT_IO_WRITE_START,
                                       writeStartAddress, writeRegCount, NULL);
        for (uint16_t i = 0; i < writeRegCount; i++) {
            data = ((uint16_t) dataPtr[i * 2]) << 8 | (((uint16_t) dataPtr[i * 2 + 1]) & 0x00FF);
            int32_t rc = mbs->callback->onDirectOperate(mbs,
                                                        mbs->address,
                                                        mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_FUNC_OFFSET],
                                                        MB_DIRECT_IO_WRITE,
                                                        writeStartAddress, i,
                                                        &data);
            if (rc != 0) {
                _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }
        }
        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[READ_WRITE_HOLDING_REQUEST_FUNC_OFFSET],
                                       MB_DIRECT_IO_WRITE_END,
                                       writeStartAddress, writeRegCount, NULL);
    }

    /// Read
    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        if (mbs->regMap.holdRegMapSize < readRegCount) {
            _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }
    }

    mbs->txPayloadCursor[READ_WRITE_HOLDING_RESPONSE_FUNC_OFFSET] = MODBUS_FUNC_READWRITE_MULTIPLE_REGISTERS;
    mbs->txPayloadCursor[READ_WRITE_HOLDING_RESPONSE_COUNT_OFFSET] = readRegCount * 2;
    mbs->txPayloadLength = 1 + 1 + readRegCount * 2;
    if (mbs->txPayloadLength + MODBUS_SL_PDU_SIZE_MIN > mbs->txBufferSize) {
        _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }

    dataPtr = &mbs->txPayloadCursor[READ_WRITE_HOLDING_RESPONSE_DATA_OFFSET];

    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        for (uint16_t i = 0; i < readRegCount; i++) {
            MBS_Register_t *handler = _mbs_findHandlerByTypeAndAddress(MBS_REGSITER_TYPE_HOLDING, readStartAddress + i);
            if (handler == NULL) {
                _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }

            *dataPtr++ = *handler->data >> 8;
            *dataPtr++ = *handler->data & 0x00FF;

            if (handler->va->onRequested != NULL) {
                handler->va->onRequested(MBS_REGSITER_HANDLER_REQUEST_TYPE_HOST_READ, handler);
            }
        }
    } else if (mbs->sync.flag.mode == MBS_MODE_WORK) {
        if (mbs->callback == NULL || mbs->callback->onDirectOperate == NULL) {
            _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }

        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[READ_WRITE_HOLDING_RESPONSE_FUNC_OFFSET],
                                       MB_DIRECT_IO_READ_START,
                                       readStartAddress, readRegCount, NULL);

        uint16_t regData;
        for (uint16_t i = 0; i < readStartAddress; i++) {
            const int32_t rc = mbs->callback->onDirectOperate(mbs,
                                                        mbs->address,
                                                        mbs->rxPayloadCursor[READ_WRITE_HOLDING_RESPONSE_FUNC_OFFSET],
                                                        MB_DIRECT_IO_READ,
                                                        readStartAddress, i,
                                                        &regData);
            if (rc != 0) {
                _onErrorResponse(mbs, READ_WRITE_HOLDING_RESPONSE_ERROR_CODE, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }
            *dataPtr++ = regData >> 8;
            *dataPtr++ = regData & 0x00FF;
        }

        mbs->callback->onDirectOperate(mbs,
                                       mbs->address, mbs->rxPayloadCursor[READ_WRITE_HOLDING_RESPONSE_FUNC_OFFSET],
                                       MB_DIRECT_IO_READ_END,
                                       readStartAddress, readRegCount, NULL);
    }
}

/*@}*/


#endif
