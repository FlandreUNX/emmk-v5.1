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

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define CONVERT_SUCCESS  0x02u
#define CONVERT_START    0x01u

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
static void *mFlag;
#else
static uint8_t mFlag;
#endif

/*@}*/

/**
 * @addtogroup IRQ define
 * @note none
 */
 
/*@{*/

void kdsdadc_DMAIrqHandler(void) {
    if (DMA_CH_INTStat(DMA_CHR_SDADC)) {
        DMA_CH_INTClr(DMA_CHR_SDADC);
        SDADC_Stop(SDADC);
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
        osEventFlagsSet(mFlag, CONVERT_SUCCESS);
#else
        mFlag |= CONVERT_SUCCESS;
#endif
    }
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

int32_t kdsdadc_init(kdsdadc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_config.instance.isInit(kd)) {
        return 0;
    }
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    mFlag = osEventFlagsNew(NULL);
    ASSERT(mFlag != NULL);
#else
    mFlag = 0;
#endif
    
    kd->_config.instance.enableFunc(kd);

    SDADC_Init(SDADC, &kd->_config.instance.init);
    SDADC->CTRL |= (1 << SDADC_CTRL_LOWCLK_Pos);
    
    if (kd->_config.instance.cfg[0].channels != 0xFFFFFFFF) {
        SDADC_Config_Set(SDADC, SDADC_CFG_A, 
            kd->_config.instance.cfg[0].gain, 
            kd->_config.instance.cfg[0].sdMode,
            kd->_config.instance.cfg[0].refM);
        SDADC_Config_Cali(SDADC, SDADC_CFG_A, SDADC_CALI_COM_GND, 0);
        SDADC_Config_Sel(SDADC, SDADC_CFG_A, kd->_config.instance.cfg[0].channels);
    }
    
    if (kd->_config.instance.cfg[1].channels != 0xFFFFFFFF) {
        SDADC_Config_Set(SDADC, SDADC_CFG_B, 
            kd->_config.instance.cfg[1].gain, 
            kd->_config.instance.cfg[1].sdMode,
            kd->_config.instance.cfg[1].refM);
        SDADC_Config_Cali(SDADC, SDADC_CFG_B, SDADC_CALI_COM_GND, 0);
        SDADC_Config_Sel(SDADC, SDADC_CFG_B, kd->_config.instance.cfg[1].channels);
    }
    
    if (kd->_config.instance.cfg[2].channels != 0xFFFFFFFF) {
        SDADC_Config_Set(SDADC, SDADC_CFG_C, 
            kd->_config.instance.cfg[2].gain, 
            kd->_config.instance.cfg[2].sdMode,
            kd->_config.instance.cfg[2].refM);
        SDADC_Config_Cali(SDADC, SDADC_CFG_C, SDADC_CALI_COM_GND, 0);
        SDADC_Config_Sel(SDADC, SDADC_CFG_C, kd->_config.instance.cfg[2].channels);
    }
    
    DMA_CH_Config(DMA_CHR_SDADC, (uint32_t) kd->_buffer.result, kd->_buffer.resultBufferLength, 1);

    return 0;
}

int32_t kdsdadc_finalize(kdsdadc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_config.instance.disableFunc(kd);
     
    SYS->CLKEN &= ~(0x01 << SYS_CLKEN_SDADC_Pos);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsDelete(mFlag);
    mFlag = NULL;
#endif
    
    return 0;
}

int32_t kdsdadc_powerUp(kdsdadc_t *kd) {
    ASSERT(kd != NULL);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsClear(mFlag, CONVERT_START | CONVERT_SUCCESS);
#else
    mFlag = 0;
#endif

    DMA_CH_INTClr(DMA_CHR_SDADC);

    SDADC_Open(SDADC);
    
    return 0;
}

int32_t kdsdadc_powerDown(kdsdadc_t *kd) {
    ASSERT(kd != NULL);
    
    SDADC_Stop(SDADC);
    SDADC_Close(SDADC);
    
    DMA_CH_INTClr(DMA_CHR_SDADC);
    DMA_CH_Close(DMA_CHR_SDADC);
    
    return 0;
}

void kdsdadc_convertStart(kdsdadc_t *kd) {
    ASSERT(kd != NULL);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if ((osEventFlagsGet(mFlag) & CONVERT_START) == CONVERT_START) {
        return;
    }
    
    osEventFlagsClear(mFlag, CONVERT_SUCCESS);
    osEventFlagsSet(mFlag, CONVERT_START);
#else
    if (mFlag & CONVERT_START) {
        return;
    }
    
    mFlag |= CONVERT_START;
    mFlag &= ~CONVERT_SUCCESS;
#endif
    
    DMA_CH_INTClr(DMA_CHR_SDADC);
    DMA_CH_Open(DMA_CHR_SDADC);
    
    SDADC_Start(SDADC);
}

void kdsdadc_convertStop(kdsdadc_t *kd) {
    ASSERT(kd != NULL);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if ((osEventFlagsGet(mFlag) & 0x00000002u) != 0x00000002u) {
        return;
    }
    
    osEventFlagsClear(mFlag, 0x00000002u);
#else
    if (!(mFlag & CONVERT_START)) {
        return;
    }
    
    mFlag &= ~CONVERT_START;
#endif
    
    DMA_CH_INTClr(DMA_CHR_SDADC);
    SDADC_Stop(SDADC);
}

uint8_t kdsdadc_isConvertCompleted(kdsdadc_t *kd, uint32_t wait) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t flag = 0;
    
    if (wait != 0) {
        flag = osEventFlagsWait(mFlag, CONVERT_SUCCESS, osFlagsNoClear | osFlagsWaitAll, wait);
    } else {
        flag = osEventFlagsGet(mFlag);
    }
    
    return (flag > 0) && (flag & CONVERT_SUCCESS) ? 1 : 0;
#else
    return mFlag & CONVERT_SUCCESS ? 1 : 0;
#endif
}

int32_t kdsdadc_getRaw(kdsdadc_t *kd, kdsdadc_Channel_t chn, int16_t *raw) {
    for (uint32_t i = 0; i < kd->_buffer.resultBufferLength; i++) {
        uint32_t _chn = (kd->_buffer.result[i] >> 16) & 0x0F;
        if ((0x01 << _chn) != chn) {
            continue;
        }
        
        *raw = (kd->_buffer.result[i] & 0xFFFF);
        return 0;
    }
    
    return -1;
}

float kdsdadc_convertRawToVolt(kdsdadc_t *kd, kdsdadc_Channel_t chn, int16_t raw) {
    uint8_t idx = 0;

    for (idx = 0; idx < 3; idx++) {
        if (kd->_config.instance.cfg[idx].channels == 0xFFFFFFFF) {
            continue;
        }
        if (kd->_config.instance.cfg[idx].channels & chn) {
            break;
        }
    }

    if (kd->_config.instance.cfg[idx].sdMode == KDSDADC_SDMODE_SINGLE) {
        return ((float) raw + 32768) * kd->_config.instance.cfg[idx].convertOffset;
    } else {
        return (float) raw * kd->_config.instance.cfg[idx].convertOffset;
    }
}

/*@}*/

#pragma GCC pop_options
