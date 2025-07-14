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

#ifndef _CM_MODULE_H_
#define _CM_MODULE_H_

#include <stdint.h>

#include "emmk-config.h"
#include "middleware/rilat/rilat.h"

/**
 * @addtogroup Version
 * @note none
 */

/*@{*/

#define CMODULE_VERISON_STR      "R241115"

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */

/*@{*/

#define CMODULE_URC_FLAG_CONN_SUCCESS           KLBIT(0)
#define CMODULE_URC_FLAG_CONN_RETRY             KLBIT(1)
#define CMODULE_URC_FLAG_CONN_FAILED            KLBIT(2)
#define CMODULE_URC_FLAG_OPEN_SUCCESS           KLBIT(3)
#define CMODULE_URC_FLAG_OPEN_FAILED            KLBIT(4)
#define CMODULE_URC_FLAG_DISC_SUCCESS           KLBIT(5)
#define CMODULE_URC_FLAG_DISC_FAILED            KLBIT(6)
#define CMODULE_URC_FLAG_SUB_SUCCESS            KLBIT(7)
#define CMODULE_URC_FLAG_SUB_FAILED             KLBIT(8)
#define CMODULE_URC_FLAG_UNS_SUCCESS            KLBIT(9)
#define CMODULE_URC_FLAG_UNS_FAILED             KLBIT(10)
#define CMODULE_URC_FLAG_PUB_SUCCESS            KLBIT(11)
#define CMODULE_URC_FLAG_PUB_FAILED             KLBIT(12)
#define CMODULE_URC_FLAG_CLOSE_SUCCESS          KLBIT(13)
#define CMODULE_URC_FLAG_CLOSE_FAILED           KLBIT(14)
#define CMODULE_URC_FLAG_NEED_RESET             KLBIT(15)
#define CMODULE_URC_FLAG_REPORTY_REPLY          KLBIT(16)
#define CMODULE_URC_FLAG_EVENT_REPLY            KLBIT(17)
#define CMODULE_URC_FLAG_IN_POLL                KLBIT(18)
#define CMODULE_URC_FLAG_ONLINE_POSTED          KLBIT(19)
#define CMODULE_URC_FLAG_STACK_READY            KLBIT(20)
#define CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS   KLBIT(21)
// #define CMODULE_URC_FLAG_STACK_BLOCK            KLBIT(22)
#define CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL    KLBIT(23)
#define CMODULE_URC_FLAG_CALL_START             KLBIT(24)
#define CMODULE_URC_FLAG_CALL_CONNECT           KLBIT(25)
#define CMODULE_URC_FLAG_CALL_END               KLBIT(26)
#define CMODULE_URC_FLAG_STACK_BLOCK_FOREVER    KLBIT(27)

#define CMODULE_ERR_COUNT 255

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
#define __CMODULE_X_SECTION      KCOMPILER_SECTION("_CMODULEX")
#define __CMODULE_X_START       ((uint32_t) &KCOMPILER_SECTION_START(_CMODULEX))
#define __CMODULE_X_END         ((uint32_t) &KCOMPILER_SECTION_END(_CMODULEX))
#define __CMODULE_X_SECTION_EXTERN \
extern uint32_t KCOMPILER_SECTION_START(_CMODULEX); \
extern uint32_t KCOMPILER_SECTION_END(_CMODULEX);
#elif defined(__GNUC__RV32_CHX)
#define __CMODULE_X_SECTION      KCOMPILER_SECTION("CMODULEX")
#define __CMODULE_X_START       ((uint32_t) &KCOMPILER_SECTION_START(CMODULEX))
#define __CMODULE_X_END         ((uint32_t) &KCOMPILER_SECTION_END(CMODULEX))
#define __CMODULE_X_SECTION_EXTERN \
    extern uint32_t KCOMPILER_SECTION_START(CMODULEX); \
    extern uint32_t KCOMPILER_SECTION_END(CMODULEX);
#elif defined (__GNUC__)
#define __CMODULE_X_SECTION      KCOMPILER_SECTION(".CMODULEX")
#define __CMODULE_X_START       ((uint32_t) &KCOMPILER_SECTION_START(_CMODULEX))
#define __CMODULE_X_END         ((uint32_t) &KCOMPILER_SECTION_END(_CMODULEX))
#define __CMODULE_X_SECTION_EXTERN \
extern uint32_t KCOMPILER_SECTION_START(_CMODULEX); \
extern uint32_t KCOMPILER_SECTION_END(_CMODULEX);
#endif


