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
#undef DBG_SECTION_NAME

#define TAG               "cModule"

#define DBG_SECTION_NAME  TAG


#define CASSERT(err) ASSERT(err)

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

__CMODULE_X_SECTION_EXTERN;

/*@}*/

/**
 * @addtogroup Configure
 * @note none
 */

/*@{*/

/*@}*/

/**
 * @addtogroup Variables
 * @note none
 */

/*@{*/

static uint32_t mGenMessageId = 0;

/*@}*/

/**
 * @addtogroup ProtectFunc
 * @note none
 */

/*@{*/

int32_t _cModule_wait(cModule_Instance_t *ins, uint32_t t, bool withRilatPoll) {
    qSTimer_t wait = QSTIMER_INITIALIZER;
    qSTimer_Set(&wait, t);
    while (!qSTimer_Expired(&wait)) {
        if (withRilatPoll) {
            if (rilat_loop(&ins->rilat.instance) < 0) {
                return -1;
            }
        } else {
            if (CREQUEST(ON_BLOCK_POLL, (void *) ins,
                     (ins->state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1 : 0).i32 < 0) {
                return -1;  
            }
        }
    }
    return 0;
}


int32_t _cModule_isAllocReqPackAccess(cModule_Instance_t *ins) {
    if (ins->transmit.queneReqCount < CONFIG_CMODULE_BUFFER_POS_LEN) {
        return 1;
    }
    return 0;
}


int32_t _cModule_isAllocRepPackAccess(cModule_Instance_t *ins) {
    if (ins->transmit.queneRepCount < CONFIG_CMODULE_BUFFER_POS_LEN) {
        return 1;
    }
    return 0;
}


void _cModule_freePack(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info, bool isForce) {
    if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_RX) {
        ins->transmit.queneRepCount--;
    } else {
        ins->transmit.queneReqCount--;
    }

    klist_delete(&info->list);
    if (info->payload != NULL) {
        if (ins->callback->onPtPackPayloadFree != NULL) {
            ins->callback->onPtPackPayloadFree(info, isForce);
        } 
        if (info->flag.packIsDynData || info->flag.packIsDynMqttMsg || info->flag.packIsDynHttpMsg || info->flag.packIsDynTcpIpData) {
            free(info->payload);
        }
    }
    if (info->var.ptr != NULL) {
        if (info->flag.varIsLengthItem) {
            free(info->var.ptr);
        } else if (info->flag.varIsObject && info->varLength != 0) {
            for (uint16_t i = 0; i < info->varLength; i++) {
                free(&((uint32_t *) info->var.ptr)[i]);
            }
        }
    }
    free(info);
}


void _cModule_packMutexLock(cModule_Instance_t *ins, bool lock) {
    if (ins->transmit.mutex != NULL) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
        if (lock) {
            osMutexAcquire(ins->transmit.mutex, osWaitForever);
        } else {
            osMutexRelease(ins->transmit.mutex);
        }
#endif
    }
}


uint32_t *_cModule_getGenMsgId(void) {
    return &mGenMessageId;
}


