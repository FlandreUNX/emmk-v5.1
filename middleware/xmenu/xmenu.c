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

#include "./xmenu.h"
#include "opts_xmenu.h"

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef TAG
#define TAG  "xmenu"

/*@}*/

/**
 * @addtogroup Callback
 * @note none
 */

/*@{*/

static void _timeoutCallback(xmenu_Entry_t *entry) {
    if (!qSTimer_Expired(&entry->timeout)) {
        return;
    }

    qSTimer_Set(&entry->timeout, 1000);

    entry->timeCount += 1;
    if (entry->timeCount > entry->timeCountMax) {
        // klTmcd_countDownMs(&entry->timeout, 0xFFFFFFFF);

        entry->flag |= XMENU_TIMEOUT;
    }
}

/*@}*/

/**
 * @addtogroup Static func
 * @note none
 */

/*@{*/

static void stackMemsetZero(xmenu_EntryStack_t *stack) {
    memset(stack->rx, 0x00, sizeof(xmenu_EntryUserData_t) * stack->rxCount);
    stack->userDataIsInit = 0;
    stack->itemSetterLen = 0;
    stack->item = NULL;
}

static void _pushStack(xmenu_Entry_t *entry, xmenu_Item_t *item) {
    xmenu_EntryStack_t *stackNow;
    xmenu_EntryStack_t *stackNext;

    if (entry->stackIndex + 1 > entry->stackCount) {
        return;
    }

    stackNow = &entry->stack[entry->stackIndex];
    entry->stackIndex++;
    stackNext = &entry->stack[entry->stackIndex];

    stackMemsetZero(stackNext);

    stackNext->item = (xmenu_Item_t *) item->subSetter[stackNow->selectedIndex];
    stackNext->itemSetterLen = 0;
    if (stackNext->item->subSetter != NULL) {
        for (xmenu_Item_t **item = (xmenu_Item_t **) stackNext->item->subSetter; *item != NULL; ++item) {
            stackNext->itemSetterLen++;
        }
    }
}


static void _popStack(xmenu_Entry_t *entry, uint8_t all) {
    if (all) {
        for (uint8_t i = 0; i < entry->stackCount; i++) {
            stackMemsetZero(&entry->stack[i]);
            entry->stack[i].access = 0;
        }
        entry->stackIndex = 0;
    } else {
        stackMemsetZero(&entry->stack[entry->stackIndex]);

        if (entry->stackIndex != 0) {
            entry->stackIndex--;
        }
    }
}


