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
#define TAG     "ML307A"

#define MODEM_VERISON_STR      "R241016"

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
#if CONFIG_CMODULE_INSTANCE_X_ENABLE == 51
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    51
#else
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    -1
#endif

/*@}*/

#if CONFIG_CMODULE_INSTANCE_TAG_ID >= 0

CMPMU_DEFINE();

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

/*@}*/

/**
 * @addtogroup AtCommand -Ext GNSS
 * @note none
 */

/*@{*/

#if CONFIG_CMODULE_GNSS_SUPPORT == 1

static int32_t gnss_startup_once(void) {
    // static const char MGNSSCFG[] = "AT+MGNSSCFG=\"nmea/mask\",63";
    static const char MGNSSLOC[] = "AT+MGNSSLOC=1";
    static const char MGNSS[] = "AT+MGNSS=2";

    Rilat_AtResponse_t *response = NULL;
    int32_t ret = -1;

    // rilat_writeLine(&mModuleInstance.rilat.instance, MGNSSCFG, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, MGNSSLOC, NULL, 500);

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, MGNSS, "+MGNSSURC:", &response, 2000) != 0
        || response == NULL || response->success == 0) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        response = NULL;
        return -1;
    }

    char *line = NULL;
    if (response->intermediates == NULL) {
        goto l_exit;
    }
    line = response->intermediates->line;
    if (klAttoken_start(&line) != 0) {
        goto l_exit;
    }
    if (strstr(response->intermediates->line, "error")) {
        klAttoken_getNextInt(&line, &ret);
        ret = -1 - ret;
    } else if (strstr(response->intermediates->line, "state")) {
        klAttoken_getNextInt(&line, &ret);
    }
    l_exit:
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    return ret;
}

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

    CREQUEST(ON_SERIAL_FLUSH, {&mModuleInstance});
        
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    for (uint8_t i = 0; i < 3; i++) {
        rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, &response, 500);
        if (response != NULL && response->success) {
            rilat_freeResponse(&mModuleInstance.rilat.instance, response);
            response = NULL;

            isSuccess = 1;
            break;
        }

        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        response = NULL;

        if (cModule_isBlocking(&mModuleInstance)) {
            isSuccess = 0;
            break;
        }
    }

    if (isSuccess) {
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+IPR=115200", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+IPR=115200", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MLPMCFG=\"delaysleep\",30", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MLPMCFG=\"sleepmode\",0,0", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MLPMCFG=\"sleepmode\",0,0", NULL, 1000);
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
    static const char AT_CGSN[] = "AT+GSN=1";

    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CGSN, "+GSN:", &response, 1000) != 0 
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

    if (rilat_writeMultiline(&mModuleInstance.rilat.instance, AT_CIMI, "", &response, 1000) != 0 || response == NULL ||
        response->success == 0) {
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
    static const char AT_CSQ[] = "AT+MCCID";

    int8_t rc = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CSQ, "+MCCID:", &response, 1000) != 0 ||
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
    uint8_t retryCount = 0;
    
    l_retry:

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
    response = NULL;
    
    if (mModuleInstance.aux.timeStamp < CONFIG_MIN_TS) {
        if (retryCount < 10) {
            retryCount++;
            _cModule_wait(&mModuleInstance, 500, true);
            goto l_retry;
        }
    }
    
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static void reboot(void) {
    static const char AT_RST[] = "AT+MREBOOT=1";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_RST, NULL, 500);
}


static int32_t atcfun1(void) {
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+CFUN=1", NULL, 3000);
    return 0;
}

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
static void mqttClose(void) {
    
}