static void instancePoll(cModule_Instance_t *ins) {
    if (ins->state.urcResponseFlags & CMODULE_URC_FLAG_IN_POLL) {
        return;
    }
    ins->state.urcResponseFlags |= CMODULE_URC_FLAG_IN_POLL;

    /// @PackageOverflowCheck
    do {
        if (_cModule_isAllocReqPackAccess(ins)) {
            if (ins->aux.flag.packageIsFull) {
                ins->aux.flag.packageIsFull = 0;
                qSTimer_Disarm(&ins->aux.txPackageOverFlowTimer);
            }
            break;
        }
        if (!ins->aux.flag.packageIsFull) {
            ins->aux.flag.packageIsFull = 1;
            qSTimer_Set(&ins->aux.txPackageOverFlowTimer, CONFIG_CMODULE_PACKAGE_FULL_CLEAN_TIMEOUT_SEC * 1000);
            break;
        } else if (qSTimer_Status(&ins->aux.txPackageOverFlowTimer) && qSTimer_Expired(&ins->aux.txPackageOverFlowTimer)) {
            qSTimer_Disarm(&ins->aux.txPackageOverFlowTimer);
            
            klist_t *pos = NULL, *t = NULL, *header = &ins->transmit.packageReqList;
            klist_forEachSafe(pos, t, header) {
                cModule_TransmitPackageInfo_t *info = klist_entry(pos, cModule_TransmitPackageInfo_t, list);
                _cModule_freePack(ins, info, true);
            }
        }
    } while (0);

    if (ins->aux.flag.pmuSupport && ins->cmmpu != NULL) {
        if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel > COMPONENT_SLEEP_LV_RUN) {
            if (!klist_empty(&ins->transmit.packageReqList) || !klist_empty(&ins->transmit.packageRepList)) {
                cModule_sleep(COMPONENT_SLEEP_LV_RUN);
            } else {
                ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_IN_POLL;
                return;
            }
        }
        if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel > COMPONENT_SLEEP_LV_IDLE) {
            ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_IN_POLL;
            return;
        }
    }

    rilat_loop(&ins->rilat.instance);

    if (ins->aux.flag.passiveRecvMode) {
        if (ins->aux.flag.pmuSupport && ins->cmmpu != NULL) {
            if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel == COMPONENT_SLEEP_LV_RUN
                && qSTimer_Expired(&((ComponentPmu_t *) ins->cmmpu)->aux.idleTimer)) {
                cModule_sleep(COMPONENT_SLEEP_LV_IDLE);
            }
        }
        ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_IN_POLL;
        return;
    }

    if (ins->state.urcResponseFlags & CMODULE_URC_FLAG_STACK_BLOCK_FOREVER
            || (qSTimer_Status(&ins->aux.blockTimer) && !qSTimer_Expired(&ins->aux.blockTimer))) {
        ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_IN_POLL;
        return;
    } else if (qSTimer_Status(&ins->aux.blockTimer)) {
        qSTimer_Disarm(&ins->aux.blockTimer);
    }

    switch (ins->state.processStep) {
        case CMODULE_PROCESS_STEP_AT: {
            if (ins->aux.flag.pollBlock) {
                if (!qSTimer_Expired(&ins->aux.pollTimer)) {
                    break;
                }
            } else {
                if (!qSTimer_Expired(&ins->aux.pollTimer) && qSTimer_Remaining(&ins->aux.pollTimer) < 10000) {
                    break;
                }
            }
            ins->aux.flag.pollBlock = 0;

            if (ins->aux.initRetryCount == CMODULE_ERR_COUNT) {
                ins->aux.flag.hasInit = 0;
                ins->aux.initRetryCount = 0;
                ins->state.processStep = 0;
                CREQUEST(ON_INIT_FAILED, (void *) ins);

                if (ins->aux.flag.pmuSupport) {
                    if (ins->aux.failedCount == 0) {
                        LOG_I("Try1, RequestPowerUp");
                        ins->callback->onReboot(true);
                    } else if (ins->aux.failedCount == 1) {
                        LOG_I("Try2, RequestPowerUp");
                        ins->callback->onReboot(true);
                    } else if (ins->aux.failedCount == 2) {
                        LOG_I("Try3, RequestReset");
                        ins->callback->onReboot(true);
                    } else {
                        ins->aux.failedCount = 0;
                        cModule_sleep(COMPONENT_SLEEP_LV_IDLE);
                        break;
                    }
                    ins->aux.failedCount++;
                } else {
#if CONFIG_CMODULE_INIT_FAILED_LONG_DELAY == 1
                    if (ins->aux.failedCount == 0) {
                        LOG_I("Try1, RequestReset+PowerUp");
                        ins->callback->onReboot(false);
                        rilat_finalize(&ins->rilat.instance);
                        rilat_init(&ins->rilat.instance);
                        _cModule_wait(ins, 3000, false);
                        ins->callback->onReboot(true);
                    } else if (ins->aux.failedCount == 1) {
                        LOG_I("Try2, RequestPowerUp");
                        ins->callback->onReboot(true);
                    } else if (ins->aux.failedCount == 2) {
                        LOG_I("Try3, RequestReset+PowerUp");
                        ins->callback->onReboot(false);
                        rilat_finalize(&ins->rilat.instance);
                        rilat_init(&ins->rilat.instance);
                        _cModule_wait(ins, 3000, false);
                        ins->callback->onReboot(true);
                    } else {
                        ins->aux.failedCount = 0;
                        ins->aux.flag.pollBlock = 1;
                        qSTimer_Set(&ins->aux.pollTimer, 60 * 1000);
                        break;
                    }
                    ins->aux.failedCount++;
#else
                    if (ins->aux.failedCount == 0) {
                        LOG_I("Try1, RequestReset+PowerUp");
                        ins->callback->onReboot(false);
                        rilat_finalize(&ins->rilat.instance);
                        rilat_init(&ins->rilat.instance);
                        _cModule_wait(ins, 3000, false);
                        ins->callback->onReboot(true);
                    } else if (ins->aux.failedCount == 1) {
                        LOG_I("Try2, RequestPowerUp");
                        ins->callback->onReboot(true);
                    } else if (ins->aux.failedCount == 2) {
                        LOG_I("Try3, RequestReset+PowerUp");
                        ins->callback->onReboot(false);
                        rilat_finalize(&ins->rilat.instance);
                        rilat_init(&ins->rilat.instance);
                        _cModule_wait(ins, 3000, false);
                        ins->callback->onReboot(true);
                    } else {
                        ins->aux.failedCount = 0;
                        ins->aux.flag.pollBlock = 1;
                        qSTimer_Set(&ins->aux.pollTimer, 2000);
                        break;
                    }
                    ins->aux.failedCount++;
#endif
                }
            } else if (ins->callback->onCommandCheckAT() == 0) {
                ins->callback->onCommandAT();
                ins->aux.initRetryCount = 0;
                ins->aux.failedCount = 0;
                ins->state.processStep++;
                CREQUEST(ON_INIT_START, (void *) ins);
                if (ins->state.currentState < CMODULE_STATE_FAILED_RES) {
                    ins->state.currentState = CMODULE_STATE_CONING;
                    CREQUEST(ON_STATE_CHANGED, (void *) ins, ins->state.currentState);
                }
            } else if (ins->aux.initRetryCount > 1) {
                ins->aux.initRetryCount = CMODULE_ERR_COUNT;
                ins->state.processStep = 0;
                ins->state.currentState = CMODULE_STATE_HAL_FAILED;
                CREQUEST(ON_STATE_CHANGED, (void *) ins, ins->state.currentState);
            }

            qSTimer_Set(&ins->aux.pollTimer, 200);
            break;
        }
        case CMODULE_PROCESS_STEP_INIT_COMPLETED: {
            if (!ins->aux.flag.hasInit) {
                CREQUEST(ON_INIT_SUCCESS, (void *) ins);
            }

            ins->aux.flag.hasInit = 1;
            ins->aux.initRetryCount = 0;
            ins->state.currentState = CMODULE_STATE_CON_IPV4;
            CREQUEST(ON_STATE_CHANGED, (void *) ins, ins->state.currentState);
            ins->state.processStep = CMODULE_PROCESS_STEP_RUNNING;
            qSTimer_Set(&ins->aux.pollTimer, ins->aux.nextPollSec * 1000);

            if (ins->cmmpu != NULL) {
                qSTimer_Set(&((ComponentPmu_t *) ins->cmmpu)->aux.idleTimer,
                                   CONFIG_CMODULE_IDLE_TIMEOUT_MS);
            }

            ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_NEED_RESET;
            break;
        }
        case CMODULE_PROCESS_STEP_RUNNING: {
            do {
                if (ins->state.urcResponseFlags & CMODULE_URC_FLAG_NEED_RESET) {
                    ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_NEED_RESET;
                    CREQUEST(ON_STACK_NEED_RESET, (void *) ins);
                    cModule_resetStack(ins);

                    ins->state.currentState = CMODULE_STATE_CONING;
                    CREQUEST(ON_STATE_CHANGED, (void *) ins, ins->state.currentState);
                }
            } while (0);

            ins->callback->onLoop();

            if (ins->aux.flag.hasWakeupSuccessHappend) {
                ins->aux.flag.hasWakeupSuccessHappend = 0;
                CREQUEST(ON_NET_CONNECT, (void *) ins, 1);
            }

            do {
                klist_t *pos = NULL, *t = NULL, *header = &ins->transmit.packageRepList;
                klist_forEachSafe(pos, t, header) {
                    cModule_TransmitPackageInfo_t *info = klist_entry(pos, cModule_TransmitPackageInfo_t, list);

                    if (ins->cmmpu != NULL) {
                        if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel == COMPONENT_SLEEP_LV_IDLE) {
                            cModule_sleep(COMPONENT_SLEEP_LV_RUN);
                        }
                        qSTimer_Set(&((ComponentPmu_t *) ins->cmmpu)->aux.idleTimer,
                                           CONFIG_CMODULE_IDLE_TIMEOUT_MS);
                    }

                    if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_RX) {
                        ins->callback->onPtPackIdReceived(info);
                        _cModule_freePack(ins, info, false);
                    }
                }
            } while (0);
            do {
                klist_t *pos = NULL, *t = NULL, *header = &ins->transmit.packageReqList;
                klist_forEachSafe(pos, t, header) {
                    cModule_TransmitPackageInfo_t *info = klist_entry(pos, cModule_TransmitPackageInfo_t, list);

                    if (ins->cmmpu != NULL) {
                        if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel == COMPONENT_SLEEP_LV_IDLE) {
                            cModule_sleep(COMPONENT_SLEEP_LV_RUN);
                        }
                        qSTimer_Set(&((ComponentPmu_t *) ins->cmmpu)->aux.idleTimer,
                                           CONFIG_CMODULE_IDLE_TIMEOUT_MS);
                    }

                    if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_TX) {
                        if (info->aux.gen.hasConfirmFrame && info->aux.gen.waitConfirmFrame) {
                            if (info->aux.gen.waitConfirmFrameRecved) {
                                CREQUEST(ON_SEND_SUCCESS, (void *) ins, info);

                                _cModule_freePack(ins, info, false);
                            } else if (qSTimer_Remaining(&info->aux.gen.waitConfirmFrameTimer)) {
                                if (info->aux.gen.waitConfirmFrameTimeoutCount != 0) {
                                    qSTimer_Set(&ins->aux.sendAccessTimer, 3000);
                                    info->aux.gen.waitConfirmFrameTimeoutCount--;

                                    info->aux.gen.waitConfirmFrameRecved = 0;
                                    info->aux.gen.waitConfirmFrame = 0;
                                } else {
                                    if (--info->aux.gen.waitConfirmFrameTimeoutAfterReboot != 0) {
                                        info->aux.gen.waitConfirmFrameRecved = 0;
                                        info->aux.gen.waitConfirmFrame = 0;
                                        cModule_resetStack(ins);
                                    } else {
                                        CREQUEST(ON_SEND_FAILED, (void *) ins, info);

                                        cModule_resetStack(ins);
                                        _cModule_freePack(ins, info, false);
                                    }
                                }
                            }
                            break;
                        } else {
                            if (info->aux.gen.sendBeforeDelaySec != 0) {
                                if (info->flag.__delayAccess == 0) {
                                    info->flag.__delayAccess = 1;
                                    qSTimer_Set(&ins->aux.sendAccessTimer, info->aux.gen.sendBeforeDelaySec * 1000);
                                } else if (info->flag.__delayAccess == 1) {
                                    if (qSTimer_Expired(&ins->aux.sendAccessTimer)) {
                                        info->flag.__delayAccess = 2;
                                    }
                                } 
                            }
                            if (qSTimer_Status(&ins->aux.sendAccessTimer) && !qSTimer_Expired(&ins->aux.sendAccessTimer)) {
                                break;
                            }
                        }

                        CREQUEST(ON_SEND_START, (void *) ins, info);
                        int32_t rc = 0;
                        ins->state.urcResponseFlags |= CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL;
                        rc = ins->callback->onPtPackIdTransmit(info);
                        ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL;
                        
                        info->flag.__delayAccess = 0;
                        
                        if (rc != 0) {
                            if (info->aux.gen.sendRetryCount != 0) {
                                if (info->aux.gen.sendRetrySec == 0) {
                                    qSTimer_Set(&ins->aux.sendAccessTimer, 3000);
                                } else {
                                    qSTimer_Set(&ins->aux.sendAccessTimer, info->aux.gen.sendRetrySec * 1000);
                                }
                                info->aux.gen.sendRetryCount--;
                            } else {
                                qSTimer_Set(&ins->aux.sendAccessTimer, 1000);
                                CREQUEST(ON_SEND_FAILED, (void *) ins, info);

                                if (info->aux.gen.sendFailedAfterReboot) {
                                    info->aux.gen.sendFailedAfterReboot--;
                                    info->aux.gen.sendRetryCount = info->aux.gen.sendRetryCountReload;

                                    cModule_resetStack(ins);
                                } else {
                                    _cModule_freePack(ins, info, false);
                                }
                            }
                        } else {
                            if (info->aux.gen.hasConfirmFrame) {
                                info->aux.gen.waitConfirmFrameRecved = 0;
                                info->aux.gen.waitConfirmFrame = 1;
                                qSTimer_Set(&info->aux.gen.waitConfirmFrameTimer,
                                                 info->aux.gen.waitConfirmFrameTimeoutSec * 1000);
                            } else {
                                // qSTimer_Set(&ins->aux.sendAccessTimer, 1000);
                                CREQUEST(ON_SEND_SUCCESS, (void *) ins, info);

                                _cModule_freePack(ins, info, false);
                            }
                        }
                        break;
                    } else {
                        ins->callback->onPtPackIdCustom(info);
                        _cModule_freePack(ins, info, false);
                    }
                }
            } while (0);

            if (ins->aux.flag.pmuSupport) {
                if (ins->cmmpu != NULL) {
                    if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel == COMPONENT_SLEEP_LV_RUN
                        && qSTimer_Expired(&((ComponentPmu_t *) ins->cmmpu)->aux.idleTimer)) {
                        cModule_sleep(COMPONENT_SLEEP_LV_IDLE);
                    }
                }
            }
            break;
        }
        default: {
            ins->callback->onInitStep(ins->state.processStep);
            break;
        }
    }

    ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_IN_POLL;
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

