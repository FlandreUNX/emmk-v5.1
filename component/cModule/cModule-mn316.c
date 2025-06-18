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

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "MN316"

#define CASSERT(err) do { \
        if (!(err)) { \
            ESP_LOGW(TAG, "<%s> assert failed, at 0x%08x, expression: %s", \
                     "", (intptr_t)__builtin_return_address(0) - 3, __ASSERT_FUNC); \
            assert(0 && #err); \
        } \
    } while(0)

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
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)

extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup Config
 * @note none
 */

/*@{*/

/// @-1=Disable
#if CONFIG_CMODULE_INSTANCE_X_ENABLE == 02
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    02
#else
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    -1
#endif

#define CONFIG_SERIAL_IF_PATH                              "tty/COMX"
// #define CONFIG_PWR_PIN_PATH                                "gpio/MODEM_PWR"
#define CONFIG_SERIAL_PHY_ENABLE                           (1)

#define MODULE_INIT_LWM2M                  (2)
#define MODULE_INIT_HTTP                   (1)
#define MODULE_INIT_MQTT                   (0)
#define CONFIG_INIT_MODE                   MODULE_INIT_LWM2M

///
/// BC
///
#define HTTP_ADDRESS     "27.128.224.148"
#define HTTP_PORT        23457

///
/// AEP_LWM2_CA
///
#define LWM2M_ADDRESS       "221.229.214.202"
#define LWM2M_PORT          "5683"

/*@}*/

#if CONFIG_CMODULE_INSTANCE_TAG_ID >= 0

#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT != 0
CMPMU_DEFINE();
#endif

/**
 * @addtogroup Protocol
 * @note none
 */

/*@{*/

/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */

/*@{*/



/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */

/*@{*/

static void onInstanceInit(void);

static void onInstancePmu(uint8_t wakeup1sleep0);

static int32_t checkAT(void);

static int32_t AT(void);

static int32_t checkNTP(void);

static void onReboot(void);

static void onLoop(void);

static void onInitStep(uint8_t step);

static void onResetStack(void);

static int32_t onPtPackIdReceived(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackIdTransmit(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackPayloadFree(cModule_TransmitPackageInfo_t *info);

static const cModule_Callback_t mCallback = {
        .onInstanceInit = onInstanceInit,
        .onInstancePmu = onInstancePmu,

        .onCommandAT = AT,
        .onCommandCheckAT = checkAT,
        .onCommandCheckNTP = checkNTP,

        .onReboot = onReboot,
        .onLoop = onLoop,
        .onInitStep = onInitStep,
        .onResetStack = onResetStack,

        .onPtPackIdReceived = onPtPackIdReceived,
        .onPtPackIdTransmit = onPtPackIdTransmit,
        .onPtPackIdCustom = onPtPackIdCustom,
        .onPtPackPayloadFree = onPtPackPayloadFree,
};

static uint8_t mModuleRxBuffer[CONFIG_CMODULE_RILAT_MEM_RECV_BUFFER_SIZE] = {0};
static cModule_Instance_t mModuleInstance = {
        .callback = &mCallback,
#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT != 0
        .cmmpu = &CMPMU,
#else
        .cmmpu = NULL,
#endif
        .rilat = {
                .tagId = CONFIG_CMODULE_INSTANCE_TAG_ID,
                .recvBuffer = mModuleRxBuffer,
        },
};
const __CMODULE_X_SECTION cModule_InstanceConst_t mModuleInstanceConst = {
        .ins = &mModuleInstance,
};

/*@}*/

/**
 * @addtogroup AtCommand
 * @note none
 */

/*@{*/

static int32_t checkAT(void) {
    static const char AT_A[] = "AT";

    uint8_t isSuccess = 0;
    Rilat_AtResponse_t *response = NULL;

#if CONFIG_SERIAL_PHY_ENABLE == 1
    kduart_flush(((kduart_t *) mModuleInstance.rilat.instance.userData));
#endif
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    for (uint8_t i = 0; i < 20; i++) {
        rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, &response, 500);
        if (response != NULL && response->success) {
            rilat_freeResponse(&mModuleInstance.rilat.instance, response);
            response = NULL;

            isSuccess = 1;
            break;
        }

        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        response = NULL;

        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_BLOCK) {
            isSuccess = 0;
            break;
        }
    }

    if (isSuccess) {
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+IPR=9600", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+IPR=9600", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+CPSMS=0", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+CEDRXS=0", NULL, 1000);
    } else {
        mModuleInstance.aux.initRetryCount++;
        return -1;
    }

    return 0;
}


static int32_t AT(void) {
    Rilat_AtResponse_t *response = NULL;

    rilat_writeLine(&mModuleInstance.rilat.instance, "AT", &response, 1000);
    if (response == NULL || response->success == 0) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return -1;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
}