static int32_t mqttConnectCheck(void) {
    static const char AT_QMTCONN_A[] = "AT+MQTTSTATE=0";
    
    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, AT_QMTCONN_A, "+MQTTSTATE:", &response, 1000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }
    char *line = response->intermediates->line;
    int32_t result = 0;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) { // state
        goto _l_retryExit;
    }
    if (result != 2) {
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
    static const char AT_QMTDISC1[] = "AT+MQTTCONN=0,\"";
    char *buffer = calloc(1, 16);
    ASSERT(buffer != NULL);
    
//    klPtf_sprintf(buffer, "AT+MQTTCONN=0,\"%s\",%d,\"%s\",\"%s\",\"%s\"", 
//        addr, port,
//        clientId, 
//        userName,
//        pwd);
    
    int32_t rc = -1;
    Rilat_AtResponse_t *response = NULL;
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_CONN_SUCCESS | CMODULE_URC_FLAG_CONN_FAILED);
    
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MQTTCFG=\"cached\",0,0", NULL, 500);
    
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTDISC1, strlen(AT_QMTDISC1), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) addr, strlen(addr), 0);
    klPtf_sprintf(buffer, "\",%d,\"", port);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) buffer, strlen(buffer), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) clientId, strlen(clientId), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",\"", 3, 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) userName, strlen(userName), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",\"", 3, 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) pwd, strlen(pwd), 0);
    if (rilat_writeLine(&mModuleInstance.rilat.instance, "\"", &response, 30000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    
    free(buffer);
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    qSTimer_t wait = QSTIMER_INITIALIZER;
    qSTimer_Set(&wait, 15000);
    while (!qSTimer_Expired(&wait)) {
        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CONN_SUCCESS
                || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CONN_FAILED) {
            break;
        }
        if (_cModule_wait(&mModuleInstance, 1, true) < 0) {
            wait = (qSTimer_t) QSTIMER_INITIALIZER;
            break;
        }
    }
    if (wait.tStart == 0 || qSTimer_Expired(&wait) || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CONN_FAILED) {
        goto _l_retryExit;
    }
    
    return 0;
    
_l_retryExit:
    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static void mqttDisconnect(void) {
    static const char AT_QMTDISC[] = "AT+MQTTDISC=0";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTDISC, NULL, 1);
}


static int32_t mqttSubTopic(const char *topicStr, uint8_t qos) {
//    static const char AT_QMTSUB[] = "AT+MQTTSUB=0,\""
//        "%s"
//        "\",%d";
    static const char AT_QMTSUB1[] = "AT+MQTTSUB=0,\"";
    
    qos = 1;

    int32_t rc = -1;
//    char *buffer = calloc(1, 512);
//    if (buffer == NULL) {
//        return -1;
//    }
//    klPtf_sprintf(buffer, AT_QMTSUB, topicStr, qos);      // Default_QOS=2
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_SUB_SUCCESS | CMODULE_URC_FLAG_SUB_FAILED);
    
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTSUB1, strlen(AT_QMTSUB1), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) topicStr, strlen(topicStr), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",", 2, 0);
    
    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeLine(&mModuleInstance.rilat.instance, "1", &response, 15000) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
//    free(buffer);
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    qSTimer_t wait = QSTIMER_INITIALIZER;
    qSTimer_Set(&wait, 15000);
    while (!qSTimer_Expired(&wait)) {
        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_SUB_SUCCESS
                || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_SUB_FAILED) {
            break;
        }
        if (_cModule_wait(&mModuleInstance, 1, true) < 0) {
            wait = (qSTimer_t) QSTIMER_INITIALIZER;
            break;
        }
    }
    if (wait.tStart == 0 || qSTimer_Expired(&wait) || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_SUB_FAILED) {
        goto _l_retryExit;
    }
    
    return 0;
    
_l_retryExit:
//    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static int32_t mqttPubTopic(cModule_ProtocolMqttMessage_t *msg, char *xPayload, uint16_t xPayloadLen, uint32_t packMsgId) {
    //static const char AT_QMTPUBEX[] = "AT+MQTTPUB=0,\"%s\",%d,0,1,%d,\"";
    static const char AT_QMTPUBEX1[] = "AT+MQTTPUB=0,\"";
    
    int32_t rc = -1;
    char *buffer = calloc(1, 8);
    if (buffer == NULL) {
        return -1;
    }
    
    msg->qos = 1;
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_PUB_FAILED | CMODULE_URC_FLAG_PUB_SUCCESS);

    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTPUBEX1, strlen(AT_QMTPUBEX1), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) msg->topic, strlen(msg->topic), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",1,0,1,", 8, 0);
    
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

    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance,
                "\"", "+MQTTPUB:", &response, msg->timeout) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    free(buffer);

    if (msg->qos != 0 && msg->timeout != 0) {
        qSTimer_t wait = QSTIMER_INITIALIZER;
        qSTimer_Set(&wait, msg->timeout);
        while (!qSTimer_Expired(&wait)) {
            if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_SUCCESS
                    || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_FAILED) {
                break;
            }
            if (_cModule_wait(&mModuleInstance, 1, true) < 0) {
                wait = (qSTimer_t) QSTIMER_INITIALIZER;
                break;
            }
        }
        if (wait.tStart == 0 || qSTimer_Expired(&wait) || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_FAILED) {
            goto _l_retryExit;
        }
    }
    
    msg->writer.isWritenSuccess = true;
    
    return 0;
    
