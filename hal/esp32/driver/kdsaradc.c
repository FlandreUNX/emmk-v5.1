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

#include "emmk-driver.h"
#include "emmk-config.h"

/**
 * @addtogroup Interfaace define
 * @note none
 */

/*@{*/

static const char *TAG = "kdsaradc";

/*@}*/

/**
 * @addtogroup Private func
 * @note none
 */

/*@{*/

static bool checkValidData(const adc_digi_output_data_t *data) {
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2
    const unsigned int unit = data->type2.unit;
    if (unit > 2) return false;
#endif
    if (data->type2.channel >= SOC_ADC_CHANNEL_NUM(unit)) return false;

    return true;
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */

/*@{*/

int32_t kdsaradc_init(kdsaradc_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountUp(&kd->_config.instance->_va->initRefs) != 0) {
        return -1;
    }

    kd->_config.instance->enableFunc(kd);

    ESP_ERROR_CHECK(adc_continuous_new_handle(&kd->_config.instance->handleCfg, &kd->_config.instance->_va->handle));
    ESP_ERROR_CHECK(adc_continuous_config(kd->_config.instance->_va->handle, &kd->_config.instance->digCfg));

    return 0;
}


int32_t kdsaradc_finalize(kdsaradc_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountDown(&kd->_config.instance->_va->initRefs) != 0) {
        return -1;
    }

    ESP_ERROR_CHECK(adc_continuous_deinit(kd->_config.instance->_va->handle));
    kd->_config.instance->disableFunc(kd);

    return 0;
}


int32_t kdsaradc_powerUp(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }

    return 0;
}


int32_t kdsaradc_powerDown(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }

    return 0;
}


void kdsaradc_convertStart(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_config.instance->_va->startRefs) != 0) {
        return;
    }

    ESP_ERROR_CHECK(adc_continuous_start(kd->_config.instance->_va->handle));
}


void kdsaradc_convertStop(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_config.instance->_va->startRefs) != 0) {
        return;
    }

    ESP_ERROR_CHECK(adc_continuous_stop(kd->_config.instance->_va->handle));
}

uint8_t kdsaradc_isConvertCompleted(kdsaradc_t *kd, uint32_t wait) {
    esp_err_t ret;
    uint32_t bufferReadNumber = 0;
    uint8_t *buffer = calloc(1, kd->_config.instance->handleCfg.conv_frame_size);
    ASSERT(buffer != NULL);

    ret = adc_continuous_read(kd->_config.instance->_va->handle,
                              buffer, kd->_config.instance->handleCfg.conv_frame_size,
                              &bufferReadNumber, wait);
    if (ret == ESP_OK || ret == ESP_ERR_INVALID_STATE) {
        for (int i = 0; i < bufferReadNumber; i += 4) {
            adc_digi_output_data_t *p = (void *) &buffer[i];
            if (checkValidData(p)) {
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32S3
                uint8_t index = (p->type2.unit) == 2
                                ? KDSARADC_ADC2_CHANNEL_0 + p->type2.channel
                                : p->type2.channel;
#elif CONFIG_IDF_TARGET_ESP32C6
                uint8_t index = p->type2.channel;
#endif
                kd->_config.instance->_va->buffer[index] = p->type2.data;
            }
        }
    } else if (ret == ESP_ERR_TIMEOUT) {
        free(buffer);
        return 0;
    }

    free(buffer);
    return 1;
}


uint16_t kdsaradc_getRaw(kdsaradc_t *kd) {
    return kd->_config.instance->_va->buffer[kd->_config.channel];
}


float kdsaradc_convertRawToVolt(kdsaradc_t *kd, uint16_t raw) {
    return ((float) raw * (float) kd->_config.instance->refVolt) / 4096.0f;
}

/*@}*/