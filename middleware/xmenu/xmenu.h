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
 
#ifndef _XMENU_H_
#define _XMENU_H_

/**
 * @addtogroup Version
 * @note none
 */
 
/*@{*/

#define XMENU_VERISON_MAJOR    1
#define XMENU_VERISON_SECOND   1
#define XMENU_VERISON_STR      "V"RILAT_VERISON_MAJOR"."RILAT_VERISON_SECOND""

/*@}*/

#include <stdint.h>

#include "emmk-config.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define XMENU_KEY_UP      0x00000001u
#define XMENU_KEY_DOWN    0x00000002u
#define XMENU_KEY_LEFT    0x00000004u
#define XMENU_KEY_RIGHT   0x00000008u
#define XMENU_KEY_OK      0x00000010u
#define XMENU_KEY_ESC     0x00000020u
#define XMENU_TIMEOUT     0x00000040u
#define XMENU_INIT        0x00000080u

// Addons key max 24
#define XMENU_KEY_ADDONS(x)   (0x00000100u << x)
#define XMENU_KEY_ADDONS_MASK  0xFFFFFF00u

#define XMENU_MASK \
    (XMENU_KEY_ADDONS_MASK \
    | XMENU_TIMEOUT \
    | XMENU_KEY_UP \
    | XMENU_KEY_DOWN \
    | XMENU_KEY_LEFT \
    | XMENU_KEY_RIGHT \
    | XMENU_KEY_OK \
    | XMENU_KEY_ESC)

/*@}*/

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct xmenu_Item xmenu_Item_t;
typedef struct xmenu_EntryStack xmenu_EntryStack_t;
typedef struct xmenu_Entry xmenu_Entry_t;

typedef enum {
    XMENU_ITEM_TYPE_FUNC,
    XMENU_ITEM_TYPE_FUNC_SUB_TEXT,
    XMENU_ITEM_TYPE_SIMPLE,
    XMENU_ITEM_TYPE_TEXT,
    XMENU_ITEM_TYPE_MENU,
} xmenu_ItemType_t;

typedef enum {
    XMENU_CALL_CMD_ON_CALL_BEFORE,
    
    XMENU_CALL_CMD_ON_CALL_SIGNAL,
    
    XMENU_CALL_CMD_ON_DRAW_SUB_TEXT,
    XMENU_CALL_CMD_ON_DRAW_FUNC,
    XMENU_CALL_CMD_ON_DRAW_SIMPLE,
    
    XMENU_CALL_CMD_ON_CALL_UNSELECT,
} xmenu_CallCommand_t;

typedef enum {
    XMENU_CALL_EXIT_POP,
    XMENU_CALL_EXIT_KEEP,
    XMENU_CALL_EXIT_POP_ALL,
    XMENU_CALL_IGNORE,
} xmenu_CallExit_t;

struct xmenu_Item {
    uint8_t id;
    const char *title;
    xmenu_ItemType_t type;
    uint32_t timeout;

    int32_t (*onCall)(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg);

    const struct xmenu_Item **subSetter;
};

typedef struct {
    uint8_t cursorAt;
    uint8_t topItemIs;
} xmenu_EntryDraw_t;

typedef union {
    void *ptr;
    float f;
    uint32_t u32;
    int32_t i32;
    uint8_t u8[4];
    uint16_t u16[2];
    int8_t i8[4];
    int16_t i16[2];
} xmenu_EntryUserData_t;

typedef struct xmenu_EntryStack {
    xmenu_Entry_t *parent;
    
    uint8_t selectedIndex;
    
    xmenu_Item_t *item;
    uint8_t access: 1;
    uint16_t itemSetterLen;
    xmenu_EntryDraw_t draw;
    
    xmenu_EntryUserData_t *rx;
    uint8_t rxCount;
    
    uint8_t userDataIsInit;
} xmenu_EntryStack;

typedef void (*xmenu_OnPreItemDraw_t)(xmenu_Entry_t *entry, xmenu_EntryStack_t *stack);
typedef void (*xmenu_OnMenuDraw_t)(xmenu_Entry_t *entry, xmenu_EntryStack_t *stack, xmenu_EntryStack_t *simple_stack, uint8_t simple_item_select);

typedef struct {
    xmenu_OnPreItemDraw_t onPreItemDraw;
    xmenu_OnMenuDraw_t onMenuDraw;
} xmenu_GDisp_t;

struct xmenu_Entry {
    xmenu_EntryStack_t *stack;
    uint8_t stackIndex;
    uint8_t stackCount;

    uint8_t mcos;
    xmenu_GDisp_t *gdisp;

    uint16_t timeCount;
    uint16_t timeCountMax;
    qSTimer_t timeout;
    
    uint32_t flag;
};


#define __XMENU_ITEM_SECTION

#define __XMENU_SETTER_SECTION

/*@}*/

/**
 * @addtogroup Func
 * @note none
 */
 
/*@{*/

////
//// Item
////
#define XMENU_ITEM_FUNC_DEFINE(_name, _id, _title, to) \
    static int32_t s_xmenu_Item_##_name##_call(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg); \
    static const __XMENU_ITEM_SECTION xmenu_Item_t s_xmenu_Item_##_name##_item = { \
        .title = _title, \
        .id = _id, \
        .type = XMENU_ITEM_TYPE_FUNC, \
        .onCall = s_xmenu_Item_##_name##_call, \
        .timeout = to, \
        .subSetter = NULL, \
    }; \
    static int32_t s_xmenu_Item_##_name##_call(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg)

