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

#ifndef NATIVE_RDB_IRESULT_SET_H
#define NATIVE_RDB_IRESULT_SET_H

#include "result_set.h"
#include "iremote_broker.h"

namespace OHOS::NativeRdb {
class IResultSet : public ResultSet, public IRemoteBroker {
public:
    virtual ~IResultSet() = default;
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS::NativeRdb.IResultSet");
};
} // namespace OHOS::NativeRdb
#endif // NATIVE_RDB_IRESULT_SET_H
