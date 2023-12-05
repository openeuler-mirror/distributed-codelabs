/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef HUKS_NAPI_COMMON_H
#define HUKS_NAPI_COMMON_H

#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "hks_mem.h"
#include "hks_type.h"

#define DATA_SIZE_64KB  (1024 * 64)

namespace HuksNapi {
const std::string HKS_OPTIONS_PROPERTY_PROPERTIES = "properties";
const std::string HKS_OPTIONS_PROPERTY_INDATA = "inData";

const std::string HKS_PARAM_PROPERTY_TAG = "tag";
const std::string HKS_PARAM_PROPERTY_VALUE = "value";

const std::string HKS_RESULT_PROPERTY_ERRORCODE = "errorCode";
const std::string HKS_RESULT_PROPERTY_OUTDATA = "outData";
const std::string HKS_RESULT_PRPPERTY_PROPERTIES = "properties";
const std::string HKS_RESULT_PRPPERTY_CERTCHAINS = "certChains";

const std::string BUSINESS_ERROR_PROPERTY_CODE = "code";

const std::string HKS_HANDLE_PROPERTY_ERRORCODE = "errorCode";
const std::string HKS_HANDLE_PROPERTY_HANDLE = "handle";
const std::string HKS_HANDLE_PROPERTY_TOKEN = "token";

napi_value GetUint8Array(napi_env env, napi_value object, HksBlob &arrayBlob);

napi_value ParseKeyAlias(napi_env env, napi_value object, HksBlob *&alias);

napi_value ParseHksParamSet(napi_env env, napi_value object, HksParamSet *&paramSet);

void FreeParsedParams(std::vector<HksParam> &params);

napi_value ParseParams(napi_env env, napi_value object, std::vector<HksParam> &params);

napi_value ParseHksParamSetAndAddParam(napi_env env, napi_value object, HksParamSet *&paramSet, HksParam *addParam);

napi_ref GetCallback(napi_env env, napi_value object);

napi_value GenerateHksResult(napi_env env, int32_t error, uint8_t *data, uint32_t size);
napi_value GenerateHksResult(napi_env env, int32_t error, uint8_t *data, uint32_t size, const HksParamSet &paramSet);

napi_value GenerateStringArray(napi_env env, const struct HksBlob *blob, const uint32_t blobCount);

napi_value GenerateHksHandle(napi_env env, int32_t error, const struct HksBlob *handle,
    const struct HksBlob *token);

napi_value GetHandleValue(napi_env env, napi_value object, struct HksBlob *&handleBlob);

void CallAsyncCallback(napi_env env, napi_ref callback, int32_t error, napi_value data);

inline napi_value GetNull(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

inline napi_value GetInt32(napi_env env, int32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &result));
    return result;
}

inline void FreeHksBlob(HksBlob *&blob)
{
    if (blob == nullptr) {
        return;
    }

    if (blob->data != nullptr) {
        HksFree(blob->data);
        blob->data = nullptr;
    }
    blob->size = 0;

    HksFree(blob);
    blob = nullptr;
}

void FreeHksCertChain(HksCertChain *&certChain);

void DeleteCommonAsyncContext(napi_env env, napi_async_work &asyncWork, napi_ref &callback,
    struct HksBlob *&blob, struct HksParamSet *&paramSet);

napi_value ParseHandleAndHksParamSet(napi_env env, napi_value *argv, size_t &index,
    HksBlob *&handleBlob, HksParamSet *&paramSet);

napi_value ParseKeyAliasAndHksParamSet(napi_env env, napi_value *argv, size_t &index,
    HksBlob *&keyAliasBlob, HksParamSet *&paramSet);

napi_value ParseKeyData(napi_env env, napi_value value, HksBlob *&keyDataBlob);
}  // namespace HuksNapi
#endif
