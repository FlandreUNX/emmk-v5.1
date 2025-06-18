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

#ifndef _CM_MODULE_EXTMOD81_H_
#define _CM_MODULE_EXTMOD81_H_

#include <stdint.h>

#include "./cModule.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

typedef enum {
    CMODULE_REQ_EXTMOD81 = CMODULE_REQ_PT_ID_START + 100,
    CMODULE_REQ_EXTMOD81_ON_PAIRC,
    CMODULE_REQ_EXTMOD81_ON_PAIRSS,
} cModule_RequestType_ExtMod81_t;

/*@}*/

#endif