/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

typedef enum {
    /// @ModuleBase
    CMODULE_REQ_ON_SERIAL_RECV,
    CMODULE_REQ_ON_SERIAL_TRANSMIT,
    CMODULE_REQ_ON_SERIAL_FLUSH,
    CMODULE_REQ_ON_SERIAL_INIT,
    CMODULE_REQ_ON_SERIAL_FINALIZE,
    CMODULE_REQ_ON_MODEM_POWER_PIN_ACCESS,
    CMODULE_REQ_ON_MODEM_POWER_PIN_LEVEL,
    CMODULE_REQ_ON_MODEM_RESET_PIN_ACCESS,
    CMODULE_REQ_ON_MODEM_RESET_PIN_LEVEL,
    CMODULE_REQ_ON_INIT_START,
    CMODULE_REQ_ON_INIT_SUCCESS,
    CMODULE_REQ_ON_INIT_FAILED,
    CMODULE_REQ_ON_NET_CONNECT,
    CMODULE_REQ_ON_NET_DISCONNECT,
    CMODULE_REQ_ON_NET_RECONNECT,
    CMODULE_REQ_ON_NET_PROTOCOL_READY,
    CMODULE_REQ_ON_SEND_START,
    CMODULE_REQ_ON_SEND_SUCCESS,
    CMODULE_REQ_ON_SEND_FAILED,
    CMODULE_REQ_ON_RECV_DATA,
    CMODULE_REQ_ON_BLOCK_POLL,
    CMODULE_REQ_ON_NTP_UPDATE,
    CMODULE_REQ_ON_STATE_CHANGED,
    CMODULE_REQ_ON_STACK_NEED_RESET,

    CMODULE_REQ_ON_GNSS_SIMPLE_RECV,

    CMODULE_REQ_ON_REQ_ID_ACK,

    /// @Protocol
    CMODULE_REQ_PT_ID_START = 100,
} cModule_RequestType_t;
typedef union {
    void *ptr;
    float f;
    uint32_t u32;
    int32_t i32;
    uint16_t u16[2];
    int16_t i16[2];
    uint8_t u8[4];
    int8_t i8[4];
} cModule_RequestVar_t;
typedef cModule_RequestVar_t (*cModule_OnRequestCallback_t)(cModule_RequestType_t type, cModule_RequestVar_t var, ...);

typedef enum {
    CMODULE_STATE_NO_CON = 0,
    CMODULE_STATE_CONING = 1,
    CMODULE_STATE_DISCON = 2, // Sleep
    CMODULE_STATE_CON_IPV4 = 3,

    CMODULE_STATE_FAILED_RES = 100,
    CMODULE_STATE_HAL_FAILED = 101,
    CMODULE_STATE_NO_COM_ID = 102,
    CMODULE_STATE_FAILED_REG = 103,
    CMODULE_STATE_FAILED_CON = 104,
} cModule_State_t;

typedef enum {
    CMODULE_PROCESS_STEP_AT = 0,
    CMODULE_PROCESS_STEP_INIT_COMPLETED = 99,
    CMODULE_PROCESS_STEP_RUNNING = 100,
} cModule_ProcessStep_t;

typedef enum {
    TRANSMIT_PACK_REQ_ID_TX,
    TRANSMIT_PACK_REQ_ID_RX,

    TRANSMIT_PACK_REQ_ID_START = 32,
    TRANSMIT_PACK_REQ_ID_ATD,
    TRANSMIT_PACK_REQ_ID_HTTP_GET,
    TRANSMIT_PACK_REQ_ID_NTP_UPDATE,
    TRANSMIT_PACK_REQ_ID_GNSS_UPDATE_ONCE,
} cModule_TransmitPackageRequestId_t;

