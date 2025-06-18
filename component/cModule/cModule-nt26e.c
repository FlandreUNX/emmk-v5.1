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
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef TAG
#define TAG  "nt26e"

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
#if CONFIG_CMODULE_INSTANCE_X_ENABLE == 50
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    50
#else
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    -1
#endif

/*@}*/

#if CONFIG_CMODULE_INSTANCE_TAG_ID >= 0

CMPMU_DEFINE();

/**
 * @addtogroup Private vars
 * @note none
 */
 
/*@{*/

static void onInstanceInit(void);

static void onInstancePmu(uint8_t wakeup1sleep0);

static int32_t checkAT(void);

static int32_t AT(void);

static void onReboot(uint8_t isPowerUpRequest);

static void onLoop(void);

static void onInitStep(uint8_t step);

static void onResetStack(void);

static int32_t onPtPackIdReceived(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackIdTransmit(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackPayloadFree(cModule_TransmitPackageInfo_t *info, bool isForce);

static const cModule_Callback_t mCallback = {
        .onInstanceInit = onInstanceInit,
        .onInstancePmu = onInstancePmu,

        .onCommandAT = AT,
        .onCommandCheckAT = checkAT,

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
        .cmmpu = &CMPMU,
        .rilat = {
                .tagId = CONFIG_CMODULE_INSTANCE_TAG_ID,
                .recvBuffer = mModuleRxBuffer,
        },
};
const __CMODULE_X_SECTION cModule_InstanceConst_t mModuleInstanceConst = {
        .ins = &mModuleInstance,
};

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
static uint32_t mMqttMsgId = 0;
#endif

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
    
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    for (uint8_t i = 0; i < 5; i++) {
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
 
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+LGMR", NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+CGMR", NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+CGMM", NULL, 500);
    
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);
    
    return 0;
}


static int32_t checkCPIN(void) {
    static const char AT_CPIN_A[] = "AT+CPIN?";
    
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CPIN_A, "+CPIN:", &response, 2000) != 0 || response == NULL || response->success == 0) {
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

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CGREG_A, "+CEREG:", &response, 1000) != 0 || response == NULL || response->success == 0) {
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
    static const char AT_CGSN[] = "AT+CGSN";
    
    char *line = NULL;
    int32_t numberResult = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeMultiline(&mModuleInstance.rilat.instance, AT_CGSN, "", &response, 1000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }
    
    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    strcpy(mModuleInstance.modemInfo.imei, response->intermediates->line);
    
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
    
_l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t queryIMSI(void) {
    static const char AT_CIMI[] = "AT+CIMI";
    
    char *line = NULL;
    int32_t numberResult = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeMultiline(&mModuleInstance.rilat.instance, AT_CIMI, "", &response, 1000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }
    
    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }

    bool vailed = true;
    memset(mModuleInstance.modemInfo.imsi, 0x00, sizeof(mModuleInstance.modemInfo.imsi));
    strcpy(mModuleInstance.modemInfo.imsi, response->intermediates->line);
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
    static const char AT_CSQ[] = "AT+LCCID";

    int8_t rc = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CSQ, "+LCCID:", &response, 1000) != 0 ||
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
    while (isspace((char) *line) != 0) {
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

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CSQ, "+CSQ:", &response, 1000) != 0 || response == NULL || response->success == 0) {
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
        goto _l_retryExit;
    }
    
    if (response == NULL || response->success == 0 || response->intermediates == NULL) {
        goto _l_retryExit;
    }
    
    line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    
    if (klAttoken_getNextString(&line, &dtStr, NULL) != 0) {
        goto _l_retryExit;
    }
    
    {
        // static const char * const NTP_FORMAT = "%d/%d/%d,%d:%d:%d%*[+-]%d";
        int32_t i32 = 0;
        
        if (klAttoken_getNextIntSep(&dtStr, '/', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.year = 2000 + i32;
        
        if (klAttoken_getNextIntSep(&dtStr, '/', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.mon = i32;
        
        if (klAttoken_getNextIntSep(&dtStr, ',', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.day = i32;
        
        if (klAttoken_getNextIntSep(&dtStr, ':', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.hour = i32;
        
        if (klAttoken_getNextIntSep(&dtStr, ':', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.min = i32;

        stm.sec = (*dtStr - '0') * 10 + (*(dtStr + 1) - '0');
        dtStr += 2;
        
        if (klAttoken_getNextInt(&dtStr, &i32) != 0) {
            goto _l_retryExit;
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
    static const char AT_RST[] = "AT+CFUN=0,1";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_RST, NULL, 500);
}


static int32_t atcfun1(void) {
    return rilat_writeLine(&mModuleInstance.rilat.instance, "AT+CFUN=1", NULL, 3000);
}

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT

static void mqttClose(void) {
    static const char AT_QMTCLOSE[] = "AT+LMQTTCLOSE=0";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTCLOSE, NULL, 30000);
}

static int32_t mqttConnectCheck(void) {
    static const char AT_QMTCONN_A[] = "AT+LMQTTCONN?";

    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, AT_QMTCONN_A, "+LMQTTCONN:", &response, 5000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }
    char *line = response->intermediates->line;
    int32_t result = 0;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) { // clientId
        goto _l_retryExit;
    }
    if (result != 0) {
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) { // state
        goto _l_retryExit;
    }
    if (result != 3) {
        goto _l_retryExit;
    }
    
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
    
    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}

static int32_t mqttOpen(char *addr, uint16_t port) {
    static const char AT_QMTCFG_CACHE[] = "AT+LMQTTCFG=\"cache\",0,0";
    static const char AT_QMTCFG_OPEN[] = "AT+LMQTTOPEN=0,\"";// "AT+LMQTTOPEN=0,\"%s\",%d";
    
    AT();
    AT();
    
    int32_t rc = -1;
    Rilat_AtResponse_t *response = NULL;
    
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTCFG_CACHE, NULL, 500);
    
    char *buffer = calloc(1, 16);
    if (buffer == NULL) {
        return -1;
    }
    klPtf_sprintf(buffer, "\",%d", port);
    
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTCFG_OPEN, strlen(AT_QMTCFG_OPEN), false);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) addr, strlen(addr), false);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, buffer, "+LMQTTOPEN:", &response, 120000) != 0 || response == NULL || response->success == 0) {
        free(buffer);
        rc = -1;
        goto _l_retryExit;
    }
    free(buffer);
    
    char *line = response->intermediates->line;
    int32_t result = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (result != 0) {
        rc = result;
        goto _l_retryExit;
    }
    
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
    
_l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}

static int32_t mqttConnect(char *addr, uint16_t port, char *clientId, char *userName, char *pwd) {
    static const char AT_QMTCFG_CACHE[] = "AT+LMQTTCFG=\"cache\",0,0";
    static const char AT_QMTCFG_OPEN[] = "AT+LMQTTOPEN=0,\"";// "AT+LMQTTOPEN=0,\"%s\",%d";
    static const char AT_MQTT_CON[] = "AT+LMQTTCONN=0,\"";
    
    char *line = NULL;
    int32_t rc = -1;
    Rilat_AtResponse_t *response = NULL;
    char *buffer = calloc(1, 16);
    ASSERT(buffer != NULL);
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_CONN_SUCCESS | CMODULE_URC_FLAG_CONN_FAILED);
    
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTCFG_CACHE, NULL, 500);
    klPtf_sprintf(buffer, "\",%d", port);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTCFG_OPEN, strlen(AT_QMTCFG_OPEN), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) addr, strlen(addr), 0);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, buffer, "+LMQTTOPEN:", &response, 30000) != 0 
            || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    line = response->intermediates->line;
    rc = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (rc != 0) {
        rc = rc;
        goto _l_retryExit;
    }
    
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_MQTT_CON, strlen(AT_MQTT_CON), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) clientId, strlen(clientId), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",\"", 3, 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) userName, strlen(userName), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",\"", 3, 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) pwd, strlen(pwd), 0);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, "\"", "+LMQTTCONN:", &response, 30000) != 0 
            || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }      
    line = response->intermediates->line;
    rc = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (rc != 0) {
        if (klAttoken_getNextInt(&line, &rc) != 0) {
            rc = -1;
            goto _l_retryExit;
        }
        rc = rc;
        goto _l_retryExit;
    }
    
    free(buffer);
    buffer = NULL;
    
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    
    mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_CONN_SUCCESS);
    
    return 0;
    
    _l_retryExit:
    free(buffer);
    buffer = NULL;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    
    mModuleInstance.aux.initRetryCount++;
    return -1;
}

