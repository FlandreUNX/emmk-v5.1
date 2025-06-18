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

#if MBS_ENABLE && MBS_ENABLE_FUNC_INPUT_REG == 1

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define READ_INPUT_REQUEST_FUNC_OFFSET          (0)
#define READ_INPUT_REQUEST_ADDR_OFFSET          (1)
#define READ_INPUT_REQUEST_REG_COUNT_OFFSET     (3)

#define READ_INPUT_RESPONSE_FUNC_CODE_OFFSET    (0)
#define READ_INPUT_RESPONSE_DATA_LENGTH_OFFSET  (1)
#define READ_INPUT_RESPONSE_DATA_OFFSET         (2)

#define READ_INPUT_RESPONSE_ERROR_CODE          (0x84)

/*@}*/

/**
 * @addtogroup Private variables
 * @note none
 */
 
/*@{*/

static void _onHandle(MBS_t *mbs);
MBS_FUNC_DEFINE(ReadInput, MODBUS_FUNC_READ_INPUT_REGISTER, _onHandle);

/*@}*/

/**
 * @addtogroup Private func
 * @note none
 */
 
/*@{*/

static void _onErrorResponse(MBS_t *mbs, MB_Exception_t exception) {
    ////
    //// Response
    //// Response PDU = errorCode + exceptionCode
    ////
    mbs->txPayloadCursor[READ_INPUT_RESPONSE_FUNC_CODE_OFFSET] = READ_INPUT_RESPONSE_ERROR_CODE;
    mbs->txPayloadCursor[READ_INPUT_RESPONSE_DATA_LENGTH_OFFSET] = exception;

    ////
    //// Response PDU length = errorCode + exceptionCode
    ////
    mbs->txPayloadLength = 2;
}


static void _onHandle(MBS_t *mbs) {
    uint8_t *ptr;
    uint16_t address;
    uint16_t registerCount;
    
    MBS_Register_t *handler = NULL;
    
    address = ((uint16_t) mbs->rxPayloadCursor[READ_INPUT_REQUEST_ADDR_OFFSET]) << 8;
    address |= ((uint16_t) mbs->rxPayloadCursor[READ_INPUT_REQUEST_ADDR_OFFSET + 1]) & 0x00FF;
    
    registerCount = ((uint16_t) mbs->rxPayloadCursor[READ_INPUT_REQUEST_REG_COUNT_OFFSET]) << 8;
    registerCount |= ((uint16_t) mbs->rxPayloadCursor[READ_INPUT_REQUEST_REG_COUNT_OFFSET + 1]) & 0x00FF;
    
    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        if (mbs->regMap.inputRegMapSize < registerCount) {
            _onErrorResponse(mbs, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }
    }
    
    mbs->txPayloadCursor[READ_INPUT_RESPONSE_FUNC_CODE_OFFSET] = MODBUS_FUNC_READ_INPUT_REGISTER;
    mbs->txPayloadCursor[READ_INPUT_RESPONSE_DATA_LENGTH_OFFSET] = registerCount * 2;
    mbs->txPayloadLength = 1 + 1 + registerCount * 2;
    if (mbs->txPayloadLength + MODBUS_SL_PDU_SIZE_MIN > mbs->txBufferSize) {
        _onErrorResponse(mbs, MB_EX_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    ptr = &mbs->txPayloadCursor[READ_INPUT_RESPONSE_DATA_OFFSET];
    
    if (mbs->sync.flag.mode == MBS_MODE_MAP) {
        for (uint16_t i = 0; i < registerCount; i++) {
            MBS_Register_t *handler = _mbs_findHandlerByTypeAndAddress(MBS_REGSITER_TYPE_INPUT, address + i);
            if (handler == NULL) {
                _onErrorResponse(mbs, MB_EX_ILLEGAL_DATA_ADDRESS);
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
            _onErrorResponse(mbs, MB_EX_ILLEGAL_DATA_ADDRESS);
            return;
        }
            
        uint16_t regData;
        for (uint16_t i = 0; i < registerCount; i++) {
            int32_t rc = mbs->callback->onDirectOperate(mbs, 
                mbs->address, mbs->rxPayloadCursor[READ_INPUT_REQUEST_FUNC_OFFSET], MB_DIRECT_IO_READ, 
                address, i,
                &regData);
            if (rc != 0) {
                _onErrorResponse(mbs, MB_EX_ILLEGAL_DATA_ADDRESS);
                return;
            }
            *ptr++ = regData >> 8;
            *ptr++ = regData & 0x00FF;
        }
    }
}

/*@}*/

#endif