typedef struct {
    struct {
        uint8_t packHasVar: 1;
        uint8_t varIsObject: 1;
        uint8_t varIsLengthItem: 1;

        uint8_t packIsDynData: 1;
        uint8_t packIsDynMqttMsg: 1;
        uint8_t packIsDynHttpMsg: 1;
        uint8_t packIsDynTcpIpData: 1;
        
        cModule_TransmitPackageRequestId_t requestId;
        
        uint8_t __delayAccess: 2;
    } flag;

    uint16_t payloadLength;
    void *payload;

    uint16_t varLength;
    cModule_RequestVar_t var;

    struct {
        struct {
            uint16_t msgId;

            uint16_t sendBeforeDelaySec;
            uint8_t sendRetryCount;
            uint8_t sendRetryCountReload;
            uint16_t sendRetrySec;
            uint8_t sendFailedAfterReboot;

            uint8_t hasConfirmFrame: 1;
            uint8_t waitConfirmFrame: 1;
            uint8_t waitConfirmFrameRecved: 1;
            uint8_t waitConfirmFrameTimeoutSec;
            uint8_t waitConfirmFrameTimeoutCount;
            uint8_t waitConfirmFrameTimeoutAfterReboot;
            qSTimer_t waitConfirmFrameTimer;
        } gen;
    } aux;

    klist_t list;
} cModule_TransmitPackageInfo_t;

typedef struct {
    int16_t rsrp;
    int16_t sinr;
    int16_t pci;
    int16_t eci;
    int32_t cellId;
} cModule_SignalExtInfo_t;

typedef struct {
    char *utc;              // (hhmmss.sss) UTC时间，时分秒.毫秒，位宽10
    char *latitude;         // (ddmm.mmmmN/S) 纬度，dd:度，mm.mmmm:分，N/S:北纬/南纬，保留4位小数
    char *longtitude;       // (dddmm.mmmmE/W) 经度，ddd:度，mm.mmmm:分，E/W:东经/西经，保留4位小数
    char *hdop;             // (x.x) 水平精度因子，保留1位小数
    char *altitude;         // (x.x) 海拔高度，单位:米，保留1位小数
    char *fix;              // (n) 定位类型。1:未定位，2:2D定位，3:3D定位；位宽1。
    char *cog;              // (ddd.dd) 运动角度，真北参照系，单位:度，保留2位小数。
    char *spkm;             // (x.x) 水平运动速度，单位Km/h，保留1位小数
    char *spkn;             // (x.x) 水平运动速度，单位Knots，保留1位小数
    char *date;             // (ddmmyy) 当前日期，日月年，位宽6
    char *nsat;             // (nn) 参与定位的卫星数量，位宽2
    char *dtype;            // (n) 差分定位标识，位宽1。0:无效，1:单点定位，2:差分定位
} cModule_GnssSimpleData_t;

typedef struct {
    void (*onInstanceInit)(void);
    void (*onInstancePmu)(uint8_t wakeup1sleep0);

    int32_t (*onCommandAT)(void);
    int32_t (*onCommandCheckAT)(void);

    void (*onReboot)(uint8_t isPowerUpRequest);
    void (*onLoop)(void);
    void (*onInitStep)(uint8_t step);
    void (*onResetStack)(void);

    int32_t (*onPtPackIdReceived)(cModule_TransmitPackageInfo_t *info);
    int32_t (*onPtPackIdTransmit)(cModule_TransmitPackageInfo_t *info);
    int32_t (*onPtPackIdCustom)(cModule_TransmitPackageInfo_t *info);
    int32_t (*onPtPackPayloadFree)(cModule_TransmitPackageInfo_t *info, bool isForce);
} cModule_Callback_t;

typedef struct {
    const cModule_Callback_t *callback;
    void *cmmpu;

    struct {
        uint8_t tagId;
        Rilat_Instance_t instance;
        uint8_t *recvBuffer;
    } rilat;

    struct {
        uint8_t processStep;
        volatile uint32_t urcResponseFlags;
        cModule_State_t currentState;
    } state;

    struct {
        char imei[16];
        char imsi[16];
        char iccid[21];

        cModule_SignalExtInfo_t sei;
        int8_t rssi;
    } modemInfo;

    struct {
        uint8_t nextPollSec;
        qSTimer_t pollTimer;
        qSTimer_t sendAccessTimer;
        qSTimer_t blockTimer;
        qSTimer_t txPackageOverFlowTimer;
        uint32_t timeStamp;
        uint8_t failedCount;
        uint8_t initRetryCount;

        union {
            uint8_t _;
            struct {
                uint8_t passiveRecvMode: 1;
                uint8_t pollBlock: 1;
                uint8_t pmuSupport: 1;
                uint8_t hasInit: 1;
                uint8_t hasWakeupSuccessHappend: 1;
                uint8_t packageIsFull: 1;
            };
        } flag;
    } aux;

    struct {
        void *mutex;
        klist_t packageReqList;
        uint8_t queneReqCount;

        klist_t packageRepList;
        uint8_t queneRepCount;
    } transmit;
} cModule_Instance_t;

