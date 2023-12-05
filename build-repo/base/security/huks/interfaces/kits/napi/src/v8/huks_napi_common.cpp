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

#include "huks_napi_common.h"

#include <vector>

#include "securec.h"

#include "hks_log.h"
#include "hks_param.h"
#include "hks_type.h"

namespace HuksNapi {
namespace {
constexpr int HKS_MAX_DATA_LEN = 0x6400000; // The maximum length is 100M
constexpr size_t ASYNCCALLBACK_ARGC = 2;
}  // namespace

napi_value ParseKeyAlias(napi_env env, napi_value object, HksBlob *&alias)
{
    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, object, nullptr, 0, &length);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
        HKS_LOG_E("could not get string length");
        return nullptr;
    }

    if (length > HKS_MAX_DATA_LEN) {
        HKS_LOG_E("input key alias length too large");
        return nullptr;
    }

    char *data = static_cast<char *>(HksMalloc(length + 1));
    if (data == nullptr) {
        napi_throw_error(env, nullptr, "could not alloc memory");
        HKS_LOG_E("could not alloc memory");
        return nullptr;
    }
    (void)memset_s(data, length + 1, 0, length + 1);

    size_t result = 0;
    status = napi_get_value_string_utf8(env, object, data, length + 1, &result);
    if (status != napi_ok) {
        HksFree(data);
        GET_AND_THROW_LAST_ERROR((env));
        HKS_LOG_E("could not get string");
        return nullptr;
    }

    alias = static_cast<HksBlob *>(HksMalloc(sizeof(HksBlob)));
    if (alias == nullptr) {
        HksFree(data);
        napi_throw_error(env, NULL, "could not alloc memory");
        HKS_LOG_E("could not alloc memory");
        return nullptr;
    }
    alias->data = reinterpret_cast<uint8_t *>(data);
    alias->size = static_cast<uint32_t>(length & UINT32_MAX);

    return GetInt32(env, 0);
}

napi_value GetUint8Array(napi_env env, napi_value object, HksBlob &arrayBlob)
{
    napi_typedarray_type arrayType;
    napi_value arrayBuffer = nullptr;
    size_t length = 0;
    size_t offset = 0;
    void *rawData = nullptr;

    NAPI_CALL(
        env, napi_get_typedarray_info(env, object, &arrayType, &length, &rawData, &arrayBuffer, &offset));
    NAPI_ASSERT(env, arrayType == napi_uint8_array, "it's not uint8 array");

    if (length > HKS_MAX_DATA_LEN) {
        HKS_LOG_E("data len is too large, len = %" LOG_PUBLIC "zx", length);
        return nullptr;
    }
    if (length == 0) {
        HKS_LOG_I("the created memory length just 1 Byte");
        // the created memory length just 1 Byte
        arrayBlob.data = static_cast<uint8_t *>(HksMalloc(1));
    } else {
        arrayBlob.data = static_cast<uint8_t *>(HksMalloc(length));
    }
    if (arrayBlob.data == nullptr) {
        return nullptr;
    }
    (void)memcpy_s(arrayBlob.data, length, rawData, length);
    arrayBlob.size = static_cast<uint32_t>(length);

    return GetInt32(env, 0);
}

static napi_value GetHksParam(napi_env env, napi_value object, HksParam &param)
{
    napi_value tag = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, object, HKS_PARAM_PROPERTY_TAG.c_str(), &tag));
    NAPI_CALL(env, napi_get_value_uint32(env, tag, &param.tag));

    napi_value value = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, object, HKS_PARAM_PROPERTY_VALUE.c_str(), &value));

    napi_value result = nullptr;

    switch (param.tag & HKS_TAG_TYPE_MASK) {
        case HKS_TAG_TYPE_INT:
            NAPI_CALL(env, napi_get_value_int32(env, value, &param.int32Param));
            result = GetInt32(env, 0);
            break;
        case HKS_TAG_TYPE_UINT:
            NAPI_CALL(env, napi_get_value_uint32(env, value, &param.uint32Param));
            result = GetInt32(env, 0);
            break;
        case HKS_TAG_TYPE_ULONG:
            NAPI_CALL(env, napi_get_value_int64(env, value, reinterpret_cast<int64_t *>(&param.uint64Param)));
            result = GetInt32(env, 0);
            break;
        case HKS_TAG_TYPE_BOOL:
            NAPI_CALL(env, napi_get_value_bool(env, value, &param.boolParam));
            result = GetInt32(env, 0);
            break;
        case HKS_TAG_TYPE_BYTES:
            result = GetUint8Array(env, value, param.blob);
            if (result == nullptr) {
                HKS_LOG_E("get uint8 array fail.");
            } else {
                HKS_LOG_D("tag 0x%" LOG_PUBLIC "x, len 0x%" LOG_PUBLIC "x", param.tag, param.blob.size);
            }
            break;
        default:
            HKS_LOG_E("invalid tag value 0x%" LOG_PUBLIC "x", param.tag);
            break;
    }

    return result;
}

