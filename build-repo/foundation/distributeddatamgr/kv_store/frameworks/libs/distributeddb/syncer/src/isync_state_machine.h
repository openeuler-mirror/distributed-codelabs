/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef I_SYNC_STATE_MACHINE_H
#define I_SYNC_STATE_MACHINE_H

#include <string>

#include "icommunicator.h"
#include "ikvdb_sync_interface.h"
#include "query_sync_object.h"
#include "sync_target.h"
#include "sync_task_context.h"

namespace DistributedDB {
class ISyncStateMachine {
public:
    virtual ~ISyncStateMachine() {};

    // Init the SyncStateMachine, this function must be called before any other call.
    virtual int Initialize(ISyncTaskContext *context, ISyncInterface *syncInterface,
        std::shared_ptr<Metadata> &metadata, ICommunicator *communicator) = 0;

    // start a sync step
    virtual int StartSync() = 0;

    // send Message to the StateMachine
    virtual int ReceiveMessageCallback(Message *inMsg) = 0;

    // call when timeout
    virtual int TimeoutCallback(TimerId timerId) = 0;

    // Force stop the state machine
    virtual void Abort() = 0;

    // Force stop the state machine now
    virtual void AbortImmediately() = 0;

    // Force stop current task with sessionId
    virtual void InnerErrorAbort(uint32_t sessionId) = 0;

    // Called by CommErrHandler, Sub class should realize this function to abort sync when handle err
    virtual void CommErrAbort(uint32_t sessionId = 0) = 0;

    // start a timer to ResetWatchDog when sync data one (key,value) size bigger than mtu
    virtual bool StartFeedDogForSync(uint32_t time, SyncDirectionFlag flag) = 0;

    // stop timer to ResetWatchDog when sync data one (key,value) size bigger than mtu
    virtual void StopFeedDogForSync(SyncDirectionFlag flag) = 0;

    // check if need trigger query auto sync and get query from inMsg
    virtual bool IsNeedTriggerQueryAutoSync(Message *inMsg, QuerySyncObject &query) = 0;

    // Notify machine is closing, should release some lock
    virtual void NotifyClosing() = 0;

    // start a timer to ResetWatchDog when get data and send notify ack if need
    virtual void StartFeedDogForGetData(uint32_t sessionId) = 0;

    // start a timer to ResetWatchDog when get data
    virtual void StopFeedDogForGetData() = 0;
};
} // namespace DistributedDB

#endif // I_SYNC_STATE_MACHINE_H