static void mqttDisconnect(void) {
    static const char AT_QMTDISC[] = "AT+LMQTTDISC=0";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTDISC, NULL, 1);
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_CONN_SUCCESS | CMODULE_URC_FLAG_CONN_FAILED);
}

static int32_t mqttSubTopic(const char *topicStr, uint8_t qos) {
//    static const char AT_QMTSUB[] = "AT+LMQTTSUBUNSUB=0,0,%d,\""
//        "%s"
//        "\",%d";
    static const char AT_QMTSUB[] = "AT+LMQTTSUBUNSUB=0,0,";
        
    qos = 1;
        
    int32_t rc = -1;
    Rilat_AtResponse_t *response = NULL;
    char *line = NULL;
    
    char *buffer = calloc(1, 16);
    ASSERT(buffer != NULL);
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_SUB_SUCCESS | CMODULE_URC_FLAG_SUB_FAILED);

    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTSUB, strlen(AT_QMTSUB), 0);
    klPtf_sprintf(buffer, "%d,\"", mMqttMsgId++);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) buffer, strlen(buffer), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) topicStr, strlen(topicStr), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",", 2, 0);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, "1", "+LMQTTSUBUNSUB:", &response, 15000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    line = response->intermediates->line;
    rc = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {    // clientId
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {    // msgId
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {    // result
        rc = -1;
        goto _l_retryExit;
    }
    if (rc != 0) {
        rc = rc;
        goto _l_retryExit;
    }
    
    free(buffer);
    buffer = NULL;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_SUB_SUCCESS );
    
    return 0;
    
_l_retryExit:
    free(buffer);
    buffer = NULL;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    mModuleInstance.aux.initRetryCount++;
    return -1;
}