void FreeParsedParams(std::vector<HksParam> &params)
{
    HksParam *param = params.data();
    size_t paramCount = params.size();
    if (param == nullptr) {
        return;
    }
    while (paramCount > 0) {
        paramCount--;
        if ((param->tag & HKS_TAG_TYPE_MASK) == HKS_TAG_TYPE_BYTES) {
            HKS_FREE_PTR(param->blob.data);
            param->blob.size = 0;
        }
        ++param;
    }
}

napi_value ParseParams(napi_env env, napi_value object, std::vector<HksParam> &params)
{
    bool hasNextElement = false;
    napi_value result = nullptr;
    size_t index = 0;
    while ((napi_has_element(env, object, index, &hasNextElement) == napi_ok) && hasNextElement) {
        napi_value element = nullptr;
        NAPI_CALL(env, napi_get_element(env, object, index, &element));

        HksParam param = { 0 };
        result = GetHksParam(env, element, param);
        if (result == nullptr) {
            HKS_LOG_E("get param failed when parse input params.");
            return nullptr;
        }

        params.push_back(param);
        index++;
    }
    return GetInt32(env, 0);
}

static int32_t AddParams(const std::vector<HksParam> &params, struct HksParamSet *&paramSet)
{
    for (auto &param : params) {
        int32_t ret = HksAddParams(paramSet, &param, 1);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("add param.tag[%" LOG_PUBLIC "x] failed", param.tag);
            return ret;
        }
    }
    return HKS_SUCCESS;
}

static napi_value ParseHksParamSetOrAddParam(napi_env env, napi_value object, HksParamSet *&paramSet,
    HksParam *addParam)
{
    if (paramSet != nullptr) {
        HKS_LOG_E("param input invalid");
        return nullptr;
    }

    std::vector<HksParam> params;
    HksParamSet *outParamSet = nullptr;
    do {
        if (HksInitParamSet(&outParamSet) != HKS_SUCCESS) {
            napi_throw_error(env, NULL, "native error");
            HKS_LOG_E("paramset init failed");
            break;
        }

        if (ParseParams(env, object, params) == nullptr) {
            HKS_LOG_E("parse params failed");
            break;
        }

        if (addParam != nullptr) {
            params.push_back(*addParam);
        }

        if (AddParams(params, outParamSet) != HKS_SUCCESS) {
            HKS_LOG_E("add params failed");
            break;
        }

        if (HksBuildParamSet(&outParamSet) != HKS_SUCCESS) {
            HKS_LOG_E("HksBuildParamSet failed");
            break;
        }

        FreeParsedParams(params);
        paramSet = outParamSet;
        return GetInt32(env, 0);
    } while (0);

    HksFreeParamSet(&outParamSet);
    FreeParsedParams(params);
    return nullptr;
}

napi_value ParseHksParamSet(napi_env env, napi_value object, HksParamSet *&paramSet)
{
    return ParseHksParamSetOrAddParam(env, object, paramSet, nullptr);
}

napi_value ParseHksParamSetAndAddParam(napi_env env, napi_value object, HksParamSet *&paramSet, HksParam *addParam)
{
    return ParseHksParamSetOrAddParam(env, object, paramSet, addParam);
}

napi_ref GetCallback(napi_env env, napi_value object)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, object, &valueType);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
        HKS_LOG_E("could not get object type");
        return nullptr;
    }

    if (valueType != napi_function) {
        HKS_LOG_E("invalid type");
        return nullptr;
    }

    napi_ref ref = nullptr;
    status = napi_create_reference(env, object, 1, &ref);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
        HKS_LOG_E("could not create reference");
        return nullptr;
    }
    return ref;
}

