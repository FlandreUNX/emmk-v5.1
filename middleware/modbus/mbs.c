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

#include "./mbs.h"

#if MBS_ENABLE == 1

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#undef TAG
#define DBG_SECTION_NAME  "MBS"
#define TAG  DBG_SECTION_NAME

/*@}*/

/**
 * @addtogroup Protect func
 * @note none
 */
 
/*@{*/

MBS_Register_t* _mbs_findHandlerByTypeAndAddress(MBS_RegisterType_t type, uint16_t address) {
    MBS_REGISTER_SECTION_EXTEND;
    
    for (MBS_Register_t *reg = (MBS_Register_t *) MBS_REGISTER_SECTION_START;
            reg != (MBS_Register_t *) MBS_REGISTER_SECTION_END; 
            reg++) {
        if (reg->type != type) {
            continue;
        }
        
        switch (type) {
            case MBS_REGSITER_TYPE_INPUT:
            case MBS_REGSITER_TYPE_HOLDING: {
                if (address != reg->address) {
                    break;
                }
            } return reg;
            
            case MBS_REGSITER_TYPE_COILS:
                return NULL;
            
            default:
                return NULL;
        }
    }
            
    return NULL;
}

/*@}*/

/**
 * @addtogroup Private func
 * @note none
 */
 
/*@{*/

static MBS_Register_t* _findByAddress(uint16_t address) {
    MBS_REGISTER_SECTION_EXTEND;

    for (MBS_Register_t *reg = (MBS_Register_t *) MBS_REGISTER_SECTION_START;
            reg != (MBS_Register_t *) MBS_REGISTER_SECTION_END; 
            reg++) {
        if (reg->address != address) {
            continue;
        }
        
        return reg;
    }
            
    return NULL;
}


static MBS_Error_t _checkRequest(MBS_t *instance, bool skipCRC) {
    if (instance->rxBufferDataLength < MODBUS_SL_PDU_SIZE_MIN
            || (!skipCRC && klCRC16_modbus(instance->rxBuffer, instance->rxBufferDataLength) != 0)) {
        return MBS_ERR_RECV_CRC;
    }

    if (instance->sync.flag.mode == MBS_MODE_MAP) {
        if (instance->rxBuffer[MODBUS_SL_PDU_ADDR_OFFSET] != instance->address) {
            return MBS_ERR_RECV_DEV_ADDR;
        }
    } else if (instance->sync.flag.mode == MBS_MODE_WORK) {
        if (instance->callback->onDirectOperate(instance, instance->rxBuffer[MODBUS_SL_PDU_ADDR_OFFSET], 1, MB_DIRECT_IO_CHECK_ADDRESS, 0, 0, NULL) != 0) {
            return MBS_ERR_RECV_DEV_ADDR;
        }
        instance->address = instance->rxBuffer[MODBUS_SL_PDU_ADDR_OFFSET];
    }
    
    instance->rxPayloadLength = instance->rxBufferDataLength - MODBUS_SL_PDU_PDU_OFFSET - MODBUS_SL_PDU_CRC_SIZE;
    instance->rxPayloadCursor = &instance->rxBuffer[MODBUS_SL_PDU_PDU_OFFSET];

    return MBS_ERR_OK;
}


static MBS_Error_t _handleRequest(MBS_t *instance) {
    MBS_FUNC_SECTION_EXTEND;
    
    uint8_t funcCode;
    
    funcCode = instance->rxPayloadCursor[MODBUS_PDU_FUNC_OFFSET];
    if (funcCode >> 7) {
        return MBS_ERR_ILLEGAL_REQUEST;
    }

    ////
    //// Handle And Resopnse
    ////
    instance->txPayloadCursor = &instance->txBuffer[MODBUS_SL_PDU_PDU_OFFSET];
    instance->txPayloadLength = 0;
    
    for (MBS_FuncHandler_t *func = (MBS_FuncHandler_t *) MBS_FUNC_SECTION_START;
            func != (MBS_FuncHandler_t *) MBS_FUNC_SECTION_END; 
            func++) {
        if (func->funcCode != funcCode) {
            continue;
        }

        func->onHandle(instance);

        funcCode = 0;
        break;
    }
       
    ////    
    //// Clear rx buffer
    ////
    instance->rxBufferDataLength = 0;
    instance->rxPayloadCursor = NULL;
    instance->rxPayloadLength = 0;
            
    if (funcCode != 0) {
        return MBS_ERR_NOT_SUPPORT_FUNC_CODE;
    }

    return MBS_ERR_OK;
}