void cModule_init(void) {
    cModule_InstanceConst_t *start = (cModule_InstanceConst_t *) __CMODULE_X_START;
    int32_t tlen = ((cModule_InstanceConst_t *) __CMODULE_X_END) - start;
    for (cModule_InstanceConst_t *ckd = start; ckd != start + tlen; ckd++) {
        cModule_Instance_t *ins = ckd->ins;
        ins->callback->onInstanceInit();
        
        qSTimer_Set(&ins->aux.pollTimer, 100);
        qSTimer_Set(&ins->aux.sendAccessTimer, 100);
        qSTimer_Disarm(&ins->aux.txPackageOverFlowTimer);
        qSTimer_Disarm(&ins->aux.blockTimer);
    }
    mGenMessageId = 0;
}


void cModule_sleep(uint8_t nextSleepLevel) {
    cModule_InstanceConst_t *start = (cModule_InstanceConst_t *) __CMODULE_X_START;
    int32_t tlen = ((cModule_InstanceConst_t *) __CMODULE_X_END) - start;
    for (cModule_InstanceConst_t *ckd = start; ckd != start + tlen; ckd++) {
        cModule_Instance_t *kd = ckd->ins;
        if (!kd->aux.flag.pmuSupport || kd->cmmpu == NULL) {
            continue;
        }
        ComponentPmu_t *pmu = ((ComponentPmu_t *) kd->cmmpu);

        if (pmu->sleep.currentSleepLevel && !nextSleepLevel) {
            switch (pmu->sleep.currentSleepLevel) {
                case COMPONENT_SLEEP_LV_IDLE:
                    qSTimer_Set(&pmu->aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);
                    break;
                case COMPONENT_SLEEP_LV_SLEEP:
                case COMPONENT_SLEEP_LV_SHUTDOWN: {
                    if (!pmu->flag.isReady) {
                        pmu->flag.isReady = 1;
                        LOG_I("PmuUp, %s", kd->rilat.instance.name);
                        kd->callback->onInstancePmu(1);
                    }
                    break;
                }
            }

            pmu->sleep.currentSleepLevel = COMPONENT_SLEEP_LV_RUN;
        } else if ((!pmu->sleep.currentSleepLevel && nextSleepLevel)
                   || (pmu->sleep.currentSleepLevel && pmu->sleep.currentSleepLevel < nextSleepLevel)) {
            pmu->sleep.currentSleepLevel = nextSleepLevel;
            switch (nextSleepLevel) {
                case COMPONENT_SLEEP_LV_IDLE: {
                    LOG_I("PmuIdle, %s", kd->rilat.instance.name);
                    break;
                }
                case COMPONENT_SLEEP_LV_SLEEP:
                case COMPONENT_SLEEP_LV_SHUTDOWN: {
                    if (pmu->flag.isReady) {
                        pmu->flag.isReady = 0;
                        LOG_I("PmuDown, %s", kd->rilat.instance.name);
                        kd->callback->onInstancePmu(0);
                    }
                    break;
                }
            }
        }
    }
}


