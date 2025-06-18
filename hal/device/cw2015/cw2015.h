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
 
#ifndef _CW2015_H_
#define _CW2015_H_
 
#include <stdint.h>

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

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

extern void cw2015_initBus(void *devIf);
extern void cw2015_finalizeBus(void);
extern int32_t cw2015_initSoft(void);
extern void cw2015_finalizeSoft(void);

extern uint16_t cw2015_getMVCell(void);
extern int32_t cw2015_getSOC(void);
extern int32_t cw2015_getRRT(void);

extern int32_t cw2015_poll(void);
extern uint16_t cw2015_getVolt(void);

/*@}*/

#endif
