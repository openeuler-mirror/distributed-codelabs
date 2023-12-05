/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SEC_UTILS_LIST_H
#define SEC_UTILS_LIST_H

typedef struct TagListHead {
    struct TagListHead *next;
    struct TagListHead *prev;
} ListHead;

typedef ListHead ListNode;

static inline void AddListNode(ListNode *item, ListNode *where)
{
    item->next = where->next;
    item->prev = where;
    where->next = item;
    item->next->prev = item;
}

static inline void AddListNodeBefore(ListNode *item, ListNode *where)
{
    AddListNode(item, where->prev);
}

static inline void RemoveListNode(ListNode *item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
}

static inline int IsEmptyList(ListHead *head)
{
    return head->next == head;
}

static inline void InitListHead(ListHead *head)
{
    head->next = head;
    head->prev = head;
}

#define INIT_LIST(list)  \
    {                    \
        &(list), &(list) \
    }

#define FOREACH_LIST_NODE(item, head) for ((item) = (head)->next; (item) != (head); (item) = (item)->next)

#define FOREACH_LIST_NODE_SAFE(item, head, temp) \
    for ((item) = (head)->next, (temp) = (item)->next; (item) != (head); (item) = (temp), (temp) = (item)->next)

#define LIST_ENTRY(item, type, member) ((type *)((char *)(item) - (char *)(&((type *)0)->member)))

#endif /* SEC_UTILS_LIST_H */