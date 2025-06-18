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

/**
 *	left <tail>--[node head]--right <head>
 *
 *           left ----> right
 *  --<tail>---[node_Head]---<head>--
 *  |                               |
 *  |                               |
 *  |-------------------------------|
 */

#ifndef _KLIB_LIST_H_
#define _KLIB_LIST_H_

#include <stdint.h>

/**
 * @addtogroup List type define
 * @note none
 */

/*@{*/

struct klist {
    struct klist *previous;
    struct klist *next;
};


typedef struct klist klist_t;

/*@}*/

/**
 * @addtogroup Public function
 * @note none
 */

/*@{*/

/**
 * @brief 获取结构体中成员的位置
 * @note none
 *
 * @param tpye, 结构体类型
 * @param name, 成员名称
 * 
 * @retval none
 */
#define __klist_offsetof(type, name) \
    ((uint32_t) &((type *) 0)->name)


/**
 * @brief 获取当前list_head链表节点所在的宿主结构项
 * @note none
 *
 * @param ptr, 宿主结构体下的klist*
 * @param type, 宿主类型
 * @param name, 宿主结构类型定义中klist成员名
 * 
 * @retval 宿主结构体指针
 */
#define klist_entry(ptr, type, name) \
    (type *) ((uint8_t *) (ptr) - __klist_offsetof(type, name))
    
    
#define Klist_firstEntry(ptr, type, member) \
    klist_entry((ptr)->next, type, member)

	
/**
 * @brief 遍历链表中的所有list_head节点
 * @note none
 *
 * @param pos, 位置
 * @param head, 节点头
 * 
 * @retval none
 */
#define klist_forEach(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)


/**
 * @brief (安全模式)遍历链表中的所有list_head节点
 * @note none
 *
 * @param pos, 位置
 * @param head, 节点头
 * 
 * @retval none
 */
#define klist_forEachSafe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)
        
#define klist_forEachSafePrevious(pos, n, head) \
    for (pos = (head)->previous, n = pos->previous; pos != (head); \
        pos = n, n = pos->previous)
        
        
#define klist_forEachSafe2(pos, n, head, member, type) \
    for (pos = klist_entry((head)->next, type, member), \
         n = klist_entry(pos->member.next, type, member); \
         &pos->member != (head); \
         pos = n, n = klist_entry(n->member.next, type, member))

/**
 * @brief 获取结构体成员宿主指针
 * @note none
 *
 * @param ptr, 成员指针
 * @param type, 宿主类型
 * @param member, 成员在宿主结构体中的名称
 * 
 * @retval 宿主结构体指针
 */
#define container_of(ptr, type, member) ({ \
    const typeof(((type *)0)->member ) *__mptr = (ptr); \
        (type *)((char *)__mptr - __offsetof(type,member));})

        
extern void klist_init(struct klist* head);
extern void klist_addTail(struct klist* list, struct klist* newNode);
extern void klist_add(struct klist* list, struct klist* newNode);
extern void klist_delete(struct klist* node);
extern int klist_checkIsLast(struct klist* head, struct klist* node);
extern int klist_empty(struct klist* head);
extern int klist_len(struct klist* head);
extern void klist_move(struct klist* head, struct klist* node);
extern void klist_moveTail(struct klist* head, struct klist* node);
extern void klist_splice(struct klist* head, struct klist* list);

/*@}*/

#endif
