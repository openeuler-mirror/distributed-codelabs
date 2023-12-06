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

#include "huks_napi_update_finish.h"

#include <vector>

#include "securec.h"

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type.h"
#include "huks_napi_common.h"

namespace HuksNapi {
namespace {
constexpr int HUKS_NAPI_UPDATE_MIN_ARGS = 2;
constexpr int HUKS_NAPI_UPDATE_MAX_ARGS = 4;
}  // namespace

struct UpdateAsyncContextT {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;

    int32_t result = 0;
    struct HksBlob *handle = nullptr;
    struct HksParamSet *paramSet = nullptr;
    struct HksBlob *inData = nullptr;
    struct HksBlob *outData = nullptr;
    struct HksBlob *token = nullptr;
    bool isUpdate = false;
};
using UpdateAsyncContext = UpdateAsyncContextT *;

static UpdateAsyncContext CreateUpdateAsyncContext()
{
    UpdateAsyncContext context = (UpdateAsyncContext)HksMalloc(sizeof(UpdateAsyncContextT));
    if (context != nullptr) {
        (void)memset_s(context, sizeof(UpdateAsyncContextT), 0, sizeof(UpdateAsyncContextT));
    }
    return context;
}

static void DeleteUpdateAsyncContext(napi_env env, UpdateAsyncContext &context)
{
    if (context == nullptr) {
        return;
    }

    DeleteCommonAsyncContext(env, context->asyncWork, context->callback, context->handle, context->paramSet);

    if (context->inData != nullptr) {
        if (context->inData->data != nullptr && context->inData->size != 0) {
            (void)memset_s(context->inData->data, context->inData->size, 0, context->inData->size);
        }
        FreeHksBlob(context->inData);
    }

    if (context->outData != nullptr) {
        if (context->outData->data != nullptr && context->outData->size != 0) {
            (void)memset_s(context->outData->data, context->outData->size, 0, context->outData->size);
        }
        FreeHksBlob(context->outData);
    }

    if (context->token != nullptr) {
        FreeHksBlob(context->token);
    }

    HksFree(context);
    context = nullptr;
}

static int32_t FillContextInDataAndOutData(napi_env env, napi_value *argv, UpdateAsyncContext context, size_t index)
{
    napi_value inData = nullptr;
    bool hasInData = false;
    napi_has_named_property(env, argv[index], HKS_OPTIONS_PROPERTY_INDATA.c_str(), &hasInData);
    napi_status status = napi_get_named_property(env, argv[index], HKS_OPTIONS_PROPERTY_INDATA.c_str(), &inData);
    if (status == napi_ok && inData != nullptr && hasInData) {
        napi_value result = GetUint8Array(env, inData, *context->inData);
        if (result == nullptr) {
            HKS_LOG_E("could not get inData");
            return HKS_ERROR_BAD_STATE;
        }
    } else {
        context->inData->size = 0;
        context->inData->data = nullptr;
    }

    context->outData->size = context->inData->size + DATA_SIZE_64KB;
    context->outData->data = static_cast<uint8_t *>(HksMalloc(context->outData->size));
    if (context->outData->data == nullptr) {
        HKS_LOG_E("malloc memory failed");
        return HKS_ERROR_MALLOC_FAIL;
    }

    return HKS_SUCCESS;
}

static int32_t FillContextInDataAndOutBlob(napi_env env, napi_value *argv, UpdateAsyncContext context, size_t index)
{
    context->outData = static_cast<HksBlob *>(HksMalloc(sizeof(HksBlob)));
    if (context->outData == nullptr) {
        HKS_LOG_E("could not alloc out blob memory");
        return HKS_ERROR_MALLOC_FAIL;
    }
    (void)memset_s(context->outData, sizeof(HksBlob), 0, sizeof(HksBlob));

    context->inData = static_cast<HksBlob *>(HksMalloc(sizeof(HksBlob)));
    if (context->inData == nullptr) {
        HKS_LOG_E("could not alloc in blob memory");
        return HKS_ERROR_MALLOC_FAIL;
    }
    (void)memset_s(context->inData, sizeof(HksBlob), 0, sizeof(HksBlob));

    int32_t ret = FillContextInDataAndOutData(env, argv, context, index);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("fill data failed");
    }
    return ret;
}

static int32_t CheckIsCallbackFuction(napi_env env, napi_value object, bool &isFunc)
{
    isFunc = false;
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, object, &valueType);
    if (status != napi_ok) {
        HKS_LOG_E("could not get object type");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (valueType == napi_function) {
        isFunc = true;
    }
    return HKS_SUCCESS;
}

