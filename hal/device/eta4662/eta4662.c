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

#include "emmk-config.h"
#include "emmk-driver.h"

#include "./eta4662.h"

/**
 * @addtogroup ProgramProfile
 * @note none
 */
 
/*@{*/

#define CONFIG_DRIVER_USING_CONST        (1)

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define _ADDRESS     (0x0E >> 1)

#define INPUT_SOURCE_CONTROL_REG        0x00

#define POWER_ON_CONFIG_REG          0x01
#define POWER_ON_CONFIG_REG_CEB      (0x08)  
#define POWER_ON_CONFIG_REG_EN_HIZ   (0x10)  

#define CHAREGE_BATTERY_VOLT_REG                   0x04
#define CHAREGE_BATTERY_VOLT_REG_VBAT_REG_MASK     (0x3F << 2u)

#define CHAREGE_CURRENT_CTL_REG                   0x02
#define CHAREGE_CURRENT_CTL_REG_ICHG_MASK         (0x3F)

#define DIS_CHARGE_CTL_REG                   0x03
#define DIS_CHARGE_CTL_REG_IDSCHG_MASK         (0xF0)

#define CHARGE_TERMINATION_TIMER_REG                   0x05
#define CHARGE_TERMINATION_TIMER_REG_WATCHDOG_MASK         (0x60)

#define MISCELLANEOUS_OPERATION_CONTROL_REG             0x06
#define MISCELLANEOUS_OPERATION_CONTROL_REG_BFET_DIS   (0x20)
#define MISCELLANEOUS_OPERATION_CONTROL_REG_EN_NTC     (0x80)

#define SYSTEM_STATUS_REG            0x08
#define SYSTEM_STATUS_REG_CHG_STAT   (0x08 | 0x10)
#define SYSTEM_STATUS_REG_PG_STAT    (0x02)

#define OTP_REG                   0x0A
#define OTP_REG_DIS_VDD           (0x04)

/*@}*/

/**
 * @addtogroup Local variables
 * @note none
 */
 
/*@{*/

#if CONFIG_DRIVER_USING_CONST == 0
static kdI2C_t *gDevEta4662Interface = NULL;
#else
extern const kdi2c_t * const gDevEta4662Interface;
#endif
static uint8_t mIsInterfaceInit = 0;

static uint8_t mStatusReg = 0;
static uint8_t mFaultReg = 0;

/*@}*/

/**
 * @addtogroup Local func
 * @note none
 */
 
/*@{*/

static inline int32_t reg_read(uint8_t reg, uint8_t *data) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regRead((void *) gDevEta4662Interface, _ADDRESS, reg, 1, data, 1);
}


static inline int32_t reg_write(uint8_t reg, uint8_t data) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regWrite((void *) gDevEta4662Interface, _ADDRESS, reg, 1, &data, 1); 
}

/*@}*/

/**
 * @addtogroup 
 * @note none
 */
 
/*@{*/

void eta4662_initBus(void *devIf) {
    if (mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 1;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevEta4662Interface != NULL) {
        return;
    }
#endif
        
#if CONFIG_DRIVER_USING_CONST == 0
    gDevEta4662Interface = devIf;
    ASSERT(gDevEta4662Interface != NULL);
#endif
    kdi2c_init((void *) gDevEta4662Interface);
    kdi2c_powerUp((void *) gDevEta4662Interface);
}


