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
#include "./mb_misc.h"

#if MBM_ENABLE && MBM_ENABLE_FUNC_COILS_REG == 1

/**
 * @addtogroup Coils register define
 * @note none
 */
 
/*@{*/

/**
 * Read Coils Req
 * @note none
 */
#define REQ_READ_ADDR_OFFSET            (MODBUS_PDU_DATA_OFFSET + 0)
#define REQ_READ_COILCNT_OFFSET         (MODBUS_PDU_DATA_OFFSET + 2)
#define REQ_READ_SIZE                   (4)

/**
 * Read Coils Func
 * @note none
 */
#define COILS_PDU_FUNC_READ_COILCNT_OFFSET        (MODBUS_PDU_DATA_OFFSET + 0)
#define COILS_PDU_FUNC_READ_VALUES_OFFSET         (MODBUS_PDU_DATA_OFFSET + 1)
#define COILS_PDU_FUNC_READ_SIZE_MIN               (1)
       
/**
 * Write Coils Req
 * @note none
 */       
#define REQ_WRITE_ADDR_OFFSET           (MODBUS_PDU_DATA_OFFSET)
#define REQ_WRITE_VALUE_OFFSET          (MODBUS_PDU_DATA_OFFSET + 2)
#define REQ_WRITE_SIZE                  (4)

/**
 * Write Coils Func
 * @note none
 */
#define COILS_PDU_FUNC_WRITE_ADDR_OFFSET          (MODBUS_PDU_DATA_OFFSET)
#define COILS_PDU_FUNC_WRITE_VALUE_OFFSET         (MODBUS_PDU_DATA_OFFSET + 2)
#define COILS_PDU_FUNC_WRITE_SIZE                  (4)
      
/**
 * Multi Write Coils Req
 * @note none
 */      
#define REQ_WRITE_MUL_ADDR_OFFSET       (MODBUS_PDU_DATA_OFFSET)
#define REQ_WRITE_MUL_COILCNT_OFFSET    (MODBUS_PDU_DATA_OFFSET + 2)
#define REQ_WRITE_MUL_BYTECNT_OFFSET    (MODBUS_PDU_DATA_OFFSET + 4)
#define REQ_WRITE_MUL_VALUES_OFFSET     (MODBUS_PDU_DATA_OFFSET + 5)
#define REQ_WRITE_MUL_SIZE_MIN          (5)
#define REQ_WRITE_MUL_COILCNT_MAX       (0x07B0)

/**
 * Multi Write Coils Func
 * @note none
 */    
#define COILS_PDU_FUNC_WRITE_MUL_ADDR_OFFSET      (MODBUS_PDU_DATA_OFFSET)
#define COILS_PDU_FUNC_WRITE_MUL_COILCNT_OFFSET   (MODBUS_PDU_DATA_OFFSET + 2)
#define COILS_PDU_FUNC_WRITE_MUL_SIZE             (5)

/*@}*/

/**
 * @addtogroup private function
 * @note none
 */
 
/*@{*/

static void set_coil(uint8_t *reg, uint8_t *src, uint16_t coil_addr, uint16_t coil_count) {
    uint8_t bytes = coil_count / 8 + 1;
    uint8_t byte_index = 0;
    uint8_t byte_bit_index = coil_addr % 8;
    
    while (bytes > 1) {
        mbMisc_SetBits(&reg[byte_index++], byte_bit_index, 8, *src++);
        bytes--;
    }
    
    // last
    bytes = bytes % 8;
    if (bytes != 0) {
        mbMisc_SetBits(&reg[byte_index++], byte_bit_index, bytes, *src++);
    }
}

/*@}*/

/**
 * @addtogroup coils read
 * @note none
 */
 
/*@{*/

