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
 * @addtogroup Interface define
 * @note none
 */
 
/*@{*/

static const char *TAG = "kdpwm";

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

int32_t kdpwm_init(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_instance->va->initRefs) != 0) {
        return -1;
    }

    if (kd->_instance->isInitFunc(kd) != 0) {
        return 0;
    }

    ESP_ERROR_CHECK(ledc_timer_config(&kd->_instance->config));

    return 0;
}

int32_t kdpwm_finalize(kdpwm_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountDown(&kd->_instance->va->initRefs) != 0) {
        return -1;
    }

    return 0;
}

int32_t kdpwm_powerUp(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    ESP_ERROR_CHECK(ledc_channel_config(&kd->_config.channel));
    return 0;
}

int32_t kdpwm_powerDown(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    ESP_ERROR_CHECK(ledc_stop(kd->_config.channel.speed_mode, kd->_config.channel.channel, kd->_config.idleLevel));
    kdgpio_powerDown(kd->_config.gpio);
    return 0;
}

void kdpwm_setPrescaler(kdpwm_t *kd, uint32_t prescaler) {
}

void kdpwm_setAutoReload(kdpwm_t *kd, uint32_t autoReload) {
}

uint32_t kdpwm_getAutoReload(kdpwm_t *kd) {
    return (1 << kd->_instance->config.duty_resolution);
}

void kdpwm_setDuty(kdpwm_t *kd, uint32_t duty) {
    if (kd->_va->initRefs == 0) {
        return;
    }
    ESP_ERROR_CHECK(ledc_set_duty(kd->_config.channel.speed_mode, kd->_config.channel.channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(kd->_config.channel.speed_mode, kd->_config.channel.channel));
}

uint32_t kdpwm_getDuty(kdpwm_t *kd) {
    if (kd->_va->initRefs == 0) {
        return 0;
    }
    return ledc_get_duty(kd->_config.channel.speed_mode, kd->_config.channel.channel);
}

void kdpwm_setFreq(kdpwm_t *kd, uint32_t freq) {
    if (kd->_instance->va->initRefs == 0) {
        return;
    }
    ESP_ERROR_CHECK(ledc_set_freq(kd->_instance->config.speed_mode, kd->_instance->config.timer_num, freq));
}

void kdpwm_counter(kdpwm_t *kd, uint8_t enable) {
    if (kd->_instance->va->initRefs == 0) {
        return;
    }
    if (enable) {
        ledc_timer_resume(kd->_instance->config.speed_mode, kd->_instance->config.timer_num);
    } else {
        ledc_timer_pause(kd->_instance->config.speed_mode, kd->_instance->config.timer_num);
    }
}

void kdpwm_counterReset(kdpwm_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_instance->va->initRefs) == 0) {
        return;
    }
    ledc_timer_rst(kd->_instance->config.speed_mode, kd->_instance->config.timer_num);
}

void kdpwm_irqEnable(kdpwm_t *kd, kdpwm_Event_t evt, bool enable, kdpwm_SignalEvent_t cbEvent) {

}

/*@}*/