static int32_t mqttPubTopic(cModule_ProtocolMqttMessage_t *msg, char *xPayload, uint16_t xPayloadLen, uint32_t packMsgId) {
    // static const char AT_QMTPUBEX[] = "AT+LMQTTPUB=0,%d,%d,0,\"%s\",%d,\"";
    static const char AT_QMTPUBEX[] = "AT+LMQTTPUB=0,";

    int32_t rc = -1;
    Rilat_AtResponse_t *response = NULL;
    char *line = NULL;
    
    char *buffer = calloc(1, 16);
    ASSERT(buffer != NULL);
    
    msg->qos = 1;
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_PUB_FAILED | CMODULE_URC_FLAG_PUB_SUCCESS);
    
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTPUBEX, strlen(AT_QMTPUBEX), 0);
    klPtf_sprintf(buffer, "%d,1,0,\"", mMqttMsgId++);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) buffer, strlen(buffer), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) msg->topic, strlen(msg->topic), 0);
    klPtf_sprintf(buffer, "\",", mMqttMsgId++);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) buffer, strlen(buffer), 0);
    
    if (xPayload == NULL) {
        if (msg->len == 0) {
            msg->len = msg->writer.onDorectGetLength(&mModuleInstance.rilat.instance, packMsgId);
        }
    } else {
        msg->len = xPayloadLen;
    }
    klPtf_sprintf(buffer, "%d,\"", msg->len);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) buffer, strlen(buffer), 0);
    
    if (xPayload != NULL) {
        rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) xPayload, xPayloadLen, 0);
    } else {
        if (msg->payload == NULL) {
            if (msg->writer.onDirectWrite != NULL) {
                msg->writer.onDirectWrite(&mModuleInstance.rilat.instance, packMsgId);
            }
        } else {
            rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) msg->payload, msg->len, 0);
        }
    }

    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, 
                "\"", "+LMQTTPUB:", &response, 15000) != 0 
            || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }      
    line = response->intermediates->line;
    rc = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &rc) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (rc != 0) {
        rc = rc;
        goto _l_retryExit;
    }
    
    free(buffer);
    buffer = NULL;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    return 0;
    
