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

#ifndef NATIVE_RDB_RESULT_SET_PROXY_H
#define NATIVE_RDB_RESULT_SET_PROXY_H

#include "iremote_proxy.h"
#include "iresult_set.h"

namespace OHOS::NativeRdb {
class ResultSetProxy : public IRemoteProxy<IResultSet> {
public:
    explicit ResultSetProxy(const sptr<IRemoteObject> &impl);
    ~ResultSetProxy();
    int GetAllColumnNames(std::vector<std::string> &columnNames) override;
    int GetColumnCount(int &count) override;
    int GetColumnType(int columnIndex, ColumnType &columnType) override;
    int GetColumnIndex(const std::string &columnName, int &columnIndex) override;
    int GetColumnName(int columnIndex, std::string &columnName) override;
    int GetRowCount(int &count) override;
    int GetRowIndex(int &position) const override;
    int GoTo(int offset) override;
    int GoToRow(int position) override;
    int GoToFirstRow() override;
    int GoToLastRow() override;
    int GoToNextRow() override;
    int GoToPreviousRow() override;
    int IsEnded(bool &result) override;
    int IsStarted(bool &result) const override;
    int IsAtFirstRow(bool &result) const override;
    int IsAtLastRow(bool &result) override;
    int GetBlob(int columnIndex, std::vector<uint8_t> &blob) override;
    int GetString(int columnIndex, std::string &value) override;
    int GetInt(int columnIndex, int &value) override;
    int GetLong(int columnIndex, int64_t &value) override;
    int GetDouble(int columnIndex, double &value) override;
    int IsColumnNull(int columnIndex, bool &isNull) override;
    bool IsClosed() const override;
    int Close() override;

private:
    // the max capacity for ipc is 800KB.
    static const size_t MAX_IPC_CAPACITY = 800 * 1024;
    sptr<IRemoteObject> remote_;
    int SendRequest(uint32_t code);
    int SendIntRequest(uint32_t code, int value);
    int SendRequestRetBool(uint32_t code, bool &result) const;
    int SendRequestRetInt(uint32_t code, int &result) const;
    int SendRequestRetReply(uint32_t code, int columnIndex, MessageParcel &reply);
};
} // namespace OHOS::NativeRdb
#endif // NATIVE_RDB_RESULT_SET_PROXY_H