static int32_t GetCallBackFunction(napi_env env, napi_value object, UpdateAsyncContext context)
{
    napi_ref ref = nullptr;
    napi_status status = napi_create_reference(env, object, 1, &ref);
    if (status != napi_ok) {
        HKS_LOG_E("could not create reference");
        return HKS_ERROR_BAD_STATE;
    }
    context->callback = ref;
    return HKS_SUCCESS;
}

static int32_t GetToken(napi_env env, napi_value object, UpdateAsyncContext context)
{
    context->token = static_cast<HksBlob *>(HksMalloc(sizeof(HksBlob)));
    if (context->token == nullptr) {
        HKS_LOG_E("could not alloc token blob memory");
        return HKS_ERROR_MALLOC_FAIL;
    }
    (void)memset_s(context->token, sizeof(HksBlob), 0, sizeof(HksBlob));

    napi_value result = GetUint8Array(env, object, *(context->token));
    if (result == nullptr) {
        HKS_LOG_E("could not get token data");
        return HKS_ERROR_BAD_STATE;
    }

    return HKS_SUCCESS;
}

static int32_t GetTokenOrCallback(napi_env env, napi_value *argv, UpdateAsyncContext context,
    size_t index, size_t maxIndex)
{
    if (index >= maxIndex) { /* only 2 input params */
        return HKS_SUCCESS;
    }

    /*
     * check wether arg 3 is callback: if true, get callback function and return;
     * else get token, then check wether has arg 4: if true, get arg 4 as callback function
     */
    bool isFunc = false;
    int32_t ret = CheckIsCallbackFuction(env, argv[index], isFunc);
    if (ret != HKS_SUCCESS) {
        return ret;
    }
    if (isFunc) {
        return GetCallBackFunction(env, argv[index], context); /* return if arg 3 is callback */
    }

    /* get token */
    ret = GetToken(env, argv[index], context);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    index++;
    if (index < maxIndex) { /* has arg 4: can only be callback */
        ret = CheckIsCallbackFuction(env, argv[index], isFunc);
        if (ret != HKS_SUCCESS || !isFunc) {
            HKS_LOG_E("check param4 failed[ret = %" LOG_PUBLIC "d], or param4 is not func.", ret);
            return HKS_ERROR_INVALID_ARGUMENT;
        }
        return GetCallBackFunction(env, argv[index], context);
    }

    return HKS_SUCCESS;
}

static int32_t AddParams(const std::vector<HksParam> &params, struct HksParamSet *&paramSet)
{
    const HksParam *param = params.data();
    size_t paramCount = params.size();
    if (param == nullptr) {
        return HKS_SUCCESS;
    }

    for (uint32_t i = 0; i < paramCount; ++i) {
        int32_t ret = HksAddParams(paramSet, param, 1);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("add param[%" LOG_PUBLIC "u] failed", i);
            return ret;
        }
        param++;
    }
    return HKS_SUCCESS;
}