static int32_t process(xmenu_Entry_t *entry) {
    xmenu_EntryStack_t *stackNow;
    xmenu_Item_t *item;
    int32_t signal;

    l_prrocess:
    stackNow = &entry->stack[entry->stackIndex];
    stackNow->parent = entry;
    item = stackNow->item;
    signal = entry->flag & XMENU_MASK;
    entry->flag &= ~XMENU_MASK;

    if (signal > 0) {
        entry->timeCount = 0;
    }

    if (item->type == XMENU_ITEM_TYPE_FUNC) { // Item - Func  
        xmenu_CallExit_t rc;

        rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_BEFORE, NULL);
        if (rc != XMENU_CALL_EXIT_KEEP) {
            goto l_func_on_call_rc_check;
        }

        if (entry->gdisp->onPreItemDraw != NULL) {
            entry->gdisp->onPreItemDraw(entry, stackNow);
        }

        rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_SIGNAL, (void *) signal);
        if (rc != XMENU_CALL_EXIT_KEEP) {
            goto l_func_on_call_rc_check;
        }

        l_func_on_call_rc_check:
        switch (rc) {
            case XMENU_CALL_IGNORE:
            case XMENU_CALL_EXIT_KEEP:
                rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_DRAW_FUNC, NULL);
                if (rc != XMENU_CALL_EXIT_KEEP) {
                    goto l_func_on_call_rc_check;
                }
                break;

            case XMENU_CALL_EXIT_POP:
                item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_UNSELECT, NULL);

                _popStack(entry, 0);
                if (entry->stack[entry->stackIndex].item == NULL) {
                    return -1;
                }
                goto l_prrocess;

            case XMENU_CALL_EXIT_POP_ALL:
                item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_UNSELECT, NULL);

                _popStack(entry, 1);
                return -1;
        }
    } else if (item->type == XMENU_ITEM_TYPE_SIMPLE) { // Item - Simple
        xmenu_CallExit_t rc;
        xmenu_EntryStack_t *pre_stack;

        pre_stack = &entry->stack[entry->stackIndex - 1];

        rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_BEFORE, NULL);
        if (rc != XMENU_CALL_EXIT_KEEP) {
            goto l_simple_on_call_rc_check;
        }

        rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_SIGNAL, (void *) signal);

        l_simple_on_call_rc_check:
        switch (rc) {
            case XMENU_CALL_IGNORE:
            case XMENU_CALL_EXIT_KEEP:
                if (entry->gdisp->onPreItemDraw != NULL) {
                    entry->gdisp->onPreItemDraw(entry, pre_stack);
                }
                entry->gdisp->onMenuDraw(entry, pre_stack, stackNow, 1);
                break;

            case XMENU_CALL_EXIT_POP:
                item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_UNSELECT, NULL);

                _popStack(entry, 0);
                goto l_prrocess;

            case XMENU_CALL_EXIT_POP_ALL:
                item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_UNSELECT, NULL);

                _popStack(entry, 1);
                return -1;
        }
    } else { // Item - Menu
        xmenu_CallExit_t rc;

        switch (signal) {
            case XMENU_INIT:
                goto l_prrocess;

#if CONFIG_XMENU_MENU_ITEM_ENABLE_UP_DOWN == 1
#if CONFIG_XMENU_MENU_ITEM_UP_DOWN_REVERSE == 1
                case XMENU_KEY_UP:
#else
                case XMENU_KEY_DOWN:
#endif
#endif

#if CONFIG_XMENU_MENU_ITEM_ENABLE_LEFT_RIGHT == 1
#if CONFIG_XMENU_MENU_ITEM_LEFT_RIGHT_REVERSE == 1
                case XMENU_KEY_LEFT:
#else
            case XMENU_KEY_RIGHT:
#endif
#endif
            {
                if (item->onCall != NULL) {
                    rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_SIGNAL, (void *) signal);
                    if (rc == XMENU_CALL_IGNORE) {
                        rc = XMENU_CALL_EXIT_KEEP;
                    } else {
                        if (rc != XMENU_CALL_EXIT_KEEP) {
                            goto l_menu_on_call_rc_check;
                        }
                        goto l_prrocess;
                    }
                }

                xmenu_EntryDraw_t *draw = &stackNow->draw;

                if (stackNow->itemSetterLen == 0) {
                    draw->topItemIs = 0;
                    draw->cursorAt = 0;
                    stackNow->selectedIndex = 0;
                } else if (stackNow->selectedIndex != (stackNow->itemSetterLen - 1)) {
                    stackNow->selectedIndex++;

                    if (draw->cursorAt >= (entry->mcos - 1) || ((draw->cursorAt == (entry->mcos / 2))
                                                                && (stackNow->itemSetterLen - stackNow->selectedIndex) >
                                                                   ((entry->mcos - 1) / 2))) {
                        draw->topItemIs++;
                    } else {
                        draw->cursorAt++;
                    }
                }
#if CONFIG_XMENU_MENU_ENABLE_SELECT_ROLLBACK == 1
                else {
                    draw->topItemIs = 0;
                    draw->cursorAt = 0;
                    stackNow->selectedIndex = 0;
                }
#endif
                goto l_prrocess;
            }

#if CONFIG_XMENU_MENU_ITEM_ENABLE_UP_DOWN == 1
#if CONFIG_XMENU_MENU_ITEM_UP_DOWN_REVERSE == 1
                case XMENU_KEY_DOWN:
#else
                case XMENU_KEY_UP:
#endif
#endif

#if CONFIG_XMENU_MENU_ITEM_ENABLE_LEFT_RIGHT == 1
#if CONFIG_XMENU_MENU_ITEM_LEFT_RIGHT_REVERSE == 1
                case XMENU_KEY_RIGHT:
#else
            case XMENU_KEY_LEFT:
#endif
#endif
            {
                if (item->onCall != NULL) {
                    rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_SIGNAL, (void *) signal);
                    if (rc == XMENU_CALL_IGNORE) {
                        rc = XMENU_CALL_EXIT_KEEP;
                    } else {
                        if (rc != XMENU_CALL_EXIT_KEEP) {
                            goto l_menu_on_call_rc_check;
                        }
                        goto l_prrocess;
                    }
                }

                xmenu_EntryDraw_t *draw = &stackNow->draw;

                if (stackNow->itemSetterLen == 0) {
                    draw->topItemIs = 0;
                    draw->cursorAt = 0;
                    stackNow->selectedIndex = 0;
                } else if (stackNow->selectedIndex != 0) {
                    stackNow->selectedIndex--;

                    if (draw->cursorAt > 0 &&
                        !((draw->cursorAt == entry->mcos / 2)
                          && (stackNow->selectedIndex >= entry->mcos / 2))) {
                        draw->cursorAt--;
                    } else {
                        draw->topItemIs--;
                    }
                }
#if CONFIG_XMENU_MENU_ENABLE_SELECT_ROLLBACK == 1
                else {
                    stackNow->selectedIndex = stackNow->itemSetterLen - 1;

                    if (stackNow->itemSetterLen <= entry->mcos) {
                        draw->topItemIs = 0;
                    } else {
                        draw->topItemIs = stackNow->selectedIndex;
                    }

                    if (draw->topItemIs > stackNow->itemSetterLen - entry->mcos
                        && stackNow->itemSetterLen >= entry->mcos) {
                        draw->topItemIs = stackNow->itemSetterLen - entry->mcos;
                    }

                    draw->cursorAt = stackNow->selectedIndex - draw->topItemIs;
                }
#endif
                goto l_prrocess;
            }

            case XMENU_KEY_OK: {
                if (item->onCall != NULL) {
                    rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_SIGNAL, (void *) signal);
                    if (rc == XMENU_CALL_IGNORE) {
                        rc = XMENU_CALL_EXIT_KEEP;
                    } else {
                        if (rc != XMENU_CALL_EXIT_KEEP) {
                            goto l_menu_on_call_rc_check;
                        }
                        goto l_prrocess;
                    }
                }

                if (item->subSetter[stackNow->selectedIndex]->type == XMENU_ITEM_TYPE_TEXT) {
                    goto l_prrocess;
                }

                _pushStack(entry, item);

                goto l_prrocess;
            }
                break;

            case XMENU_KEY_ESC: {
                if (item->onCall != NULL) {
                    rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_SIGNAL, (void *) signal);
                    if (rc == XMENU_CALL_IGNORE) {
                        rc = XMENU_CALL_EXIT_KEEP;
                    } else {
                        if (rc != XMENU_CALL_EXIT_KEEP) {
                            goto l_menu_on_call_rc_check;
                        }
                        goto l_prrocess;
                    }
                }
                _popStack(entry, 0);

                if (entry->stack[entry->stackIndex].item == NULL) {
                    return -1;
                }

                goto l_prrocess;
            }
                break;

            case XMENU_TIMEOUT: {
                if (item->onCall != NULL && !stackNow->access) {
                    if ((rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_SIGNAL, (void *) signal)) !=
                        XMENU_CALL_EXIT_KEEP) {
                        goto l_menu_on_call_rc_check;
                    }
                    goto l_prrocess;
                }

                _popStack(entry, 1);
                return -1;
            }
                break;

            default: {
                if (item->onCall != NULL) {
                    rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_BEFORE, NULL);
                    if (rc != XMENU_CALL_EXIT_KEEP) {
                        goto l_menu_on_call_rc_check;
                    }

                    if (entry->gdisp->onPreItemDraw != NULL) {
                        entry->gdisp->onPreItemDraw(entry, stackNow);
                    }

                    if (!stackNow->access) {
                        rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_DRAW_FUNC, NULL);
                        if (rc != XMENU_CALL_EXIT_KEEP) {
                            goto l_menu_on_call_rc_check;
                        }
                    } else {
                        entry->gdisp->onMenuDraw(entry, stackNow, NULL, 0);
                    }

                    if (signal & XMENU_KEY_ADDONS_MASK) {
                        rc = item->onCall(stackNow, item, XMENU_CALL_CMD_ON_CALL_SIGNAL, (void *) signal);
                        if (rc != XMENU_CALL_EXIT_KEEP) {
                            goto l_menu_on_call_rc_check;
                        }
                    }

                    break;

                    l_menu_on_call_rc_check:
                    switch (rc) {
                        case XMENU_CALL_IGNORE:
                        case XMENU_CALL_EXIT_KEEP:
                            goto l_prrocess;

                        case XMENU_CALL_EXIT_POP:
                            _popStack(entry, 0);
                            if (entry->stack[entry->stackIndex].item == NULL) {
                                return -1;
                            }
                            goto l_prrocess;

                        case XMENU_CALL_EXIT_POP_ALL:
                            _popStack(entry, 1);
                            return -1;
                    }
                } else {
                    if (entry->gdisp->onPreItemDraw != NULL) {
                        entry->gdisp->onPreItemDraw(entry, stackNow);
                    }
                    entry->gdisp->onMenuDraw(entry, stackNow, NULL, 0);
                }
            }
                break;
        }
    }

    return 0;
}

