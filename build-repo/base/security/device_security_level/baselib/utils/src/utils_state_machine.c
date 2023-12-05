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

#include "utils_state_machine.h"
#include "utils_log.h"
#include "utils_mem.h"

#include <stddef.h>

typedef struct {
    ListNode link;
    uint32_t event;
    const void *para;
} PendingEvent;

void InitStateMachine(StateMachine *machine, uint32_t machineId, uint32_t initState)
{
    if (machine == NULL) {
        return;
    }
    machine->machineId = machineId;
    machine->isScheduling = false;
    machine->currState = initState;
    InitListHead(&machine->pendingEventList);
    InitRecursiveMutex(&machine->mutex);
}

static const StateNode *GetScheduleStateNode(const StateNode *nodes, uint32_t nodeCnt, uint32_t state, uint32_t event)
{
    for (uint32_t i = 0; i < nodeCnt; i++) {
        const StateNode *node = nodes + i;
        if ((node->state == state) && (node->event == event)) {
            return node;
        }
    }
    return NULL;
}

static inline void PushPendingEvent(StateMachine *machine, uint32_t event, const void *para)
{
    PendingEvent *pending = MALLOC(sizeof(PendingEvent));
    if (pending == NULL) {
        return;
    }
    pending->event = event;
    pending->para = para;
    AddListNodeBefore(&pending->link, &machine->pendingEventList);
}

static inline bool PopPendingEvent(StateMachine *machine, uint32_t *event, const void **para)
{
    ListHead *head = &machine->pendingEventList;
    if (IsEmptyList(head)) {
        return false;
    }
    PendingEvent *pending = LIST_ENTRY(head->next, PendingEvent, link);
    RemoveListNode(&pending->link);
    *event = pending->event;
    *para = pending->para;
    FREE(pending);
    return true;
}

void ScheduleMachine(const StateNode *nodes, uint32_t nodeCnt, StateMachine *machine, uint32_t event, const void *para)
{
    // EventPara could be null, need not to check
    if ((nodes == NULL) || (nodeCnt == 0) || (machine == NULL)) {
        SECURITY_LOG_ERROR("invlid params, nodes or context is null");
        return;
    }
    LockMutex(&machine->mutex);

    if (machine->isScheduling) {
        PushPendingEvent(machine, event, para);
        UnlockMutex(&machine->mutex);
        return;
    }

    uint32_t state = machine->currState;
    const StateNode *node = GetScheduleStateNode(nodes, nodeCnt, state, event);
    if (node != NULL) {
        bool result = true;
        if (node->process != NULL) {
            machine->isScheduling = true;
            result = node->process(machine, event, para);
            machine->isScheduling = false;
        }
        machine->currState = (result == true) ? node->nextStateT : node->nextStateF;
    }
    SECURITY_LOG_INFO("Statemachine(%{public}x) schedule state(%{public}u) + event(%{public}u) -> newState(%{public}u)",
        machine->machineId, state, event, machine->currState);
    uint32_t nextEvent = 0;
    const void *nextPara = NULL;
    bool isPending = PopPendingEvent(machine, &nextEvent, &nextPara);
    UnlockMutex(&machine->mutex);
    if (isPending) {
        return ScheduleMachine(nodes, nodeCnt, machine, nextEvent, nextPara);
    }
}