void cModule_loop(void *specialInstance) {
    if (specialInstance != NULL) {
        instancePoll(specialInstance);
    } else {
        cModule_InstanceConst_t *start = (cModule_InstanceConst_t *) __CMODULE_X_START;
        int32_t tlen = ((cModule_InstanceConst_t *) __CMODULE_X_END) - start;
        for (cModule_InstanceConst_t *ckd = start; ckd != start + tlen; ckd++) {
            cModule_Instance_t *kd = ckd->ins;
            instancePoll(kd);
        }
    }
}


void cModule_resetStack(cModule_Instance_t *ins) {
    if (ins->aux.flag.pmuSupport && ins->cmmpu != NULL) {
        if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel > COMPONENT_SLEEP_LV_IDLE) {
            return;
        }
    }

    ins->callback->onResetStack();

    ins->aux.initRetryCount = 0;
    ins->state.processStep = 0;
    ins->state.urcResponseFlags = 0;
    rilat_finalize(&ins->rilat.instance);
    rilat_init(&ins->rilat.instance);
    
    qSTimer_Set(&ins->aux.pollTimer, 100);
    qSTimer_Set(&ins->aux.sendAccessTimer, 100);
    qSTimer_Disarm(&ins->aux.blockTimer);
    
    if (ins->cmmpu != NULL) {
        if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel == COMPONENT_SLEEP_LV_IDLE) {
            cModule_sleep(COMPONENT_SLEEP_LV_RUN);
        }
        qSTimer_Set(&((ComponentPmu_t *) ins->cmmpu)->aux.idleTimer,  CONFIG_CMODULE_IDLE_TIMEOUT_MS);
    }
}