static int32_t GetInputParamSet(napi_env env, napi_value object, struct HksBlob *&token, HksParamSet *&paramSet)
{
    std::vector<HksParam> params;
    napi_value result = ParseParams(env, object, params);
    if (result == nullptr) {
        HKS_LOG_E("parse params failed");
        FreeParsedParams(params);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    HksParamSet *outParamSet = nullptr;
    int32_t ret;
    do {
        ret = HksInitParamSet(&outParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("init paramSet failed");
            break;
        }

        if (CheckBlob(token) == HKS_SUCCESS) { /* has token param */
            HksParam tokenParam = {
                .tag = HKS_TAG_AUTH_TOKEN,
                .blob = *token
            };
            ret = HksAddParams(outParamSet, &tokenParam, 1);
            if (ret != HKS_SUCCESS) {
                HKS_LOG_E("add token param failed.");
                break;
            }
        }

        ret = AddParams(params, outParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("add params failed");
            break;
        }

        ret = HksBuildParamSet(&outParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("build params failed");
            break;
        }
    } while (0);
    FreeParsedParams(params);
    if (ret != HKS_SUCCESS) {
        HksFreeParamSet(&outParamSet);
    }
    paramSet = outParamSet;
    return ret;
}

static napi_value ParseUpdateParams(napi_env env, napi_callback_info info, UpdateAsyncContext context)
{
    size_t argc = HUKS_NAPI_UPDATE_MAX_ARGS;
    napi_value argv[HUKS_NAPI_UPDATE_MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (argc < HUKS_NAPI_UPDATE_MIN_ARGS) {
        napi_throw_error(env, nullptr, "invalid arguments");
        HKS_LOG_E("no enough params");
        return nullptr;
    }

    size_t index = 0;
    napi_value result = GetHandleValue(env, argv[index], context->handle);
    if (result == nullptr) {
        HKS_LOG_E("update could not get handle value");
        return nullptr;
    }

    index++;
    napi_value properties = nullptr;
    napi_status status = napi_get_named_property(env, argv[index],
        HKS_OPTIONS_PROPERTY_PROPERTIES.c_str(), &properties);
    if (status != napi_ok || properties == nullptr) {
        GET_AND_THROW_LAST_ERROR((env));
        HKS_LOG_E("update could not get property %" LOG_PUBLIC "s", HKS_OPTIONS_PROPERTY_PROPERTIES.c_str());
        return nullptr;
    }

    if (FillContextInDataAndOutBlob(env, argv, context, index) != HKS_SUCCESS) {
        HKS_LOG_E("fill in or out blob failed");
        return nullptr;
    }

    index++;
    if (GetTokenOrCallback(env, argv, context, index, argc) != HKS_SUCCESS) {
        HKS_LOG_E("get token or callback failed");
        return nullptr;
    }

    if (GetInputParamSet(env, properties, context->token, context->paramSet) != HKS_SUCCESS) {
        HKS_LOG_E("could not get paramset");
        return nullptr;
    }

    return GetInt32(env, 0);
}

static napi_value UpdateWriteResult(napi_env env, UpdateAsyncContext context)
{
    return GenerateHksResult(env,
        context->result,
        ((context->result == HKS_SUCCESS && context->outData != nullptr) ? context->outData->data : nullptr),
        (context->result == HKS_SUCCESS && context->outData != nullptr) ? context->outData->size : 0);
}

static napi_value UpdateFinishAsyncWork(napi_env env, UpdateAsyncContext context)
{
    napi_value promise = nullptr;
    if (context->callback == nullptr) {
        NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));
    }

    napi_value resourceName;
    napi_create_string_latin1(env, "UpdateAsyncWork", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            UpdateAsyncContext napiContext = static_cast<UpdateAsyncContext>(data);

            if (napiContext->isUpdate) {
                napiContext->result = HksUpdate(napiContext->handle,
                    napiContext->paramSet, napiContext->inData, napiContext->outData);
            } else {
                napiContext->result = HksFinish(napiContext->handle,
                    napiContext->paramSet, napiContext->inData, napiContext->outData);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            UpdateAsyncContext napiContext = static_cast<UpdateAsyncContext>(data);
            napi_value result = UpdateWriteResult(env, napiContext);
            if (napiContext->callback == nullptr) {
                napi_resolve_deferred(env, napiContext->deferred, result);
            } else if (result != nullptr) {
                CallAsyncCallback(env, napiContext->callback, napiContext->result, result);
            }
            DeleteUpdateAsyncContext(env, napiContext);
        },
        static_cast<void *>(context),
        &context->asyncWork);

    napi_status status = napi_queue_async_work(env, context->asyncWork);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
        DeleteUpdateAsyncContext(env, context);
        HKS_LOG_E("could not queue async work");
        return nullptr;
    }

    if (context->callback == nullptr) {
        return promise;
    } else {
        return GetNull(env);
    }
}

napi_value HuksNapiUpdate(napi_env env, napi_callback_info info)
{
    UpdateAsyncContext context = CreateUpdateAsyncContext();
    if (context == nullptr) {
        HKS_LOG_E("could not create context");
        return nullptr;
    }

    napi_value result = ParseUpdateParams(env, info, context);
    if (result == nullptr) {
        HKS_LOG_E("could not parse params");
        DeleteUpdateAsyncContext(env, context);
        return nullptr;
    }
    context->isUpdate = true;

    result = UpdateFinishAsyncWork(env, context);
    if (result == nullptr) {
        HKS_LOG_E("could not start async work");
        DeleteUpdateAsyncContext(env, context);
        return nullptr;
    }
    return result;
}

napi_value HuksNapiFinish(napi_env env, napi_callback_info info)
{
    UpdateAsyncContext context = CreateUpdateAsyncContext();
    if (context == nullptr) {
        HKS_LOG_E("could not create context");
        return nullptr;
    }

    napi_value result = ParseUpdateParams(env, info, context);
    if (result == nullptr) {
        HKS_LOG_E("could not parse params");
        DeleteUpdateAsyncContext(env, context);
        return nullptr;
    }
    context->isUpdate = false;

    result = UpdateFinishAsyncWork(env, context);
    if (result == nullptr) {
        HKS_LOG_E("could not start async work");
        DeleteUpdateAsyncContext(env, context);
        return nullptr;
    }
    return result;
}
}  // namespace HuksNapi
