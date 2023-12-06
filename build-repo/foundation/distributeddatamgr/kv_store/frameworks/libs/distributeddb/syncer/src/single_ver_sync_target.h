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

#ifndef SINGLE_VER_SYNC_TARGET_H
#define SINGLE_VER_SYNC_TARGET_H

#include "db_types.h"
#include "query_sync_object.h"
#include "sync_target.h"

namespace DistributedDB {
class SingleVerSyncTarget final : public SyncTarget {
public:
    SingleVerSyncTarget();
    ~SingleVerSyncTarget() override;

    // Set a syncOperation
    void SetSyncOperation(SyncOperation *operation) override;

    // Set the end water mark of this task
    void SetEndWaterMark(WaterMark waterMark);

    // Get the end water mark of this task
    WaterMark GetEndWaterMark() const;

    // For pull response sync
    void SetResponseSessionId(uint32_t responseSessionId);
    uint32_t GetResponseSessionId() const override;

    // For query sync
    void SetQuery(const QuerySyncObject &query);
    QuerySyncObject GetQuery() const;
    void SetQuerySync(bool isQuerySync);
    bool IsQuerySync() const;
private:
    WaterMark endWaterMark_;
    uint32_t responseSessionId_ = 0;
    QuerySyncObject query_;
    bool isQuerySync_ = false;
};
} // namespace DistributedDB

#endif // SINGLE_VER_SYNC_TARGET_H
