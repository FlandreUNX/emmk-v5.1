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

#include "emmk-config.h"
#include "emmk-driver.h"

#include "./rv3032.h"

/**
 * @addtogroup ProgramProfile
 * @note none
 */
 
/*@{*/

#define CONFIG_DRIVER_USING_CONST        (1)

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

#define RV3032_ADDR							0x51

#define SUNDAY 0x01
#define MONDAY 0x02
#define TUESDAY 0x04
#define WEDNESDAY 0x08
#define THURSDAY 0x10
#define FRIDAY 0x20
#define SATURDAY 0x40

#define RV3032_HUNDREDTHS            0x00
#define RV3032_SECONDS               0x01
#define RV3032_MINUTES               0x02
#define RV3032_HOURS                 0x03
#define RV3032_WEEKDAYS              0x04
#define RV3032_DATE                  0x05
#define RV3032_MONTHS                0x06
#define RV3032_YEARS                 0x07
#define RV3032_MINUTES_ALARM         0x08
#define RV3032_HOURS_ALARM           0x09
#define RV3032_DATE_ALARM            0x0A
#define RV3032_TIMER_0               0x0B
#define RV3032_TIMER_1               0x0C
#define RV3032_STATUS                0x0D
#define RV3032_TEMP_LSB              0x0E
#define RV3032_TEMP_MSB              0x0F
#define RV3032_CONTROL1              0x10
#define RV3032_CONTROL2              0x11
#define RV3032_CONTROL3              0x12
#define RV3032_TS_CONTROL            0x13
#define RV3032_CLOCK_INT_MASK        0x14
#define RV3032_EVI_CONTROL           0x15
#define RV3032_HUNDREDTHS_CAPTURE    0x27
#define RV3032_SECONDS_CAPTURE       0x28
#define RV3032_MINUTES_CAPTURE       0x29
#define RV3032_HOURS_CAPTURE         0x2A
//#define RV3032_DAY_CAPTURE         0x2B //Not used, can configure EVI Timestamps to display day, month, and year if desired
//#define RV3032_MONTH_CAPTURE       0x2C
//#define RV3032_YEAR_CAPTURE        0x2D
#define RV3032_EEPROM_OFFSET         0xC1
//#define RV3032_EEPROM_CLKOUT_1     0xC2 //Used for HF mode CLKOUT readings, default is XTAL mode
#define RV3032_EEPROM_CLKOUT_2       0xC3
#define RV3032_PMU			0xC0
//Enable Bits for Alarm Registers
#define ALARM_ENABLE						7

////Extension Register Bits
//#define EXTENSION_TEST				7
//#define EXTENSION_WADA				6
//#define EXTENSION_USEL				5
//#define EXTENSION_TE					4
//#define EXTENSION_FD					2
//#define EXTENSION_TD					0

////Flag Register Bits
//#define FLAG_UPDATE						5
//#define FLAG_TIMER						4
//#define FLAG_ALARM					  3
//#define FLAG_EVI							2
//#define FLAG_V2F							1
//#define FLAG_V1F							0

#define RV3032_PMU_TCM			GENMASK(1, 0)
#define RV3032_PMU_TCR			GENMASK(3, 2)
#define RV3032_PMU_BSM			GENMASK(5, 4)
#define RV3032_PMU_NCLKE		BIT(6)
#define RV3032_PMU_BSM_DSM		1
#define RV3032_PMU_BSM_LSM		2

//Status Register Bits (1 is triggered, remember to reset!)
#define STATUS_THF              BIT(7) // Temp. High Flag
#define STATUS_TLF              BIT(6) // Temp Low Flag
#define STATUS_UF               BIT(5) // Periodic Time Update Flag
#define STATUS_TF               BIT(4) // Periodic Countdown Update Flag
#define STATUS_AF               BIT(3) // Alarm Flag
#define STATUS_EVF              BIT(2) // External Event Flag
#define STATUS_PORF             BIT(1) // Power On Reset Flag
#define STATUS_VLF              BIT(0) // Voltage Low Flag