int32_t eta4662_initSoft(uint8_t vbatCharegeOffset, 
        uint8_t chrageCurrent,
        uint8_t chgInAmpLimit,
        uint8_t chgIterm,
        uint8_t vsys
        ) {
    uint8_t regData = 0;
    
    /// @PowerOnConfigRegister, 0x01
    if (reg_read(POWER_ON_CONFIG_REG, &regData) != 0) {
        return -1;
    }
    regData |= (1 << 3); // [CEB=1]
    regData &= ~(1 << 4); // [EN_HIZ=0]
    regData &= ~GENMASK(2, 0); // [UVLO=2.58V]
    regData |= FIELD_PREP(GENMASK(2, 0), 0b010);
    if (reg_write(POWER_ON_CONFIG_REG, regData) != 0) {
        return -1;
    }
    
    /// @InputSourceControlRegister, 0x00
    if (reg_read(INPUT_SOURCE_CONTROL_REG, &regData) != 0) {
        return -1;
    }
    regData &= ~GENMASK(3, 0); // IIN_LIM[3:0]
    // regData &= ~GENMASK(7, 4); // VINDPM[3:0]
    regData |= FIELD_PREP(GENMASK(3, 0), chgInAmpLimit);
        // | FIELD_PREP(GENMASK(7, 4), 0b1111);
    if (reg_write(INPUT_SOURCE_CONTROL_REG, regData) != 0) {
        return -1;
    }
    
    /// @AddressOTPRegister, 0x0A
    if (reg_read(OTP_REG, &regData) != 0) {
        return -1;
    }
    regData &= ~(1 << 2); // [DIS_VDD=0]
    regData &= ~(1 << 3); // [SWITCH_MODE=0]
    if (reg_write(OTP_REG, regData) != 0) {
        return -1;
    }
    
    /// @ChargeCurrentControlRegister, 0x02
    if (reg_read(CHAREGE_CURRENT_CTL_REG, &regData) != 0) {
        return -1;
    }
    regData &= ~CHAREGE_CURRENT_CTL_REG_ICHG_MASK;
    regData |= chrageCurrent;
    if (reg_write(CHAREGE_CURRENT_CTL_REG, regData) != 0) {
        return -1;
    }
    
    /// @DischargeCurrentRegister, 0x03
    if (reg_read(DIS_CHARGE_CTL_REG, &regData) != 0) {
        return -1;
    }
    regData &= ~DIS_CHARGE_CTL_REG_IDSCHG_MASK;
    regData |= DIS_CHARGE_CTL_REG_IDSCHG_MASK;
    regData &= ~GENMASK(3, 0);
    regData |= FIELD_PREP(GENMASK(3, 0), chgIterm);
    if (reg_write(DIS_CHARGE_CTL_REG, regData) != 0) {
        return -1;
    }
    
    /// @SystemVoltageRegulationRegister, 0x07
    if (reg_read(0x07, &regData) != 0) {
        return -1;
    }
    regData &= ~GENMASK(3, 0);
    regData |= FIELD_PREP(GENMASK(3, 0), vsys);  // VSYS_REG
    regData |= KLBIT(7);    // ENB_PCB_OTP=0
    if (reg_write(0x07, regData) != 0) {
        return -1;
    }
    
    /// @ChargeVoltageControlRegister, 0x04
    if (reg_read(CHAREGE_BATTERY_VOLT_REG, &regData) != 0) {
        return -1;
    }
    regData &= ~CHAREGE_BATTERY_VOLT_REG_VBAT_REG_MASK;
    regData |= vbatCharegeOffset << 2u;
    if (reg_write(CHAREGE_BATTERY_VOLT_REG, regData) != 0) {
        return -1;
    }

    /// @ChargeControlRegister, 0x05
    if (reg_read(CHARGE_TERMINATION_TIMER_REG, &regData) != 0) {
        return -1;
    }
    regData &= ~(0x60); // [WATCHDOG[1:0]=00]
    if (reg_write(CHARGE_TERMINATION_TIMER_REG, regData) != 0) {
        return -1;
    }
    
    /// @MiscellaneousOperationControlRegister, 0x06
    if (reg_read(MISCELLANEOUS_OPERATION_CONTROL_REG, &regData) != 0) {
        return -1;
    }
    regData |= KLBIT(7); // [EN_NTC=1]
    regData &= ~KLBIT(7);
    if (reg_write(MISCELLANEOUS_OPERATION_CONTROL_REG, regData) != 0) {
        return -1;
    }

    /// @PowerOnConfigRegister, 0x01
    if (reg_read(POWER_ON_CONFIG_REG, &regData) != 0) {
        return -1;
    }
    regData &= ~(1 << 3); // [CEB=0]
    if (reg_write(POWER_ON_CONFIG_REG, regData) != 0) {
        return -1;
    }
    
    return 0;
}


void eta4662_finalizeBus(void) {
    if (!mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 0;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevEta4662Interface == NULL) {
        return -1;
    }
#endif
    
    kdi2c_powerDown((void *) gDevEta4662Interface);
    kdi2c_finalize((void *) gDevEta4662Interface);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevEta4662Interface = NULL;
#endif
}


