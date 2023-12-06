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

#include "huks_napi_import_wrapped_key_item.h"

#include "securec.h"

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type.h"
#include "huks_napi_common_item.h"

namespace HuksNapiItem {
namespace {
    constexpr int HUKS_NAPI_IMPORT_WRAPPED_KEY_MIN_ARGS = 3;
    constexpr int HUKS_NAPI_IMPORT_WRAPPED_KEY_MAX_ARGS = 4;
}  // namespace

struct ImportWrappedKeyAsyncContextT {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;

    int32_t result = 0;
    struct HksBlob *keyAlias = nullptr;
    struct HksBlob *wrappingKeyAlias = nullptr;
    struct HksParamSet *paramSet = nullptr;
    struct HksBlob *wrappedData = nullptr;
};
using ImportWrappedKeyAsyncContext = ImportWrappedKeyAsyncContextT *;

static ImportWrappedKeyAsyncContext CreateImportWrappedKeyAsyncContext()
{
    ImportWrappedKeyAsyncContext context =
        static_cast<ImportWrappedKeyAsyncContext>(HksMalloc(sizeof(ImportWrappedKeyAsyncContextT)));
    if (context != nullptr) {
        (void)memset_s(context, sizeof(ImportWrappedKeyAsyncContextT), 0, sizeof(ImportWrappedKeyAsyncContextT));
    }
    return context;
}

static void DeleteImportWrappedKeyAsyncContext(napi_env env, ImportWrappedKeyAsyncContext &context)
{
    if (context == nullptr) {
        return;
    }

    DeleteCommonAsyncContext(env, context->asyncWork, context->callback, context->keyAlias, context->paramSet);

    if (context->wrappingKeyAlias != nullptr) {
        FreeHksBlob(context->wrappingKeyAlias);
    }

    if (context->wrappedData != nullptr) {
        if (context->wrappedData->data != nullptr && context->wrappedData->size != 0) {
            (void)memset_s(context->wrappedData->data, context->wrappedData->size, 0, context->wrappedData->size);
        }
        FreeHksBlob(context->wrappedData);
    }

    HksFree(context);
    context = nullptr;
}

static napi_value ImportWrappedKeyParseParams(napi_env env, napi_callback_info info,
    ImportWrappedKeyAsyncContext context)
{
    size_t argc = HUKS_NAPI_IMPORT_WRAPPED_KEY_MAX_ARGS;
    napi_value argv[HUKS_NAPI_IMPORT_WRAPPED_KEY_MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (argc < HUKS_NAPI_IMPORT_WRAPPED_KEY_MIN_ARGS) {
        napi_throw_error(env, std::to_string(HUKS_ERR_CODE_ILLEGAL_ARGUMENT).c_str(), "no enough params input");
        HKS_LOG_E("no enough params");
        return nullptr;
    }

    size_t index = 0;
    napi_value result = ParseKeyAlias(env, argv[index], context->keyAlias);
    if (result == nullptr) {
        HKS_LOG_E("importWrappedKey parse keyAlias failed");
        return nullptr;
    }

    index++;
    result = ParseKeyAliasAndHksParamSet(env, argv, index, context->wrappingKeyAlias, context->paramSet);
    if (result == nullptr) {
        HKS_LOG_E("importWrappedKey parse params failed");
        return nullptr;
    }

    result = ParseKeyData(env, argv[index], context->wrappedData);
    if (result == nullptr) {
        HKS_LOG_E("importWrappedKey parse keyData failed");
        return nullptr;
    }

    index++;
    if (index < argc) {
        context->callback = GetCallback(env, argv[index]);
    }

    return GetInt32(env, 0);
}

static napi_value ImportWrappedKeyAsyncWork(napi_env env, ImportWrappedKeyAsyncContext context)
{
    napi_value promise = nullptr;
    if (context->callback == nullptr) {
        NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "ImportWrappedKeyAsyncWork", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ImportWrappedKeyAsyncContext napiContext = static_cast<ImportWrappedKeyAsyncContext>(data);

            napiContext->result = HksImportWrappedKey(napiContext->keyAlias, napiContext->wrappingKeyAlias,
                napiContext->paramSet, napiContext->wrappedData);
        },
        [](napi_env env, napi_status status, void *data) {
            ImportWrappedKeyAsyncContext napiContext = static_cast<ImportWrappedKeyAsyncContext>(data);
            HksSuccessReturnResult resultData;
            SuccessReturnResultInit(resultData);
            HksReturnNapiResult(env, napiContext->callback, napiContext->deferred, napiContext->result, resultData);
            DeleteImportWrappedKeyAsyncContext(env, napiContext);
        },
        static_cast<void *>(context),
        &context->asyncWork);

    napi_status status = napi_queue_async_work(env, context->asyncWork);
    if (status != napi_ok) {
        DeleteImportWrappedKeyAsyncContext(env, context);
        HKS_LOG_E("could not queue async work");
        return nullptr;
    }

    if (context->callback == nullptr) {
        return promise;
    } else {
        return GetNull(env);
    }
}

napi_value HuksNapiImportWrappedKeyItem(napi_env env, napi_callback_info info)
{
    ImportWrappedKeyAsyncContext context = CreateImportWrappedKeyAsyncContext();
    if (context == nullptr) {
        HKS_LOG_E("could not create context");
        return nullptr;
    }

    napi_value result = ImportWrappedKeyParseParams(env, info, context);
    if (result == nullptr) {
        HKS_LOG_E("could not parse params");
        DeleteImportWrappedKeyAsyncContext(env, context);
        return nullptr;
    }

    result = ImportWrappedKeyAsyncWork(env, context);
    if (result == nullptr) {
        HKS_LOG_E("could not start async work");
        DeleteImportWrappedKeyAsyncContext(env, context);
        return nullptr;
    }
    return result;
}
}  // namespace HuksNapiItem