static inline int32_t ATE0(void) {
    static const char AT_ATE0[] = "ATE0";

    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);

    return 0;
}


static int32_t checkCPIN(void) {
    static const char AT_CPIN_A[] = "AT+CPIN?";

    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CPIN_A, "+CPIN:", &response, 2000) != 0 ||
        response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    if (strstr(response->intermediates->line, "READY") == NULL) {
        goto _l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t checkCGREG(void) {
    static const char AT_CGREG_A[] = "AT+CEREG?";

    char *line = NULL;
    int32_t numberResult = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CGREG_A, "+CEREG:", &response, 500) != 0 ||
        response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }

    if (klAttoken_getNextInt(&line, &numberResult) != 0) {
        goto _l_retryExit;
    }

    if (numberResult != 0) {
        goto _l_retryExit;
    }

    if (klAttoken_getNextInt(&line, &numberResult) != 0) {
        goto _l_retryExit;
    }

    if (!(numberResult == 1 || numberResult == 5)) {
        goto _l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);

    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t queryIMEI(void) {
    static const char AT_CGSN[] = "AT+CGSN=1";

    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CGSN, "+CGSN:", &response, 1000) != 0 
            || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    
    char *line = response->intermediates->line;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    while (isspace((unsigned char) *line) != 0) {
        line++;
    }
    strcpy(mModuleInstance.modemInfo.imei, line);

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t queryIMSI(void) {
    static const char AT_CIMI[] = "AT+CIMI";

    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CIMI, "+CIMI:", &response, 1000) != 0 || response == NULL ||
        response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    
    char *line = response->intermediates->line;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    while (isspace((unsigned char) *line) != 0) {
        line++;
    }
    bool vailed = true;
    memset(mModuleInstance.modemInfo.imsi, 0x00, sizeof(mModuleInstance.modemInfo.imsi));
    strcpy(mModuleInstance.modemInfo.imsi, line);
    for (uint32_t i = 0; i < strlen(mModuleInstance.modemInfo.imsi); i++) {
        if (!isdigit((int) mModuleInstance.modemInfo.imsi[i])) {
            vailed = false;
            break;
        }
    }
    if (!vailed) {
        goto _l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t readIccid(void) {
    static const char AT_CSQ[] = "AT+NCCID";

    int8_t rc = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CSQ, "+NCCID:", &response, 1000) != 0 ||
        response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    char *line = response->intermediates->line;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    while (isspace((unsigned char) *line) != 0) {
        line++;
    }
    memset(mModuleInstance.modemInfo.iccid, 0x00, sizeof(mModuleInstance.modemInfo.iccid));
    strncpy(mModuleInstance.modemInfo.iccid, line, 20);

    rc = 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return rc;
}


static int8_t readSignal(void) {
    static const char AT_CSQ[] = "AT+CSQ";

    int8_t dbm = -113;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CSQ, "+CSQ:", &response, 1000) != 0 ||
        response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    char *line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }

    int32_t rssi = 99;
    if (klAttoken_getNextInt(&line, &rssi) == 0) {
        dbm = -113 + (rssi * 2);
    } else {
        goto _l_retryExit;
    }

#if 1
    mModuleInstance.modemInfo.rssi = rssi;
#else
    mNetSignal = dbm;
#endif

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return dbm;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return dbm;
}


static int32_t checkNTP(void) {
    char *line = NULL;
    char *dtStr = NULL;
    Rilat_AtResponse_t *response = NULL;
    klDateTime_SampleTm_t stm = {0};

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, "AT+CCLK?", "+CCLK:", &response, 1000) != 0) {
        // LOG_W("0");
        goto _l_retryExit;
    }

    if (response == NULL || response->success == 0 || response->intermediates == NULL) {
        // LOG_W("1");
        goto _l_retryExit;
    }

    line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        // LOG_W("2");
        goto _l_retryExit;
    }

    if (klAttoken_getNextString(&line, &dtStr, NULL) != 0) {
        // LOG_W("3");
        goto _l_retryExit;
    }

    {
        // static const char * const NTP_FORMAT = "%d/%d/%d,%d:%d:%d%*[+-]%d";
        int32_t i32 = 0;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != '/') {
            (dtStr)++;
        }
        if (*dtStr == '/') {
            dtStr++;
        }
        stm.year = i32 + 2000;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != '/') {
            (dtStr)++;
        }
        if (*dtStr == '/') {
            dtStr++;
        }
        stm.mon = i32;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != ',') {
            (dtStr)++;
        }
        if (*dtStr == ',') {
            dtStr++;
        }
        stm.day = i32;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != ':') {
            (dtStr)++;
        }
        if (*dtStr == ':') {
            dtStr++;
        }
        stm.hour = i32;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != ':') {
            (dtStr)++;
        }
        if (*dtStr == ':') {
            dtStr++;
        }
        stm.min = i32;

        stm.sec = (*dtStr - '0') * 10 + (*(dtStr + 1) - '0');
        dtStr += 2;

        if (*dtStr == '+') {
            dtStr++;
            i32 = atoi(dtStr);
        } else {
            dtStr++;
            i32 = 0 - atoi(dtStr);
        }

        uint32_t ts = klDateTime_mktime(&stm);

        if (strstr(line, "GMT")) {
            ts += i32 * 60 * 60;
        } else {
            ts += (i32 / 4) * 60 * 60;
        }

        // To UTC+0
        mModuleInstance.aux.timeStamp = ts - 28800;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static void reboot(void) {
    static const char AT_RST[] = "AT+NRB";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_RST, NULL, 500);
}