typedef struct {
    cModule_Instance_t *ins;
} cModule_InstanceConst_t;

typedef struct {
    char *topic;
    bool topicConstant: 1;

    char *payload;
    bool isPayloadJson: 1;
    bool isPayloadString: 1;

    uint16_t len;
    uint8_t qos;

    uint32_t timeout;
    
    struct {
        bool isWritenSuccess: 1;
        void (*onDirectWrite)(Rilat_Instance_t *ins, uint32_t packMsgId);
        uint32_t (*onDorectGetLength)(Rilat_Instance_t *ins, uint32_t packMsgId);
        void (*onDirectWriteResponse)(Rilat_Instance_t *ins, uint32_t packMsgId, bool isSuccess, bool isForce);
    } writer;
} cModule_ProtocolMqttMessage_t;

typedef struct {
    char *host;
    bool hostContant: 1;
    uint16_t port;
    char *path;
    bool pathContant: 1;

    char *payload;
    uint16_t payloadLength;

    bool isWritenSuccess: 1;
    void (*onDirectWrite)(Rilat_Instance_t *ins, uint32_t packMsgId);
    uint32_t (*onDorectGetLength)(Rilat_Instance_t *ins, uint32_t packMsgId);
    void (*onDirectWriteResponse)(Rilat_Instance_t *ins, uint32_t packMsgId, bool isSuccess, bool isForce);
} cModule_ProtocolHttpMessage_t;

typedef struct {
    char *payload;
    uint16_t payloadLength;

    struct {
        bool isWritenSuccess: 1;
        void (*onDirectWrite)(Rilat_Instance_t *ins, uint32_t packMsgId);
        uint32_t (*onDorectGetLength)(Rilat_Instance_t *ins, uint32_t packMsgId);
        void (*onDirectWriteResponse)(Rilat_Instance_t *ins, uint32_t packMsgId, bool isSuccess, bool isForce);
    } writer;
} cModule_ProtocolGenMessage_t;

typedef struct {
    char *payload;
    uint16_t payloadLength;
    
    char *host;
    bool hostContant: 1;
    uint16_t port;
    
    bool dStream: 1;
    uint32_t timeout;

    struct {
        bool isWritenSuccess: 1;
        void (*onDirectWrite)(Rilat_Instance_t *ins, uint32_t packMsgId);
        uint32_t (*onDirectGetLength)(Rilat_Instance_t *ins, uint32_t packMsgId);
        void (*onDirectWriteResponse)(Rilat_Instance_t *ins, uint32_t packMsgId, bool isSuccess, bool isForce);
    } writer;
} cModule_ProtocolTcpIpMessage_t;

typedef struct {
    cModule_TransmitPackageRequestId_t id;
    uint16_t messageId;
    int32_t rc;
    cModule_RequestVar_t var;
} cModule_ReqIdAck_t;

typedef struct {
    const char *number[5];
    uint8_t numberCount;
} cModule_ReqId_Atd_t;

typedef struct {
    const char *url;
    uint32_t startAt;
    uint32_t readLength;
    uint32_t totalSize;
} cModule_ReqId_HttpGetReq_t;

typedef struct {
    void *data;
    uint32_t startAt;
    uint32_t dataSize;
    uint32_t totalSize;
} cModule_ReqId_HttpGetPackage_t;

/*@}*/

/**
 * @addtogroup ProtectFunc
 * @note none
 */

/*@{*/

extern uint8_t _cModule_a2x(const char c);
extern void _cModule_convertMacStr2Hex(const char *str, uint8_t *hex);

extern int32_t _cModule_wait(cModule_Instance_t *ins, uint32_t t, bool withRilatPoll);