#define XMENU_ITEM_SIMPLE_DEFINE(_name, _id, _title, to) \
    static int32_t s_xmenu_Item_##_name##_call(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg); \
    static const __XMENU_ITEM_SECTION xmenu_Item_t s_xmenu_Item_##_name##_item = { \
        .title = _title, \
        .id = _id, \
        .type = XMENU_ITEM_TYPE_SIMPLE, \
        .onCall = s_xmenu_Item_##_name##_call, \
        .timeout = to, \
        .subSetter = NULL, \
    }; \
    static int32_t s_xmenu_Item_##_name##_call(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg)
        
#define XMENU_ITEM_TEXT_DEFINE(_name, _id, _title) \
    static int32_t s_xmenu_Item_##_name##_call(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg); \
    static const __XMENU_ITEM_SECTION xmenu_Item_t s_xmenu_Item_##_name##_item = { \
        .title = _title, \
        .id = _id, \
        .type = XMENU_ITEM_TYPE_TEXT, \
        .onCall = s_xmenu_Item_##_name##_call, \
        .timeout = 0xFFFFFFFF, \
        .subSetter = NULL, \
    }; \
    static int32_t s_xmenu_Item_##_name##_call(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg)
    
#define XMENU_ITEM_MENU_DEFINE(_name, _id, _title, _subSetter, to) \
    static int32_t s_xmenu_Item_##_name##_call(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg); \
    static const __XMENU_ITEM_SECTION xmenu_Item_t s_xmenu_Item_##_name##_item = { \
        .title = _title, \
        .id = _id, \
        .type = XMENU_ITEM_TYPE_MENU, \
        .timeout = to, \
        .onCall = s_xmenu_Item_##_name##_call, \
        .subSetter = (const struct xmenu_Item **) _subSetter, \
    }; \
    static int32_t s_xmenu_Item_##_name##_call(xmenu_EntryStack_t *stack, xmenu_Item_t *item, xmenu_CallCommand_t cmd, void *arg)
    
#define XMENU_ITEM_MENU_DEFINE_NO_CALL(_name, _id, _title, _subSetter, to) \
    static const __XMENU_ITEM_SECTION xmenu_Item_t s_xmenu_Item_##_name##_item = { \
        .title = _title, \
        .id = _id, \
        .type = XMENU_ITEM_TYPE_MENU, \
        .timeout = to, \
        .onCall = NULL, \
        .subSetter = (const struct xmenu_Item **) _subSetter, \
    }
    
////
//// Menu setter
////
    
#define XMENU_ITEM(_name) \
    (s_xmenu_Item_##_name##_item)
    
#define XMENU_SETTER(_name) \
    (s_xmenu_setter_##_name)
    
#define XMENU_SETTER_DEFINE(_name) \
    static const __XMENU_SETTER_SECTION xmenu_Item_t* const s_xmenu_setter_##_name[]
        
#define XMENU_SETTER_SET_ITEM(_name) \
    &s_xmenu_Item_##_name##_item
    
#define XMENU_SETTER_SET_END() \
    NULL
    
////
//// In callback
////
#define XMENU_CB_GET_TITLE() \
    ((char * ) item->title)
        
#define XMENU_CB_GET_MENU_ITEM_ACCESS() \
    (stack->access)
#define XMENU_CB_SET_MENU_ITEM_ACCESS(x) \
    stack->access = x
    
#define XMENU_CB_GET_COMMAND() \
    (cmd)

#define XMENU_CB_CLEAR_TIMEOUT() \
    stack->parent->timeCount = 0
    
//// ONLY FOR SUB DRAW
#define XMENU_CB_SUB_DRAW_GET_BUFFER() \
    ((char *) arg)
    
//// ONLY FOR MAIN DRAWs
#define XMENU_CB_GET_SIGNAL() \
    ((int32_t) arg)
    
#define XMENU_CB_GET_USER_DATA(_rx) \
    (stack->rx[_rx])
    
#define XMENU_CB_MARK_USER_DATA_INIT(idx, x) { \
    if (x) { \
        stack->userDataIsInit |= (0x01 << idx); \
    } else { \
        stack->userDataIsInit &= ~(0x01 << idx); \
    } \
}
    
#define XMENU_CB_GET_USER_DATA_INIT(idx) \
    (stack->userDataIsInit & (0x01 << idx))
    
/*@}*/
    
/**
 * @addtogroup gdisp
 * @note none
 */
 
/*@{*/

extern int32_t _xmenu_ix_itemCallSubText(xmenu_EntryStack_t *stack, xmenu_Item_t *item, char *buf);
extern int32_t _xmenu_ix_itemCallSimple(xmenu_EntryStack_t *stack, xmenu_Item_t *item, char *buf); 

/*@}*/
    
/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

extern int32_t xmenu_init(xmenu_Entry_t *entry, 
        xmenu_Item_t *rootItem,
        xmenu_GDisp_t *gdisp, uint8_t mcos);
extern void xmenu_finalize(xmenu_Entry_t *entry);    
extern int32_t xmenu_process(xmenu_Entry_t *entry);
extern void xmenu_pushKey(xmenu_Entry_t *entry, uint32_t key);
extern uint8_t xmenu_isInit(xmenu_Entry_t *entry);

/*@}*/
    
#endif