void eta4662_finalizeSoft(uint8_t usingSwitchMode) {
    uint8_t regData = 0;
    
    /// @PowerOnConfigRegister, 0x01
    if (reg_read(POWER_ON_CONFIG_REG, &regData) != 0) {
        return;
    }
    regData &= ~(1 << 3); // [CEB=0]
    regData |= (1 << 4); // [EN_HIZ=1]
    if (reg_write(POWER_ON_CONFIG_REG, regData) != 0) {
        return;
    }
    
    /// @MiscellaneousOperationControlRegister, 0x06
    if (reg_read(MISCELLANEOUS_OPERATION_CONTROL_REG, &regData) != 0) {
        return;
    }
    regData &= ~(1 << 7); // [EN_NTC=0]
    if (reg_write(MISCELLANEOUS_OPERATION_CONTROL_REG, regData) != 0) {
        return;
    }
    
    /// @AddressOTPRegister, 0x0A
    if (reg_read(OTP_REG, &regData) != 0) {
        return;
    }
    regData |= (1 << 2); // [DIS_VDD=1]
    if (usingSwitchMode) {
        regData |= (1 << 3); // [SWITCH_MODE=1] // WARNING: SHUTDOWN_SOC
    }
    if (reg_write(OTP_REG, regData) != 0) {
        return;
    }
    
#if 0
    eta4662_enterShippingMode();
#endif
}


void eta4662_debug(void) {
#if 0
    for (uint8_t i = 0; i < 12; i++) {
        uint8_t reg = 0;
        if (reg_read(i, &reg) != 0) {
            LOG_W("ETA4662 <[0x%02X]: Failed", i);
            return;
        }
        LOG_W("ETA4662 <[0x%02X]: 0x%02X", i, reg);
        osDelay(200);
    }
#endif
}


int32_t eta4662_poll(void) {
    uint8_t regData = 0;
    if (reg_read(SYSTEM_STATUS_REG, &regData) != 0) {
        return -1;
    }
    mStatusReg = regData;
    
    // FaultRegister
    if (reg_read(0x09, &regData) != 0) {
        return -1;
    }
    mFaultReg = regData;

    return 0;
}


void eta4662_softReset(void) {
    uint8_t regData;
    if (reg_read(OTP_REG, &regData) != 0) {
        return;
    }
    regData |= KLBIT(4); // [COLD_RESET=1]
    if (reg_write(OTP_REG, regData) != 0) {
        return;
    }
}


void eta4662_chargerSetup(bool enable) {
    uint8_t regData = 0;
    if (enable) {
        if (!eta4662_getOtpStatus()) {
            if (reg_read(POWER_ON_CONFIG_REG, &regData) != 0) {
                return;
            }
            regData &= ~(1 << 3); // [CEB=0]
            if (reg_write(POWER_ON_CONFIG_REG, regData) != 0) {
                return;
            }
        } else {
            return;
        }
    } else {
        if (reg_read(POWER_ON_CONFIG_REG, &regData) != 0) {
            return;
        }
        regData |= (1 << 3); // [CEB=1]
        if (reg_write(POWER_ON_CONFIG_REG, regData) != 0) {
            return;
        }
    }
}


ETA4662_ChargeStatus_t eta4662_getChageStatus(void) {
    return (mStatusReg & SYSTEM_STATUS_REG_CHG_STAT) >> 3;
}


ETA4662_PowerGood_t eta4662_getPowerGood(void) {
    return (mStatusReg & SYSTEM_STATUS_REG_PG_STAT) >> 1 ? 1 : 0;
}


uint8_t eta4662_getOvpStatus(void) {
    return (mStatusReg & BIT(3)) ? 1 : 0;
}


uint8_t eta4662_getOtpStatus(void) {
    return (mFaultReg & BIT(0)) || (mFaultReg & BIT(1)) || (mFaultReg & BIT(4));
}



void eta4662_enterShippingMode(void) {
    uint8_t regData = 0;
    if (reg_read(MISCELLANEOUS_OPERATION_CONTROL_REG, &regData) != 0) {
        ASSERT(0);
    }
    regData |= MISCELLANEOUS_OPERATION_CONTROL_REG_BFET_DIS;
    if (reg_write(MISCELLANEOUS_OPERATION_CONTROL_REG, regData) != 0) {
        ASSERT(0);
    }
}

/*@}*/