static MBS_Error_t _sendResponse(MBS_t *instance, bool skipCRC, bool skipWrite) {
    uint16_t crc;
    
    ////
    //// Addresses
    ////
    instance->txBuffer[MODBUS_SL_PDU_ADDR_OFFSET] = instance->address;
    instance->txBufferDataLength = 1;
    
    //// Payload[N]
    instance->txBufferDataLength += instance->txPayloadLength;

    if (!skipCRC) {
        //// CRC[2]-> [0] = L, [1] = H
        crc = klCRC16_modbus(instance->txBuffer, instance->txBufferDataLength);
        instance->txBuffer[instance->txBufferDataLength++] = (uint8_t) (crc & 0x00FF);
        instance->txBuffer[instance->txBufferDataLength++] = (uint8_t) (crc >> 8);
    }
    
    if (!skipWrite) {
        return instance->interface->write(instance, instance->txBuffer, instance->txBufferDataLength, (uint32_t) -1) == instance->txBufferDataLength
            ? MBS_ERR_OK 
            : MBS_ERR_SENT;
    } else {
        return MBS_ERR_OK;
    }
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

void mbs_init(MBS_t *instance, 
        MB_Interface_t *interface, 
        MB_Callback_t *callback,
        uint16_t *holdRegMap, uint16_t holdRegMapSize,
        uint16_t *inputRegMap, uint16_t inputRegMapSize,
        uint8_t devAddr, uint32_t baudrate) {
    ASSERT(instance != NULL);
    ASSERT(interface != NULL);

    instance->address = devAddr;
    instance->regMap.holdRegMap = holdRegMap;
    instance->regMap.holdRegMapSize = holdRegMapSize;
    instance->regMap.inputRegMap = inputRegMap;
    instance->regMap.inputRegMapSize = inputRegMapSize;
    instance->interface = interface;
    instance->interface->open(instance, baudrate);
    instance->interface->flush(instance);
    
    instance->interface->beforeRead(instance);
    instance->sync.flag._ = 0;
    instance->callback = callback;
}


void mbs_finalize(MBS_t *instance) {
    ASSERT(instance != NULL);
    instance->interface->flush(instance);
    instance->interface->close(instance);
}


void mbs_setMode(MBS_t *instance, MBS_Mode_t mode) {
    ASSERT(instance != NULL);
    
    instance->sync.flag.mode = mode;
}


int32_t mbs_updateDeviceAddress(MBS_t *instance, uint8_t newAddr) {
    ASSERT(instance != NULL);

    instance->address = newAddr;
    
    return 0;
}


int32_t mbs_updateBaudRate(MBS_t *instance, uint32_t baudRate) {
    ASSERT(instance != NULL);

    instance->interface->close(instance);
    instance->interface->open(instance, baudRate);
    
    return 0;
}


int32_t mbs_inputOutput(MBS_t *instance, uint8_t *pduData, uint16_t *pduLength, bool skipCRC, bool skipWrite) {
    ASSERT(instance != NULL);
    instance->rxBufferDataLength = *pduLength;
    if (instance->rxBufferDataLength <= 0) {
        if (instance->callback != NULL && instance->callback->onDecodeFailed != NULL) {
            instance->callback->onDecodeFailed(instance);
        }
        return -1;
    }
    memcpy(instance->rxBuffer, pduData, instance->rxBufferDataLength);
    
    if (_checkRequest(instance, skipCRC) != MBS_ERR_OK) {
        if (instance->callback != NULL && instance->callback->onDecodeFailed != NULL) {
            instance->callback->onDecodeFailed(instance);
        }
        return -1;
    }
    
    if (_handleRequest(instance) != MBS_ERR_OK) {
        if (instance->callback != NULL && instance->callback->onDecodeFailed != NULL) {
            instance->callback->onDecodeFailed(instance);
        }
        return -1;
    }
    
    if (_sendResponse(instance, skipCRC, skipWrite) != MBS_ERR_OK) {
        if (instance->callback != NULL && instance->callback->onDecodeFailed != NULL) {
            instance->callback->onDecodeFailed(instance);
        }
        return -1;
    }
    if (pduData != NULL) {
        memcpy(pduData, instance->txBuffer, instance->txBufferDataLength);
    }
    if (pduLength != NULL) {
        *pduLength = instance->txBufferDataLength;
    }
    if (instance->callback != NULL && instance->callback->onDecodeSuccess != NULL) {
        instance->callback->onDecodeSuccess(instance);
    }  
    return 0;
}


void mbs_loop(MBS_t *instance) {
    ASSERT(instance != NULL);
    
    if (!instance->sync.flag.isRecving) {
        instance->interface->beforeRead(instance);
        instance->sync.flag.isRecving = 1;
    }

    instance->rxBufferDataLength = instance->interface->read(instance, instance->rxBuffer, instance->rxBufferSize, 0);
    if (instance->rxBufferDataLength <= 0) {
        if (instance->callback != NULL && instance->callback->onDecodeFailed != NULL) {
            instance->callback->onDecodeFailed(instance);
        } 
        return;
    }
    
    instance->interface->afterRead(instance);
    
    if (_checkRequest(instance, false) != MBS_ERR_OK) {
        if (instance->callback != NULL && instance->callback->onDecodeFailed != NULL) {
            instance->callback->onDecodeFailed(instance);
        } 
        return;
    }
    
    if (_handleRequest(instance) != MBS_ERR_OK) {
        if (instance->callback != NULL && instance->callback->onDecodeFailed != NULL) {
            instance->callback->onDecodeFailed(instance);
        } 
        return;
    }
    
    instance->sync.flag.isRecving = 0;
    
    instance->interface->beforeWrite(instance);
    if (_sendResponse(instance, false, false) != MBS_ERR_OK) {
        if (instance->callback != NULL && instance->callback->onDecodeFailed != NULL) {
            instance->callback->onDecodeFailed(instance);
        } 
        instance->interface->afterWrite(instance);
        return;
    }
    instance->interface->afterWrite(instance);
    
    if (instance->callback != NULL && instance->callback->onDecodeSuccess != NULL) {
        instance->callback->onDecodeSuccess(instance);
    }        
}


int16_t mbs_setRegisterData(MBS_t *instance, uint16_t startAddress, uint16_t *data, uint16_t length) {
    ASSERT(instance != NULL);
    ASSERT(data != NULL);
    ASSERT(length != 0);

    if (instance->sync.flag.mode == MBS_MODE_MAP) {
        for (uint16_t i = 0; i < length; i++) {
            MBS_Register_t *handler = _findByAddress(startAddress + i);
            if (handler == NULL) {
                continue;
            }

            memcpy(&handler->data[i], &data[i], sizeof(uint16_t));

            if (handler->va->onRequested != NULL) {
                handler->va->onRequested(MBS_REGSITER_HANDLER_REQUEST_TYPE_USER_WRITE, handler);
            }
        }
    } else if (instance->sync.flag.mode == MBS_MODE_WORK) {
        if (instance->callback == NULL || instance->callback->onDirectOperate == NULL) {
            return 0;
        }
        
        for (uint16_t i = 0; i < length; i++) {
            instance->callback->onDirectOperate(instance, 
                0, 0, MB_DIRECT_IO_WRITE_INTERNAL, 
                startAddress, i,
                &data[i]);
        }
    }

    return length;
}
        

int16_t mbs_getRegisterData(MBS_t *instance, uint16_t startAddress, uint16_t *data, uint16_t length) {
    ASSERT(instance != NULL);
    ASSERT(data != NULL);
    ASSERT(length != 0);
    
    for (uint16_t i = 0; i < length; i++) {
        MBS_Register_t *handler = _findByAddress(startAddress + i);
        if (handler == NULL) {
            continue;
        }

        data[i] = *handler->data;
        memcpy(&data[i], &handler->data[i], sizeof(uint16_t));

        if (handler->va->onRequested != NULL) {
            handler->va->onRequested(MBS_REGSITER_HANDLER_REQUEST_TYPE_USER_READ, handler);
        }
    }

    return length;
}
  

void mbs_setRegisterOnRequested(uint16_t address, MBS_ResigsterOnRequested_t onRequested) {
    MBS_Register_t *handler = _findByAddress(address);
    
    if (handler == NULL) {
        return;
    }

    handler->va->onRequested = onRequested;
}
        

void mbs_setRegisterOnWriteAuth(uint16_t address, MBS_ResigsterOnAuth_t onWriteAuth) {
    MBS_Register_t *handler = _findByAddress(address);
    
    if (handler == NULL) {
        return;
    }

    handler->va->onWriteAuth = onWriteAuth;
}


void mbs_setRegisterOnValueCheckAuth(uint16_t address, MBS_ResigsterOnAuth_t onValueCheckAuth) {
    MBS_Register_t *handler = _findByAddress(address);
    
    if (handler == NULL) {
        return;
    }

    handler->va->onValueCheckAuth = onValueCheckAuth;
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
