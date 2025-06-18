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

#ifndef _MODBUS_DEF_H_
#define _MODBUS_DEF_H_

#include <stdint.h>

#include "emmk-config.h"


//////////////////////////////////////////////////////////
/////////////// Modbus force to BIG-ENDIAN ///////////////
//////////////////////////////////////////////////////////
// send 0x1234 ->> [0] = 0x12, [1] = 0x34
// crc 0x1234 ->> [0] = x034, [1] = 0x12

//////////////////////////////////////////////////////////
// The device must have the RX_ILDE of the UART and the DMA function 
// in order to use the module perfectly and perfectly.
//////////////////////////////////////////////////////////

//// 
//// Constants which defines the format of a modbus frame. The example is
//// shown for a Modbus RTU/ASCII frame. Note that the Modbus PDU is not
//// dependent on the underlying transport.
//// 
//// <code>
//// <------------------------ MODBUS SERIAL LINE PDU (1) ------------------->
////              <----------- MODBUS PDU (1') ---------------->
////  +-----------+---------------+----------------------------+-------------+
////  | Address   | Function Code | Data                       | CRC/LRC     |
////  +-----------+---------------+----------------------------+-------------+
////  |           |               |                                   |
//// (2)        (3/2')           (3')                                (4)
//// 
//// (1)  ... MBM_SER_PDU_SIZE_MAX = 256
//// (2)  ... MBM_SER_PDU_ADDR_OFF = 0
//// (3)  ... MBM_SER_PDU_PDU_OFF  = 1
//// (4)  ... MBM_SER_PDU_SIZE_CRC = 2
//// 
//// (1') ... MBM_PDU_SIZE_MAX     = 253
//// (2') ... MBM_PDU_FUNC_OFF     = 0
//// (3') ... MBM_PDU_DATA_OFF     = 1
//// </code>
//// 

/**
 * @addtogroup modbus PDU/Serial Lin PDU define
 * @note none
 */
 
/*@{*/

#define MODBUS_SL_PDU_SIZE_MIN      (4)    // Serial line PDU: Minimum size of a Modbus RTU frame
#define MODBUS_SL_PDU_SIZE_MAX      (256)  // Serial line PDU: Maximum size of a Modbus RTU frame
#define MODBUS_SL_PDU_CRC_SIZE      (2)    // Serial line PDU: Size of CRC field in PDU
#define MODBUS_SL_PDU_ADDR_OFFSET   (0)    // Serial line PDU: Offset of slave address in Ser-PDU
#define MODBUS_SL_PDU_PDU_OFFSET    (1)    // Serial line PDU: Offset of Modbus-PDU in Ser-PDU
                                           
#define MODBUS_PDU_SIZE_MAX         (253)  // Maximum size of a PDU
#define MODBUS_PDU_SIZE_MIN         (1)    // Function Code
#define MODBUS_PDU_FUNC_OFFSET      (0)    // Offset of function code in PDU
#define MODBUS_PDU_DATA_OFFSET      (1)    // Offset for response data in PDU

/*@}*/

/**
 * @addtogroup modbus function define
 * @note none
 */

/*@{*/
   
#define MODBUS_FUNC_NONE                          (0)
#define MODBUS_FUNC_READ_COILS                    (1)
#define MODBUS_FUNC_READ_DISCRETE_INPUTS          (2)
#define MODBUS_FUNC_WRITE_SINGLE_COIL             (5)
#define MODBUS_FUNC_WRITE_MULTIPLE_COILS          (15)
#define MODBUS_FUNC_READ_HOLDING_REGISTER         (3)
#define MODBUS_FUNC_READ_INPUT_REGISTER           (4)
#define MODBUS_FUNC_WRITE_REGISTER                (6)
#define MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS      (16)
#define MODBUS_FUNC_READWRITE_MULTIPLE_REGISTERS  (23)
#define MODBUS_FUNC_DIAG_READ_EXCEPTION           (7)
#define MODBUS_FUNC_DIAG_DIAGNOSTIC               (8)
#define MODBUS_FUNC_DIAG_GET_COM_EVENT_CNT        (11)
#define MODBUS_FUNC_DIAG_GET_COM_EVENT_LOG        (12)
#define MODBUS_FUNC_OTHER_REPORT_SLAVEID          (17)
        
#define MODBUS_FUNC_ERROR                         (128)

/*@}*/

/**
 * @addtogroup Modbus genernal
 * @note none
 */
 
/*@{*/