//Control 1 Register Bits
#define CONTROL1_USEL           4 // Update Interrupt Select
#define CONTROL1_TE             3 // Periodic Countdown Timer Enable
#define CONTROL1_EERD           2 // EEPROM Memory Refresh Disable
#define CONTROL1_TD             0 // Timer Clock Frequency selection

//Control 2 Register Bits
#define CONTROL2_CLKIE          6 // Interrupt Controlled Clock Output Enable
#define CONTROL2_UIE            5 // Periodic Time Update Interrupt Enable
#define CONTROL2_TIE            4 // Periodic Countdown Timer Interrupt Enable
#define CONTROL2_AIE            3 // Alarm Interrupt Enable
#define CONTROL2_EIE            2 // External Event Interrupt Enable bit
#define CONTROL2_STOP           0 // Stop, used for synchronization

//Control 3 Register Bits
#define CONTROL3_BSIE           4 // Backup Switchover Interrupt Enable
#define CONTROL3_THE            3 // Temperature High Enable
#define CONTROL3_TLE            2 // Temperature Low Enable
#define CONTROL3_THIE           1 // Temperature High Interrupt Enable
#define CONTROL3_TLIE           0 // Temperature Low Interrupt Enable

//TS Control Register Bits
#define TS_CONTROL_EVR          7 // Time Stamp EVI Reset
#define TS_CONTROL_EVOW         2 // Time Stamp EVI Overwrite

//EVI Control Register Bits
#define EVI_CONTROL_EHL         6 // Event High/Low Level (Rising/Falling Edge) selection
#define EVI_CONTROL_ET          4 // Event Filtering Time set
#define EVI_CONTROL_ESYN        0 // Event Filtering Time set

//EEPROM CLKOUT Register Bits
#define EEPROM_CLKOUT2_OS       7 // Oscillator Selection
#define EEPROM_CLKOUT2_FD       5 // CLKOUT Frequency Selection in XTAL mode

#define RV3032_CTRL1_EERD		BIT(3)

// Possible Settings
#define TWELVE_HOUR_MODE					         true
#define TWENTYFOUR_HOUR_MODE				       false
#define COUNTDOWN_TIMER_FREQUENCY_4096_HZ	 0b00
#define COUNTDOWN_TIMER_FREQUENCY_64_HZ		 0b01
#define COUNTDOWN_TIMER_FREQUENCY_1_HZ		 0b10
#define COUNTDOWN_TIMER_FREQUENCY_1_60_HZ	 0b11
#define CLKOUT_FREQUENCY_32768_HZ	      	 0b00
#define CLKOUT_FREQUENCY_1024_HZ		     	 0b01
#define CLKOUT_FREQUENCY_64_HZ		      	 0b10
#define CLKOUT_FREQUENCY_1_HZ              0b11

#define COUNTDOWN_TIMER_ON				       	 true
#define COUNTDOWN_TIMER_OFF					       false
#define TIME_UPDATE_1_SECOND				       false
#define TIME_UPDATE_1_MINUTE				       true

#define ENABLE_EVI_CALIBRATION				     true
#define DISABLE_EVI_CALIBRATION				     false
#define EVI_DEBOUNCE_NONE					         0b00 // 0ms, default
#define EVI_DEBOUNCE_256HZ					       0b01 // 256Hz ~= 3.9ms
#define EVI_DEBOUNCE_64HZ					         0b10 // 64Hz ~= 15.6ms
#define EVI_DEBOUNCE_8HZ					         0b11 // 8Hz ~= 125ms
#define RISING_EDGE							           true
#define FALLING_EDGE						           false
#define EVI_CAPTURE_ENABLE			        	 true
#define EVI_CAPTURE_DISABLE					       false

#define ENABLE								             true
#define DISABLE								             false

#define TIME_ARRAY_LENGTH                  7 // Total number of writable values in device

#define RV3032_EEPROM_CMD           0x3F
#define RV3032_EEPROM_CMD_UPDATE	0x11
#define RV3032_EEPROM_CMD_WRITE		0x21
#define RV3032_EEPROM_CMD_READ		0x22

