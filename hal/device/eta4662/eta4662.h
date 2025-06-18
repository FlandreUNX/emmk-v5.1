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
 
#ifndef _ETA4662_H_
#define _ETA4662_H_
 
#include <stdint.h>

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

typedef enum {
    ETA4662_CHARGE_STATUS_NO_CHARGING = 0x00,
    ETA4662_CHARGE_STATUS_PRE_CHARGING = 0x01,
    ETA4662_CHARGE_STATUS_CHARGING = 0x02,
    ETA4662_CHARGE_STATUS_CHARGE_DONE = 0x03,
} ETA4662_ChargeStatus_t;

typedef enum {
    ETA4662_POWER_FAIL = 0,
    ETA4662_POWER_GOOD = 1,
} ETA4662_PowerGood_t;

/*@}*/

/**
 * @addtogroup VFS iotcl support
 * @note nones
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

extern void eta4662_initBus(void *devIf);
extern void eta4662_finalizeBus(void);

extern int32_t eta4662_initSoft(uint8_t vbatCharegeOffset, 
        uint8_t chrageCurrent,
        uint8_t chgInAmpLimit,
        uint8_t chgIterm,
        uint8_t vsys
        );
extern void eta4662_finalizeSoft(uint8_t usingSwitchMode);

extern void eta4662_debug(void);

extern void eta4662_softReset(void);
extern void eta4662_chargerSetup(bool enable);
extern void eta4662_enterShippingMode(void);

extern int32_t eta4662_poll(void);

extern ETA4662_ChargeStatus_t eta4662_getChageStatus(void);
extern ETA4662_PowerGood_t eta4662_getPowerGood(void);
extern uint8_t eta4662_getOvpStatus(void);
extern uint8_t eta4662_getOtpStatus(void);

/*@}*/

#endif