void cModule_blockFuncPoll(cModule_Instance_t *ins, uint32_t block) {
    if (ins->aux.flag.pmuSupport && ins->cmmpu != NULL) {
        if (((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel > COMPONENT_SLEEP_LV_IDLE) {
            return;
        }
    }

    if (block == 0xFFFFFFFF) {
        ins->state.urcResponseFlags |= CMODULE_URC_FLAG_STACK_BLOCK_FOREVER;
        qSTimer_Disarm(&ins->aux.blockTimer);
    } else if (block == 0) {
        ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_STACK_BLOCK_FOREVER;
        qSTimer_Disarm(&ins->aux.blockTimer);
    } else {
        ins->state.urcResponseFlags &= ~CMODULE_URC_FLAG_STACK_BLOCK_FOREVER;
        qSTimer_Set(&ins->aux.blockTimer, block);
    }
}


int32_t cModule_directWrite(cModule_Instance_t *ins, uint8_t *data, uint16_t dataLength) {
    return rilat_directWrite(&ins->rilat.instance, data, dataLength, 1);
}


void cModule_enablePmu(cModule_Instance_t *ins, bool en) {
    if (en && !ins->aux.flag.pmuSupport) {
        qSTimer_Set(&((ComponentPmu_t *) ins->cmmpu)->aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);
        ins->aux.flag.pmuSupport = true;
    } else if (!en && ins->aux.flag.pmuSupport) {
        cModule_sleep(COMPONENT_SLEEP_LV_RUN);
        ins->aux.flag.pmuSupport = false;
    }
}



inline uint8_t cModule_getSleepStatus(cModule_Instance_t *ins) {
    if (ins->cmmpu != NULL) {
        return ((ComponentPmu_t *) ins->cmmpu)->sleep.currentSleepLevel;
    } else {
        return COMPONENT_SLEEP_LV_RUN;
    }
}


inline cModule_State_t cModule_getNetState(cModule_Instance_t *ins) {
    return ins->state.currentState;
}


inline uint8_t cModule_isBlocking(cModule_Instance_t *ins) {
    if (ins->aux.flag.passiveRecvMode) {
        return 0;
    }
    if (ins->state.urcResponseFlags & CMODULE_URC_FLAG_STACK_BLOCK_FOREVER
            || (qSTimer_Status(&ins->aux.blockTimer) && !qSTimer_Expired(&ins->aux.blockTimer))) {
        return 1;
    }
    return 0;
}


inline uint8_t cModule_getProcessStep(cModule_Instance_t *ins) {
    return ins->state.processStep;
}


inline uint32_t cModule_getTimeStamp(cModule_Instance_t *ins) {
    return ins->aux.timeStamp;
}


inline uint8_t cModule_getRssi(cModule_Instance_t *ins) {
    return ins->modemInfo.rssi;
}


inline char *cModule_getModuleComId(cModule_Instance_t *ins) {
    return ins->modemInfo.imsi;
}


inline char *cModule_getModuleUId(cModule_Instance_t *ins) {
    return ins->modemInfo.imei;
}


inline char *cModule_getModuleIccid(cModule_Instance_t *ins) {
    return ins->modemInfo.iccid;
}

inline cModule_SignalExtInfo_t *cModule_getModuleSignalExtInfo(cModule_Instance_t *ins) {
    return &ins->modemInfo.sei;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef DBG_SECTION_NAME
#undef TAG

/*@}*/