_l_retryExit:
    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}

#endif

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_NONE
static void httpDelete(void) {
    static const char AT_MHTTPDEL[] = "AT+MHTTPDEL=0";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_MHTTPDEL, NULL, 3000);
}


static int32_t httpPost(cModule_ProtocolHttpMessage_t *message, uint32_t packMsgId) {
    static const char AT_MHTTPREQUEST[] = "AT+MHTTPREQUEST=0,2,0,";
    static const char AT_MHTTPCONTENT[] = "AT+MHTTPCONTENT=0,0,%d";
    static const char AT_MHTTPCREATE[] = "AT+MHTTPCREATE=\"http://";
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_PUB_SUCCESS | CMODULE_URC_FLAG_PUB_FAILED);
    
    httpDelete();
    
    int32_t rc = -1;
    char *buffer = calloc(1, 32);
    if (buffer == NULL) {
        return -1;
    }
    if (message->port != 0) {
        klPtf_sprintf(buffer, ":%d\"", message->port);
    } else {
        klPtf_sprintf(buffer, "\"");
    }
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_MHTTPCREATE, strlen(AT_MHTTPCREATE), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) message->host, strlen(message->host), 0);
    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, 
                buffer, "+MHTTPCREATE:", &response, 5000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    // rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MHTTPCFG=\"header\",0,\"Connection: close\"", NULL, 3000);
    // rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MHTTPCFG=\"header\",0,\"Content-Type: text/plain\"", NULL, 3000);
    
    if (message->onDirectWrite != NULL) {
        if (message->payloadLength == 0) {
            message->payloadLength = message->onDorectGetLength(&mModuleInstance.rilat.instance, packMsgId);
        }
    }
    
    klPtf_sprintf(buffer, AT_MHTTPCONTENT, message->payloadLength);
    if (rilat_writeLineWithPDU(&mModuleInstance.rilat.instance,
                                    buffer, NULL,
                                    message->onDirectWrite != NULL ? (void *) message->onDirectWrite : message->payload,
                                    message->onDirectWrite != NULL ? 0 : message->payloadLength, 
                                    message->onDirectWrite != NULL ? RILAT_PDU_S_DIRECT_WRITE : 0, 
                                    packMsgId,
                                    &response, 5000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }   
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_MHTTPREQUEST, strlen(AT_MHTTPREQUEST), 0);
    klPtf_sprintf(buffer, "\"%s\"", message->path);
    rilat_writeLine(&mModuleInstance.rilat.instance, buffer, NULL, 3000);
    
    free(buffer);
    buffer = NULL;
    
    qSTimer_t wait;
    qSTimer_Set(&wait, 10 * 1000);
    while (!qSTimer_Expired(&wait)) {
        if (_cModule_wait(&mModuleInstance, 1, true) < 0) {
           rc = -1;
            goto _l_retryExit;
        }
        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_SUCCESS) {
            break;
        } else if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_FAILED) {
            rc = -1;
            goto _l_retryExit;
        }
    }
    if (qSTimer_Expired(&wait)) {
        rc = -1;
        message->isWritenSuccess = false;
    } else {
        rc = 0;
        message->isWritenSuccess = true;
    }
    
    _cModule_wait(&mModuleInstance, 1000, true);
    httpDelete();
    
    return rc;

    _l_retryExit:
    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    
    httpDelete();
    
    return rc;
}
#endif

