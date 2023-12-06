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

#ifndef SYNC_ABLE_KVDB_H
#define SYNC_ABLE_KVDB_H

#include <shared_mutex>

#include "generic_kvdb.h"
#include "ikvdb_sync_interface.h"
#include "intercepted_data.h"
#include "sync_able_kvdb_connection.h"
#include "syncer_proxy.h"

namespace DistributedDB {
class SyncAbleKvDB : public GenericKvDB {
public:
    SyncAbleKvDB();
    ~SyncAbleKvDB() override;
    DISABLE_COPY_ASSIGN_MOVE(SyncAbleKvDB);

    // Delete a connection object.
    void DelConnection(GenericKvDBConnection *connection) override;

    // Used to notify Syncer and other listeners data has changed
    void CommitNotify(int notifyEvent, KvDBCommitNotifyFilterAbleData *data) override;

    // Invoked automatically when connection count is zero
    void Close() override;

    // Start a sync action.
    int Sync(const ISyncer::SyncParma &parma, uint64_t connectionId);

    // Enable auto sync
    void EnableAutoSync(bool enable);

    // Stop a sync action in progress.
    void StopSync(uint64_t connectionId);

    // Get The current virtual timestamp
    uint64_t GetTimestamp();

    void WakeUpSyncer() override;

    // Get manual sync queue size
    int GetQueuedSyncSize(int *queuedSyncSize) const;

    // Set manual sync queue limit
    int SetQueuedSyncLimit(const int *queuedSyncLimit);

    // Get manual sync queue limit
    int GetQueuedSyncLimit(int *queuedSyncLimit) const;

    // Disable add new manual sync , for rekey
    int DisableManualSync(void);

    // Enable add new manual sync , for rekey
    int EnableManualSync(void);

    int SetStaleDataWipePolicy(WipePolicy policy);

    int EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash);

    NotificationChain::Listener *AddRemotePushFinishedNotify(const RemotePushFinishedNotifier &notifier, int &errCode);

    void NotifyRemotePushFinishedInner(const std::string &targetId) const;

    int SetSyncRetry(bool isRetry);
    // Set an equal identifier for this database, After this called, send msg to the target will use this identifier
    int SetEqualIdentifier(const std::string &identifier, const std::vector<std::string> &targets);

    virtual void SetDataInterceptor(const PushDataInterceptor &interceptor) = 0;

    void Dump(int fd) override;

protected:
    virtual IKvDBSyncInterface *GetSyncInterface() = 0;

    void SetSyncModuleActive();

    bool GetSyncModuleActive();

    void ReSetSyncModuleActive();
    // Start syncer
    int StartSyncer(bool isCheckSyncActive = false, bool isNeedActive = true);

    int StartSyncerWithNoLock(bool isCheckSyncActive, bool isNeedActive);

    // Stop syncer
    void StopSyncer(bool isClosedOperation = false);

    void StopSyncerWithNoLock(bool isClosedOperation = false);

    void UserChangeHandle();

    void ChangeUserListerner();

    // Get the dataItem's append length, the append length = after-serialized-len - original-dataItem-len
    uint32_t GetAppendedLen() const;

    int GetLocalIdentity(std::string &outTarget);

    void TriggerSync(int notifyEvent);

private:
    int RegisterEventType(EventType type);

    SyncerProxy syncer_;
    std::atomic<bool> started_;
    std::atomic<bool> closed_;
    std::atomic<bool> isSyncModuleActiveCheck_;
    std::atomic<bool> isSyncNeedActive_;
    mutable std::shared_mutex notifyChainLock_;
    NotificationChain *notifyChain_;

    mutable std::mutex syncerOperateLock_;
    NotificationChain::Listener *userChangeListener_;

    static const EventType REMOTE_PUSH_FINISHED;
};
}

#endif // SYNC_ABLE_KVDB_H
