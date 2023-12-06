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

#ifndef I_KV_DB_SNAP_SHOT_H
#define I_KV_DB_SNAP_SHOT_H

#include "db_types.h"
#include "kv_store_changed_data.h"

namespace DistributedDB {
class IKvDBSnapshot {
public:
    virtual ~IKvDBSnapshot() {}

    // Get the value according the key in the snapshot
    virtual int Get(const Key &key, Value &value) const = 0;

    // Get the data according the prefix key in the snapshot
    virtual int GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const = 0;
};
} // namespace DistributedDB

#endif // I_KV_DB_SNAP_SHOT_H