static int32_t socketTcpState(void) {
    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, "AT+MIPSTATE=0", "+MIPSTATE:", &response, 1000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }
    char *line = response->intermediates->line;
    char *st = NULL;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    klAttoken_skip(&line); // <connect_id>
    klAttoken_skip(&line); // <service_type>
    klAttoken_skip(&line); // <address>
    klAttoken_skip(&line); // <remote_port>
    if (klAttoken_getNextString(&line, &st, NULL) != 0) { // <state>
        goto _l_retryExit;
    } 
    if (strstr(st, "CONNECTED")) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        response = NULL;
        return 0;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static int32_t socketTcpCreate(char *ip, uint16_t port, uint32_t timeout) {
    static const char MIPOPEN[] = "AT+MIPOPEN=0,\"TCP\",\"";

    Rilat_AtResponse_t *response = NULL;
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_SUB_SUCCESS | CMODULE_URC_FLAG_SUB_FAILED);
    
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MIPCFG=\"encoding\",0,1,1", NULL, 3000);

    char *buffer = calloc(1, 32);
    ASSERT(buffer != NULL);
    klPtf_sprintf(buffer, "%d,%d,0", port, timeout / 1000);
    
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) MIPOPEN, strlen(MIPOPEN), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) ip, strlen(ip), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",", 2, 0);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, buffer, "+MIPOPEN:", &response, timeout + 3000) != 0) {
        goto l_retryExit;
    }
    free(buffer);
    buffer = NULL;
    
    char *line = response->intermediates->line;
    int32_t st = 0;
    if (klAttoken_start(&line) != 0) {
        goto l_retryExit;
    }
    klAttoken_skip(&line); // <connect_id>
    if (klAttoken_getNextInt(&line, &st) != 0) { // <state>
        goto l_retryExit;
    } 
    if (st != 0) {
        goto l_retryExit;
    }
    
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MIPMODE=0,0", NULL, 3000);

    return 0;
    
    l_retryExit:
    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}

static int32_t socketTcpClose(void) {
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MIPCLOSE=0", NULL, 3000);
    return 0;
}

static int32_t socketTcpSend(cModule_ProtocolTcpIpMessage_t *msg, uint32_t packMsgId) {
    static const char MIPSEND[] = "AT+MIPSEND=0,";  // HEX-Send
    
    int32_t rc = -1;
    char *buffer = calloc(1, 16);
    ASSERT(buffer != NULL);
 
    if (msg->payloadLength == 0) {
        if (msg->writer.onDirectGetLength != NULL) {
            msg->payloadLength = msg->writer.onDirectGetLength(&mModuleInstance.rilat.instance, packMsgId);
        }
    }
    
    klPtf_sprintf(buffer, "%d,\"", msg->payloadLength);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) MIPSEND, strlen(MIPSEND), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) buffer, strlen(buffer), 0);
    
    if (msg->payload == NULL) {
        if (msg->writer.onDirectWrite != NULL) {
            msg->writer.onDirectWrite(&mModuleInstance.rilat.instance, packMsgId);
        }
    } else {
        uint32_t cur = 0;
        uint8_t *hexBuffer = calloc(1, 129);
        ASSERT(hexBuffer != NULL);
        while (cur < msg->payloadLength) {
            uint8_t writeLen = msg->payloadLength - cur;
            if (writeLen >= 64) {
                writeLen = 64;
            }
            uint8_t hexLen = klStr_hex2str((uint8_t *) msg->payload + cur, writeLen, (char *) hexBuffer);
            
            rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) hexBuffer, hexLen, 0);
            memset(hexBuffer, 0x00, 129);
            
            cur += writeLen;
        }
        free(hexBuffer);
    }

    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeLine(&mModuleInstance.rilat.instance, "\"", &response, msg->timeout) != 0 
            || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    free(buffer);
    
    msg->writer.isWritenSuccess = true;
    
    return 0;
    
