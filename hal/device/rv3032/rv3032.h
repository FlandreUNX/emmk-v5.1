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
 
#ifndef _RV3032_H_
#define _RV3032_H_

#include <stdint.h>
#include "kernel/lib/utils/kldatetime.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

typedef enum {
    RV3032_INT_ENABLE_CLKIE = 0x01 << 6,
    RV3032_INT_ENABLE_UIE = 0x01 << 5,
    RV3032_INT_ENABLE_TIE = 0x01 << 4,
    RV3032_INT_ENABLE_AIE = 0x01 << 3,
    RV3032_INT_ENABLE_EIE = 0x01 << 2,
} RV3032_IntEnableBit_t;


typedef enum {
    RV3032_STATUS_THF = 0x80,
    RV3032_STATUS_TLF = 0x40,
    RV3032_STATUS_UF = 0x20,
    RV3032_STATUS_TF = 0x10,
    RV3032_STATUS_AF = 0x08,
    RV3032_STATUS_EVF = 0x04,
    RV3032_STATUS_PORF = 0x02,
    RV3032_STATUS_VLF = 0x01,
} RV3032_StatusBit_t;

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

extern void rv3032_initBus(const char *path);
extern void rv3032_finalizeBus(void);

extern int32_t rv3032_initSoft(void);
extern void rv3032_finalizeSoft(void);

extern int32_t rv3032_updateConfigToEeprom(void);
extern int32_t rv3032_update(void);

extern int32_t rv3032_setupCharge(void);
extern int32_t rv3032_disableCharge(void);

extern int32_t rv3032_setTimestamp(uint32_t ts);
extern int32_t rv3032_setTimerStruct(klDateTime_SampleTm_t *tm);

extern uint32_t rv3032_getTimestamp(uint32_t *ts);
extern int32_t rv3032_getTimeStruct(klDateTime_SampleTm_t *tm);

extern RV3032_StatusBit_t rv3032_getStatus(void);
extern int32_t rv3032_enableInt(RV3032_IntEnableBit_t sourceBit);
extern int32_t rv3032_disableInt(RV3032_IntEnableBit_t sourceBit);
extern int32_t rv3032_disableAllInt(void);
extern int32_t rv3032_clearAllInt(void);
extern int32_t rv3032_clearInt(RV3032_StatusBit_t sourceBit);

extern int32_t rv3032_setAlarmDate(uint8_t enable, uint8_t date);
extern int32_t rv3032_getAlarmDate(uint8_t *date);

extern int32_t rv3032_setAlarmHour(uint8_t enable, uint8_t hour24);
extern int32_t rv3032_getAlarmHour(uint8_t *hour);

extern int32_t rv3032_setAlarmMinute(uint8_t enable, uint8_t minute);
extern int32_t rv3032_getAlarmMinuter(uint8_t *minute);

extern void rv3032_debugRegData(void);

/*@}*/

#endif