typedef enum {
    MB_EX_NONE = 0x00,
    MB_EX_ILLEGAL_FUNCTION = 0x01,
    MB_EX_ILLEGAL_DATA_ADDRESS = 0x02,
    MB_EX_ILLEGAL_DATA_VALUE = 0x03,
    MB_EX_SLAVE_DEVICE_FAILURE = 0x04,
    MB_EX_ACKNOWLEDGE = 0x05,
    MB_EX_SLAVE_BUSY = 0x06,
    MB_EX_MEMORY_PARITY_ERROR = 0x08,
    MB_EX_GATEWAY_PATH_FAILED = 0x0A,
    MB_EX_GATEWAY_TGT_FAILED = 0x0B
} MB_Exception_t;

typedef struct {
    void (*open)(void *handler, uint32_t newBaudRate);
    
    void (*beforeWrite)(void *handler);
    void (*beforeRead)(void *handler);
    
    void (*afterWrite)(void *handler);
    void (*afterRead)(void *handler);
    
    int32_t (*write)(void *handler, uint8_t *data, uint32_t len, uint32_t timeout);
    int32_t (*read)(void *handler, uint8_t *data, uint32_t len, uint32_t timeout);
    
    void (*flush)(void *handler);
    
    void (*close)(void *handler);
} MB_Interface_t;


typedef enum {
    MB_DIRECT_IO_READ_START,
    MB_DIRECT_IO_READ,
    MB_DIRECT_IO_READ_END,

    MB_DIRECT_IO_WRITE_START,
    MB_DIRECT_IO_WRITE,
    MB_DIRECT_IO_WRITE_END,

    MB_DIRECT_IO_WRITE_INTERNAL,

    MB_DIRECT_IO_CHECK_ADDRESS,
} MB_Direct_IO_t;

typedef struct {
    int32_t (*onDirectOperate)(void *handler, 
        uint8_t address, uint8_t func, 
        MB_Direct_IO_t io, 
        uint16_t regAddressStart, uint16_t regAddressOffset, 
        uint16_t *regData);
    
    void (*onDecodeSuccess)(void *handler);
    void (*onDecodeFailed)(void *handler);
} MB_Callback_t;

/*@}*/

/**
 * @addtogroup Modbus master
 * @note none
 */
 
/*@{*/

typedef enum {
    MBM_ERR_OK,
    MBM_ERR_ILLEGAL_REQUEST,
    MBM_ERR_BUSY,
    MBM_ERR_RESPONSE_TIMEOUT,
    MBM_ERR_ILLEGAL_DATA,
    MBM_ERR_ILLEGAL_DATA_LENGTH,
    MBM_ERR_ILLEGAL_ARG,
    
    MBM_ERR_HOST_CLOSED,
    MBM_ERR_SENT,
    MBM_ERR_RECEIVED,
    MBM_ERR_FUNC_EXECUTE,
    MBM_ERR_FUNC_NO_SUPPORT
} MBM_Error_t;

typedef struct {
    MB_Interface_t *interface;

    uint8_t destAddr;
    uint8_t rcvAddr;

    // Tx buffer
    uint8_t *txBuf;
    uint16_t txBufSize;
    uint8_t *txBufCur;
    int16_t txDataLen;

    // Rx buffer
    uint8_t *rxBuf;
    uint16_t rxBufSize;
    int16_t rxDataLen;
    
    // Tx pdu
    int16_t txPDULen;
    
    // Rx pdu
    uint8_t *rxPDUFrameCur;
    int16_t rxPDULen;
    
    struct {
        qSTimer_t tmcd;

        union {
            uint8_t _;
            struct {
                uint8_t isWaiting : 1;
                uint8_t isCompleted : 1;
            };
        } flag;
    } sync;

    void *userData;
} MBM_t;

typedef void (*MBM_AsyncCB_t)(void *req, void *rep, MBM_Error_t err);

/*@}*/

/**
 * @addtogroup Modbus slave
 * @note none
 */
 
/*@{*/

typedef struct MBS_RegisterHandler MBS_Register_t;

typedef enum {
    MBS_MODE_MAP = 0,
    MBS_MODE_WORK = 1,
} MBS_Mode_t;

typedef enum {
    MBS_ERR_OK,
    MBS_ERR_RECV_CRC,
    MBS_ERR_RECV_DEV_ADDR,
    MBS_ERR_ILLEGAL_REQUEST,
    MBS_ERR_ILLEGAL_FUNC_CODE,
    MBS_ERR_NOT_SUPPORT_FUNC_CODE,
    MBS_ERR_SENT,
} MBS_Error_t;