static int32_t atcfun1(void) {
    return rilat_writeLine(&mModuleInstance.rilat.instance, "AT+CFUN=1", NULL, 3000);
}


#if CONFIG_INIT_MODE == MODULE_INIT_HTTP

static void httpDelete(void) {
    static const char AT_MHTTPDEL[] = "AT+HTTPCLOSE=0";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_MHTTPDEL, NULL, 3000);
}


static int32_t httpPost(const char *ip, const uint16_t port, const char *path, void *data, uint16_t dataLength, bool directWriteMode) {
    static const char AT_MHTTPREQUEST[] = "AT+HTTPSEND=0,1,";
    static const char AT_MHTTPCONTENT[] = "AT+HTTPCONTENT=0,\"";
    static const char AT_MHTTPCREATE[] = "AT+HTTPCREATE=\"http://";
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_PUB_SUCCESS | CMODULE_URC_FLAG_PUB_FAILED);
    
    httpDelete();
    
    int32_t rc = -1;
    char *buffer = calloc(1, 32);
    if (buffer == NULL) {
        return -1;
    }
    
    // CreateClient
    if (port != 0) {
        klPtf_sprintf(buffer, ":%d\"", port);
    } else {
        klPtf_sprintf(buffer, "\"");
    }
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_MHTTPCREATE, strlen(AT_MHTTPCREATE), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) ip, strlen(ip), 0);
    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, 
                buffer, "+HTTPCREATE:", &response, 15000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    // SetHeader
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+HTTPHEADER=0,\"Connection: close\",0", NULL, 3000);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+HTTPHEADER=0,\"Content-Type: text/plain\",0", NULL, 3000);
    
    // SetContent, HEX-STR
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_MHTTPCONTENT, strlen(AT_MHTTPCONTENT), 0);
    if (directWriteMode) {
        Rilat_PduDirectWrite_t call = data;
        call(&mModuleInstance.rilat.instance);
    } else {
        char hexStr[2] = {0};
        for (uint32_t i = 0; i < dataLength; i++) {
            klStr_hex2str(&data[0], 1, hexStr);
            rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) hexStr, 2, 0);
        }   
    }
    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, "\",1", "+HTTPCONTENT:",  &response, 30000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }   
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    // Post
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_MHTTPREQUEST, strlen(AT_MHTTPREQUEST), 0);
    klPtf_sprintf(buffer, "\"%s\"", path);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, buffer, "CONNECT OK", &response, 30000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    } 
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    free(buffer);
    buffer = NULL;

    httpDelete();
    
    rc = 0;
    
    return rc;

    _l_retryExit:
    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    
    httpDelete();
    
    return rc;
}


#endif

#if CONFIG_INIT_MODE == MODULE_INIT_LWM2M

static int32_t lwm2mInit(uint32_t commandTimeout) {
    static const char AT_CTM2MSETPM[] = "AT+MCWCREATE=" LWM2M_ADDRESS "," LWM2M_PORT;
    static const char AT_CTM2MREG[] = "AT+MCWOPEN=1";
    static const char MCWCFGEX[] = "AT+MCWCFGEX=0,1";
    
    Rilat_AtResponse_t *response = NULL;

    mModuleInstance.state.urcResponseFlags &= ~(
        CMODULE_URC_FLAG_CONN_SUCCESS | CMODULE_URC_FLAG_CONN_FAILED | CMODULE_URC_FLAG_CONN_RETRY | CMODULE_URC_FLAG_STACK_READY | CMODULE_URC_FLAG_NEED_RESET
    );
    
    if (rilat_writeLine(&mModuleInstance.rilat.instance, AT_CTM2MSETPM, &response, commandTimeout) != 0 || response == NULL || response->success == 0) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return -1;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    if (rilat_writeLine(&mModuleInstance.rilat.instance, AT_CTM2MREG, NULL, commandTimeout) != 0) {
        return -1;
    }
    
    if (rilat_writeLine(&mModuleInstance.rilat.instance, MCWCFGEX, NULL, commandTimeout) != 0) {
        return -1;
    }

    klTmcd_t wait;
    klTmcd_countDownMs(&wait, commandTimeout);
    while (!klTmcd_isExpired(&wait)) {
        rilat_poll(&mModuleInstance.rilat.instance);
        
        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CONN_SUCCESS 
                && mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_READY) {
            break;
        } else if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CONN_FAILED) {
            return -1;
        }
    }
    if (klTmcd_isExpired(&wait)) {
        return -1;
    }

    return 0;
}