extern int32_t _cModule_isAllocReqPackAccess(cModule_Instance_t *ins);
extern int32_t _cModule_isAllocRepPackAccess(cModule_Instance_t *ins);
extern void _cModule_freePack(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info, bool isForce);

extern void _cModule_packMutexLock(cModule_Instance_t *ins, bool lock);

extern uint32_t *_cModule_getGenMsgId(void);

/*@}*/

/**
 * @addtogroup Base func
 * @note none
 */

/*@{*/

extern void cModule_init(void);
extern void cModule_sleep(uint8_t nextSleepLevel);
extern void cModule_loop(void *specialInstance);

extern void cModule_blockFuncPoll(cModule_Instance_t *ins, uint32_t block);
extern void cModule_resetStack(cModule_Instance_t *ins);

extern int32_t cModule_directWrite(cModule_Instance_t *ins, uint8_t *data, uint16_t dataLength);

extern void cModule_enablePmu(cModule_Instance_t *ins, bool en);
extern uint8_t cModule_getSleepStatus(cModule_Instance_t *ins);

extern uint8_t cModule_isBlocking(cModule_Instance_t *ins);
extern uint8_t cModule_getProcessStep(cModule_Instance_t *ins);
extern cModule_State_t cModule_getNetState(cModule_Instance_t *ins);
extern uint32_t cModule_getTimeStamp(cModule_Instance_t *ins);

extern uint8_t cModule_getRssi(cModule_Instance_t *ins);
extern char* cModule_getModuleComId(cModule_Instance_t *ins);
extern char* cModule_getModuleUId(cModule_Instance_t *ins);
extern char *cModule_getModuleIccid(cModule_Instance_t *ins);
extern cModule_SignalExtInfo_t *cModule_getModuleSignalExtInfo(cModule_Instance_t *ins);

/*@}*/

/**
 * @addtogroup Instance
 * @note none
 */

/*@{*/

extern cModule_Instance_t *cModule_getInstance_cu101(void);
extern cModule_Instance_t *cModule_getInstance_ec800m(void);
extern cModule_Instance_t *cModule_getInstance_ml307a(void);
extern cModule_Instance_t *cModule_getInstance_nt26e(void);

extern cModule_Instance_t *cModule_getInstance_mb26(void);
extern cModule_Instance_t *cModule_getInstance_nb81(void);
extern cModule_Instance_t *cModule_getInstance_nb63(void);

extern cModule_Instance_t *cModule_getInstance_xg307dat(void);
extern cModule_Instance_t *cModule_getInstance_xbg01(void);

/*@}*/

/**
 * @addtogroup ProtocolCallback
 * @note none
 */

/*@{*/

extern void _cModule_onProtocolMqttConnectAlloc(cModule_Instance_t *ins,
                                                char **borkerIp,
                                                char **clientId,
                                                char **userName, char **password);
extern void _cModule_onProtocolMqttConnect(cModule_Instance_t *ins,
                                           char **borkerIp, uint16_t *port,
                                           char **clientId,
                                           char **userName, char **password);
extern void _cModule_onProtocolMqttConnectFree(cModule_Instance_t *ins,
                                               char **borkerIp,
                                               char **clientId,
                                               char **userName, char **password);

extern uint8_t _cModule_onProtocolMqttSubCount(cModule_Instance_t *ins);
extern void _cModule_onProtocolMqttSubAlloc(cModule_Instance_t *ins, char **topic, uint8_t index);
extern void _cModule_onProtocolMqttSub(cModule_Instance_t *ins, char **topic, uint8_t *qos, uint8_t index);
extern void _cModule_onProtocolMqttSubFree(cModule_Instance_t *ins, char **topic, uint8_t index);
                                               
extern void _cModule_onProtocolTcpConnectAlloc(cModule_Instance_t *ins,
                                                char **borkerIp);
extern void _cModule_onProtocolTcpConnect(cModule_Instance_t *ins,
                                                char **borkerIp, uint16_t *port);
extern void _cModule_onProtocolTcpConnectFree(cModule_Instance_t *ins,
                                                char **borkerIp);

extern int32_t _cModule_onProtocolTransmited(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info);
extern int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info);

/*@}*/


#endif
