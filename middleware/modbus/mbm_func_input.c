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

////
//// Local include
////
#include "./mbm.h"

#if MBM_ENABLE && MBM_ENABLE_FUNC_INPUT_REG == 1

/**
 * @addtogroup Register define
 * @note none
 */
 
/*@{*/

#define REQ_READ_ADDR_OFFSET       (MODBUS_PDU_DATA_OFFSET + 0)
#define REQ_READ_REGCNT_OFFSET     (MODBUS_PDU_DATA_OFFSET + 2)
#define REQ_READ_SIZE              (4)

#define PDU_FUNC_READ_BYTECNT_OFFSET        (MODBUS_PDU_DATA_OFFSET + 0)
#define PDU_FUNC_READ_VALUES_OFFSET         (MODBUS_PDU_DATA_OFFSET + 1)
#define PDU_FUNC_READ_SIZE_MIN              (1)
#define PDU_FUNC_READ_RSP_BYTECNT_OFFSET    (MODBUS_PDU_DATA_OFFSET)

/*@}*/

/**
 * @addtogroup functions
 * @note none
 */
 
/*@{*/

static int32_t mbm_func_input_read_pre(MBM_t *drv, MBM_RequestHandler_t *req) {
    MBM_RequestContextInput_t *input = &req->opts.requestContext.input;
    
    if (!(0x01 <= input->readRegCount && input->readRegCount <= 0x7D)) {
        return MBM_ERR_ILLEGAL_REQUEST;
    }
    
    if (2 + MODBUS_PDU_SIZE_MIN + REQ_READ_SIZE > drv->txBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    if ((input->readRegCount) * 2 + MODBUS_PDU_SIZE_MIN + REQ_READ_SIZE > drv->rxBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    // setup dest
    __mbm_setDest(drv, req->opts.destAddr);
    
    // setup pdu
    uint8_t *pdu = __mbm_getTxPDU(drv);
    pdu[MODBUS_PDU_FUNC_OFFSET] = MODBUS_FUNC_READ_INPUT_REGISTER;
    pdu[REQ_READ_ADDR_OFFSET] = input->readRegStartAddr >> 8;
    pdu[REQ_READ_ADDR_OFFSET + 1] = input->readRegStartAddr & 0x00FF;
    pdu[REQ_READ_REGCNT_OFFSET] = 0x00;
    pdu[REQ_READ_REGCNT_OFFSET + 1] = input->readRegCount;
    
    // setup pdu len
    __mbm_setTxPDULen(drv, MODBUS_PDU_SIZE_MIN + REQ_READ_SIZE);
    
    return 0;
}


static MB_Exception_t mbm_func_input_read_last(MBM_t *drv, MBM_ResponseHandler_t *rep) {
    MB_Exception_t exception = MB_EX_NONE;
    
    if (drv->rxPDULen >= MODBUS_PDU_SIZE_MIN + PDU_FUNC_READ_SIZE_MIN) {
        uint8_t byte_count = __mbm_getRxPDU(drv)[PDU_FUNC_READ_BYTECNT_OFFSET];
        uint8_t reg_count = __mbm_getTxPDU(drv)[REQ_READ_REGCNT_OFFSET + 1];
        if (reg_count * 2 == byte_count) {
            if (rep->data != NULL) {
                rep->dataLen = reg_count;
                
                uint16_t *rep_data = rep->data;
                uint8_t *src = &drv->rxPDUFrameCur[PDU_FUNC_READ_VALUES_OFFSET];
                for (uint8_t index = 0; index < reg_count; index++) {
                    rep_data[index] = (uint16_t) *src++ << 8;
                    rep_data[index] |= (uint16_t) *src++;
                }
            }
        } else {
            exception = MB_EX_ILLEGAL_DATA_VALUE;
        }
    } else {
        exception = MB_EX_ILLEGAL_DATA_VALUE;
    }
    
    return exception;
}

/*@}*/

/**
 * @addtogroup define
 * @note none
 */
 
/*@{*/

MBM_FUNC_DEFINE(input_read, 
    MODBUS_FUNC_READ_INPUT_REGISTER, 
    mbm_func_input_read_pre, 
    mbm_func_input_read_last
);

/*@}*/

#endif