static int32_t lwm2mDelete(uint32_t commandTimeout) {
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MCWCLOSE", NULL, commandTimeout);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MCWDELETE", NULL, commandTimeout);
    
    return 0;
}


static int32_t lwm2mPost(uint8_t *data, uint16_t dataLength, uint32_t commandTimeout, uint8_t usingRAI) {
    static const char AT_CTM2MSEND[] = "AT+MCWSEND=%d,";
    
    ASSERT(data != NULL);
    ASSERT(dataLength != 0);

    Rilat_AtResponse_t *response = NULL;
    char *sendBuffer = NULL;

    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_PUB_SUCCESS | CMODULE_URC_FLAG_PUB_FAILED);
    
    AT();
    AT();
    _cModule_wait(&mModuleInstance, 500, true);

    sendBuffer = rilat_calloc(&mModuleInstance.rilat.instance, 1, 16); 
    if (sendBuffer == NULL) {
        goto l_errorReturn;
    }
    
    klPtf_sprintf(sendBuffer, AT_CTM2MSEND, dataLength);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) sendBuffer, strlen(sendBuffer), 0);
    
    uint16_t writenLength = 0;
    do {
        uint32_t wlen = dataLength - writenLength;
        if (wlen > 8) {
            wlen = 8;
        }
        
        memset((void *) sendBuffer, 0, 8);
        klStr_hex2str((uint8_t *) data + writenLength, wlen, sendBuffer);
        rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) sendBuffer, wlen * 2, 0);
        
        writenLength += wlen;
    } while (writenLength != dataLength);

    if (rilat_writeLine(&mModuleInstance.rilat.instance, usingRAI ? ",1,0,2" : ",1,0,1", &response, commandTimeout) != 0 || response == NULL || response->success == 0) {
        goto l_errorReturn;
    }
    rilat_free(&mModuleInstance.rilat.instance, sendBuffer);
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    
    klTmcd_t wait;
    klTmcd_countDownMs(&wait, commandTimeout);
    while (!klTmcd_isExpired(&wait)) {
        rilat_poll(&mModuleInstance.rilat.instance);
        
        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_SUCCESS) {
            break;
        } else if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_FAILED) {
            return -1;
        }
    }
    if (klTmcd_isExpired(&wait)) {
        return -1;
    }
    
    return 0;

l_errorReturn:
    rilat_free(&mModuleInstance.rilat.instance, sendBuffer);
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static int32_t lwm2mResponse(uint32_t id) {
    return 0;
}


static int32_t lwm2mUpdate(uint32_t commandTimeout) {
    static const char * const AT_CMD_MIPLUPDATE = "AT+MCWUPDATE";
    
    int32_t rc = -1;
    int32_t waitCount;
    Rilat_AtResponse_t *response = NULL;
    
    mModuleInstance.state.urcResponseFlags &= ~CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS;

    if (rilat_writeLine(&mModuleInstance.rilat.instance, AT_CMD_MIPLUPDATE, &response, commandTimeout) != 0 || response == NULL || response->success == 0) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return -1;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    klTmcd_t wait;
    klTmcd_countDownMs(&wait, commandTimeout);
    while (!klTmcd_isExpired(&wait)) {
        rilat_poll(&mModuleInstance.rilat.instance);
        
        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS) {
            break;
        }
    }
    if (klTmcd_isExpired(&wait)) {
        return -1;
    }
    
    return 0;
}

#endif


static void powerDown(void) {
//    static const char AT_QPOWD[] = "AT+CPOF=2";
//    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QPOWD, NULL, 1000);
}


static void __onHalSleep(void) {

}


static void __onHalWakeup(void) {

}


static void __onRilatInit(void) {

}


static void __onRilatFinalize(void) {

}


static void __onSoftWakeup(void) {

}


static void __onSoftSleep(void) {
    AT();
    AT();
#if CONFIG_INIT_MODE == MODULE_INIT_HTTP
    httpDelete();
#endif
#if CONFIG_INIT_MODE == MODULE_INIT_LWM2M
    lwm2mDelete(10000);
#endif
    powerDown();
}


