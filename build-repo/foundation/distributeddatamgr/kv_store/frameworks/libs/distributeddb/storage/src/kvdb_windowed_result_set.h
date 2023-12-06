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

#ifndef KV_DB_WINDOWED_RESULT_SET_H
#define KV_DB_WINDOWED_RESULT_SET_H

#include "ikvdb_result_set.h"
#include "result_entries_window.h"

namespace DistributedDB {
class KvDBWindowedResultSet : public IKvDBResultSet {
public:
    KvDBWindowedResultSet();
    ~KvDBWindowedResultSet() override;
    DISABLE_COPY_ASSIGN_MOVE(KvDBWindowedResultSet);

    // Initialize logic
    int Open(bool isMemDb) override;

    // Get total entries count.
    // >= 0: count, < 0: errCode.
    int GetCount() const override;

    // Get current read position.
    // >= 0: position, < 0: errCode
    int GetPosition() const override;

    // Move the read position to an absolute position value.
    int MoveTo(int position) const override;

    // Get the entry of current position.
    int GetEntry(Entry &entry) const override;

    // Finalize logic
    void Close() override;

private:
    ResultEntriesWindow *window_;
};
} // namespace DistributedDB

#endif // KV_DB_WINDOWED_RESULT_SET_H
