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
 
#ifndef _CJSON_CJCONF_H_
#define _CJSON_CJCONF_H_

#include <stdint.h>

#include "../cJSON.h"

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct jconf_file {
    char filename[128];
    cJSON *root;
    void *mutex;
} CJSONF_File_t;

/*@}*/

/**
 * @addtogroup Functions
 * @note none
 */
 
/*@{*/

extern CJSONF_File_t *cjsonf_load(char *filename);
extern int32_t cjsonf_close(CJSONF_File_t *file);

extern int32_t cjsonf_get(CJSONF_File_t *file, char *key, char *value);
extern int32_t cjsonf_set(CJSONF_File_t *file, char *key, char *value);

extern int32_t cjsonf_setUINT(CJSONF_File_t *file, char *key, uint32_t value);
extern int32_t cjsonf_getUINT(CJSONF_File_t *file, char *key, uint32_t *out);

extern int32_t cjsonf_setINT(CJSONF_File_t *file, char *key, int32_t value);
extern int32_t cjsonf_getINT(CJSONF_File_t *file, char *key, int32_t *out);

extern int32_t cjsonf_getFloat(CJSONF_File_t *file, char *key, float *v);
extern int32_t cjsonf_setFloat(CJSONF_File_t *file, char *key, float value);

extern int32_t cjsonf_del(CJSONF_File_t *file, char *key);
extern int32_t cjsonf_destory(char *filename);

/*@}*/

#endif