static void onResetStack(void) {
#if CONFIG_INIT_MODE == MODULE_INIT_HTTP
    httpDelete();
#endif
#if CONFIG_INIT_MODE == MODULE_INIT_LWM2M
    lwm2mDelete(10000);
#endif
    powerDown();
}


static void onReboot(void) {
#ifdef CONFIG_PWR_PIN_PATH
    kdGPIO_t *pwrPin = KDINSTANCE(GPIO, CONFIG_PWR_PIN_PATH);
    ASSERT(pwrPin != NULL);
    kdgpio_init((kdGPIO_t *) pwrPin);
    kdgpio_powerUp((kdGPIO_t *) pwrPin, 1);
    kdgpio_output((kdGPIO_t *) pwrPin, 1);
    _cModule_wait(&mModuleInstance, 750, 0);
    kdgpio_output((kdGPIO_t *) pwrPin, 0);
    kdgpio_powerDown((kdGPIO_t *) pwrPin);
    kdgpio_finalize((kdGPIO_t *) pwrPin);
#endif
}


static void onLoop(void) {
    if (klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
        klTmcd_countDown(&mModuleInstance.aux.pollTimer, 60);

        AT();
        AT();
        if (checkNTP() != 0 || readSignal() <= -113) {
            if (++mModuleInstance.aux.initRetryCount > 2) {
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_NEED_RESET;
            }
        } else {
            CREQUEST(ON_NTP_UPDATE, (void *) &mModuleInstance,
                     mModuleInstance.aux.timeStamp);
        }
    }
}

/*@}*/

/**
 * @addtogroup EC800E-URC
 * @note none
 */

/*@{*/

#if CONFIG_INIT_MODE == MODULE_INIT_HTTP

#endif

#if CONFIG_INIT_MODE == MODULE_INIT_LWM2M
RILAT_COMMAND_MATCH_DEFINE(MCWEVENT, "+MCWEVENT:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    char *line = (char *) data;
    
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    
    uint32_t event = 0;
    if (klAttoken_getNextUInt(&line, &event) != 0) {
        return 0;
    }
    if (event == 1) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_CONN_SUCCESS;
    } else if (event == 7) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_STACK_READY;
    } else if (event == 9 || event == 2) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_PUB_SUCCESS;
    } else if (event == 3) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS;
    } else if (event == 4) {
        mModuleInstance.state.urcResponseFlags &= ~CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS;
    } else if (event == 8 || event == 12) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_CONN_FAILED | CMODULE_URC_FLAG_NEED_RESET;
        mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_STACK_READY);
    }  else if (event >= 14) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_NEED_RESET;
        mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_STACK_READY);
    } 
    return 0;
}