enum time_order {
	TIME_SECONDS,		
	TIME_MINUTES,		
	TIME_HOURS,			
	TIME_WEEKDAY,		
	TIME_DATE,			
	TIME_MONTH,			
	TIME_YEAR,			
};

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

#if CONFIG_DRIVER_USING_CONST == 0
static kdI2C_t *gDevRv3032Interface = NULL;
#else
extern const kdi2c_t * const gDevRv3032Interface;
#endif
static uint8_t mIsInterfaceInit = 0;

static uint8_t mTime[TIME_ARRAY_LENGTH];

/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */
 
/*@{*/

static inline int32_t _readRegs(uint8_t reg, uint8_t count, uint8_t *value) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regRead((void *) gDevRv3032Interface, RV3032_ADDR, reg, 1, value, count);
}


static inline int32_t _readReg(uint8_t reg, uint8_t *value) {
    return _readRegs(reg, 1, value);
}


static inline int32_t _writeRegs(uint8_t reg, uint8_t count, uint8_t *value) {
    if (!mIsInterfaceInit) {
        return -1;
    } 
    return kdi2c_regWrite((void *) gDevRv3032Interface, RV3032_ADDR, reg, 1, value, count);
}


static inline int32_t _writeReg(uint8_t reg, uint8_t value) {
    return _writeRegs(reg, 1, &value);
}


static inline int32_t _writeTimes(uint8_t dateTime[7]) {
    return _writeRegs(RV3032_SECONDS, 7, dateTime);
}


static inline int32_t _readTimes(uint8_t dateTime[7]) {
    return _readRegs(RV3032_SECONDS, 7, dateTime);
}

/*@}*/

/**
 * @addtogroup PrivateFunc
 * @note none
 */
 
/*@{*/

static int32_t enterEerd(uint32_t *eerd) {
    uint8_t reg = 0;
    
    if (_readReg(RV3032_CONTROL1, &reg) != 0) {
        return -1;
    }
    *eerd = reg & RV3032_CTRL1_EERD;
    if (*eerd) {
		return 0;
    }
    
    reg |= RV3032_CTRL1_EERD;
    if (_writeReg(RV3032_CONTROL1, reg) != 0) {
        return -1;
    }
    return 0;
}


static int32_t exitEerd(uint32_t eerd) {
    uint8_t reg = 0;
    
    if (eerd) {
		return 0;
    }
    
    if (_readReg(RV3032_CONTROL1, &reg) != 0) {
        return -1;
    }
    reg &= ~RV3032_CTRL1_EERD;
    if (_writeReg(RV3032_CONTROL1, reg) != 0) {
        return -1;
    }
    
    return 0;
}