_l_retryExit:
    free(buffer);
    buffer = NULL;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    mModuleInstance.aux.initRetryCount++;
    return -1;
}

#endif

static void powerSet(bool up) {
    void *pin = CREQUEST(ON_MODEM_POWER_PIN_ACCESS, {.ptr = &mModuleInstance}).ptr;
    uint16_t delay = 0;

    if (up) {
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            delay = 3100;
        }
    } else {
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            delay = 3600;
        } else if (pin == NULL) {
            rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MPOF=1", NULL, 1000);
            return;
        }
    }

    if (delay != 0) {
        kdgpio_t *pwrPin = pin;
        kdgpio_init((kdgpio_t *) pwrPin);
        kdgpio_powerUp((kdgpio_t *) pwrPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
        kdgpio_output((kdgpio_t *) pwrPin, 1);
        _cModule_wait(&mModuleInstance, delay, 0);
        kdgpio_output((kdgpio_t *) pwrPin, 0);
        kdgpio_powerDown((kdgpio_t *) pwrPin);
        kdgpio_finalize((kdgpio_t *) pwrPin);
    }
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
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
    mqttDisconnect();
    mqttClose();
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_TCP
    socketTcpClose();
#endif
    if (mModuleInstance.aux.flag.pmuSupport) {
        powerSet(false);
    }
}


static void onResetStack(void) {
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
    mqttDisconnect();
    mqttClose();
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_TCP
    socketTcpClose();
#endif
    onReboot(false);
}


static void onReboot(uint8_t isPowerUpRequest) {
    if (isPowerUpRequest) {
        powerSet(true);
    } else {
        void *pin = CREQUEST(ON_MODEM_RESET_PIN_ACCESS, {.ptr = &mModuleInstance}).ptr;
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            kdgpio_t *rstPin = pin;
            kdgpio_init((kdgpio_t *) rstPin);
            kdgpio_powerUp((kdgpio_t *) rstPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
            kdgpio_output((kdgpio_t *) rstPin, 1);
            _cModule_wait(&mModuleInstance, 400, 0);
            kdgpio_output((kdgpio_t *) rstPin, 0);
            kdgpio_powerDown((kdgpio_t *) rstPin);
            kdgpio_finalize((kdgpio_t *) rstPin);
        } else {
            reboot();
        }
    }
}


