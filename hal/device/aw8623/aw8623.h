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

#ifndef _AW8623_H_
#define _AW8623_H_

#include <stdint.h>

/**
 * @addtogroup Public functions
 * @note none
 */
 
/*@{*/

extern void aw8623_initBus(void *devIf, void *pinRst, void *pinInt);
extern int32_t aw8623_initSoft(uint8_t caliValue);
extern void aw8623_finalizeBus(void);
extern void aw8623_finalizeSoft(void);

extern void aw8623_powerDown(void);
extern void aw8623_powerUp(void);

extern void aw8623_updateLib(void);

extern int32_t aw8623_enableIrqDones(uint8_t enable);

extern int32_t aw8623_vibrationSequence(uint8_t *seqIndex, uint8_t indexCount);
extern int32_t aw8623_vibrationLoop(uint8_t *seqIndex, uint8_t *seqLoopCount, uint8_t indexCount);

extern uint8_t aw8623_isDones(void);

extern void aw8623_start(void);
extern void aw8623_stop(void);

extern uint8_t aw8623_getTrimLra(void);


/*@}*/

#endif