_l_retryExit:
    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


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
        uint8_t lv = CREQUEST(ON_MODEM_POWER_PIN_LEVEL, {}).u32 ? true : false;
        kdgpio_t *pwrPin = pin;
        kdgpio_init((kdgpio_t *) pwrPin);
        kdgpio_powerUp((kdgpio_t *) pwrPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
        kdgpio_output((kdgpio_t *) pwrPin, lv);
        _cModule_wait(&mModuleInstance, delay, 0);
        kdgpio_output((kdgpio_t *) pwrPin, !lv);
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
            uint8_t lv = CREQUEST(ON_MODEM_RESET_PIN_LEVEL, {}).u32 ? true : false;
            kdgpio_t *rstPin = pin;
            kdgpio_init((kdgpio_t *) rstPin);
            kdgpio_powerUp((kdgpio_t *) rstPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
            kdgpio_output((kdgpio_t *) rstPin, lv);
            _cModule_wait(&mModuleInstance, 400, 0);
            kdgpio_output((kdgpio_t *) rstPin, !lv);
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

#if CONFIG_CMODULE_GNSS_SUPPORT == 1

RILAT_COMMAND_MATCH_DEFINE(_MGNSSLOC, "+MGNSSLOC:", data, len) {
    char *line = (char *) data;
    cModule_GnssSimpleData_t gnssData = {0};
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    klAttoken_getNextString(&line, &gnssData.utc, NULL);
    klAttoken_getNextString(&line, &gnssData.latitude, NULL);
    klAttoken_getNextString(&line, &gnssData.longtitude, NULL);
    klAttoken_getNextString(&line, &gnssData.hdop, NULL);
    klAttoken_getNextString(&line, &gnssData.altitude, NULL);
    klAttoken_getNextString(&line, &gnssData.fix, NULL);
    klAttoken_getNextString(&line, &gnssData.cog, NULL);
    klAttoken_getNextString(&line, &gnssData.spkm, NULL);
    klAttoken_getNextString(&line, &gnssData.spkn, NULL);
    klAttoken_getNextString(&line, &gnssData.date, NULL);
    klAttoken_getNextString(&line, &gnssData.nsat, NULL);
    klAttoken_getNextString(&line, &gnssData.dtype, NULL);
    CREQUEST(ON_GNSS_SIMPLE_RECV, {.ptr = &gnssData});
    return 0;
}

#endif

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_HTTP
RILAT_COMMAND_MATCH_DEFINE(_MHTTPURC, "+MHTTPURC:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    
    // StatusString
    char *statusStr = NULL;
    int32_t dataLength = 0;
    if (klAttoken_getNextString(&line, &statusStr, &dataLength) != 0) {
        return -1;
    }
    
    /// @TODO, 简单http完成
    if (strstr(statusStr, "content")) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_PUB_SUCCESS;
    } else if (strstr(statusStr, "err")) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_PUB_FAILED;
    }
    
    return 0;
}
#endif

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
RILAT_COMMAND_MATCH_DEFINE(_QIURC, "+MQTTURC:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;
    char *statusStr = NULL;
    uint32_t dataLength = 0;
    cModule_TransmitPackageInfo_t *pack = NULL;

    if (klAttoken_start(&line) != 0) {
        return -1;
    }

    // StatusString
    if (klAttoken_getNextString(&line, &statusStr, (int32_t *) &dataLength) != 0) {
        return -1;
    }

    if (strstr(statusStr, "publish")) {
        klAttoken_skip(&line);   // clientId
        klAttoken_skip(&line);   // mid
        
        char *topic = NULL;
        int32_t payloadLength = 0;
        int32_t topicLength = 0;
        if (klAttoken_getNextString(&line, &topic, &topicLength) != 0) {
            return -1;
        }
        klAttoken_skip(&line);   // topicLength
        if (klAttoken_getNextInt(&line, &payloadLength) != 0) {
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
        
//        pack->var.ptr = calloc(1, topicLength + 1);
//        if (pack->var.ptr == NULL) {
//            goto l_errorExit;
//        }
//        pack->varLength = topicLength;
//        strncpy(pack->var.ptr, topic, topicLength);
//        pack->flag.varIsLengthItem = 1;
//        pack->flag.packHasVar = 1;
        
        pack->flag.requestId = TRANSMIT_PACK_REQ_ID_RX;
        klist_init(&pack->list);
        klist_addTail(&mModuleInstance.transmit.packageRepList, &pack->list);
        mModuleInstance.transmit.queneRepCount++;
        _cModule_packMutexLock(&mModuleInstance, false);
    } else if (strstr(statusStr, "conn")) {
        uint32_t resultId = 0;
        klAttoken_skip(&line);   // clientId
        if (klAttoken_getNextUInt(&line, &resultId) != 0) {
            return -1;
        }
        if (resultId == 0) {
            mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_CONN_SUCCESS);
        } else if (resultId == 3 || resultId == 2) {
            if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CONN_SUCCESS) {
                mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_NEED_RESET | CMODULE_URC_FLAG_CONN_FAILED);
                mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_CONN_SUCCESS);
            } else {
                mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_CONN_FAILED);
            }
        } else if (resultId == 1) {
        } else {
            mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_NEED_RESET | CMODULE_URC_FLAG_CONN_FAILED);
        }
    } else if (strstr(statusStr, "suback")) {
        uint32_t resultId = 0;
        klAttoken_skip(&line);   // clientId
        klAttoken_skip(&line);   // messageId
        if (klAttoken_getNextUInt(&line, &resultId) != 0) {
            return -1;
        }
        if (resultId == 128) {
            mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_SUB_FAILED);
        } else {
            mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_SUB_SUCCESS);
        }
    } else if (strstr(statusStr, "pubcomp") || strstr(statusStr, "puback")) {
        uint32_t resultId = 0;
        klAttoken_skip(&line);   // clientId
        klAttoken_skip(&line);   // messageId
        if (klAttoken_getNextUInt(&line, &resultId) != 0) {
            return -1;
        }
        if (resultId == 128) {
            mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_PUB_FAILED);
        } else {
            mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_PUB_SUCCESS);
        }
    }
    
    l_errorExit:
    _cModule_packMutexLock(&mModuleInstance, false);
    if (pack != NULL) {
        if (pack->payload != NULL) {
            free(pack->payload);
        }
        free(pack);
    }
    return -1;
}
#endif

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_TCP
RILAT_COMMAND_MATCH_DEFINE(_MIPURC, "+MIPURC:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;
    char *statusStr = NULL;
    int32_t dataLength = 0;
    cModule_TransmitPackageInfo_t *pack = NULL;

    if (klAttoken_start(&line) != 0) {
        return -1;
    }

    // StatusString
    if (klAttoken_getNextString(&line, &statusStr, &dataLength) != 0) {
        return -1;
    }

    if (strstr(statusStr, "rtcp") || strstr(statusStr, "rudp")) {
        uint32_t payloadLength = 0;
        char *payload = NULL;
        
        klAttoken_skip(&line);   // <connect_id>
        klAttoken_skip(&line);   // <recv_length>
        
        if (!_cModule_isAllocRepPackAccess(&mModuleInstance)) {
            return -1;
        }
        _cModule_packMutexLock(&mModuleInstance, true);
        
        while (*line != '\0' && isspace(*line)) {
            (line)++;
        }
        payloadLength = strlen(line);
        payload = calloc(1, payloadLength + 1);
        if (payload == NULL) {
            _cModule_packMutexLock(&mModuleInstance, false);
            return -1;
        }
        payloadLength = klStr_hexStr2hex(line, payloadLength, payload);
        
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
    } else if (strstr(statusStr, "disconn") || strstr(statusStr, "drop")) {
        mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_NEED_RESET | CMODULE_URC_FLAG_CONN_FAILED);
    }
    
    l_errorExit:
    _cModule_packMutexLock(&mModuleInstance, false);
    if (pack != NULL) {
        if (pack->payload != NULL) {
            free(pack->payload);
        }
        free(pack);
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
            rc = socketTcpSend(msg, info->aux.gen.msgId);
        } else {
            // TODO UdpSend
        }
    }
    return rc;
}


static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info) {
    if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_GNSS_UPDATE_ONCE) {
        gnss_startup_once();
    }
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

cModule_Instance_t *cModule_getInstance_ml307a(void) {
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