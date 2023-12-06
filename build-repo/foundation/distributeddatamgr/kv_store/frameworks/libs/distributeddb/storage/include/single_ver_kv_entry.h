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

#ifndef SINGLE_VER_KV_ENTRY_H
#define SINGLE_VER_KV_ENTRY_H

#include <vector>
#include "parcel.h"

namespace DistributedDB {
class SingleVerKvEntry {
public:
    virtual ~SingleVerKvEntry() {};
    virtual std::string GetOrigDevice() const = 0;
    virtual void SetOrigDevice(const std::string &device) = 0;
    virtual Timestamp GetTimestamp() const = 0;
    virtual void SetTimestamp(Timestamp timestamp) = 0;
    virtual Timestamp GetWriteTimestamp() const = 0;
    virtual void SetWriteTimestamp(Timestamp timestamp) = 0;
    virtual uint64_t GetFlag() const = 0;
    virtual int SerializeData(Parcel &parcel, uint32_t softWareVersion) = 0;
    virtual int DeSerializeData(Parcel &parcel) = 0;
    virtual uint32_t CalculateLen(uint32_t softWareVersion) = 0;
    virtual const Key &GetKey() const = 0;
    virtual const Value &GetValue() const = 0;
    virtual void SetKey(const Key &key) = 0;
    virtual void SetValue(const Value &value) = 0;
    virtual void SetHashKey(const Key &hashKey) = 0;

    static void Release(std::vector<SingleVerKvEntry *> &entries)
    {
        for (auto &entry : entries) {
            delete entry;
            entry = nullptr;
        }
        entries.clear();
    };
};
} // namespace DistributedDB

#endif // SINGLE_VER_KV_ENTRY_H
