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

#include "./mbm.h"

#if MBM_ENABLE && MBM_ENABLE_FUNC_HOLDING_REG == 1

/**
 * @addtogroup Holding register define
 * @note none
 */
 
/*@{*/

/**
 * Read Holding Req
 * @note none
 */
#define REQ_READ_ADDR_OFFSET          (MODBUS_PDU_DATA_OFFSET + 0)
#define REQ_READ_REGCNT_OFFSET        (MODBUS_PDU_DATA_OFFSET + 2)
#define REQ_READ_SIZE                 (4)

/**
 * Read Holding Func
 * @note none
 */
#define HOLDING_PDU_FUNC_READ_REGCNT_MAX          (0x007D)
#define HOLDING_PDU_FUNC_READ_BYTECNT_OFFSET      (MODBUS_PDU_DATA_OFFSET + 0)
#define HOLDING_PDU_FUNC_READ_VALUES_OFFSET       (MODBUS_PDU_DATA_OFFSET + 1)
#define HOLDING_PDU_FUNC_READ_SIZE_MIN            (1)

/**
 * Write Holding Req
 * @note none
 */
#define REQ_WRITE_ADDR_OFFSET         (MODBUS_PDU_DATA_OFFSET + 0)
#define REQ_WRITE_VALUE_OFFSET        (MODBUS_PDU_DATA_OFFSET + 2)
#define REQ_WRITE_SIZE                (4)
     
/**
 * Write Holding Func
 * @note none
 */     
#define HOLDING_PDU_FUNC_WRITE_ADDR_OFFSET        (MODBUS_PDU_DATA_OFFSET + 0)
#define HOLDING_PDU_FUNC_WRITE_VALUE_OFFSET       (MODBUS_PDU_DATA_OFFSET + 2)
#define HOLDING_PDU_FUNC_WRITE_SIZE               (4)


/**
 * Multi Write Holding Req
 * @note none
 */     
#define REQ_WRITE_MUL_ADDR_OFFSET        (MODBUS_PDU_DATA_OFFSET + 0)
#define REQ_WRITE_MUL_REGCNT_OFFSET      (MODBUS_PDU_DATA_OFFSET + 2)
#define REQ_WRITE_MUL_BYTECNT_OFFSET     (MODBUS_PDU_DATA_OFFSET + 4)
#define REQ_WRITE_MUL_VALUES_OFFSET      (MODBUS_PDU_DATA_OFFSET + 5)
#define REQ_WRITE_MUL_SIZE_MIN           (5)
#define REQ_WRITE_MUL_REGCNT_MAX         (0x0078)


/**
 * Multi Read Holding Func
 * @note none
 */     
#define HOLDING_PDU_FUNC_WRITE_MUL_ADDR_OFFSET       (MODBUS_PDU_DATA_OFFSET + 0)
#define HOLDING_PDU_FUNC_WRITE_MUL_REGCNT_OFFSET     (MODBUS_PDU_DATA_OFFSET + 2)
#define HOLDING_PDU_FUNC_WRITE_MUL_SIZE              (4)

/**
 * Multi Read Write Holding Req
 * @note none
 */
#define REQ_READWRITE_READ_ADDR_OFFSET      (MODBUS_PDU_DATA_OFFSET + 0)
#define REQ_READWRITE_READ_REGCNT_OFFSET    (MODBUS_PDU_DATA_OFFSET + 2)
#define REQ_READWRITE_WRITE_ADDR_OFFSET     (MODBUS_PDU_DATA_OFFSET + 4)
#define REQ_READWRITE_WRITE_REGCNT_OFFSET   (MODBUS_PDU_DATA_OFFSET + 6)
#define REQ_READWRITE_WRITE_BYTECNT_OFFSET  (MODBUS_PDU_DATA_OFFSET + 8)
#define REQ_READWRITE_WRITE_VALUES_OFFSET   (MODBUS_PDU_DATA_OFFSET + 9)
#define REQ_READWRITE_SIZE_MIN              (9)              
  
/**
 * Multi Read Write Holding Func
 * @note none
 */  
#define HOLDING_PDU_FUNC_READWRITE_READ_BYTECNT_OFFSET  (MODBUS_PDU_DATA_OFFSET + 0)
#define HOLDING_PDU_FUNC_READWRITE_READ_VALUES_OFFSET   (MODBUS_PDU_DATA_OFFSET + 1)
#define HOLDING_PDU_FUNC_READWRITE_SIZE_MIN             (1)

/*@}*/

/**
 * @addtogroup func read
 * @note none
 */
 
/*@{*/