static int32_t read_pre(MBM_t *drv, MBM_RequestHandler_t *req) {
    MBM_RequestContextCoils_t *coil = &req->opts.requestContext.coils;
    
    if (!(0x0001 <= coil->readCoilCount && coil->readCoilCount <= 0x07D0)) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    if ((MODBUS_PDU_SIZE_MIN + REQ_READ_SIZE) > drv->txBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    // setup dest
    __mbm_setDest(drv, req->opts.destAddr);
    
    // setup pdu
    uint8_t *pdu = __mbm_getTxPDU(drv);
    pdu[MODBUS_PDU_FUNC_OFFSET] = MODBUS_FUNC_READ_COILS;
    pdu[REQ_READ_ADDR_OFFSET] = coil->readCoilStartAddr >> 8;
    pdu[REQ_READ_ADDR_OFFSET + 1] = coil->readCoilStartAddr & 0x00FF;
    pdu[REQ_READ_COILCNT_OFFSET] = coil->readCoilCount >> 8;
    pdu[REQ_READ_COILCNT_OFFSET + 1] = coil->readCoilCount & 0x00FF;
    
    // setup pdu len
    __mbm_setTxPDULen(drv, MODBUS_PDU_SIZE_MIN + REQ_READ_SIZE);
    
    return 0;
}


static MB_Exception_t read_last(MBM_t *drv, MBM_ResponseHandler_t *rep) {
    MB_Exception_t exception = MB_EX_NONE;
    
    if (drv->rxPDULen >= MODBUS_PDU_SIZE_MIN + COILS_PDU_FUNC_READ_SIZE_MIN) {
        uint8_t *tx_pdu = __mbm_getTxPDU(drv);
        
        uint16_t req_coil_count = (uint16_t) tx_pdu[REQ_READ_COILCNT_OFFSET] << 8;
        req_coil_count |= (uint16_t) tx_pdu[REQ_READ_COILCNT_OFFSET + 1];
        
        uint8_t req_byte_count;
        if ((req_coil_count & 0x0007) != 0) {
            req_byte_count = req_coil_count / 8 + 1;
        } else {
            req_byte_count = req_coil_count / 8;
        }
        
        if (req_coil_count >= 1 && req_byte_count == drv->rxPDUFrameCur[COILS_PDU_FUNC_READ_COILCNT_OFFSET]) {            
            if (rep->data != NULL) {
                uint16_t req_readCoilStartAddr = (uint16_t) tx_pdu[REQ_READ_ADDR_OFFSET] << 8;
                req_readCoilStartAddr |= (uint16_t) tx_pdu[REQ_READ_ADDR_OFFSET + 1];
                
                set_coil((uint8_t *) rep->data,
                    &drv->rxPDUFrameCur[COILS_PDU_FUNC_READ_VALUES_OFFSET], 
                    req_readCoilStartAddr, 
                    req_coil_count);
            }
        } else {
            exception = MB_EX_ILLEGAL_DATA_VALUE;
        }
    } else {
        exception = MB_EX_ILLEGAL_DATA_VALUE;
    }
    
    return exception;
}


MBM_FUNC_DEFINE(coil_read, 
    MODBUS_FUNC_READ_COILS, 
    read_pre, 
    read_last
);

/*@}*/

/**
 * @addtogroup coils read
 * @note none
 */
 
/*@{*/

static int32_t single_write_pre(MBM_t *drv, MBM_RequestHandler_t *req) {
    MBM_RequestContextCoils_t *coil = &req->opts.requestContext.coils;
    
    if (*coil->writeCoilData != 0xFF && *coil->writeCoilData != 0x00) {
        return MBM_ERR_ILLEGAL_ARG;
    }
    
    // setup dest
    __mbm_setDest(drv, req->opts.destAddr);
    
    // setup pdu
    uint8_t *pdu = __mbm_getTxPDU(drv);
    pdu[MODBUS_PDU_FUNC_OFFSET] = MODBUS_FUNC_WRITE_SINGLE_COIL;
    pdu[REQ_WRITE_ADDR_OFFSET] = coil->writeCoilStartAddr >> 8;
    pdu[REQ_WRITE_ADDR_OFFSET + 1] = coil->writeCoilStartAddr & 0x00FF;
    pdu[REQ_WRITE_VALUE_OFFSET] = *coil->writeCoilData;
    pdu[REQ_WRITE_VALUE_OFFSET + 1] = 0x00;
    
    // setup pdu len
    __mbm_setTxPDULen(drv, MODBUS_PDU_SIZE_MIN + REQ_WRITE_SIZE);
    
    return 0;
}


static MB_Exception_t single_write_last(MBM_t *drv, MBM_ResponseHandler_t *rep) {
    MB_Exception_t exception = MB_EX_NONE;
    
    if (drv->rxPDULen == MODBUS_PDU_SIZE_MIN + COILS_PDU_FUNC_WRITE_SIZE) {
        uint8_t *rxPdu = drv->rxPDUFrameCur;
        
        uint16_t repWriteCoilAddr = (uint16_t) rxPdu[COILS_PDU_FUNC_WRITE_ADDR_OFFSET] << 8;
        repWriteCoilAddr |= rxPdu[COILS_PDU_FUNC_WRITE_ADDR_OFFSET + 1];
        
        uint16_t reqWriteCoilAddr = (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_ADDR_OFFSET] << 8;
        reqWriteCoilAddr |= (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_ADDR_OFFSET + 1];
        
        if (repWriteCoilAddr == reqWriteCoilAddr && rxPdu[COILS_PDU_FUNC_WRITE_VALUE_OFFSET + 1] == 0x00) {
            if (rep->data != NULL) {
                uint8_t isSet[2] = {rxPdu[COILS_PDU_FUNC_WRITE_VALUE_OFFSET] & 0x01, 0x00};
                set_coil((uint8_t *) rep->data, isSet, repWriteCoilAddr, 1);
            }
        } else {
            exception = MB_EX_ILLEGAL_DATA_VALUE;
        }
    } else {
        exception = MB_EX_ILLEGAL_DATA_VALUE;
    }
    
    return exception;
}


MBM_FUNC_DEFINE(coil_single_write, 
    MODBUS_FUNC_WRITE_SINGLE_COIL, 
    single_write_pre, 
    single_write_last
);

/*@}*/

/**
 * @addtogroup coils read
 * @note none
 */
 
/*@{*/

static int32_t multi_write_pre(MBM_t *drv, MBM_RequestHandler_t *req) {
    MBM_RequestContextCoils_t *coil = &req->opts.requestContext.coils;
    
    if (!(0x0001 <= coil->readCoilCount && coil->readCoilCount <= 0x07B0)) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    uint8_t byte_count = 0;
    if ((coil->writeCoilCount & 0x0007) != 0) {
        byte_count = coil->writeCoilCount / 8 + 1;
    } else {
        byte_count = coil->writeCoilCount / 8;
    }
    
    if ((MODBUS_PDU_SIZE_MIN + REQ_WRITE_MUL_SIZE_MIN + byte_count) > drv->txBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    // setup dest
    __mbm_setDest(drv, req->opts.destAddr);
    
    // setup pdu
    uint8_t *pdu = __mbm_getTxPDU(drv);
    pdu[MODBUS_PDU_FUNC_OFFSET] = MODBUS_FUNC_WRITE_MULTIPLE_COILS;
    pdu[REQ_WRITE_MUL_ADDR_OFFSET] = coil->writeCoilStartAddr >> 8;
    pdu[REQ_WRITE_MUL_ADDR_OFFSET + 1] = coil->writeCoilStartAddr & 0x00FF;
    pdu[REQ_WRITE_MUL_COILCNT_OFFSET] = coil->writeCoilCount >> 8;
    pdu[REQ_WRITE_MUL_COILCNT_OFFSET + 1] = coil->writeCoilCount & 0x00FF;
    
    uint8_t byte_count = 0;
    if ((coil->writeCoilCount & 0x0007) != 0) {
        byte_count = coil->writeCoilCount / 8 + 1;
    } else {
        byte_count = coil->writeCoilCount / 8;
    }
    
    pdu[REQ_WRITE_MUL_BYTECNT_OFFSET] = byte_count;
    
    pdu += REQ_WRITE_MUL_VALUES_OFFSET;
    
    for (uint8_t index = 0; index < byte_count; index++) {
        *pdu++ = coil->writeCoilData[index];
    }
    
    // setup pdu len
    __mbm_setTxPDULen(drv, MODBUS_PDU_SIZE_MIN + REQ_WRITE_MUL_SIZE_MIN + byte_count);
    
    return 0;
}


static MB_Exception_t multi_write_last(MBM_t *drv, MBM_ResponseHandler_t *rep) {
    MB_Exception_t exception = MB_EX_NONE;
    
    if (drv->rxPDULen == MODBUS_PDU_SIZE_MIN + COILS_PDU_FUNC_WRITE_MUL_SIZE) {
        uint16_t reqWriteCoilCount = (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_MUL_COILCNT_OFFSET] << 8;
        reqWriteCoilCount |= (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_MUL_COILCNT_OFFSET + 1];
        
        uint16_t repWroteCoilCount = (uint16_t) drv->rxPDUFrameCur[COILS_PDU_FUNC_WRITE_MUL_COILCNT_OFFSET] << 8;
        repWroteCoilCount |= (uint16_t) drv->rxPDUFrameCur[COILS_PDU_FUNC_WRITE_MUL_COILCNT_OFFSET + 1];
        
        uint16_t reqWriteCoilStartAddr = (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_MUL_ADDR_OFFSET] << 8;
        reqWriteCoilStartAddr |= (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_MUL_ADDR_OFFSET + 1];
        
        uint16_t repWroteCoilStartAddr = (uint16_t) drv->rxPDUFrameCur[COILS_PDU_FUNC_WRITE_MUL_ADDR_OFFSET] << 8;
        repWroteCoilStartAddr |= (uint16_t) drv->rxPDUFrameCur[COILS_PDU_FUNC_WRITE_MUL_ADDR_OFFSET + 1];
        
        if (repWroteCoilCount != reqWriteCoilCount) {
            exception = MB_EX_ILLEGAL_DATA_VALUE;
        } else if (repWroteCoilStartAddr != reqWriteCoilStartAddr) {
            exception = MB_EX_ILLEGAL_DATA_ADDRESS;
        }
    } else {
        exception = MB_EX_ILLEGAL_DATA_VALUE;
    }
    
    return exception;
}


MBM_FUNC_DEFINE(coil_multi_write, 
    MODBUS_FUNC_WRITE_MULTIPLE_COILS, 
    multi_write_pre, 
    multi_write_last
);

/*@}*/

#endif