static napi_value GenerateAarrayBuffer(napi_env env, uint8_t *data, uint32_t size)
{
    uint8_t *buffer = static_cast<uint8_t *>(HksMalloc(size));
    if (buffer == nullptr) {
        return nullptr;
    }

    napi_value outBuffer = nullptr;
    (void)memcpy_s(buffer, size, data, size);

    napi_status status = napi_create_external_arraybuffer(
        env, buffer, size, [](napi_env env, void *data, void *hint) { HksFree(data); }, nullptr, &outBuffer);
    if (status == napi_ok) {
        // free by finalize callback
        buffer = nullptr;
    } else {
        HksFree(buffer);
        GET_AND_THROW_LAST_ERROR((env));
    }

    return outBuffer;
}

static napi_value GenerateHksParam(napi_env env, const HksParam &param)
{
    napi_value hksParam = nullptr;
    NAPI_CALL(env, napi_create_object(env, &hksParam));

    napi_value tag = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, param.tag, &tag));
    NAPI_CALL(env, napi_set_named_property(env, hksParam, HKS_PARAM_PROPERTY_TAG.c_str(), tag));

    napi_value value = nullptr;
    switch (param.tag & HKS_TAG_TYPE_MASK) {
        case HKS_TAG_TYPE_INT:
            NAPI_CALL(env, napi_create_int32(env, param.int32Param, &value));
            break;
        case HKS_TAG_TYPE_UINT:
            NAPI_CALL(env, napi_create_uint32(env, param.uint32Param, &value));
            break;
        case HKS_TAG_TYPE_ULONG:
            NAPI_CALL(env, napi_create_int64(env, param.uint64Param, &value));
            break;
        case HKS_TAG_TYPE_BOOL:
            NAPI_CALL(env, napi_get_boolean(env, param.boolParam, &value));
            break;
        case HKS_TAG_TYPE_BYTES:
            value = GenerateAarrayBuffer(env, param.blob.data, param.blob.size);
            break;
        default:
            value = GetNull(env);
            break;
    }
    NAPI_CALL(env, napi_set_named_property(env, hksParam, HKS_PARAM_PROPERTY_VALUE.c_str(), value));

    return hksParam;
}

static napi_value GenerateHksParamArray(napi_env env, const HksParamSet &paramSet)
{
    napi_value paramArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &paramArray));

    for (uint32_t i = 0; i < paramSet.paramsCnt; i++) {
        napi_value element = nullptr;
        element = GenerateHksParam(env, paramSet.params[i]);
        napi_set_element(env, paramArray, i, element);
    }

    return paramArray;
}

static napi_value GenerateResult(napi_env env, int32_t error, uint8_t *data, uint32_t size,
    const HksParamSet *paramSet)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, error, &errorCode));
    NAPI_CALL(env, napi_set_named_property(env, result, HKS_RESULT_PROPERTY_ERRORCODE.c_str(), errorCode));

    napi_value outData = nullptr;
    if (data != nullptr && size != 0) {
        napi_value outBuffer = GenerateAarrayBuffer(env, data, size);
        if (outBuffer != nullptr) {
            NAPI_CALL(env, napi_create_typedarray(env, napi_uint8_array, size, outBuffer, 0, &outData));
        }
    } else {
        outData = GetNull(env);
    }
    NAPI_CALL(env, napi_set_named_property(env, result, HKS_RESULT_PROPERTY_OUTDATA.c_str(), outData));

    napi_value properties = nullptr;
    if (paramSet == nullptr) {
        properties = GetNull(env);
    } else {
        properties = GenerateHksParamArray(env, *paramSet);
    }
    NAPI_CALL(env, napi_set_named_property(env, result, HKS_RESULT_PRPPERTY_PROPERTIES.c_str(), properties));

    return result;
}

napi_value GenerateHksResult(napi_env env, int32_t error, uint8_t *data, uint32_t size)
{
    return GenerateResult(env, error, data, size, nullptr);
}

napi_value GenerateHksResult(napi_env env, int32_t error, uint8_t *data, uint32_t size, const HksParamSet &paramSet)
{
    return GenerateResult(env, error, data, size, &paramSet);
}

static napi_value GenerateBusinessError(napi_env env, int32_t errorCode)
{
    napi_value businessError = nullptr;
    NAPI_CALL(env, napi_create_object(env, &businessError));

    napi_value code = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errorCode, &code));
    NAPI_CALL(env, napi_set_named_property(env, businessError, BUSINESS_ERROR_PROPERTY_CODE.c_str(), code));

    return businessError;
}