static int32_t read_pre(MBM_t *drv, MBM_RequestHandler_t *req) {
    MBM_RequestContextHolding_t *r = &req->opts.requestContext.holding;
    
    if (!(0x01 <= r->readRegCount && r->readRegCount <= 0x7D)) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    if (2 + MODBUS_PDU_SIZE_MIN + REQ_READ_SIZE > drv->txBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    if (r->readRegCount * 2 + MODBUS_PDU_SIZE_MIN + REQ_READ_SIZE > drv->rxBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    // setup dest
    __mbm_setDest(drv, req->opts.destAddr);
    
    // setup pdu
    uint8_t *pdu = __mbm_getTxPDU(drv);
    pdu[MODBUS_PDU_FUNC_OFFSET] = MODBUS_FUNC_READ_HOLDING_REGISTER;
    pdu[REQ_READ_ADDR_OFFSET] = r->readRegStartAddr >> 8;
    pdu[REQ_READ_ADDR_OFFSET + 1] = r->readRegStartAddr & 0x00FF;
    pdu[REQ_READ_REGCNT_OFFSET] = 0x00;
    pdu[REQ_READ_REGCNT_OFFSET + 1] = r->readRegCount;
    
    // setup pdu len
    __mbm_setTxPDULen(drv, MODBUS_PDU_SIZE_MIN + REQ_READ_SIZE);
    
    return 0;
}


static MB_Exception_t read_last(MBM_t *drv, MBM_ResponseHandler_t *rep) {
    MB_Exception_t exception = MB_EX_NONE;
    
    if (drv->rxPDULen >= MODBUS_PDU_SIZE_MIN + HOLDING_PDU_FUNC_READ_SIZE_MIN) {
        uint8_t *rx_pdu = drv->rxPDUFrameCur;
        
        uint8_t req_reg_count = __mbm_getTxPDU(drv)[REQ_READ_REGCNT_OFFSET + 1];
        uint8_t rep_reg_count = rx_pdu[HOLDING_PDU_FUNC_READ_BYTECNT_OFFSET] / 2;
        
        if (0x01 <= rep_reg_count && rep_reg_count <= 0x7D && req_reg_count == rep_reg_count) {
            if (rep->data != NULL) {
                rep->dataLen = rep_reg_count;
                
                uint16_t *rep_data = rep->data;
                uint8_t *src = &rx_pdu[HOLDING_PDU_FUNC_READ_VALUES_OFFSET];
                for (uint8_t index = 0; index < rep->dataLen; index++) {
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


MBM_FUNC_DEFINE(holding_read, 
    MODBUS_FUNC_READ_HOLDING_REGISTER, 
    read_pre, 
    read_last
);

/*@}*/

/**
 * @addtogroup func signal write 
 * @note none
 */
 
/*@{*/

static int32_t single_write_pre(MBM_t *drv, MBM_RequestHandler_t *req) {
    MBM_RequestContextHolding_t *r = &req->opts.requestContext.holding;
    
    // setup dest
    __mbm_setDest(drv, req->opts.destAddr);
    
    // setup pdu
    uint8_t *pdu = __mbm_getTxPDU(drv);
    pdu[MODBUS_PDU_FUNC_OFFSET] = MODBUS_FUNC_WRITE_REGISTER;
    pdu[REQ_WRITE_ADDR_OFFSET] = r->writeRegStartAddr >> 8;
    pdu[REQ_WRITE_ADDR_OFFSET + 1] = r->writeRegStartAddr & 0x00FF;
    pdu[REQ_WRITE_VALUE_OFFSET] = *((uint16_t *) r->writeRegData) >> 8;
    pdu[REQ_WRITE_VALUE_OFFSET + 1] = *((uint16_t *) r->writeRegData) & 0x00FF;
    
    // setup pdu len
    __mbm_setTxPDULen(drv, MODBUS_PDU_SIZE_MIN + REQ_WRITE_SIZE);
    
    return 0;
}


static MB_Exception_t single_write_last(MBM_t *drv, MBM_ResponseHandler_t *rep) {
    MB_Exception_t exception = MB_EX_NONE;
    
    if (drv->rxPDULen == MODBUS_PDU_SIZE_MIN + HOLDING_PDU_FUNC_WRITE_SIZE) {
        uint16_t rep_reg_start_addr = (uint16_t) drv->rxPDUFrameCur[HOLDING_PDU_FUNC_WRITE_ADDR_OFFSET] << 8;
        rep_reg_start_addr |= (uint16_t) drv->rxPDUFrameCur[HOLDING_PDU_FUNC_WRITE_ADDR_OFFSET + 1];
        
        uint16_t req_reg_start_addr = (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_ADDR_OFFSET] << 8;
        req_reg_start_addr |= (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_ADDR_OFFSET + 1];
        
        if (rep_reg_start_addr == req_reg_start_addr) {
            if (rep->data != NULL) {
                // Fill out data, only one
                uint8_t *src = &drv->rxPDUFrameCur[HOLDING_PDU_FUNC_WRITE_VALUE_OFFSET];
                uint16_t *rep_data = rep->data;
                
                *rep_data = (uint16_t) *src++ << 8;
                *rep_data |= (uint16_t) *src++;
            }
        } else {
            exception = MB_EX_ILLEGAL_DATA_ADDRESS;
        }
    } else {
        exception = MB_EX_ILLEGAL_DATA_VALUE;
    }
    
    return exception;
}


MBM_FUNC_DEFINE(holding_single_write, 
    MODBUS_FUNC_WRITE_REGISTER, 
    single_write_pre, 
    single_write_last
);

/*@}*/

/**
 * @addtogroup func signal write 
 * @note none
 */
 
/*@{*/

static int32_t multi_write_pre(MBM_t *drv, MBM_RequestHandler_t *req) {
    uint8_t *pdu;
    
    MBM_RequestContextHolding_t *r = &req->opts.requestContext.holding;
    
    if (!(0x01 <= r->writeRegCount && r->writeRegCount <= 0x78)) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    if ((r->writeRegCount) * 2 + MODBUS_PDU_SIZE_MIN + REQ_WRITE_MUL_SIZE_MIN > drv->txBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }

    // setup dest
    __mbm_setDest(drv, req->opts.destAddr);
    
    // setup pdu
    pdu = __mbm_getTxPDU(drv);
    pdu[MODBUS_PDU_FUNC_OFFSET] = MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS;
    pdu[REQ_WRITE_MUL_ADDR_OFFSET] = r->writeRegStartAddr >> 8;
    pdu[REQ_WRITE_MUL_ADDR_OFFSET + 1] = r->writeRegStartAddr & 0x00FF;
    pdu[REQ_WRITE_MUL_REGCNT_OFFSET] = 0x00;
    pdu[REQ_WRITE_MUL_REGCNT_OFFSET + 1] = r->writeRegCount;
    pdu[REQ_WRITE_MUL_BYTECNT_OFFSET] = r->writeRegCount * 2;
    
    pdu += REQ_WRITE_MUL_VALUES_OFFSET;
    
    for (uint8_t index = 0; index < r->writeRegCount; index++) {
        *pdu++ = r->writeRegData[index] >> 8;
        *pdu++ = r->writeRegData[index] & 0x00FF;
    }
    
    // Reset pdu postions
    pdu = __mbm_getTxPDU(drv);
    
    // setup pdu len
    __mbm_setTxPDULen(drv, MODBUS_PDU_SIZE_MIN + REQ_WRITE_MUL_SIZE_MIN + pdu[REQ_WRITE_MUL_BYTECNT_OFFSET]);
    
    return 0;
}


static MB_Exception_t multi_write_last(MBM_t *drv, MBM_ResponseHandler_t *rep) {
    MB_Exception_t exception = MB_EX_NONE;
    
    if (drv->rxPDULen >= MODBUS_PDU_SIZE_MIN + HOLDING_PDU_FUNC_WRITE_MUL_SIZE) {
        uint16_t req_reg_start_addr = (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_MUL_ADDR_OFFSET] << 8;
        req_reg_start_addr |= (uint16_t) __mbm_getTxPDU(drv)[REQ_WRITE_MUL_ADDR_OFFSET + 1];
        
        uint16_t rep_reg_start_addr = (uint16_t) drv->rxPDUFrameCur[HOLDING_PDU_FUNC_WRITE_MUL_ADDR_OFFSET] << 8;
        rep_reg_start_addr |= (uint16_t) drv->rxPDUFrameCur[HOLDING_PDU_FUNC_WRITE_MUL_ADDR_OFFSET + 1];
        
        uint8_t rep_reg_count = drv->rxPDUFrameCur[REQ_WRITE_MUL_REGCNT_OFFSET + 1];
        uint8_t req_byte_count = __mbm_getTxPDU(drv)[REQ_WRITE_MUL_BYTECNT_OFFSET];
        
        if (rep_reg_count * 2 != req_byte_count) {
            exception = MB_EX_ILLEGAL_DATA_VALUE;
        } else if (req_reg_start_addr != rep_reg_start_addr) {
            exception = MB_EX_ILLEGAL_DATA_ADDRESS;
        }
    } else {
        exception = MB_EX_ILLEGAL_DATA_VALUE;
    }
    
    return exception;
}


MBM_FUNC_DEFINE(holding_multi_write, 
    MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS, 
    multi_write_pre, 
    multi_write_last
);

/*@}*/

/**
 * @addtogroup func signal write 
 * @note none
 */
 
/*@{*/

static int32_t multi_read_write_pre(MBM_t *drv, MBM_RequestHandler_t *req) {
    uint8_t *pdu;
    
    MBM_RequestContextHolding_t *r = &req->opts.requestContext.holding;
    
    if (!(0x01 <= r->writeRegCount && r->writeRegCount <= 0x76)) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    if (!(0x01 <= r->readRegCount && r->readRegCount <= 0x76)) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    if ((r->writeRegCount * 2) + MODBUS_PDU_SIZE_MIN + REQ_READWRITE_SIZE_MIN > drv->txBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }

    if ((r->readRegCount * 2) + MODBUS_PDU_SIZE_MIN + REQ_READWRITE_SIZE_MIN > drv->rxBufSize) {
        return MBM_ERR_ILLEGAL_DATA_LENGTH;
    }
    
    // setup dest
    __mbm_setDest(drv, req->opts.destAddr);
    
    // setup pdu
    pdu = __mbm_getTxPDU(drv);
    pdu[MODBUS_PDU_FUNC_OFFSET] = MODBUS_FUNC_READWRITE_MULTIPLE_REGISTERS;
    
    pdu[REQ_READWRITE_READ_ADDR_OFFSET] = r->readRegStartAddr >> 8;
    pdu[REQ_READWRITE_READ_ADDR_OFFSET + 1] = r->readRegStartAddr & 0x00FF;
    pdu[REQ_READWRITE_READ_REGCNT_OFFSET] = 0x00;
    pdu[REQ_READWRITE_READ_REGCNT_OFFSET + 1] = r->readRegCount;
    
    pdu[REQ_READWRITE_WRITE_ADDR_OFFSET] = r->writeRegStartAddr >> 8;
    pdu[REQ_READWRITE_WRITE_ADDR_OFFSET + 1] = r->writeRegStartAddr & 0x00FF;
    pdu[REQ_READWRITE_WRITE_REGCNT_OFFSET] = 0x00;
    pdu[REQ_READWRITE_WRITE_REGCNT_OFFSET + 1] = r->writeRegCount;
    pdu[REQ_READWRITE_WRITE_BYTECNT_OFFSET] = r->writeRegCount * 2;
    
    pdu += REQ_READWRITE_WRITE_VALUES_OFFSET;
    
    for (uint8_t index = 0; index < r->writeRegCount; index++) {
        *pdu++ = r->writeRegData[index] >> 8;
        *pdu++ = r->writeRegData[index] & 0x00FF;
    }
    
    // Reset pdu postions
    pdu = __mbm_getTxPDU(drv);
    
    // setup pdu len
    __mbm_setTxPDULen(drv, MODBUS_PDU_SIZE_MIN + REQ_READWRITE_SIZE_MIN + pdu[REQ_READWRITE_WRITE_BYTECNT_OFFSET]);
    
    return 0;
}


static MB_Exception_t multi_read_write_last(MBM_t *drv, MBM_ResponseHandler_t *rep) {
    MB_Exception_t exception = MB_EX_NONE;
    
    if (drv->rxPDULen >= MODBUS_PDU_SIZE_MIN + HOLDING_PDU_FUNC_READWRITE_SIZE_MIN) {
        uint8_t req_readRegCount = __mbm_getTxPDU(drv)[REQ_READWRITE_READ_REGCNT_OFFSET + 1];
        uint8_t rep_read_reg_byte_count = drv->rxPDUFrameCur[HOLDING_PDU_FUNC_READWRITE_READ_BYTECNT_OFFSET];
        
        if (rep_read_reg_byte_count == req_readRegCount * 2) {
            if (rep->data != NULL) {
                uint8_t *src = &drv->rxPDUFrameCur[HOLDING_PDU_FUNC_READ_VALUES_OFFSET];
                
                uint16_t *rep_data = rep->data;
                rep->dataLen = req_readRegCount;
                
                for (uint8_t index = 0; index < req_readRegCount; index++) {
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


MBM_FUNC_DEFINE(holding_multi_read_write, 
    MODBUS_FUNC_READWRITE_MULTIPLE_REGISTERS, 
    multi_read_write_pre, 
    multi_read_write_last
);

/*@}*/

#endif
