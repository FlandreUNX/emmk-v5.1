//
// Created by unx on 2023/4/29.
//

#ifndef GET_STARTED_EMMK_DRIVER_H
#define GET_STARTED_EMMK_DRIVER_H

#include <stdint.h>

#include "esp_log.h"

#include <hal/uart_ll.h>
#include "driver/uart.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

#include "./driver/kdgpio.h"
#include "./driver/kdi2c.h"
#include "./driver/kdsaradc.h"
#include "./driver/kdpwm.h"
#include "./driver/kdspi.h"

/**
 * @addtogroup Instance method
 * @note none
 */

/*@{*/

#define KDINSTANCE(_class, _path)  kd##_class##_getInstance(_path)
#define KDMETHOD(_class)  __KD##_class##_METHODS

/*@}*/

/**
 * @addtogroup BaseFunc
 * @note none
 */

/*@{*/

extern int32_t emmkDriver_initRefsCountUp(int8_t *initRefs);
extern int32_t emmkDriver_initRefsCountDown(int8_t *initRefs);

/*@}*/

#endif //GET_STARTED_EMMK_DRIVER_H
