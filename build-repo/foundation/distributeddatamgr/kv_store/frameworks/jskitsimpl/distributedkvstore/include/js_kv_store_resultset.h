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
#ifndef OHOS_KV_STORE_RESELTSET_H
#define OHOS_KV_STORE_RESELTSET_H
#include <string>
#include "napi_queue.h"
#include "result_set_bridge.h"
#include "kvstore_result_set.h"

namespace OHOS::DistributedKVStore {
class JsKVStoreResultSet : public DataShare::ResultSetBridge::Creator {
public:
    JsKVStoreResultSet() = default;
    virtual ~JsKVStoreResultSet() = default;

    void SetKvStoreResultSetPtr(std::shared_ptr<DistributedKv::KvStoreResultSet> resultSet);
    void SetSchema(bool isSchema);
    std::shared_ptr<DistributedKv::KvStoreResultSet> GetKvStoreResultSetPtr();

    static napi_value Constructor(napi_env env);
    static napi_value New(napi_env env, napi_callback_info info);

    virtual std::shared_ptr<DataShare::ResultSetBridge> Create() override;

private:
    static napi_value GetCount(napi_env env, napi_callback_info info);
    static napi_value GetPosition(napi_env env, napi_callback_info info);
    static napi_value MoveToFirst(napi_env env, napi_callback_info info);
    static napi_value MoveToLast(napi_env env, napi_callback_info info);
    static napi_value MoveToNext(napi_env env, napi_callback_info info);
    static napi_value MoveToPrevious(napi_env env, napi_callback_info info);
    static napi_value Move(napi_env env, napi_callback_info info);
    static napi_value MoveToPosition(napi_env env, napi_callback_info info);
    static napi_value IsFirst(napi_env env, napi_callback_info info);
    static napi_value IsLast(napi_env env, napi_callback_info info);
    static napi_value IsBeforeFirst(napi_env env, napi_callback_info info);
    static napi_value IsAfterLast(napi_env env, napi_callback_info info);
    static napi_value GetEntry(napi_env env, napi_callback_info info);

    std::shared_ptr<DistributedKv::KvStoreResultSet> resultSet_ = nullptr;
    bool isSchema_ = false;
};
} // namespace OHOS::DistributedKVStore
#endif // OHOS_KV_STORE_RESELTSET_H