RILAT_COMMAND_MATCH_DEFINE(MCWREAD, "+MCWREAD:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    char *line = (char *) data;
    
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    
    uint32_t payloadLength = 0;
    char *payload = NULL;
    
    if (klAttoken_getNextUInt(&line, &payloadLength) != 0) {
        return 0;
    }
    
    while (*line != '\0' && isspace(*line)) {
        (line)++;
    }
    
    payloadLength = ALIGN_UP(strlen(line) + 1, 16);
    payload = calloc(1, payloadLength);
    if (payload == NULL) {
        return -1;
    }
    uint32_t stLen = strlen(payload) - 1;
    if (stLen != payloadLength) {
        if (stLen < CONFIG_CMODULE_BUFFER_POS_LEN) {
            payloadLength = stLen;
        }
    }

    payloadLength = strlen(line);
    memcpy(payload, line, strlen(line));
    
    if (!_cModule_isAllocRepPackAccess(&mModuleInstance)) {
        free(payload);
        return -1;
    }
    _cModule_packMutexLock(&mModuleInstance, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    if (pack == NULL) {
        goto l_errorExit;
    }
    pack->payloadLength = payloadLength;
    pack->payload = payload;
    pack->flag.packIsDynData = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_RX;
    klist_init(&pack->list);
    klist_addTail(&mModuleInstance.transmit.packageRepList, &pack->list);
    mModuleInstance.transmit.queneRepCount++;
    _cModule_packMutexLock(&mModuleInstance, false);
    
    return 0;
    
    l_errorExit:
    _cModule_packMutexLock(&mModuleInstance, false);
    if (pack != NULL) {
        free(pack);
    }
    if (payload != NULL) {
        free(payload);
    }
    return -1;
}
#endif

/*@}*/

/**
 * @addtogroup EC800E-Init
 * @note none
 */

/*@{*/

static void onInitStep(uint8_t step) {
    switch (step) {
        case 1: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            ATE0();
            mModuleInstance.aux.initRetryCount = 0;
            mModuleInstance.state.processStep++;
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
            break;
        }
        case 2: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            if (checkCPIN() == 0) {
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.processStep++;
                klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
                break;
            } else if (mModuleInstance.aux.initRetryCount > 10) {
                mModuleInstance.state.currentState = CMODULE_STATE_NO_COM_ID;
                CREQUEST(ON_STATE_CHANGED, (void *) &mModuleInstance, (uint32_t) mModuleInstance.state.currentState);
                mModuleInstance.aux.initRetryCount = CMODULE_ERR_COUNT;
                mModuleInstance.state.processStep = 0;
            }
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 1000);
            break;
        }
        case 3: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            if (queryIMEI() == 0
                && queryIMSI() == 0
                && readIccid() == 0) {
                CREQUEST(ON_NTP_UPDATE, (void *) &mModuleInstance, mModuleInstance.aux.timeStamp);
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.processStep++;
                klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
                break;
            } else if (mModuleInstance.aux.initRetryCount > 10) {
                mModuleInstance.aux.initRetryCount = CMODULE_ERR_COUNT;
                mModuleInstance.state.processStep = 0;
            }
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 1000);
            break;
        }
        case 4: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            int32_t rc = atcfun1();
            if (rc == 0) {
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.processStep++;
                klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
                break;
            } else {
                mModuleInstance.aux.initRetryCount = CMODULE_ERR_COUNT;
                mModuleInstance.state.processStep = 0;
            }
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 1000);
            break;
        }
        case 5: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            if (checkCGREG() == 0) {
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.processStep++;
                break;
            } else if (mModuleInstance.aux.initRetryCount > 120) {
                mModuleInstance.state.currentState = CMODULE_STATE_FAILED_REG;
                CREQUEST(ON_STATE_CHANGED, (void *) &mModuleInstance, (uint32_t) mModuleInstance.state.currentState);
                mModuleInstance.aux.initRetryCount = CMODULE_ERR_COUNT;
                mModuleInstance.state.processStep = 0;
            }
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
            break;
        }
        case 6: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            readSignal();
            if (checkNTP() == 0) {
                CREQUEST(ON_NTP_UPDATE, (void *) &mModuleInstance, mModuleInstance.aux.timeStamp);
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.processStep++;
                klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
                break;
            } else if (mModuleInstance.aux.initRetryCount > 10) {
                mModuleInstance.aux.initRetryCount = CMODULE_ERR_COUNT;
                mModuleInstance.state.processStep = 0;
            }
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 1000);
            break;
        }
#if CONFIG_INIT_MODE == MODULE_INIT_HTTP
        case 7: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            mModuleInstance.aux.initRetryCount = 0;
            mModuleInstance.state.processStep = CMODULE_PROCESS_STEP_INIT_COMPLETED;
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
            mModuleInstance.aux.nextPollSec = 120;
            break;
        }
#endif
#if CONFIG_INIT_MODE == MODULE_INIT_LWM2M
        case 7: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            int32_t rc = lwm2mUpdate(10000);
            if (rc == 0) {
                mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_CONN_SUCCESS | CMODULE_URC_FLAG_STACK_READY;
                
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.processStep = CMODULE_PROCESS_STEP_INIT_COMPLETED;
                klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
                mModuleInstance.aux.nextPollSec = 120;
                break;
            } else {
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.processStep += 1;
                klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
                break;
            }
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 1000);
        } break;
        case 8: {
            if (!klTmcd_isExpired(&mModuleInstance.aux.pollTimer)) {
                break;
            }
            int32_t rc = lwm2mInit(10000);
            if (rc != 0) {
                lwm2mDelete(1000);
            }
            if (rc == 0) {
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.processStep = CMODULE_PROCESS_STEP_INIT_COMPLETED;
                klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 300);
                mModuleInstance.aux.nextPollSec = 120;
                break;
            } else {
                mModuleInstance.state.currentState = CMODULE_STATE_FAILED_CON;
                CREQUEST(ON_STATE_CHANGED, (void *) &mModuleInstance, (uint32_t) mModuleInstance.state.currentState);
                mModuleInstance.aux.initRetryCount = CMODULE_ERR_COUNT;
                mModuleInstance.state.processStep = 0;
            }
            klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 1000);
        } break;
#endif
    }
}

/*@}*/

/**
 * @addtogroup Protocol-JW
 * @note none
 */

/*@{*/

int32_t onPtPackIdReceived(cModule_TransmitPackageInfo_t *info) {
    if (_cModule_onProtocolReceived(&mModuleInstance, info->payload, info->payloadLength) != 0) {
        CREQUEST(ON_RECV_DATA, (void *) &mModuleInstance, info->payload, (uint32_t) info->payloadLength);
    }
    return 0;
}