typedef enum {
    MBS_REGSITER_TYPE_INPUT,
    MBS_REGSITER_TYPE_HOLDING,
    MBS_REGSITER_TYPE_COILS,
    MBS_REGSITER_TYPE_RESERVE
} MBS_RegisterType_t;

typedef enum {
    MBS_REGSITER_HANDLER_REQUEST_TYPE_USER_WRITE = 0,
    MBS_REGSITER_HANDLER_REQUEST_TYPE_HOST_WRITE = 1,
    MBS_REGSITER_HANDLER_REQUEST_TYPE_USER_READ = 2,
    MBS_REGSITER_HANDLER_REQUEST_TYPE_HOST_READ = 3,
} MBS_RegisterRequestType_t;

typedef struct {
    MB_Interface_t *interface;
    MB_Callback_t *callback;
    
    struct {
        union {
            uint8_t _;
            struct {
                uint8_t isRecving : 1;
                
                MBS_Mode_t mode: 2;
            };
        } flag;
    } sync;
    
    uint8_t address;

    struct {
        uint16_t *holdRegMap;
        uint16_t holdRegMapSize;
        uint16_t *inputRegMap;
        uint16_t inputRegMapSize;
    } regMap;
    
    // Tx buffer & PDU
    uint8_t *txBuffer;
    uint16_t txBufferSize;
    int16_t txBufferDataLength;
    
    uint8_t *txPayloadCursor;
    int16_t txPayloadLength;
    
    // Rx buffer & PDU
    uint8_t *rxBuffer;
    uint16_t rxBufferSize;
    int16_t rxBufferDataLength;
    
    uint8_t *rxPayloadCursor;
    int16_t rxPayloadLength;

    void *userData;
} MBS_t;

typedef const struct {
    uint8_t funcCode;

    void (*onHandle)(MBS_t *mbs);
} MBS_FuncHandler_t;

typedef void (*MBS_ResigsterOnRequested_t)(MBS_RegisterRequestType_t request, MBS_Register_t *handler);
typedef uint8_t (*MBS_ResigsterOnAuth_t)(MBS_Register_t *handler, uint16_t *data);

typedef struct {
    MBS_ResigsterOnRequested_t onRequested;
    MBS_ResigsterOnAuth_t onWriteAuth;
    MBS_ResigsterOnAuth_t onValueCheckAuth;
} MBS_RegisterHandlerVa_t;

struct MBS_RegisterHandler {
    uint16_t address;
    uint16_t *data;

    MBS_RegisterType_t type;
    MBS_RegisterHandlerVa_t *va;
    
    void *userData;
};

/*@}*/

/**
 * @addtogroup Modbus master - Request & Response context
 * @note none
 */
 
/*@{*/

typedef struct {
    void *data;
    uint8_t dataLen;
    
    MBM_Error_t err;
    MB_Exception_t excep;
    uint8_t success;
} MBM_ResponseHandler_t;

typedef struct {
    uint16_t readRegStartAddr;
    uint8_t readRegCount;
    
    uint16_t writeRegStartAddr;
    uint16_t *writeRegData;
    uint8_t writeRegCount;
} MBM_RequestContextHolding_t;

typedef struct {
    uint16_t readRegStartAddr;
    uint8_t readRegCount;
} MBM_RequestContextInput_t;

typedef struct {
    uint16_t readCoilStartAddr;
    uint16_t readCoilCount;
    
    uint16_t writeCoilStartAddr;
    uint8_t *writeCoilData;
    uint16_t writeCoilCount;
} MBM_RequestContextCoils_t;

typedef struct {
    struct {
        uint8_t destAddr;
        uint8_t funcCode;
        
        union {
            MBM_RequestContextCoils_t coils;
            MBM_RequestContextInput_t input;
            MBM_RequestContextHolding_t holding;
        } requestContext;
        
        uint8_t retryCount;
    } opts;
    
    struct {
        uint8_t retry;
    } _;
} MBM_RequestHandler_t;

typedef const struct {
    uint8_t funcCode;
    int32_t (*preHandler)(MBM_t *drv, MBM_RequestHandler_t *req);
    MB_Exception_t (*lastHandler)(MBM_t *drv, MBM_ResponseHandler_t *rep);
} MBM_FuncHandler_t;

/*@}*/

#endif