static int32_t updateConfigToEeprom(void) {
    uint32_t eerd = 0;
    if (enterEerd(&eerd) != 0) {
        return -1;
    }
    
    if (_writeReg(RV3032_EEPROM_CMD, RV3032_EEPROM_CMD_UPDATE) != 0) {
        return -1;
    }
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osDelay(5);
#else
    qSTimer_t wait;
    qSTimer_Set(&wait, 5);
    while (!qSTimer_Expired(&wait));
#endif
    
    return exitEerd(eerd);
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

void rv3032_initBus(const char *path) {
    if (mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 1;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevRv3032Interface != NULL) {
        return -1;
    }
#endif
    
#if CONFIG_DRIVER_USING_CONST == 0
    gDevRv3032Interface = KDINSTANCE(I2C, path);
    ASSERT(gDevRv3032Interface != NULL);
#endif
    kdi2c_init((void *) gDevRv3032Interface);
    kdi2c_powerUp((void *) gDevRv3032Interface);
}


void rv3032_finalizeBus(void) {
    if (!mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 0;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevRv3032Interface == NULL) {
        return;
    }
#endif
    
    kdi2c_powerDown((void *) gDevRv3032Interface);
    kdi2c_finalize((void *) gDevRv3032Interface);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevRv3032Interface = NULL;
#endif
}


int32_t rv3032_initSoft(void) {
    memset(mTime, 0x00, sizeof(mTime));
    
    uint8_t reg = 0;
    if (_readReg(RV3032_STATUS, &reg) != 0) {
        return -1;
    }
//    if (reg & STATUS_PORF || reg & STATUS_VLF) {
//        reg &= ~(STATUS_PORF | STATUS_VLF);
//        if (_writeReg(RV3032_STATUS, reg) != 0) {
//            return -1;
//        }
//    }
    
    if (_readReg(RV3032_CONTROL1, &reg) != 0) {
        return -1;
    }
    reg |= BIT(5);
    if (_writeReg(RV3032_CONTROL1, reg) != 0) {
        return -1;
    }
    
    if (_writeReg(RV3032_EVI_CONTROL, 0) != 0) {
        return -1;
    }
    
    return 0;
}


void rv3032_finalizeSoft(void) {

}


int32_t rv3032_setupCharge(void) {
    uint8_t val = FIELD_PREP(RV3032_PMU_TCM, 2)             /// @Note: 239320, 下次板子改1, 输出3.3V
        | FIELD_PREP(RV3032_PMU_BSM, RV3032_PMU_BSM_LSM)
        | FIELD_PREP(RV3032_PMU_TCR, 0)
        | FIELD_PREP(RV3032_PMU_NCLKE, 1);
    if (_writeReg(RV3032_PMU, val) != 0) {
        return -1;
    }        
    
    return updateConfigToEeprom();
}


int32_t rv3032_disableCharge(void) {
    uint8_t val = FIELD_PREP(RV3032_PMU_BSM, RV3032_PMU_BSM_LSM)
        | FIELD_PREP(RV3032_PMU_NCLKE, 1);
    if (_writeReg(RV3032_PMU, val) != 0) {
        return -1;
    }        
    
    return updateConfigToEeprom();
}


int32_t rv3032_updateConfigToEeprom(void) {
    return updateConfigToEeprom();
}


int32_t rv3032_update(void) {
    uint8_t status = 0;

    if (_readTimes(mTime) != 0) {
        return -1;
    }
    
    if (bcd2bin(mTime[TIME_SECONDS & 0x7F]) == 59) {
        uint8_t tempTime[TIME_ARRAY_LENGTH];
        
        if (_readTimes(tempTime) != 0) {
            return -1;
        }
        
        if (bcd2bin(tempTime[TIME_SECONDS]) == 0) {
            memcpy(mTime, tempTime, TIME_ARRAY_LENGTH);
        }
    }
    
    return 0;
}


int32_t rv3032_setTimestamp(uint32_t ts) {
    time_t t = ts;
	klDateTime_SampleTm_t tm;
    uint8_t reg = 0;
    
    if (ts == 0) {
        mTime[TIME_SECONDS] = 0;
        mTime[TIME_MINUTES] = 0;
        mTime[TIME_HOURS] = 0;
        mTime[TIME_DATE] = 1;
        mTime[TIME_WEEKDAY] = 0; // rv3032, wday1 = 0
        mTime[TIME_MONTH] = 1; // rv3032, mon1 = 1
        mTime[TIME_YEAR] = 0;
    } else {
        klDateTime_bktime(ts, &tm);
        tm.wday = klDateTime_getWeekday(tm.year, tm.mon, tm.day); 
        
        mTime[TIME_SECONDS] = bin2bcd(tm.sec);
        mTime[TIME_MINUTES] = bin2bcd(tm.min);
        mTime[TIME_HOURS] = bin2bcd(tm.hour);
        mTime[TIME_DATE] = bin2bcd(tm.day);
        mTime[TIME_WEEKDAY] = tm.wday - 1; // rv3032, wday1 = 0
        mTime[TIME_MONTH] = bin2bcd(tm.mon); // rv3032, mon1 = 1
        mTime[TIME_YEAR] = bin2bcd(tm.year - 2000);
    }

    if (_readReg(RV3032_CONTROL2, &reg) != 0) {
        return -1;
    }
    reg |= 0x01; // STOP (B0)
    if (_writeReg(RV3032_CONTROL2, reg) != 0) {
        return -1;
    }

    if (_writeTimes(mTime) != 0) {
        return -1;
    }
    
//    if (_readReg(RV3032_STATUS, &reg) != 0) {
//        return -1;
//    }
//    if (reg & STATUS_PORF || reg & STATUS_VLF) {
//        reg &= ~(STATUS_PORF | STATUS_VLF);
//        if (_writeReg(RV3032_STATUS, reg) != 0) {
//            return -1;
//        }
//    }
    
    if (_readReg(RV3032_CONTROL2, &reg) != 0) {
        return -1;
    }
    reg &= ~0x01; // STOP (B0)
    if (_writeReg(RV3032_CONTROL2, reg) != 0) {
        return -1;
    }
    
    return 0;
}


int32_t rv3032_setTimerStruct(klDateTime_SampleTm_t *tm) {
    uint8_t reg = 0;
    
    mTime[TIME_SECONDS] = bin2bcd(tm->sec);
	mTime[TIME_MINUTES] = bin2bcd(tm->min);
	mTime[TIME_HOURS] = bin2bcd(tm->hour);
	mTime[TIME_DATE] = bin2bcd(tm->day);
	mTime[TIME_WEEKDAY] = tm->wday - 1;
	mTime[TIME_MONTH] = bin2bcd(tm->mon);
	mTime[TIME_YEAR] = bin2bcd(tm->year - 2000);
    
    if (_readReg(RV3032_CONTROL2, &reg) != 0) {
        return -1;
    }
    reg |= 0x01; // STOP (B0)
    if (_writeReg(RV3032_CONTROL2, reg) != 0) {
        return -1;
    }

    if (_writeTimes(mTime) != 0) {
        return -1;
    }
    
//    if (_readReg(RV3032_STATUS, &reg) != 0) {
//        return -1;
//    }
//    if (reg & STATUS_PORF || reg & STATUS_VLF) {
//        reg &= ~(STATUS_PORF | STATUS_VLF);
//        if (_writeReg(RV3032_STATUS, reg) != 0) {
//            return -1;
//        }
//    }
    
    if (_readReg(RV3032_CONTROL2, &reg) != 0) {
        return -1;
    }
    reg &= ~0x01; // STOP (B0)
    if (_writeReg(RV3032_CONTROL2, reg) != 0) {
        return -1;
    }
        
    return 0;
}


uint32_t rv3032_getTimestamp(uint32_t *ts) {
    klDateTime_SampleTm_t tm;
    time_t t;

	tm.year = bcd2bin(mTime[TIME_YEAR] & 0x7F) + 2000;
	tm.mon = bcd2bin(mTime[TIME_MONTH] & 0x7F);
	tm.day = bcd2bin(mTime[TIME_DATE] & 0x7F);
	tm.hour = bcd2bin(mTime[TIME_HOURS] & 0x7F);
	tm.min = bcd2bin(mTime[TIME_MINUTES] & 0x7F);
	tm.sec = bcd2bin(mTime[TIME_SECONDS] & 0x7F);
    tm.wday = mTime[TIME_WEEKDAY] + 1;
    
    t = klDateTime_mktime(&tm);
    
    if (ts == NULL) {
        return t;
    }
    
    *ts = t;
    return t;
}


int32_t rv3032_getTimeStruct(klDateTime_SampleTm_t *tm) {
	tm->year = bcd2bin(mTime[TIME_YEAR]) + 2000;
	tm->mon = bcd2bin(mTime[TIME_MONTH]);
	tm->day = bcd2bin(mTime[TIME_DATE]);
	tm->hour = bcd2bin(mTime[TIME_HOURS]);
	tm->min = bcd2bin(mTime[TIME_MINUTES]);
	tm->sec = bcd2bin(mTime[TIME_SECONDS]);
    tm->wday = mTime[TIME_WEEKDAY] + 1;
    
    return 0;
}


RV3032_StatusBit_t rv3032_getStatus(void) {
    RV3032_StatusBit_t value = 0;
    
    if (_readReg(RV3032_STATUS, (uint8_t *) &value) != 0) {
        return -1;
    }

    return value;
}


int32_t rv3032_enableInt(RV3032_IntEnableBit_t sourceBit) {
    uint8_t value = 0;
    
    if (_readReg(RV3032_CONTROL2, &value) != 0) {
        return -1;
    }
    
    value |= (sourceBit);
    
    return _writeReg(RV3032_CONTROL2, value);
}


int32_t rv3032_disableInt(RV3032_IntEnableBit_t sourceBit) {
    uint8_t value = 0;
    
    if (_readReg(RV3032_CONTROL2, &value) != 0) {
        return -1;
    }
    
    value &= ~(sourceBit);
    
    return _writeReg(RV3032_CONTROL2, value);
}


int32_t rv3032_disableAllInt(void) {
    uint8_t value = 0;
    
    if (_readReg(RV3032_CONTROL2, &value) != 0) {
        return -1;
    }
    
    value &= 1;
    
    return _writeReg(RV3032_CONTROL2, value);
}


int32_t rv3032_clearAllInt(void) {
    return _writeReg(RV3032_STATUS, 0b00000000);
}


int32_t rv3032_clearInt(RV3032_StatusBit_t sourceBit) {
    uint8_t value = 0;
    
    if (_readReg(RV3032_STATUS, &value) != 0) {
        return -1;
    }
    
    value &= ~(sourceBit); 
    
    return _writeReg(RV3032_STATUS, value);
}


int32_t rv3032_setAlarmDate(uint8_t enable, uint8_t date) {
    uint8_t value = 0;
    
    if (_readReg(RV3032_DATE_ALARM, &value) != 0) {
        return -1;
    }
    
    if (enable) {
        if (date <= 31) {
            value = 0;
            value |= bin2bcd(date) & 0x7F;
            return _writeReg(RV3032_DATE_ALARM, value);
        } else {
            value |= (1 << ALARM_ENABLE);
            return _writeReg(RV3032_DATE_ALARM, value);
        }
    } else {
        value = value & 0x7F;
        value |= (1 << ALARM_ENABLE);
        
        return _writeReg(RV3032_DATE_ALARM, value);
    }
}


int32_t rv3032_getAlarmDate(uint8_t *date) {
    uint8_t rd = 0;
    if (_readReg(RV3032_DATE_ALARM, &rd) != 0) {
        return -1;
    }
    
    *date = bcd2bin(rd & 0x7F);
    
    return 0;
}


int32_t rv3032_setAlarmHour(uint8_t enable, uint8_t hour24) {
    uint8_t value = 0;
    
    if (_readReg(RV3032_HOURS_ALARM, &value) != 0) {
        return -1;
    }
    
    if (enable) {
        if (hour24 <= 23) {
            value = 0;
            value |= bin2bcd(hour24) & 0x7F;
            return _writeReg(RV3032_HOURS_ALARM, value);
        } else {
            value |= (1 << ALARM_ENABLE);
            return _writeReg(RV3032_HOURS_ALARM, value);
        }
    } else {
        value = value & 0x7F;
        value |= (1 << ALARM_ENABLE);
        
        return _writeReg(RV3032_HOURS_ALARM, value);
    }
}


int32_t rv3032_getAlarmHour(uint8_t *hour) {
    uint8_t rd = 0;
    if (_readReg(RV3032_HOURS_ALARM, &rd) != 0) {
        return -1;
    }
    
    *hour = bcd2bin(rd & 0x7F);
    
    return 0;
}


int32_t rv3032_setAlarmMinute(uint8_t enable, uint8_t minute) {
    uint8_t value = 0;
    
    if (_readReg(RV3032_MINUTES_ALARM, &value) != 0) {
        return -1;
    }
    
    if (enable) {
        if (minute <= 60) {
            value = 0;
            value |= bin2bcd(minute) & 0x7F;
            return _writeReg(RV3032_MINUTES_ALARM, value);
        } else {
            value |= (1 << ALARM_ENABLE);
            return _writeReg(RV3032_MINUTES_ALARM, value);
        }
    } else {
        value = value & 0x7F;
        value |= (1 << ALARM_ENABLE);
        
        return _writeReg(RV3032_MINUTES_ALARM, value);
    }
}


int32_t rv3032_getAlarmMinuter(uint8_t *minute) {
    uint8_t rd = 0;
    if (_readReg(RV3032_MINUTES_ALARM, &rd) != 0) {
        return -1;
    }
    
    *minute = bcd2bin(rd & 0x7F);
    
    return 0;
}


void rv3032_debugRegData(void) {
//    LOG_I("RV3032_REG_0x%02X, 0x%02X", RV3032_YEARS, mTime[TIME_YEAR]); osDelay(100);
//    LOG_I("RV3032_REG_0x%02X, 0x%02X", RV3032_MONTHS, mTime[TIME_MONTH]); osDelay(100);
//    LOG_I("DD,RV3032_REG_0x%02X, 0x%02X", RV3032_DATE, mTime[TIME_DATE]); osDelay(100);
//    LOG_I("HH,RV3032_REG_0x%02X, 0x%02X", RV3032_HOURS, mTime[TIME_HOURS]); osDelay(100);
//    LOG_I("MM,RV3032_REG_0x%02X, 0x%02X", RV3032_MINUTES, mTime[TIME_MINUTES]); osDelay(100);
//    LOG_I("RV3032_REG_0x%02X, 0x%02X", RV3032_SECONDS, mTime[TIME_SECONDS]); osDelay(100);
//    LOG_I("RV3032_REG_0x%02X, 0x%02X", RV3032_WEEKDAYS, mTime[TIME_WEEKDAY]); osDelay(100);

//    uint8_t value = 0;
    
//    if (_readReg(RV3032_DATE_ALARM, &value) != 0) {
//        LOG_E("RV3032_REG_0x%02X, ReadError", RV3032_DATE_ALARM); osDelay(100);
//    } else {
//        LOG_I("ADD,RV3032_REG_0x%02X, 0x%02X", RV3032_DATE_ALARM, value); osDelay(100);
//    }
//    value = 0;
//    
//    if (_readReg(RV3032_HOURS_ALARM, &value) != 0) {
//        LOG_E("RV3032_REG_0x%02X, ReadError", RV3032_HOURS_ALARM); osDelay(100);
//    } else {
//        LOG_I("AHH,RV3032_REG_0x%02X, 0x%02X", RV3032_HOURS_ALARM, value); osDelay(100);
//    }
//    value = 0;
//    
//    if (_readReg(RV3032_MINUTES_ALARM, &value) != 0) {
//        LOG_E("RV3032_REG_0x%02X, ReadError", RV3032_MINUTES_ALARM); osDelay(100);
//    } else {
//        LOG_I("AMM,RV3032_REG_0x%02X, 0x%02X", RV3032_MINUTES_ALARM, value); osDelay(100);
//    }
//    value = 0;
//    
//    if (_readReg(RV3032_STATUS, &value) != 0) {
//        LOG_E("RV3032_REG_0x%02X, ReadError", RV3032_STATUS); osDelay(100);
//    } else {
//        LOG_I("ST,RV3032_REG_0x%02X, 0x%02X", RV3032_STATUS, value); osDelay(100);
//    }
//    value = 0;
//    
//    if (_readReg(RV3032_CONTROL2, &value) != 0) {
//        LOG_E("RV3032_REG_0x%02X, ReadError", RV3032_CONTROL2); osDelay(100);
//    } else {
//        LOG_I("CTL2,RV3032_REG_0x%02X, 0x%02X", RV3032_CONTROL2, value); osDelay(100);
//    }
//    value = 0;

//    if (_readReg(RV3032_CONTROL1, &value) != 0) {
//        LOG_E("RV3032_REG_0x%02X, ReadError", RV3032_CONTROL1); osDelay(100);
//    } else {
//        LOG_I("CTL1,RV3032_REG_0x%02X, 0x%02X", RV3032_CONTROL1, value); osDelay(100);
//    }
//    value = 0;
}

/*@}*/