void CallAsyncCallback(napi_env env, napi_ref callback, int32_t error, napi_value data)
{
    napi_value businessError = GenerateBusinessError(env, error);

    napi_value params[ASYNCCALLBACK_ARGC] = { businessError, data };

    napi_value func = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callback, &func));

    napi_value recv = nullptr;
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &recv));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, recv, func, ASYNCCALLBACK_ARGC, params, &result));
}

napi_value GenerateStringArray(napi_env env, const struct HksBlob *blob, const uint32_t blobCount)
{
    if (blobCount == 0 || blob == nullptr) {
        return nullptr;
    }
    napi_value array = nullptr;
    NAPI_CALL(env, napi_create_array(env, &array));
    for (uint32_t i = 0; i < blobCount; i++) {
        napi_value element = nullptr;
        napi_create_string_latin1(env, reinterpret_cast<const char *>(blob[i].data), blob[i].size, &element);
        napi_set_element(env, array, i, element);
    }
    return array;
}

void FreeHksCertChain(HksCertChain *&certChain)
{
    if (certChain == nullptr) {
        return;
    }

    if (certChain->certsCount > 0 && certChain->certs != nullptr) {
        for (uint32_t i = 0; i < certChain->certsCount; i++) {
            if (certChain->certs[i].data != nullptr) {
                HksFree(certChain->certs[i].data);
                certChain->certs[i].data = nullptr;
            }
        }
    }

    HksFree(certChain);
    certChain = nullptr;
}

napi_value GenerateHksHandle(napi_env env, int32_t error, const struct HksBlob *handle,
    const struct HksBlob *token)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, error, &errorCode));
    NAPI_CALL(env, napi_set_named_property(env, result, HKS_HANDLE_PROPERTY_ERRORCODE.c_str(), errorCode));

    if (error != HKS_SUCCESS) {
        HKS_LOG_E("init failed, ret = %" LOG_PUBLIC "d", error);
        return result;
    }

    if ((handle == nullptr) || (handle->data == nullptr) || (handle->size != sizeof(uint64_t))) {
        HKS_LOG_E("invalid handle");
        return result;
    }

    uint64_t tempHandle = *reinterpret_cast<uint64_t *>(handle->data);
    uint32_t handleValue = static_cast<uint32_t>(tempHandle); /* Temporarily only use 32 bit handle */

    napi_value handlejs = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, handleValue, &handlejs));
    NAPI_CALL(env, napi_set_named_property(env, result, HKS_HANDLE_PROPERTY_HANDLE.c_str(), handlejs));

    if (token == nullptr) {
        HKS_LOG_E("invalid token");
        return result;
    }

    napi_value tokenjs = nullptr;
    if ((token->size != 0) && (token->data != nullptr)) {
        napi_value outBuffer = GenerateAarrayBuffer(env, token->data, token->size);
        if (outBuffer != nullptr) {
            NAPI_CALL(env, napi_create_typedarray(env, napi_uint8_array, token->size, outBuffer, 0, &tokenjs));
        } else {
            tokenjs = GetNull(env);
        }
    } else {
        tokenjs = GetNull(env);
    }
    NAPI_CALL(env, napi_set_named_property(env, result, HKS_HANDLE_PROPERTY_TOKEN.c_str(), tokenjs));

    return result;
}

napi_value GetHandleValue(napi_env env, napi_value object, struct HksBlob *&handleBlob)
{
    if (handleBlob != nullptr) {
        HKS_LOG_E("param input invalid");
        return nullptr;
    }

    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, object, &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    uint32_t handleTmp = 0;
    napi_status status = napi_get_value_uint32(env, object, &handleTmp);
    if (status != napi_ok) {
        HKS_LOG_E("Retrieve field failed");
        return nullptr;
    }

    uint64_t handle = static_cast<uint64_t>(handleTmp);

    handleBlob = static_cast<struct HksBlob *>(HksMalloc(sizeof(struct HksBlob)));
    if (handleBlob == nullptr) {
        HKS_LOG_E("could not alloc memory");
        return nullptr;
    }

    handleBlob->data = static_cast<uint8_t *>(HksMalloc(sizeof(uint64_t)));
    if (handleBlob->data == nullptr) {
        HKS_FREE_PTR(handleBlob);
        HKS_LOG_E("could not alloc memory");
        return nullptr;
    }
    handleBlob->size = sizeof(uint64_t);
    if (memcpy_s(handleBlob->data, sizeof(uint64_t), &handle, sizeof(uint64_t)) != EOK) {
        // the memory of handleBlob free by finalize callback
        return nullptr;
    }

    return GetInt32(env, 0);
}