/*@}*/

/**
 * @addtogroup By drawing
 * @note none
 */

/*@{*/

int32_t _xmenu_ix_itemCallSubText(xmenu_EntryStack_t *stack, xmenu_Item_t *item, char *buf) {
    ASSERT(item != NULL);
    ASSERT(buf != NULL);

    if (item->onCall == NULL) {
        return 0;
    }

    int32_t rc = -1;
    rc = item->onCall(stack, item, XMENU_CALL_CMD_ON_DRAW_SUB_TEXT, (void *) buf);

    return rc;
}


int32_t _xmenu_ix_itemCallSimple(xmenu_EntryStack_t *stack, xmenu_Item_t *item, char *buf) {
    ASSERT(item != NULL);
    ASSERT(buf != NULL);

    if (item->onCall == NULL) {
        return 0;
    }

    int32_t rc = -1;
    rc = item->onCall(stack, item, XMENU_CALL_CMD_ON_DRAW_SIMPLE, (void *) buf);

    return rc;
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */

/*@{*/

int32_t xmenu_init(xmenu_Entry_t *entry, xmenu_Item_t *rootItem,
                   xmenu_GDisp_t *gdisp, uint8_t mcos) {
    ASSERT(entry != NULL);

    if (rootItem->type != XMENU_ITEM_TYPE_MENU) {
        return -1;
    }

    for (uint8_t i = 0; i < entry->stackCount; i++) {
        stackMemsetZero(&entry->stack[i]);
        entry->stack[i].access = 0;
    }

    entry->stack[0].item = rootItem;
    entry->gdisp = gdisp;
    entry->mcos = mcos;

    entry->stack[0].itemSetterLen = 0;
    for (xmenu_Item_t **item = (xmenu_Item_t **) entry->stack[0].item->subSetter; *item != NULL; ++item) {
        entry->stack[0].itemSetterLen++;
    }

    entry->timeout = (qSTimer_t) QSTIMER_INITIALIZER;
    entry->flag = 0;
    entry->flag |= XMENU_INIT;
    return 0;
}


void xmenu_finalize(xmenu_Entry_t *entry) {
    entry->flag = 0;
}


int32_t xmenu_process(xmenu_Entry_t *entry) {
    if (entry == NULL) {
        return -1;
    }

    if (!qSTimer_Status(&entry->timeout)) {
        qSTimer_Set(&entry->timeout, 1000);
    }

    entry->timeCountMax = entry->stack[entry->stackIndex].item->timeout;

    _timeoutCallback(entry);
    return process(entry);
}


void xmenu_pushKey(xmenu_Entry_t *entry, uint32_t key) {
    if (entry == NULL) {
        return;
    }

    entry->flag |= key;
}


uint8_t xmenu_isInit(xmenu_Entry_t *entry) {
    return entry->flag & XMENU_INIT;
}

/*@}*/

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
