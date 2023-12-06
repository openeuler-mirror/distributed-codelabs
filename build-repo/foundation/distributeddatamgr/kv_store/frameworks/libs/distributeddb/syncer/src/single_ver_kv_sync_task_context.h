/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef SINGLE_VER_KV_SYNC_TASK_CONTEXT_H
#define SINGLE_VER_KV_SYNC_TASK_CONTEXT_H

#include "single_ver_sync_task_context.h"

namespace DistributedDB {
class SingleVerKvSyncTaskContext : public SingleVerSyncTaskContext {
public:

    explicit SingleVerKvSyncTaskContext();

    DISABLE_COPY_ASSIGN_MOVE(SingleVerKvSyncTaskContext);

    std::string GetQuerySyncId() const override;
    std::string GetDeleteSyncId() const override;

    void SetSyncStrategy(const SyncStrategy &strategy);
    SyncStrategy GetSyncStrategy(QuerySyncObject &querySyncObject) const override;
protected:
    ~SingleVerKvSyncTaskContext() override;

    SyncStrategy syncStrategy_;
};
}
#endif // SINGLE_VER_KV_SYNC_TASK_CONTEXT_H