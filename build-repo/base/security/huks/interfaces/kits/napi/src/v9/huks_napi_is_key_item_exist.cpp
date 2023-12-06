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

#include "huks_napi_is_key_item_exist.h"

#include "securec.h"

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type.h"
#include "huks_napi_common_item.h"

namespace HuksNapiItem {
namespace {
constexpr int HUKS_NAPI_IS_KEY_EXIST_MIN_ARGS = 2;
constexpr int HUKS_NAPI_IS_KEY_EXIST_MAX_ARGS = 3;
}  // namespace

struct IsKeyExistAsyncContextT {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;

    int32_t result = 0;
    struct HksBlob *keyAlias = nullptr;
    struct HksParamSet *paramSet = nullptr;
};
using IsKeyExistAsyncContext = IsKeyExistAsyncContextT *;

static IsKeyExistAsyncContext CreateIsKeyExistAsyncContext()
{
    IsKeyExistAsyncContext context = static_cast<IsKeyExistAsyncContext>(HksMalloc(sizeof(IsKeyExistAsyncContextT)));
    if (context != nullptr) {
        (void)memset_s(context, sizeof(IsKeyExistAsyncContextT), 0, sizeof(IsKeyExistAsyncContextT));
    }
    return context;
}

static void DeleteIsKeyExistAsyncContext(napi_env env, IsKeyExistAsyncContext &context)
{
    if (context == nullptr) {
        return;
    }
    DeleteCommonAsyncContext(env, context->asyncWork, context->callback, context->keyAlias, context->paramSet);
    HksFree(context);
    context = nullptr;
}

static napi_value IsKeyExistParseParams(napi_env env, napi_callback_info info, IsKeyExistAsyncContext context)
{
    size_t argc = HUKS_NAPI_IS_KEY_EXIST_MAX_ARGS;
    napi_value argv[HUKS_NAPI_IS_KEY_EXIST_MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (argc < HUKS_NAPI_IS_KEY_EXIST_MIN_ARGS) {
        napi_throw_error(env, std::to_string(HUKS_ERR_CODE_ILLEGAL_ARGUMENT).c_str(), "no enough params input");
        HKS_LOG_E("no enough params");
        return nullptr;
    }

    size_t index = 0;
    napi_value result = ParseKeyAliasAndHksParamSet(env, argv, index, context->keyAlias, context->paramSet);
    if (result == nullptr) {
        HKS_LOG_E("isKeyExist parse params failed");
        return nullptr;
    }

    index++;
    if (index < argc) {
        context->callback = GetCallback(env, argv[index]);
    }

    return GetInt32(env, 0);
}

static napi_value IsKeyExistAsyncWork(napi_env env, IsKeyExistAsyncContext context)
{
    napi_value promise = nullptr;
    if (context->callback == nullptr) {
        NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "isKeyExistAsyncWork", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            IsKeyExistAsyncContext napiContext = static_cast<IsKeyExistAsyncContext>(data);

            napiContext->result = HksKeyExist(napiContext->keyAlias, napiContext->paramSet);
        },
        [](napi_env env, napi_status status, void *data) {
            IsKeyExistAsyncContext napiContext = static_cast<IsKeyExistAsyncContext>(data);
            HksSuccessReturnResult resultData;
            SuccessReturnResultInit(resultData);
            resultData.isOnlyReturnBoolResult = true;
            HksReturnNapiResult(env, napiContext->callback, napiContext->deferred, napiContext->result, resultData);
            DeleteIsKeyExistAsyncContext(env, napiContext);
        },
        static_cast<void *>(context),
        &context->asyncWork);

    napi_status status = napi_queue_async_work(env, context->asyncWork);
    if (status != napi_ok) {
        DeleteIsKeyExistAsyncContext(env, context);
        HKS_LOG_E("could not queue async work");
        return nullptr;
    }

    if (context->callback == nullptr) {
        return promise;
    } else {
        return GetNull(env);
    }
}

napi_value HuksNapiIsKeyItemExist(napi_env env, napi_callback_info info)
{
    IsKeyExistAsyncContext context = CreateIsKeyExistAsyncContext();
    if (context == nullptr) {
        HKS_LOG_E("could not create context");
        return nullptr;
    }

    napi_value result = IsKeyExistParseParams(env, info, context);
    if (result == nullptr) {
        HKS_LOG_E("could not parse params");
        DeleteIsKeyExistAsyncContext(env, context);
        return nullptr;
    }

    result = IsKeyExistAsyncWork(env, context);
    if (result == nullptr) {
        HKS_LOG_E("could not start async work");
        DeleteIsKeyExistAsyncContext(env, context);
        return nullptr;
    }
    return result;
}
}  // namespace HuksNapiItem