static void onLoop(void) {
    if (qSTimer_Expired(&mModuleInstance.aux.pollTimer)) {
        qSTimer_Set(&mModuleInstance.aux.pollTimer, 60000);
        int32_t netRc = 0;
        AT();
        readSignal();
        checkNTP();
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
        netRc = mqttConnectCheck();
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_TCP
        netRc = socketTcpState();
#endif
        if (netRc != 0) {
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

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
RILAT_PARTIAL_LINE_PATCH_DEFINE(_QMTRECV, "+LMQTTURC:", "\r\n", 2);
RILAT_COMMAND_MATCH_DEFINE(_QMTRECV, "+LMQTTURC:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    
    char *line = (char *) data;
    char *payloadStr = NULL;
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    // RECV / STAT
    if (klAttoken_getNextString(&line, &payloadStr, NULL) != 0) {
        return -1;
    }
    
    if (strstr(payloadStr, "RECV")) {
        char *topic = NULL;
        int32_t topicLength = 0;
        
        klAttoken_skip(&line); // clientId
        klAttoken_skip(&line); // msgId
        if (klAttoken_getNextString(&line, &topic, &topicLength) != 0) {
            return -1;
        }
      
        if (!_cModule_isAllocRepPackAccess(&mModuleInstance)) {
            LOG_W("!_cModule_isAllocRepPackAccess");
            return -1;
        }
        
        char *payload = calloc(1, strlen(line) + 1);
        if (payload == NULL) {
            return -1;
        }
        memcpy(payload, line, strlen(line));
        
        _cModule_packMutexLock(&mModuleInstance, true);
        cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
        if (pack == NULL) {
            goto l_errorExit;
        }
        pack->payloadLength = strlen(line);
        pack->payload = payload;
        pack->flag.packIsDynData = 1;
        
        pack->var.ptr = calloc(1, topicLength + 1);
        if (pack->var.ptr == NULL) {
            goto l_errorExit;
        }
        pack->varLength = topicLength;
        strncpy(pack->var.ptr, topic, topicLength);
        pack->flag.varIsLengthItem = 1;
        pack->flag.packHasVar = 1;
        
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
    } else if (strstr(payloadStr, "STATS")) {
        
    }
    
    return -1;
}
#endif

/*@}*/

/**
 * @addtogroup Init
 * @note none
 */
 
/*@{*/

static void setSuccessStep(uint16_t nextLoopSec) {
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.state.processStep = CMODULE_PROCESS_STEP_INIT_COMPLETED;
    qSTimer_Set(&mModuleInstance.aux.pollTimer, 300);
    mModuleInstance.aux.nextPollSec = nextLoopSec;
}


static void setNextStep(void) {
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.state.processStep++;
    qSTimer_Set(&mModuleInstance.aux.pollTimer, 300);
}


static void setNextStepWithSkip(uint8_t skip) {
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.state.processStep += skip;
    qSTimer_Set(&mModuleInstance.aux.pollTimer, 300);
}


static void setErrorStep(uint16_t errorCount, cModule_State_t st, uint32_t nextTimeout) {
    if (mModuleInstance.aux.initRetryCount > errorCount || errorCount == 0) {
        mModuleInstance.state.currentState = st;
        CREQUEST(ON_STATE_CHANGED, (void *) &mModuleInstance, (uint32_t) mModuleInstance.state.currentState);
        mModuleInstance.aux.initRetryCount = CMODULE_ERR_COUNT;
        mModuleInstance.state.processStep = 0;
    }
    qSTimer_Set(&mModuleInstance.aux.pollTimer, nextTimeout);
}

static void onInitStep(uint8_t step) {
    if (!qSTimer_Expired(&mModuleInstance.aux.pollTimer)) {
        return;
    }
    
    if (step == 1) {
        ATE0();
        setNextStep();
    } else if (step == 2) {
        if (checkCPIN() == 0) {
            setNextStep();
        } else {
            setErrorStep(10, CMODULE_STATE_NO_COM_ID, 1000);
        }
    } else if (step == 3) {
        if (queryIMEI() == 0
                && queryIMSI() == 0
                && readIccid() == 0) {
            setNextStep();
        } else {
            setErrorStep(10, CMODULE_STATE_NO_COM_ID, 1000);
        }
    } else if (step == 4) {
        if (checkCGREG() == 0) {
            _cModule_wait(&mModuleInstance, 1000, true);
            readSignal();
            _cModule_wait(&mModuleInstance, 300, true);
            checkNTP();
            CREQUEST(ON_NTP_UPDATE, (void *) &mModuleInstance, mModuleInstance.aux.timeStamp);
            _cModule_wait(&mModuleInstance, 300, true);
            atcfun1();
            setNextStep();
        } else {
            setErrorStep(120, CMODULE_STATE_FAILED_REG, 300);
        }
    } 
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
    else if (step == 5) {
        if (mqttConnectCheck() == 0) {
            setNextStepWithSkip(2);
        } else {
            setNextStep();
        }
    } else if (step == 6) {
        int32_t rc;
        
        char *ip = NULL;
        uint16_t port = 0;
        char *clientId = NULL;
        char *userName = NULL, *pwd = NULL;
        _cModule_onProtocolMqttConnectAlloc(&mModuleInstance, &ip, &clientId, &userName, &pwd);
        _cModule_onProtocolMqttConnect(&mModuleInstance, &ip, &port, &clientId, &userName, &pwd);
        rc = mqttConnect(ip, port, clientId, userName, pwd);
        _cModule_onProtocolMqttConnectFree(&mModuleInstance, &ip, &clientId, &userName, &pwd);
        
        if (rc == 0) {
            setNextStep();
        } else {
            setErrorStep(1, CMODULE_STATE_FAILED_CON, 1000);
        }
    } else if (step == 7) {
        int32_t rc;
            
        uint8_t count = _cModule_onProtocolMqttSubCount(&mModuleInstance);
        for (uint8_t i = 0; i < count; i++) {
            char *topic = NULL;
            uint8_t qos = 1;
            _cModule_onProtocolMqttSubAlloc(&mModuleInstance, &topic, i);
            _cModule_onProtocolMqttSub(&mModuleInstance, &topic, &qos, i);
            rc = mqttSubTopic(topic, qos);
            _cModule_onProtocolMqttSubFree(&mModuleInstance, &topic, i);
            if (rc != 0) {
                break;
            }
        }
        
        if (rc == 0) {
            setSuccessStep(120);
        } else {
            setErrorStep(0, CMODULE_STATE_FAILED_CON, 1000);
        }
    }
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_NONE
    else if (step == 5) {
        setSuccessStep(120);
    } 
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_TCP
    else if (step == 5) {
        if (socketTcpState() == 0) {
            setSuccessStep(120);
        } else {
            setNextStep();
        }
    } else if (step == 6) {
        int32_t rc;
        
        char *ip = NULL;
        uint16_t port = 0;
        _cModule_onProtocolTcpConnectAlloc(&mModuleInstance, &ip);
        _cModule_onProtocolTcpConnect(&mModuleInstance, &ip, &port);
        rc = socketTcpCreate(ip, port, 30000);
        _cModule_onProtocolTcpConnectFree(&mModuleInstance, &ip);
        
        if (rc == 0) {
            setSuccessStep(120);
        } else {
            setErrorStep(0, CMODULE_STATE_FAILED_CON, 1000);
        }
    } 
#endif
}

/*@}*/

/**
 * @addtogroup Protocol-JW
 * @note none
 */

/*@{*/

int32_t onPtPackIdReceived(cModule_TransmitPackageInfo_t *info) {
    if (_cModule_onProtocolReceived(&mModuleInstance, info) != 0) {
        CREQUEST(ON_RECV_DATA, (void *) &mModuleInstance, info->payload, (uint32_t) info->payloadLength);
    }
    return 0;
}


static int32_t onPtPackIdTransmit(cModule_TransmitPackageInfo_t *info) {
    int32_t rc = -1;
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
    if (info->flag.packIsDynMqttMsg) {
        cModule_ProtocolMqttMessage_t *msg = info->payload;
        if (msg->isPayloadJson) {
#if CONFIG_CMODULE_CJSON_SUPPORT == 1
            cModule_ProtocolMqttMessage_t *msg = info->payload;
            if (_cModule_onProtocolTransmited(&mModuleInstance, info) != 0) {
                return rc;
            }

            cJSON *cjValue = (void *) msg->payload;
            char *payload = cJSON_PrintUnformatted(cjValue);
            if (payload != NULL) {
                rc = mqttPubTopic(msg, payload, strlen(payload), info->aux.gen.msgId);
            }
            cJSON_free(payload);
            if (rc != 0) {
                LOG_W("onPtPackIdTransmit, Failed!");
            }
            return rc;
#else
            ASSERT(0);
#endif
        } else if (msg->isPayloadString) {
            rc = mqttPubTopic(msg, NULL, 0, info->aux.gen.msgId);
        } else {
            return rc;
        }
    } else
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_NONE
    if (info->flag.packIsDynHttpMsg) {
        cModule_ProtocolHttpMessage_t *msg = info->payload;
        rc = httpPost(msg, info->aux.gen.msgId);
    } else 
#endif
    if (info->flag.packIsDynTcpIpData) {
        cModule_ProtocolTcpIpMessage_t *msg = info->payload;
        if (msg->dStream == false) {
            // TODO rc = socketTcpSend(msg, info->aux.gen.msgId);
        } else {
            // TODO UdpSend
        }
    }
    return rc;
}


static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info) {
    return 0;
}


static int32_t onPtPackPayloadFree(cModule_TransmitPackageInfo_t *info, bool isForce) {
    if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_RX) {
        return 0;
    }
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
    if (info->flag.packIsDynMqttMsg) {
        cModule_ProtocolMqttMessage_t *msg = info->payload;
        if (msg->payload == NULL) {
            if (msg->writer.onDirectWriteResponse != NULL) {
                msg->writer.onDirectWriteResponse(&mModuleInstance.rilat.instance, info->aux.gen.msgId, msg->writer.isWritenSuccess, isForce);
            }
        } else {
            if (msg->isPayloadString) {
                free(msg->payload);
            } else if (msg->isPayloadJson) {
#if CONFIG_CMODULE_CJSON_SUPPORT == 1
                cJSON_Delete((void *) msg->payload);
#else    
                ASSERT(0);
#endif
            } else {
                ASSERT(0);
            }
        }
        if (!msg->topicConstant) {
            free(msg->topic);
        }
    } else 
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_NONE
    if (info->flag.packIsDynHttpMsg) {
        cModule_ProtocolHttpMessage_t *msg = info->payload;
        if (msg->onDirectWriteResponse != NULL) {
            msg->onDirectWriteResponse(&mModuleInstance.rilat.instance, info->aux.gen.msgId, msg->isWritenSuccess, isForce);
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
    } else 
#endif
    if (info->flag.packIsDynTcpIpData) {
        cModule_ProtocolTcpIpMessage_t *msg = info->payload;
        if (msg->payload == NULL) {
            if (msg->writer.onDirectWriteResponse != NULL) {
                msg->writer.onDirectWriteResponse(&mModuleInstance.rilat.instance, info->aux.gen.msgId, msg->writer.isWritenSuccess, isForce);
            }
        } else {
            free(msg->payload);
        }
        if (msg->host != NULL && !msg->hostContant) {
            free(msg->host);
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

static int32_t rilat_onCallback(Rilat_Instance_t *instance, Rilat_CallbackEvent_t event, Rilat_CallbackVar_t var1, Rilat_CallbackVar_t var2, Rilat_CallbackVar_t var3) {
    if (event == RILAT_CALL_EVENT_ON_INIT) {
        CREQUEST(ON_SERIAL_INIT, {&mModuleInstance});
    } else if (event == RILAT_CALL_EVENT_ON_FINALIZE) {
        CREQUEST(ON_SERIAL_FINALIZE, {&mModuleInstance});
    } else if (event == RILAT_CALL_EVENT_ON_READ) {
        *((uint32_t *) var3.ptr) = CREQUEST(ON_SERIAL_RECV, (void *) &mModuleInstance, (void *) var1.ptr, (uint32_t) var2.u32).u32;
    } else if (event == RILAT_CALL_EVENT_ON_WRITE) {
        return CREQUEST(ON_SERIAL_TRANSMIT, (void *) &mModuleInstance, (void *) var1.ptr, (uint32_t) var2.u32).i32;
    } else if (event == RILAT_CALL_EVENT_ON_BLOCK_WAIT) {
        return CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance, (uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1 : 0).i32;
    } else if (event == RILAT_CALL_EVENT_ON_POLL_HOCK) {
        return CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance,(uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1 : 0).i32;
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
    CMPMU.flag._ = 0;
    CMPMU.flag.isReady = 1;
    CMPMU.sleep.currentSleepLevel = COMPONENT_SLEEP_LV_RUN;
    qSTimer_Set(&CMPMU.aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);

    qSTimer_Set(&mModuleInstance.aux.pollTimer, 200);
    mModuleInstance.state.processStep = 0;
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.aux.flag.hasInit = 0;
    mModuleInstance.aux.flag.pmuSupport = 0;
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
        CREQUEST(ON_SERIAL_INIT, {&mModuleInstance});
        mModuleInstance.aux.initRetryCount = 0;
        mModuleInstance.state.processStep = 0;
        mModuleInstance.state.urcResponseFlags = 0;
        mModuleInstance.aux.flag.hasWakeupSuccessHappend = 1;
        qSTimer_Set(&mModuleInstance.aux.pollTimer, 200);

        __onHalWakeup();
        __onSoftWakeup();
    } else {
        __onSoftSleep();
        __onHalSleep();
        mModuleInstance.state.currentState = CMODULE_STATE_DISCON;
        CREQUEST(ON_STATE_CHANGED, (void *) &mModuleInstance,
                 (uint32_t) mModuleInstance.state.currentState);
        mModuleInstance.aux.flag.hasWakeupSuccessHappend = 0;
        CREQUEST(ON_SERIAL_FINALIZE, {&mModuleInstance});
    }
}

/*@}*/

/**
 * @addtogroup Instance
 * @note none
 */

/*@{*/

cModule_Instance_t *cModule_getInstance_nt26e(void) {
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