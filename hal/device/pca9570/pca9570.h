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
 
#ifndef _PCA9570_H_
#define _PCA9570_H_
 
#include <stdint.h>

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

typedef struct {
    void *driver;
} Pca9570_Instance_t;

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

extern void pca9570_initBus(Pca9570_Instance_t *ins, void *devIf);
extern void pca9570_finalizeBus(Pca9570_Instance_t *ins);
extern int32_t pca9570_initSoft(Pca9570_Instance_t *ins);
extern void pca9570_finalizeSoft(Pca9570_Instance_t *ins);

extern int32_t pca9570_writePin(Pca9570_Instance_t *ins, uint8_t pin, bool v);
extern int32_t pca9570_writePort(Pca9570_Instance_t *ins, uint8_t portData);

extern int32_t pca9570_readPin(Pca9570_Instance_t *ins, uint8_t pin);
extern int32_t pca9570_readPort(Pca9570_Instance_t *ins);

/*@}*/

#endif
