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

#ifndef KV_DB_PRAGMA_H
#define KV_DB_PRAGMA_H

#include <vector>
#include <string>
#include <functional>

#include "store_types.h"
#include "query_sync_object.h"

namespace DistributedDB {
enum : int {
    PRAGMA_AUTO_SYNC = 1,
    PRAGMA_SYNC_DEVICES,
    PRAGMA_RM_DEVICE_DATA, // remove the device data synced from remote by device name
    PRAGMA_PERFORMANCE_ANALYSIS_GET_REPORT,
    PRAGMA_PERFORMANCE_ANALYSIS_OPEN,
    PRAGMA_PERFORMANCE_ANALYSIS_CLOSE,
    PRAGMA_PERFORMANCE_ANALYSIS_SET_REPORTFILENAME,
    PRAGMA_GET_IDENTIFIER_OF_DEVICE,
    PRAGMA_GET_DEVICE_IDENTIFIER_OF_ENTRY,
    PRAGMA_GET_QUEUED_SYNC_SIZE,
    PRAGMA_SET_QUEUED_SYNC_LIMIT,
    PRAGMA_GET_QUEUED_SYNC_LIMIT,
    PRAGMA_SET_WIPE_POLICY,
    PRAGMA_PUBLISH_LOCAL,
    PRAGMA_UNPUBLISH_SYNC,
    PRAGMA_SET_AUTO_LIFE_CYCLE,
    PRAGMA_RESULT_SET_CACHE_MODE,
    PRAGMA_RESULT_SET_CACHE_MAX_SIZE,
    PRAGMA_TRIGGER_TO_MIGRATE_DATA,
    PRAGMA_REMOTE_PUSH_FINISHED_NOTIFY,
    PRAGMA_SET_SYNC_RETRY,
    PRAGMA_ADD_EQUAL_IDENTIFIER,
    PRAGMA_INTERCEPT_SYNC_DATA,
    PRAGMA_SUBSCRIBE_QUERY,
    PRAGMA_SET_MAX_LOG_LIMIT,
    PRAGMA_EXEC_CHECKPOINT,
};

struct PragmaSync {
    PragmaSync(const std::vector<std::string> &devices, int mode, const QuerySyncObject &query,
        const std::function<void(const std::map<std::string, int> &devicesMap)> &onComplete,
        bool wait = false)
        : devices_(devices),
          mode_(mode),
          onComplete_(onComplete),
          wait_(wait),
          isQuerySync_(true),
          query_(query)
    {
    }

    PragmaSync(const std::vector<std::string> &devices, int mode,
        const std::function<void(const std::map<std::string, int> &devicesMap)> &onComplete,
        bool wait = false)
        : devices_(devices),
          mode_(mode),
          onComplete_(onComplete),
          wait_(wait),
          isQuerySync_(false),
          query_(Query::Select())
    {
    }

    std::vector<std::string> devices_;
    int mode_;
    std::function<void(const std::map<std::string, int> &devicesMap)> onComplete_;
    bool wait_;
    bool isQuerySync_;
    QuerySyncObject query_;
};

struct PragmaRemotePushNotify {
    explicit PragmaRemotePushNotify(RemotePushFinishedNotifier notifier) : notifier_(notifier) {}

    RemotePushFinishedNotifier notifier_;
};

struct PragmaSetEqualIdentifier {
    PragmaSetEqualIdentifier(const std::string &identifier, const std::vector<std::string> &targets)
        : identifier_(identifier),
          targets_(targets) {}

    std::string identifier_;
    std::vector<std::string> targets_;
};
} // namespace DistributedDB

#endif // KV_DB_PRAGMA_H
