/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "huks_napi_init_session.h"

#include "securec.h"

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type.h"
#include "huks_napi_common_item.h"

namespace HuksNapiItem {
namespace {
constexpr int HUKS_NAPI_INIT_MIN_ARGS = 2;
constexpr int HUKS_NAPI_INIT_MAX_ARGS = 3;

constexpr int HKS_MAX_TOKEN_SIZE = 2048;
}  // namespace

struct InitAsyncContext {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;

    int64_t result = 0;
    struct HksBlob *keyAlias = nullptr;
    struct HksParamSet *paramSet = nullptr;
    struct HksBlob *handle = nullptr;
    struct HksBlob *token = nullptr;
};

using InitAsyncCtxPtr = InitAsyncContext *;

static InitAsyncCtxPtr CreateInitAsyncContext()
{
    InitAsyncCtxPtr context = static_cast<InitAsyncCtxPtr>(HksMalloc(sizeof(InitAsyncContext)));
    if (context != nullptr) {
        (void)memset_s(context, sizeof(InitAsyncContext), 0, sizeof(InitAsyncContext));
    }
    return context;
}

static void DeleteInitAsyncContext(napi_env env, InitAsyncCtxPtr &context)
{
    if (context == nullptr) {
        return;
    }

    DeleteCommonAsyncContext(env, context->asyncWork, context->callback, context->keyAlias, context->paramSet);

    if (context->handle != nullptr) {
        FreeHksBlob(context->handle);
    }

    if (context->token != nullptr) {
        FreeHksBlob(context->token);
    }

    HksFree(context);
    context = nullptr;
}

static napi_value ParseInitParams(napi_env env, napi_callback_info info, InitAsyncCtxPtr context)
{
    size_t argc = HUKS_NAPI_INIT_MAX_ARGS;
    napi_value argv[HUKS_NAPI_INIT_MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (argc < HUKS_NAPI_INIT_MIN_ARGS) {
        napi_throw_error(env, std::to_string(HUKS_ERR_CODE_ILLEGAL_ARGUMENT).c_str(), "no enough params input");
        HKS_LOG_E("no enough params");
        return nullptr;
    }

    size_t index = 0;
    napi_value result = ParseKeyAliasAndHksParamSet(env, argv, index, context->keyAlias, context->paramSet);
    if (result == nullptr) {
        HKS_LOG_E("init parse params failed");
        return nullptr;
    }

    index++;
    if (index < argc) {
        context->callback = GetCallback(env, argv[index]);
    }

    return GetInt32(env, 0);
}

static int32_t InitOutParams(InitAsyncCtxPtr context)
{
    /* free buffer use DeleteInitAsyncContext */
    context->handle = static_cast<struct HksBlob *>(HksMalloc(sizeof(HksBlob)));
    if (context->handle == nullptr) {
        HKS_LOG_E("malloc handle failed");
        return HKS_ERROR_MALLOC_FAIL;
    }
    context->handle->data = static_cast<uint8_t *>(HksMalloc(HKS_MAX_TOKEN_SIZE));
    if (context->handle->data == nullptr) {
        HKS_LOG_E("malloc handle data failed");
        return HKS_ERROR_MALLOC_FAIL;
    }
    context->handle->size = HKS_MAX_TOKEN_SIZE;

    context->token = static_cast<struct HksBlob *>(HksMalloc(sizeof(HksBlob)));
    if (context->token == nullptr) {
        HKS_LOG_E("malloc token failed");
        return HKS_ERROR_MALLOC_FAIL;
    }
    context->token->data = static_cast<uint8_t *>(HksMalloc(HKS_MAX_TOKEN_SIZE));
    if (context->token->data == nullptr) {
        HKS_LOG_E("malloc token data failed");
        return HKS_ERROR_MALLOC_FAIL;
    }
    context->token->size = HKS_MAX_TOKEN_SIZE;
    return HKS_SUCCESS;
}

static napi_value InitAsyncWork(napi_env env, InitAsyncCtxPtr context)
{
    napi_value promise = nullptr;
    if (context->callback == nullptr) {
        NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));
    }

    napi_value resourceName;
    napi_create_string_latin1(env, "InitAsyncWork", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            (void)env;
            InitAsyncCtxPtr napiContext = static_cast<InitAsyncCtxPtr>(data);
            int32_t ret = InitOutParams(napiContext);
            if (ret != HKS_SUCCESS) {
                napiContext->result = ret;
                return;
            }
            napiContext->result = HksInit(napiContext->keyAlias, napiContext->paramSet,
                napiContext->handle, napiContext->token);
        },
        [](napi_env env, napi_status status, void *data) {
            InitAsyncCtxPtr napiContext = static_cast<InitAsyncCtxPtr>(data);
            HksSuccessReturnResult resultData;
            SuccessReturnResultInit(resultData);
            resultData.handle = napiContext->handle;
            resultData.challenge = napiContext->token;
            HksReturnNapiResult(env, napiContext->callback, napiContext->deferred, napiContext->result, resultData);
            DeleteInitAsyncContext(env, napiContext);
        },
        static_cast<void *>(context),
        &context->asyncWork);

    napi_status status = napi_queue_async_work(env, context->asyncWork);
    if (status != napi_ok) {
        DeleteInitAsyncContext(env, context);
        HKS_LOG_E("could not queue async work");
        return nullptr;
    }

    if (context->callback == nullptr) {
        return promise;
    }
    return GetNull(env);
}

napi_value HuksNapiInitSession(napi_env env, napi_callback_info info)
{
    InitAsyncCtxPtr context = CreateInitAsyncContext();
    if (context == nullptr) {
        HKS_LOG_E("could not create context");
        return nullptr;
    }

    napi_value result = ParseInitParams(env, info, context);
    if (result == nullptr) {
        HKS_LOG_E("could not parse params");
        DeleteInitAsyncContext(env, context);
        return nullptr;
    }

    result = InitAsyncWork(env, context);
    if (result == nullptr) {
        HKS_LOG_E("could not start async work");
        DeleteInitAsyncContext(env, context);
        return nullptr;
    }
    return result;
}
}  // namespace HuksNapiItem
