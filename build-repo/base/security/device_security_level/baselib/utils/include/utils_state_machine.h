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

#ifndef SEC_UTILS_FINITE_STATE_MACHINE_H
#define SEC_UTILS_FINITE_STATE_MACHINE_H

#include <stdbool.h>
#include <stdint.h>

#include "utils_list.h"
#include "utils_mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct StateMachine StateMachine;

typedef bool (*StateMachineProcessFunc)(const StateMachine *machine, uint32_t event, const void *para);

typedef struct {
    uint32_t state;
    uint32_t event;
    StateMachineProcessFunc process;
    uint32_t nextStateT;
    uint32_t nextStateF;
} StateNode;

typedef struct StateMachine {
    uint32_t currState;
    uint32_t machineId;
    bool isScheduling;
    ListHead pendingEventList;
    Mutex mutex;
} StateMachine;

void InitStateMachine(StateMachine *machine, uint32_t machineId, uint32_t initState);

void ScheduleMachine(const StateNode *nodes, uint32_t nodeCnt, StateMachine *machine, uint32_t event, const void *para);

#define STATE_MACHINE_ENTRY(item, type, member) ((type *)((char *)(item) - (char *)(&((type *)0)->member)))

#ifdef __cplusplus
}
#endif

#endif // SEC_UTILS_FINITE_STATE_MACHINE_H
