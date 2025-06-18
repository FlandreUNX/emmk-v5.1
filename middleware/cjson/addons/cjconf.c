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
 
#include "emmk/emmk.h"

#include "./cjconf.h"

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "CJCONF"

/*@}*/

/**
 * @addtogroup 
 * @note none
 */
 
/*@{*/

static int32_t save_file(CJSONF_File_t *f) {
    char *j_str = NULL;
    int32_t fd;
    int32_t ret;
    
    j_str = cJSON_PrintUnformatted(f->root);
    if (!j_str) {
        return -1;
    }

    fd = kvfs_open(f->filename, O_CREAT | O_RDWR | O_TRUNC);
    if (fd < 0) {
        kmem_free(j_str);
        return -1;
    }
    
    ret = kvfs_write(fd, j_str, strlen(j_str));
    kvfs_close(fd);
   
    return ret < 0 ? -1 : 0;
}


static char *read_file(const char *filename) {
    int32_t fd = kvfs_open(filename, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    
    struct kvfs_stat st;
    if (kvfs_stat(filename, &st) < 0) {
        kvfs_close(fd);
        return NULL;
    }
    
    char *buf = kmem_malloc(st.st_size);
    if (buf == NULL) {
        kvfs_close(fd);
        return NULL;
    }
    
    if (kvfs_read(fd, buf, st.st_size) != st.st_size) {
        kmem_free(buf);
        kvfs_close(fd);
        return NULL;
    }
    
    kvfs_close(fd);
    return buf;
}


static int32_t create_json_file(char *filename) {
    int32_t fd = kvfs_open(filename, O_CREAT | O_RDWR);
    if (fd < 0) {
        return -1;
    }

    if (kvfs_write(fd, "{}", 3) != 3) {
        kvfs_close(fd);
        return -1;
    }

    if (kvfs_sync(fd) < 0) {
        kvfs_close(fd);
        return -1;
    }

    kvfs_close(fd);
    return 0;
}


CJSONF_File_t *cjsonf_load(char *filename) {
    struct kvfs_stat st;
    char *json;
    osMutexAttr_t attr;
    
    CJSONF_File_t *file = kmem_calloc(1, sizeof(CJSONF_File_t));
    if (!file) {
        return NULL;
    }

    if (strlen(filename) > sizeof(file->filename) - 1) {
        LOG_E("filename %s is too long\n", filename);
        goto fail;
    }

    strncpy(file->filename, filename, sizeof(file->filename) - 1);
    
    // create KV file when not exist
    if (kvfs_stat(filename, &st) < 0) {
        if (create_json_file(file->filename) < 0) {
            goto fail;
        }
    }

    json = read_file(filename);
    if (!json) {
        goto fail;
    }

    file->root = cJSON_Parse(json);
    if (file->root == NULL) {
        kmem_free(json);
        goto fail;
    }
    memset(&attr, 0x00, sizeof(osMutexAttr_t));
    attr.attr_bits = osMutexRobust;
    file->mutex = osMutexNew(&attr);

    kmem_free(json);

    return file;

fail:
    if (file->root != NULL) {
        cJSON_Delete(file->root);
    }
    
    kmem_free(file);

    return NULL;
}


int32_t cjsonf_close(CJSONF_File_t *file) {
    if (!file) {
        return -1;
    }

    osMutexDelete(file->mutex);

    if (file->root) {
        cJSON_Delete(file->root);
    }
    
    kmem_free(file);

    return 0;
}


int32_t cjsonf_get(CJSONF_File_t *file, char *key, char *value) {
    cJSON *obj;
    
    if (!file || !file->root || !key || !value) {
        return -1;
    }

    osMutexAcquire(file->mutex, osWaitForever);

    obj = cJSON_GetObjectItem(file->root, key);
    if (!obj) {
        osMutexRelease(file->mutex);
        return -1;
    }
    
    strcpy(value, obj->valuestring);

    osMutexRelease(file->mutex);
    
    return 0;
}


int32_t cjsonf_set(CJSONF_File_t *file, char *key, char *value) {
    int32_t ret;
    
    if (!file || !file->root || !key || !value) {
        return -1;
    }

    osMutexAcquire(file->mutex, osWaitForever);
    
    // Remove old value if exist
    cJSON_DeleteItemFromObject(file->root, key);
    cJSON_AddItemToObject(file->root, key, cJSON_CreateString(value));

    ret = save_file(file);
    
    osMutexRelease(file->mutex);

    return ret;
}


int32_t cjsonf_setUINT(CJSONF_File_t *file, char *key, uint32_t value) {
    int32_t ret;
    char str_buf[16];
    
    memset(str_buf, 0x00, sizeof(str_buf));
    klPtf_sprintf(str_buf, "%d", value);
    
    if (!file || !file->root || !key) {
        return -1;
    }

    osMutexAcquire(file->mutex, osWaitForever);
    
    // Remove old value if exist
    cJSON_DeleteItemFromObject(file->root, key);
    cJSON_AddItemToObject(file->root, key, cJSON_CreateString(str_buf));

    ret = save_file(file);
    
    osMutexRelease(file->mutex);

    return ret;
}


int32_t cjsonf_getUINT(CJSONF_File_t *file, char *key, uint32_t *out) {
    cJSON *obj;
    uint32_t value = 0;
    
    if (!file || !file->root || !key) {
        return -1;
    }

    osMutexAcquire(file->mutex, osWaitForever);

    obj = cJSON_GetObjectItem(file->root, key);
    if (!obj) {
        osMutexRelease(file->mutex);
        return -1;
    }
    
    value = atoi(obj->valuestring);
    
    osMutexRelease(file->mutex);

    *out = value;
    
    return 0;
}


int32_t cjsonf_setINT(CJSONF_File_t *file, char *key, int32_t value) {
    int32_t ret;
    char str_buf[16];
    
    memset(str_buf, 0x00, sizeof(str_buf));
    kPtf_sprintf(str_buf, "%d", value);
    
    if (!file || !file->root || !key) {
        return -1;
    }

    osMutexAcquire(file->mutex, osWaitForever);
    
    // Remove old value if exist
    cJSON_DeleteItemFromObject(file->root, key);
    cJSON_AddItemToObject(file->root, key, cJSON_CreateString(str_buf));

    ret = save_file(file);
    
    osMutexRelease(file->mutex);

    return ret;
}


int32_t cjsonf_getINT(CJSONF_File_t *file, char *key, int32_t *out) {
    cJSON *obj;
    uint32_t value = 0;
    
    if (!file || !file->root || !key) {
        return -1;
    }

    osMutexAcquire(file->mutex, osWaitForever);

    obj = cJSON_GetObjectItem(file->root, key);
    if (!obj) {
        osMutexRelease(file->mutex);
        return -1;
    }
    
    value = atoi(obj->valuestring);
    
    osMutexRelease(file->mutex);

    *out = value;
    
    return 0;
}


int32_t cjsonf_getFloat(CJSONF_File_t *file, char *key, float *v) {
    cJSON *obj;
    float value = 0.0f;
    
    if (!file || !file->root || !key) {
        return -1;
    }

    osMutexAcquire(file->mutex, osWaitForever);

    obj = cJSON_GetObjectItem(file->root, key);
    if (!obj) {
        osMutexRelease(file->mutex);
        return -1;
    }
    
    *v = obj->valuedouble;
    
    osMutexRelease(file->mutex);

    return 0;
}


int32_t cjsonf_setFloat(CJSONF_File_t *file, char *key, float value) {
    int32_t ret;
    
    if (!file || !file->root || !key) {
        return -1;
    }

    osMutexAcquire(file->mutex, osWaitForever);
    
    // Remove old value if exist
    cJSON_DeleteItemFromObject(file->root, key);
    cJSON_AddItemToObject(file->root, key, cJSON_CreateNumber(value));

    ret = save_file(file);
    
    osMutexRelease(file->mutex);

    return ret;
}


int32_t cjsonf_del(CJSONF_File_t *file, char *key) {
    int32_t ret;
    
    if (!file || !file->root || !key) {
        return -1;
    }

    // Remove old value if exist
    osMutexAcquire(file->mutex, osWaitForever);
    
    cJSON_DeleteItemFromObject(file->root, key);
    
    ret = save_file(file);
    
    osMutexRelease(file->mutex);

    return ret;
}


int32_t cjsonf_destory(char *filename) {
    int32_t ret;
    ret = kvfs_unlink(filename);
    return ret;
}

/*@}*/