static int32_t onPtPackIdTransmit(cModule_TransmitPackageInfo_t *info) {
    if (_cModule_onProtocolTransmited(&mModuleInstance, info) != 0) {
        return -1;
    }
    
    int32_t rc = -1;
    
#if CONFIG_INIT_MODE == MODULE_INIT_HTTP
    cModule_ProtocolHttpMessage_t *msg = info->payload;
    if (msg->payload == NULL) {
        if (msg->onDirectWrite != NULL) {
            rc = httpPost(msg->host, msg->port, msg->path, (void *) msg->onDirectWrite, msg->payloadLength, true);
        }
    } else {
        rc = httpPost(msg->host, msg->port, msg->path, msg->payload, msg->payloadLength, false);
    }
#endif
#if CONFIG_INIT_MODE == MODULE_INIT_LWM2M
    rc = lwm2mPost(info->payload, info->payloadLength, 10000, 0);
#endif
    
    return rc;
}


static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info) {
    return 0;
}


static int32_t onPtPackPayloadFree(cModule_TransmitPackageInfo_t *info) {
    if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_RX) {
        return -1;
    }
    if (info->flag.packIsDynMqttMsg) {
        cModule_ProtocolMqttMessage_t *msg = info->payload;
        free(msg->topic);
        free(msg->payload);
    }
    if (info->flag.packIsDynHttpMsg) {
        cModule_ProtocolHttpMessage_t *msg = info->payload;
        if (msg->onDirectWriteResponse != NULL) {
            msg->onDirectWriteResponse(&mModuleInstance.rilat.instance, msg->isWritenSuccess);
        }
        if (msg->host != NULL) {
            if (!msg->hostContant) {
                free(msg->host);
            }
        }
        if (msg->path != NULL) {
            if (!msg->pathContant) {
                free(msg->path);
            }
        }
        if (msg->payload != NULL) {
            if (msg->payloadLength != 0) {
                free(msg->payload);
            }
        }
    }
    return 0;
}

/*@}*/

/**
 * @addtogroup rilat callback
 * @note none
 */

/*@{*/

static int32_t rilat_onCallback(Rilat_Instance_t *instance, Rilat_CallbackEvent_t event, Rilat_CallbackVar_t var1,
                                Rilat_CallbackVar_t var2, Rilat_CallbackVar_t var3) {
    switch (event) {
        case RILAT_CALL_EVENT_ON_INIT: {
#if CONFIG_SERIAL_PHY_ENABLE == 1
            instance->userData = KDINSTANCE(UART, CONFIG_SERIAL_IF_PATH);
            ASSERT(instance->userData != NULL);
            kduart_init(((kduart_t *) instance->userData));
            kduart_powerUp(((kduart_t *) instance->userData));
            // kduart_updateBaudRate(((kduart_t *) instance->userData), 115200);
#endif
            __onRilatInit();
            break;
        }
        case RILAT_CALL_EVENT_ON_FINALIZE: {
#if CONFIG_SERIAL_PHY_ENABLE == 1
            if (instance->userData == NULL) {
                break;
            }
            kduart_powerDown(((kduart_t *) instance->userData));
            kduart_finalize(((kduart_t *) instance->userData));
            instance->userData = NULL;
            __onRilatFinalize();
#endif
            break;
        }
        case RILAT_CALL_EVENT_ON_READ: {
#if CONFIG_SERIAL_PHY_ENABLE == 1
            if (instance->userData == NULL) {
                return 0;
            }
            uint32_t recvActualLength = 0;
            kduart_recvs(instance->userData, var1.ptr, var2.u32, &recvActualLength, 0);
            if (recvActualLength == 0) {
                *((uint32_t *) var3.ptr) = 0;
                return 0;
            }
            *((uint32_t *) var3.ptr) = CREQUEST(ON_SERIAL_RECV, (void *) &mModuleInstance,
                                                var1.ptr,
                                                recvActualLength).u32;
#else
            *((uint32_t *) var3.ptr) = CREQUEST(ON_SERIAL_RECV, (void *) &mModuleInstance,
                                                (void *) var1.ptr, (uint32_t) var2.u32).u32;
#endif
#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT != 0
            klTmcd_countDownMs(&CMPMU.aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);
#endif
            return 0;
        }
        case RILAT_CALL_EVENT_ON_WRITE: {
#if CONFIG_SERIAL_PHY_ENABLE == 1
            if (instance->userData == NULL) {
                return 0;
            }
            return kduart_sends(instance->userData, var1.ptr, var2.u32, -1);
#else
            return CREQUEST(ON_SERIAL_TRANSMIT, (void *) &mModuleInstance,
                            (void *) var1.ptr, (uint32_t) var2.u32).i32;
#endif
            break;
        }
        case RILAT_CALL_EVENT_ON_TO_COMMAND: {
            break;
        }
        case RILAT_CALL_EVENT_ON_TO_STREAM: {
            break;
        }
        case RILAT_CALL_EVENT_ON_RAW_DATA_RECV: {
            break;
        }
        case RILAT_CALL_EVENT_ON_BLOCK_WAIT: {
            return CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance,
                            (uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL)
                            ? 1 : 0
            ).i32;
        }
        case RILAT_CALL_EVENT_ON_POLL_HOCK: {
            CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance,
                     (uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1
                                                                                                                : 0
            );
            break;
        }
    }

    return 0;
}

