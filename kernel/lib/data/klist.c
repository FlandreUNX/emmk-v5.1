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
 
#include "./klist.h"

/**
 * @addtogroup Public function
 * @note none
 */

/*@{*/

/**
 * @brief 初始化节点
 * @note none
 *
 * @param head, 节点 
 * 
 * @retval none
 */	
inline void klist_init(struct klist* head) {
    head->next = head;
    head->previous = head;
}


/**
 * @brief 将newNode添加到list节点尾部
 * @note none
 *
 * @param list, 原节点 
 * @param newNode, 新节点 
 * 
 * @retval none
 */	
inline void klist_addTail(struct klist* list, struct klist* newNode) {
    //__ListAdd(node, list->previous, list);
    list->previous->next = newNode;
    newNode->previous = list->previous;

    list->previous = newNode;
    newNode->next = list;
}


/**
 * @brief 将newNode添加到list节点头部
 * @note none
 *
 * @param list, 原节点 
 * @param newNode, 新节点 
 * 
 * @retval none
 */	
inline void klist_add(struct klist* list, struct klist* newNode) {
    //__ListAdd(node, list, node->next);
    list->next->previous = newNode;
    newNode->next = list->next;

    list->next = newNode;
    newNode->previous = list;
}


/**
 * @brief 删除一个节点
 * @note none
 *
 * @param node, 节点 
 * 
 * @retval none
 */	
inline void klist_delete(struct klist* node) {
	node->next->previous = node->previous;
	node->previous->next = node->next;

	node->next = node->previous = node;
}


/**
 * @brief 检查node是不是head的最后一个节点
 * @note none
 *
 * @param head, 对象所在所在列表 
 * @param node, 被检查对象 
 * 
 * @retval bool,
 */	
inline int klist_checkIsLast(struct klist* head, struct klist* node) {
	return (node->next == head);
}


/**
 * @brief 检查链表是不是空
 * @note none
 *
 * @param head, 节点 
 * 
 * @retval bool,
 */	
inline int klist_empty(struct klist* head) {
	return (head->next == head);
}


inline int klist_len(struct klist* head) {
    klist_t *pos;
    uint32_t len = 0;
    klist_forEach(pos, head) {
        len++;
    }
    
    return len;
}


/**
 * @brief 将node节点移动到head的头部
 * @note none
 *
 * @param head, 节点 
 * @param node, 被移动节点
 * 
 * @retval none
 */	
inline void klist_move(struct klist* head, struct klist* node) {
	klist_delete(node);
	klist_add(node, head);
}


/**
 * @brief 将node移动到head的尾部
 * @note none
 *
 * @param head, 节点 
 * @param node, 被移动节点
 * 
 * @retval none
 */	
inline void klist_moveTail(struct klist* head, struct klist* node) {
	klist_delete(node);
	klist_addTail(node, head);
}


/**
 * @brief 将外源列表list合并到head列表中
 * @note none
 *
 * @param head, 原节点 
 * @param list, 外源节点
 * 
 * @retval none
 */	
inline void klist_splice(struct klist* head, struct klist* list) {
	if (!klist_empty(list)) {
		struct klist *first = list->next;
		struct klist *last = list->previous;
		struct klist *at  = head->next;

		first->previous = head;
		head->next = first;

		last->next = at;
		at->previous = last;
	}
}

/*@}*/