void DeleteCommonAsyncContext(napi_env env, napi_async_work &asyncWork, napi_ref &callback,
    struct HksBlob *&blob, struct HksParamSet *&paramSet)
{
    if (asyncWork != nullptr) {
        napi_delete_async_work(env, asyncWork);
        asyncWork = nullptr;
    }

    if (callback != nullptr) {
        napi_delete_reference(env, callback);
        callback = nullptr;
    }

    if (blob != nullptr) {
        FreeHksBlob(blob);
    }

    if (paramSet != nullptr) {
        HksFreeParamSet(&paramSet);
    }
}

static napi_value ParseGetHksParamSet(napi_env env, napi_value value, HksParamSet *&paramSet)
{
    napi_value properties = nullptr;
    napi_status status = napi_get_named_property(env, value,
        HKS_OPTIONS_PROPERTY_PROPERTIES.c_str(), &properties);
    if (status != napi_ok || properties == nullptr) {
        GET_AND_THROW_LAST_ERROR((env));
        HKS_LOG_E("could not get property %" LOG_PUBLIC "s", HKS_OPTIONS_PROPERTY_PROPERTIES.c_str());
        return nullptr;
    }
    napi_value result = ParseHksParamSet(env, properties, paramSet);
    if (result == nullptr) {
        HKS_LOG_E("could not get paramset");
        return nullptr;
    }

    return GetInt32(env, 0);
}

napi_value ParseHandleAndHksParamSet(napi_env env, napi_value *argv, size_t &index,
    HksBlob *&handleBlob, HksParamSet *&paramSet)
{
    // the index is controlled by the caller and needs to ensure that it does not overflow
    if (argv == nullptr || handleBlob != nullptr || paramSet != nullptr) {
        HKS_LOG_E("param input invalid");
        return nullptr;
    }

    napi_value result = GetHandleValue(env, argv[index], handleBlob);
    if (result == nullptr) {
        HKS_LOG_E("could not get handle value");
        return nullptr;
    }

    index++;
    result = ParseGetHksParamSet(env, argv[index], paramSet);
    if (result == nullptr) {
        HKS_LOG_E("could not get hksParamSet");
        return nullptr;
    }

    return GetInt32(env, 0);
}

napi_value ParseKeyAliasAndHksParamSet(napi_env env, napi_value *argv, size_t &index,
    HksBlob *&keyAliasBlob, HksParamSet *&paramSet)
{
    // the index is controlled by the caller and needs to ensure that it does not overflow
    if (argv == nullptr || keyAliasBlob != nullptr || paramSet != nullptr) {
        HKS_LOG_E("param input invalid");
        return nullptr;
    }

    napi_value result = ParseKeyAlias(env, argv[index], keyAliasBlob);
    if (result == nullptr) {
        HKS_LOG_E("could not get keyAlias");
        return nullptr;
    }

    index++;
    result = ParseGetHksParamSet(env, argv[index], paramSet);
    if (result == nullptr) {
        HKS_LOG_E("get hksParamSet failed");
        return nullptr;
    }

    return GetInt32(env, 0);
}

napi_value ParseKeyData(napi_env env, napi_value value, HksBlob *&keyDataBlob)
{
    if (keyDataBlob != nullptr) {
        HKS_LOG_E("param input invalid");
        return nullptr;
    }

    napi_value inData = nullptr;
    napi_status status = napi_get_named_property(env, value, HKS_OPTIONS_PROPERTY_INDATA.c_str(), &inData);
    if (status != napi_ok || inData == nullptr) {
        GET_AND_THROW_LAST_ERROR((env));
        HKS_LOG_E("could not get property %" LOG_PUBLIC "s", HKS_OPTIONS_PROPERTY_INDATA.c_str());
        return nullptr;
    }

    keyDataBlob = static_cast<HksBlob *>(HksMalloc(sizeof(HksBlob)));
    if (keyDataBlob == nullptr) {
        HKS_LOG_E("could not alloc memory");
        return nullptr;
    }
    (void)memset_s(keyDataBlob, sizeof(HksBlob), 0, sizeof(HksBlob));

    if (GetUint8Array(env, inData, *keyDataBlob) == nullptr) {
        FreeHksBlob(keyDataBlob);
        HKS_LOG_E("could not get indata");
        return nullptr;
    }

    return GetInt32(env, 0);
}
}  // namespace HuksNapi