/*@}*/

/**
 * @addtogroup BaseFunc
 * @note none
 */

/*@{*/

static void onInstanceInit(void) {
#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT != 0
    CMPMU.flag._ = 0;
    CMPMU.flag.isReady = 1;
    CMPMU.sleep.currentSleepLevel = COMPONENT_SLEEP_LV_RUN;
    klTmcd_countDownMs(&CMPMU.aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);
#endif
    
    klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 200);
    mModuleInstance.state.processStep = 0;
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.aux.flag.hasInit = 0;
    mModuleInstance.aux.flag.pmuSupport = CONFIG_CMODULE_INSTANCE_PMU_SUPPORT;
    mModuleInstance.aux.flag.passiveRecvMode = CONFIG_CMODULE_INSTANCE_PASSIVE_RECV_MODE;
    mModuleInstance.aux.failedCount = 0;
    if (mModuleInstance.transmit.queneReqCount == 0) {
        klist_init(&mModuleInstance.transmit.packageReqList);
    }
    if (mModuleInstance.transmit.queneRepCount == 0) {
        klist_init(&mModuleInstance.transmit.packageRepList);
    }

    mModuleInstance.rilat.instance.callback = rilat_onCallback;
    mModuleInstance.rilat.instance.name = TAG;
    mModuleInstance.rilat.instance.transmitBuffer.buffer = NULL;
    mModuleInstance.rilat.instance.transmitBuffer.size = 0;
    mModuleInstance.rilat.instance.recvBuffer.buffer = (char *) mModuleInstance.rilat.recvBuffer;
    mModuleInstance.rilat.instance.recvBuffer.size = CONFIG_CMODULE_RILAT_MEM_RECV_BUFFER_SIZE;
    rilat_init(&mModuleInstance.rilat.instance);
}


static void onInstancePmu(uint8_t wakeup1sleep0) {
    if (wakeup1sleep0) {
#if CONFIG_SERIAL_PHY_ENABLE == 1
        if (mModuleInstance.rilat.instance.userData == NULL) {
            mModuleInstance.rilat.instance.userData = KDINSTANCE(UART, CONFIG_SERIAL_IF_PATH);
            ASSERT(mModuleInstance.rilat.instance.userData != NULL);
            kduart_init(((kduart_t *) mModuleInstance.rilat.instance.userData));
            kduart_powerUp(((kduart_t *) mModuleInstance.rilat.instance.userData));
        }
#endif

        mModuleInstance.aux.initRetryCount = 0;
        mModuleInstance.state.processStep = 0;
        mModuleInstance.state.urcResponseFlags = 0;
        mModuleInstance.aux.flag.hasWakeupSuccessHappend = 1;
        klTmcd_countDownMs(&mModuleInstance.aux.pollTimer, 200);

        __onHalWakeup();
        __onSoftWakeup();
    } else {
        __onSoftSleep();
        __onHalSleep();
        mModuleInstance.state.currentState = CMODULE_STATE_DISCON;
        CREQUEST(ON_STATE_CHANGED, (void *) &mModuleInstance,
                 (uint32_t) mModuleInstance.state.currentState);
        mModuleInstance.aux.flag.hasWakeupSuccessHappend = 0;

#if CONFIG_SERIAL_PHY_ENABLE == 1
        if (mModuleInstance.rilat.instance.userData != NULL) {
            kduart_powerDown(((kduart_t *) mModuleInstance.rilat.instance.userData));
            kduart_finalize(((kduart_t *) mModuleInstance.rilat.instance.userData));
            mModuleInstance.rilat.instance.userData = NULL;
        }
#endif
    }
}

/*@}*/

/**
 * @addtogroup Instance
 * @note none
 */

/*@{*/

cModule_Instance_t *cModule_getInstance_mn316(void) {
    return &mModuleInstance;
}

/*@}*/

/**
 * @addtogroup ProtocolFunc
 * @note none
 */

/*@{*/

/*@}*/

#endif

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/