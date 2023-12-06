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
#define LOG_TAG "JSUtil"
#include "js_util.h"
#include <endian.h>
#include <securec.h>
#include "ability.h"
#include "hap_module_info.h"
#include "js_schema.h"
#include "kv_utils.h"
#include "log_print.h"
#include "napi_base_context.h"
#include "napi_queue.h"
#include "types.h"

using namespace OHOS::DistributedKv;
using namespace OHOS::DataShare;
namespace OHOS::DistributedKVStore {
constexpr int32_t STR_MAX_LENGTH = 4096;
constexpr size_t STR_TAIL_LENGTH = 1;
struct PredicatesProxy {
    std::shared_ptr<DataShareAbsPredicates> predicates_;
};

napi_status JSUtil::GetValue(napi_env env, napi_value in, napi_value& out)
{
    out = in;
    return napi_ok;
}

napi_status JSUtil::SetValue(napi_env env, napi_value in, napi_value& out)
{
    out = in;
    return napi_ok;
}

/* napi_value <-> bool */
napi_status JSUtil::GetValue(napi_env env, napi_value in, bool& out)
{
    return napi_get_value_bool(env, in, &out);
}

napi_status JSUtil::SetValue(napi_env env, const bool& in, napi_value& out)
{
    return napi_get_boolean(env, in, &out);
}

/* napi_value <-> int32_t */
napi_status JSUtil::GetValue(napi_env env, napi_value in, int32_t& out)
{
    return napi_get_value_int32(env, in, &out);
}

napi_status JSUtil::SetValue(napi_env env, const int32_t& in, napi_value& out)
{
    return napi_create_int32(env, in, &out);
}

/* napi_value <-> uint32_t */
napi_status JSUtil::GetValue(napi_env env, napi_value in, uint32_t& out)
{
    return napi_get_value_uint32(env, in, &out);
}

napi_status JSUtil::SetValue(napi_env env, const uint32_t& in, napi_value& out)
{
    return napi_create_uint32(env, in, &out);
}

/* napi_value <-> int64_t */
napi_status JSUtil::GetValue(napi_env env, napi_value in, int64_t& out)
{
    return napi_get_value_int64(env, in, &out);
}

napi_status JSUtil::SetValue(napi_env env, const int64_t& in, napi_value& out)
{
    return napi_create_int64(env, in, &out);
}

/* napi_value <-> double */
napi_status JSUtil::GetValue(napi_env env, napi_value in, double& out)
{
    return napi_get_value_double(env, in, &out);
}

napi_status JSUtil::SetValue(napi_env env, const double& in, napi_value& out)
{
    return napi_create_double(env, in, &out);
}

/* napi_value <-> std::string */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::string& out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    ASSERT((status == napi_ok) && (type == napi_string), "invalid type", napi_invalid_arg);

    size_t maxLen = STR_MAX_LENGTH;
    status = napi_get_value_string_utf8(env, in, NULL, 0, &maxLen);
    if (maxLen <= 0) {
        return status;
    }
    ZLOGD("napi_value -> std::string get length %{public}d", (int)maxLen);
    char* buf = new (std::nothrow) char[maxLen + STR_TAIL_LENGTH];
    if (buf != nullptr) {
        size_t len = 0;
        status = napi_get_value_string_utf8(env, in, buf, maxLen + STR_TAIL_LENGTH, &len);
        if (status == napi_ok) {
            buf[len] = 0;
            out = std::string(buf);
        }
        delete[] buf;
    } else {
        status = napi_generic_failure;
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::string& in, napi_value& out)
{
    return napi_create_string_utf8(env, in.c_str(), in.size(), &out);
}

/* napi_value <-> std::vector<std::string> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<std::string>& out)
{
    ZLOGD("napi_value -> std::vector<std::string>");
    out.clear();
    bool isArray = false;
    napi_is_array(env, in, &isArray);
    ASSERT(isArray, "not an array", napi_invalid_arg);

    uint32_t length = 0;
    napi_status status = napi_get_array_length(env, in, &length);
    ASSERT((status == napi_ok) && (length > 0), "get_array failed!", napi_invalid_arg);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value item = nullptr;
        status = napi_get_element(env, in, i, &item);
        ASSERT((item != nullptr) && (status == napi_ok), "no element", napi_invalid_arg);
        std::string value;
        status = GetValue(env, item, value);
        ASSERT(status == napi_ok, "not a string", napi_invalid_arg);
        out.push_back(value);
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<std::string>& in, napi_value& out)
{
    ZLOGD("napi_value <- std::vector<std::string>");
    napi_status status = napi_create_array_with_length(env, in.size(), &out);
    ASSERT(status == napi_ok, "create array failed!", status);
    int index = 0;
    for (auto& item : in) {
        napi_value element = nullptr;
        SetValue(env, item, element);
        status = napi_set_element(env, out, index++, element);
        ASSERT((status == napi_ok), "napi_set_element failed!", status);
    }
    return status;
}

JSUtil::KvStoreVariant JSUtil::Blob2VariantValue(const DistributedKv::Blob& blob)
{
    auto& data = blob.Data();
    // number 2 means: valid Blob must have more than 2 bytes.
    if (data.size() < 1) {
        ZLOGE("Blob have no data!");
        return JSUtil::KvStoreVariant();
    }
    // number 1 means: skip the first byte, byte[0] is real data type.
    std::vector<uint8_t> real(data.begin() + 1, data.end());
    ZLOGD("Blob::type %{public}d size=%{public}d", static_cast<int>(data[0]), static_cast<int>(real.size()));
    if (data[0] == JSUtil::INTEGER) {
        uint32_t tmp4int = be32toh(*reinterpret_cast<uint32_t*>(&(real[0])));
        return JSUtil::KvStoreVariant(*reinterpret_cast<int32_t*>(&tmp4int));
    } else if (data[0] == JSUtil::FLOAT) {
        uint32_t tmp4flt = be32toh(*reinterpret_cast<uint32_t*>(&(real[0])));
        return JSUtil::KvStoreVariant(*reinterpret_cast<float*>((void*)(&tmp4flt)));
    } else if (data[0] == JSUtil::BYTE_ARRAY) {
        return JSUtil::KvStoreVariant(std::vector<uint8_t>(real.begin(), real.end()));
    } else if (data[0] == JSUtil::BOOLEAN) {
        return JSUtil::KvStoreVariant(static_cast<bool>(real[0]));
    } else if (data[0] == JSUtil::DOUBLE) {
        uint64_t tmp4dbl = be64toh(*reinterpret_cast<uint64_t*>(&(real[0])));
        return JSUtil::KvStoreVariant(*reinterpret_cast<double*>((void*)(&tmp4dbl)));
    } else if (data[0] == JSUtil::STRING){
        return JSUtil::KvStoreVariant(std::string(real.begin(), real.end()));
    } else {
        // for schema-db, if (data[0] == JSUtil::STRING), no beginning byte!
        return JSUtil::KvStoreVariant(std::string(data.begin(), data.end()));
    }
}

DistributedKv::Blob JSUtil::VariantValue2Blob(const JSUtil::KvStoreVariant& value)
{
    std::vector<uint8_t> data;
    auto strValue = std::get_if<std::string>(&value);
    if (strValue != nullptr) {
        data.push_back(JSUtil::STRING);
        data.insert(data.end(), (*strValue).begin(), (*strValue).end());
    }
    auto u8ArrayValue = std::get_if<std::vector<uint8_t>>(&value);
    if (u8ArrayValue != nullptr) {
        data.push_back(JSUtil::BYTE_ARRAY);
        data.insert(data.end(), (*u8ArrayValue).begin(), (*u8ArrayValue).end());
    }
    auto boolValue = std::get_if<bool>(&value);
    if (boolValue != nullptr) {
        data.push_back(JSUtil::BOOLEAN);
        data.push_back(static_cast<uint8_t>(*boolValue));
    }
    uint8_t *tmp = nullptr;
    auto intValue = std::get_if<int32_t>(&value);
    if (intValue != nullptr) {
        int32_t tmp4int = *intValue; // copy value, and make it available in stack space.
        uint32_t tmp32 = htobe32(*reinterpret_cast<uint32_t*>(&tmp4int));
        tmp = reinterpret_cast<uint8_t*>(&tmp32);
        data.push_back(JSUtil::INTEGER);
        data.insert(data.end(), tmp, tmp + sizeof(int32_t) / sizeof(uint8_t));
    }
    auto fltValue = std::get_if<float>(&value);
    if (fltValue != nullptr) {
        float tmp4flt = *fltValue; // copy value, and make it available in stack space.
        uint32_t tmp32 = htobe32(*reinterpret_cast<uint32_t*>(&tmp4flt));
        tmp = reinterpret_cast<uint8_t*>(&tmp32);
        data.push_back(JSUtil::FLOAT);
        data.insert(data.end(), tmp, tmp + sizeof(float) / sizeof(uint8_t));
    }
    auto dblValue = std::get_if<double>(&value);
    if (dblValue != nullptr) {
        double tmp4dbl = *dblValue; // copy value, and make it available in stack space.
        uint64_t tmp64 = htobe64(*reinterpret_cast<uint64_t*>(&tmp4dbl));
        tmp = reinterpret_cast<uint8_t*>(&tmp64);
        data.push_back(JSUtil::DOUBLE);
        data.insert(data.end(), tmp, tmp + sizeof(double) / sizeof(uint8_t));
    }
    return DistributedKv::Blob(data);
}

/* napi_value <-> KvStoreVariant */
napi_status JSUtil::GetValue(napi_env env, napi_value in, JSUtil::KvStoreVariant& out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    ASSERT((status == napi_ok), "invalid type", status);
    switch (type) {
        case napi_boolean: {
            bool vBool = false;
            status = JSUtil::GetValue(env, in, vBool);
            out = vBool;
            break;
        }
        case napi_number: {
            double vNum = 0.0f;
            status = JSUtil::GetValue(env, in, vNum);
            out = vNum;
            break;
        }
        case napi_string: {
            std::string vString;
            status = JSUtil::GetValue(env, in, vString);
            out = vString;
            break;
        }
        case napi_object: {
            std::vector<uint8_t> vct;
            status = JSUtil::GetValue(env, in, vct);
            out = vct;
            break;
        }
        default:
            ZLOGE(" napi_value -> KvStoreVariant not [Uint8Array | string | boolean | number]  type=%{public}d", type);
            status = napi_invalid_arg;
            break;
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const JSUtil::KvStoreVariant& in, napi_value& out)
{
    auto strValue = std::get_if<std::string>(&in);
    if (strValue != nullptr) {
        return SetValue(env, *strValue, out);
    }
    auto intValue = std::get_if<int32_t>(&in);
    if (intValue != nullptr) {
        return SetValue(env, *intValue, out);
    }
    auto fltValue = std::get_if<float>(&in);
    if (fltValue != nullptr) {
        return SetValue(env, *fltValue, out);
    }
    auto pUint8 = std::get_if<std::vector<uint8_t>>(&in);
    if (pUint8 != nullptr) {
        return SetValue(env, *pUint8, out);
    }
    auto boolValue = std::get_if<bool>(&in);
    if (boolValue != nullptr) {
        return SetValue(env, *boolValue, out);
    }
    auto dblValue = std::get_if<double>(&in);
    if (dblValue != nullptr) {
        return SetValue(env, *dblValue, out);
    }

    ZLOGE("napi_value <- KvStoreVariant  INVALID value type");
    return napi_invalid_arg;
}

/* napi_value <-> QueryVariant */
napi_status JSUtil::GetValue(napi_env env, napi_value in, JSUtil::QueryVariant& out)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, in, &type);
    ASSERT((status == napi_ok), "invalid type", status);
    ZLOGD("napi_value -> QueryVariant  type=%{public}d", type);
    switch (type) {
        case napi_boolean: {
            bool vBool = false;
            status = JSUtil::GetValue(env, in, vBool);
            out = vBool;
            break;
        }
        case napi_number: {
            double vNum = 0.0f;
            status = JSUtil::GetValue(env, in, vNum);
            out = vNum;
            break;
        }
        case napi_string: {
            std::string vString;
            status = JSUtil::GetValue(env, in, vString);
            out = vString;
            break;
        }
        default:
            status = napi_invalid_arg;
            break;
    }
    ASSERT((status == napi_ok), "napi_value -> QueryVariant bad value!", status);
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const JSUtil::QueryVariant& in, napi_value& out)
{
    ZLOGD("napi_value <- QueryVariant ");
    napi_status status = napi_invalid_arg;
    auto strValue = std::get_if<std::string>(&in);
    if (strValue != nullptr) {
        status = SetValue(env, *strValue, out);
    }
    auto boolValue = std::get_if<bool>(&in);
    if (boolValue != nullptr) {
        status = SetValue(env, *boolValue, out);
    }
    auto dblValue = std::get_if<double>(&in);
    if (dblValue != nullptr) {
        status = SetValue(env, *dblValue, out);
    } else {
        ZLOGD("napi_value <- QueryVariant  INVALID value type");
    }
    return status;
}

/* napi_value <-> std::vector<uint8_t> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<uint8_t>& out)
{
    out.clear();
    ZLOGD("napi_value -> std::vector<uint8_t> ");
    napi_typedarray_type type = napi_biguint64_array;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    void* data = nullptr;
    napi_status status = napi_get_typedarray_info(env, in, &type, &length, &data, &buffer, &offset);
    ZLOGD("array type=%{public}d length=%{public}d offset=%{public}d", (int)type, (int)length, (int)offset);
    ASSERT(status == napi_ok, "napi_get_typedarray_info failed!", napi_invalid_arg);
    ASSERT(type == napi_uint8_array, "is not Uint8Array!", napi_invalid_arg);
    ASSERT((length > 0) && (data != nullptr), "invalid data!", napi_invalid_arg);
    out.assign((uint8_t*)data, ((uint8_t*)data) + length);
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<uint8_t>& in, napi_value& out)
{
    ZLOGD("napi_value <- std::vector<uint8_t> ");
    ASSERT(in.size() > 0, "invalid std::vector<uint8_t>", napi_invalid_arg);
    void* data = nullptr;
    napi_value buffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, in.size(), &data, &buffer);
    ASSERT((status == napi_ok), "create array buffer failed!", status);

    if (memcpy_s(data, in.size(), in.data(), in.size()) != EOK) {
        ZLOGE("memcpy_s not EOK");
        return napi_invalid_arg;
    }
    status = napi_create_typedarray(env, napi_uint8_array, in.size(), buffer, 0, &out);
    ASSERT((status == napi_ok), "napi_value <- std::vector<uint8_t> invalid value", status);
    return status;
}

template <typename T>
void TypedArray2Vector(uint8_t* data, size_t length, napi_typedarray_type type, std::vector<T>& out)
{
    auto convert = [&out](auto* data, size_t elements) {
        for (size_t index = 0; index < elements; index++) {
            out.push_back(static_cast<T>(data[index]));
        }
    };

    switch (type) {
        case napi_int8_array:
            convert(reinterpret_cast<int8_t*>(data), length);
            break;
        case napi_uint8_array:
            convert(data, length);
            break;
        case napi_uint8_clamped_array:
            convert(data, length);
            break;
        case napi_int16_array:
            convert(reinterpret_cast<int16_t*>(data), length / sizeof(int16_t));
            break;
        case napi_uint16_array:
            convert(reinterpret_cast<uint16_t*>(data), length / sizeof(uint16_t));
            break;
        case napi_int32_array:
            convert(reinterpret_cast<int32_t*>(data), length / sizeof(int32_t));
            break;
        case napi_uint32_array:
            convert(reinterpret_cast<uint32_t*>(data), length / sizeof(uint32_t));
            break;
        case napi_float32_array:
            convert(reinterpret_cast<float*>(data), length / sizeof(float));
            break;
        case napi_float64_array:
            convert(reinterpret_cast<double*>(data), length / sizeof(double));
            break;
        case napi_bigint64_array:
            convert(reinterpret_cast<int64_t*>(data), length / sizeof(int64_t));
            break;
        case napi_biguint64_array:
            convert(reinterpret_cast<uint64_t*>(data), length / sizeof(uint64_t));
            break;
        default:
            ASSERT_VOID(false, "[FATAL] invalid napi_typedarray_type!");
    }
}

/* napi_value <-> std::vector<int32_t> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<int32_t>& out)
{
    out.clear();
    ZLOGD("napi_value -> std::vector<int32_t> ");
    napi_typedarray_type type = napi_biguint64_array;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    uint8_t* data = nullptr;
    napi_status status = napi_get_typedarray_info(env, in, &type, &length,
                                                  reinterpret_cast<void**>(&data), &buffer, &offset);
    ZLOGD("array type=%{public}d length=%{public}d offset=%{public}d", (int)type, (int)length, (int)offset);
    ASSERT(status == napi_ok, "napi_get_typedarray_info failed!", napi_invalid_arg);
    ASSERT(type <= napi_int32_array, "is not int32 supported typed array!", napi_invalid_arg);
    ASSERT((length > 0) && (data != nullptr), "invalid data!", napi_invalid_arg);
    TypedArray2Vector<int32_t>(data, length, type, out);
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<int32_t>& in, napi_value& out)
{
    ZLOGD("napi_value <- std::vector<int32_t> ");
    size_t bytes = in.size() * sizeof(int32_t);
    ASSERT(bytes > 0, "invalid std::vector<int32_t>", napi_invalid_arg);
    void* data = nullptr;
    napi_value buffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, bytes, &data, &buffer);
    ASSERT((status == napi_ok), "invalid buffer", status);

    if (memcpy_s(data, bytes, in.data(), bytes) != EOK) {
        ZLOGE("memcpy_s not EOK");
        return napi_invalid_arg;
    }
    status = napi_create_typedarray(env, napi_int32_array, in.size(), buffer, 0, &out);
    ASSERT((status == napi_ok), "invalid buffer", status);
    return status;
}

/* napi_value <-> std::vector<uint32_t> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<uint32_t>& out)
{
    out.clear();
    ZLOGD("napi_value -> std::vector<uint32_t> ");
    napi_typedarray_type type = napi_biguint64_array;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    uint8_t* data = nullptr;
    napi_status status = napi_get_typedarray_info(env, in, &type, &length,
                                                  reinterpret_cast<void**>(&data), &buffer, &offset);
    ZLOGD("napi_get_typedarray_info type=%{public}d", (int)type);
    ASSERT(status == napi_ok, "napi_get_typedarray_info failed!", napi_invalid_arg);
    ASSERT((type <= napi_uint16_array) || (type == napi_uint32_array), "invalid type!", napi_invalid_arg);
    ASSERT((length > 0) && (data != nullptr), "invalid data!", napi_invalid_arg);
    TypedArray2Vector<uint32_t>(data, length, type, out);
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<uint32_t>& in, napi_value& out)
{
    ZLOGD("napi_value <- std::vector<uint32_t> ");
    size_t bytes = in.size() * sizeof(uint32_t);
    ASSERT(bytes > 0, "invalid std::vector<uint32_t>", napi_invalid_arg);
    void* data = nullptr;
    napi_value buffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, bytes, &data, &buffer);
    ASSERT((status == napi_ok), "invalid buffer", status);

    if (memcpy_s(data, bytes, in.data(), bytes) != EOK) {
        ZLOGE("memcpy_s not EOK");
        return napi_invalid_arg;
    }
    status = napi_create_typedarray(env, napi_uint32_array, in.size(), buffer, 0, &out);
    ASSERT((status == napi_ok), "invalid buffer", status);
    return status;
}

/* napi_value <-> std::vector<int64_t> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<int64_t>& out)
{
    out.clear();
    ZLOGD("napi_value -> std::vector<int64_t> ");
    napi_typedarray_type type = napi_biguint64_array;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    uint8_t* data = nullptr;
    napi_status status = napi_get_typedarray_info(env, in, &type, &length,
                                                  reinterpret_cast<void**>(&data), &buffer, &offset);
    ZLOGD("array type=%{public}d length=%{public}d offset=%{public}d", (int)type, (int)length, (int)offset);
    ASSERT(status == napi_ok, "napi_get_typedarray_info failed!", napi_invalid_arg);
    ASSERT((type <= napi_uint32_array) || (type == napi_bigint64_array), "invalid type!", napi_invalid_arg);
    ASSERT((length > 0) && (data != nullptr), "invalid data!", napi_invalid_arg);
    TypedArray2Vector<int64_t>(data, length, type, out);
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<int64_t>& in, napi_value& out)
{
    ZLOGD("napi_value <- std::vector<int64_t> ");
    size_t bytes = in.size() * sizeof(int64_t);
    ASSERT(bytes > 0, "invalid std::vector<uint32_t>", napi_invalid_arg);
    void* data = nullptr;
    napi_value buffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, bytes, &data, &buffer);
    ASSERT((status == napi_ok), "invalid buffer", status);

    if (memcpy_s(data, bytes, in.data(), bytes) != EOK) {
        ZLOGE("memcpy_s not EOK");
        return napi_invalid_arg;
    }
    status = napi_create_typedarray(env, napi_bigint64_array, in.size(), buffer, 0, &out);
    ASSERT((status == napi_ok), "invalid buffer", status);
    return status;
}
/* napi_value <-> std::vector<double> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<double>& out)
{
    out.clear();
    bool isTypedArray = false;
    napi_status status = napi_is_typedarray(env, in, &isTypedArray);
    ZLOGD("napi_value -> std::vector<double> input %{public}s a TypedArray", isTypedArray ? "is" : "is not");
    ASSERT((status == napi_ok), "napi_is_typedarray failed!", status);
    if (isTypedArray) {
        ZLOGD("napi_value -> std::vector<double> ");
        napi_typedarray_type type = napi_biguint64_array;
        size_t length = 0;
        napi_value buffer = nullptr;
        size_t offset = 0;
        uint8_t* data = nullptr;
        status = napi_get_typedarray_info(env, in, &type, &length, reinterpret_cast<void**>(&data), &buffer, &offset);
        ZLOGD("napi_get_typedarray_info status=%{public}d type=%{public}d", status, (int)type);
        ASSERT(status == napi_ok, "napi_get_typedarray_info failed!", napi_invalid_arg);
        ASSERT((length > 0) && (data != nullptr), "invalid data!", napi_invalid_arg);
        TypedArray2Vector<double>(data, length, type, out);
    } else {
        bool isArray = false;
        status = napi_is_array(env, in, &isArray);
        ZLOGD("napi_value -> std::vector<double> input %{public}s an Array", isArray ? "is" : "is not");
        ASSERT((status == napi_ok) && isArray, "invalid data!", napi_invalid_arg);
        uint32_t length = 0;
        status = napi_get_array_length(env, in, &length);
        ASSERT((status == napi_ok) && (length > 0), "invalid data!", napi_invalid_arg);
        for (uint32_t i = 0; i < length; ++i) {
            napi_value item = nullptr;
            status = napi_get_element(env, in, i, &item);
            ASSERT((item != nullptr) && (status == napi_ok), "no element", napi_invalid_arg);
            double vi = 0.0f;
            status = napi_get_value_double(env, item, &vi);
            ASSERT(status == napi_ok, "element not a double", napi_invalid_arg);
            out.push_back(vi);
        }
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<double>& in, napi_value& out)
{
    ZLOGD("napi_value <- std::vector<double> ");
    (void)(env);
    (void)(in);
    (void)(out);
    ASSERT(false, "std::vector<double> to napi_value, unsupported!", napi_invalid_arg);
    return napi_invalid_arg;
}

/* napi_value <-> std::map<std::string, int32_t> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::map<std::string, DistributedKv::Status>& out)
{
    ZLOGD("napi_value -> std::map<std::string, int32_t> ");
    (void)(env);
    (void)(in);
    (void)(out);
    ASSERT(false, "std::map<std::string, uint32_t> from napi_value, unsupported!", napi_invalid_arg);
    return napi_invalid_arg;
}

napi_status JSUtil::SetValue(napi_env env, const std::map<std::string, DistributedKv::Status>& in, napi_value& out)
{
    ZLOGD("napi_value <- std::map<std::string, int32_t> ");
    napi_status status = napi_create_array_with_length(env, in.size(), &out);
    ASSERT((status == napi_ok), "invalid object", status);
    int index = 0;
    for (const auto& [key, value] : in) {
        napi_value element = nullptr;
        napi_create_array_with_length(env, TUPLE_SIZE, &element);
        napi_value jsKey = nullptr;
        napi_create_string_utf8(env, key.c_str(), key.size(), &jsKey);
        napi_set_element(env, element, TUPLE_KEY, jsKey);
        napi_value jsValue = nullptr;
        napi_create_int32(env, static_cast<int32_t>(value), &jsValue);
        napi_set_element(env, element, TUPLE_VALUE, jsValue);
        napi_set_element(env, out, index++, element);
    }
    return status;
}

/*
 *  interface Value {
 *       type: ValueType;
 *       value: Uint8Array | string | number | boolean;
 *   }
 *    interface Entry {
 *        key: string;
 *        value: Value;
 *  }
 */
/* napi_value <-> DistributedKv::Entry */
napi_status JSUtil::GetValue(napi_env env, napi_value in, DistributedKv::Entry& out, bool hasSchema)
{
    ZLOGD("napi_value -> DistributedKv::Entry ");
    napi_value propKey = nullptr;
    napi_status status = napi_get_named_property(env, in, "key", &propKey);
    ASSERT((status == napi_ok), "no property key", status);
    std::string key;
    status = GetValue(env, propKey, key);
    ASSERT((status == napi_ok), "no value of key", status);

    napi_value propValue = nullptr;
    status = napi_get_named_property(env, in, "value", &propValue);
    ASSERT((status == napi_ok), "no property value", status);

    napi_value propVType = nullptr;
    status = napi_get_named_property(env, propValue, "type", &propVType);
    ASSERT((status == napi_ok), "no property value.type", status);
    int32_t type = 0; // int8_t
    status = GetValue(env, propVType, type);
    ASSERT((status == napi_ok), "no value of value.type", status);

    napi_value propVValue = nullptr;
    status = napi_get_named_property(env, propValue, "value", &propVValue);
    ASSERT((status == napi_ok), "no property value.value", status);
    KvStoreVariant value = 0;
    status = GetValue(env, propVValue, value);
    ASSERT((status == napi_ok), "no value of value.value", status);

    out.key = key;
    if (hasSchema) {
        out.value = std::get<std::string>(value);
    } else {
        out.value = JSUtil::VariantValue2Blob(value);
    }
    if (type != out.value[0]) {
        ZLOGE("unmarch type[%{public}d] to value.type[%{public}d]", (int)type, (int)out.value[0]);
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const DistributedKv::Entry& in, napi_value& out, bool hasSchema)
{
    ZLOGD("napi_value <- DistributedKv::Entry ");
    napi_status status = napi_create_object(env, &out);
    ASSERT((status == napi_ok), "invalid entry object", status);

    napi_value key = nullptr;
    status = SetValue(env, in.key.ToString(), key);
    ASSERT((status == napi_ok), "invalid entry key", status);
    napi_set_named_property(env, out, "key", key);

    ASSERT((in.value.Size() > 0), "invalid entry value", status);
    napi_value value = nullptr;

    status = napi_create_object(env, &value);
    ASSERT((status == napi_ok), "invalid value object", status);
    napi_value vType = nullptr;
    napi_create_int32(env, in.value[0], &vType);
    napi_set_named_property(env, value, "type", vType);

    napi_value vValue = nullptr;
    if (hasSchema) {
        status = SetValue(env, in.value.ToString(), vValue);
    } else {
        status = SetValue(env, Blob2VariantValue(in.value), vValue);
    }
    ASSERT((status == napi_ok), "invalid entry value", status);
    napi_set_named_property(env, value, "value", vValue);

    napi_set_named_property(env, out, "value", value);
    return status;
}

/* napi_value <-> std::list<DistributedKv::Entry> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::list<DistributedKv::Entry>& out, bool hasSchema)
{
    ZLOGD("napi_value -> std::list<DistributedKv::Entry> ");
    bool isArray = false;
    napi_is_array(env, in, &isArray);
    ASSERT(isArray, "not array", napi_invalid_arg);

    uint32_t length = 0;
    napi_status status = napi_get_array_length(env, in, &length);
    ASSERT((status == napi_ok) && (length > 0), "get_array failed!", status);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value item = nullptr;
        status = napi_get_element(env, in, i, &item);
        ASSERT((status == napi_ok), "no element", status);
        if ((status != napi_ok) || (item == nullptr)) {
            continue;
        }
        DistributedKv::Entry entry;
        status = GetValue(env, item, entry, hasSchema);
        out.push_back(entry);
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::list<DistributedKv::Entry>& in, napi_value& out, bool hasSchema)
{
    ZLOGD("napi_value <- std::list<DistributedKv::Entry> %{public}d", static_cast<int>(in.size()));
    napi_status status = napi_create_array_with_length(env, in.size(), &out);
    ASSERT(status == napi_ok, "create array failed!", status);
    int index = 0;
    for (const auto& item : in) {
        napi_value entry = nullptr;
        SetValue(env, item, entry, hasSchema);
        napi_set_element(env, out, index++, entry);
    }
    return status;
}

napi_status JSUtil::GetValue(napi_env env, napi_value jsValue, ValueObject &valueObject)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, jsValue, &type);
    if (type == napi_string) {
        std::string value;
        JSUtil::GetValue(env, jsValue, value);
        valueObject = value;
    } else if (type == napi_number) {
        double value = 0;
        napi_get_value_double(env, jsValue, &value);
        valueObject = value;
    } else if (type == napi_boolean) {
        bool value = false;
        napi_get_value_bool(env, jsValue, &value);
        valueObject = value;
    } else if (type == napi_object) {
        std::vector<uint8_t> value;
        JSUtil::GetValue(env, jsValue, value);
        valueObject = std::move(value);
    }
    return napi_ok;
}

napi_status JSUtil::GetValue(napi_env env, napi_value jsValue, ValuesBucket &valuesBucket)
{
    napi_value keys = 0;
    napi_get_property_names(env, jsValue, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        return status;
    }
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value jsKey = 0;
        status = napi_get_element(env, keys, i, &jsKey);
        ASSERT((status == napi_ok), "no element", status);
        std::string key;
        JSUtil::GetValue(env, jsKey, key);
        napi_value valueJs = 0;
        napi_get_property(env, jsValue, jsKey, &valueJs);
        GetValue(env, valueJs, valuesBucket.valuesMap[key]);
    }
    return napi_ok;
}

/* napi_value <-> std::vector<DistributedKv::Entry> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<DistributedKv::Entry> &out, bool hasSchema)
{
    out.clear();
    ZLOGD("napi_value -> std::vector<DistributedKv::Entry> ");
    bool isArray = false;
    napi_is_array(env, in, &isArray);
    ASSERT(isArray, "not array", napi_invalid_arg);

    uint32_t length = 0;
    napi_status status = napi_get_array_length(env, in, &length);
    ASSERT((status == napi_ok) && (length > 0), "get_array failed!", status);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value item = nullptr;
        status = napi_get_element(env, in, i, &item);
        ASSERT((status == napi_ok), "no element", status);
        if ((status != napi_ok) || (item == nullptr)) {
            continue;
        }
        DistributedKv::Entry entry;
        status = GetValue(env, item, entry, hasSchema);
        if (status != napi_ok) {
            ZLOGD("maybe valubucket type");
            DataShareValuesBucket values;
            GetValue(env, item, values);
            entry = KvUtils::ToEntry(values);
            entry.key = std::vector<uint8_t>(entry.key.Data().begin(), entry.key.Data().end());
            if (hasSchema) {
                entry.value = std::vector<uint8_t>(entry.value.Data().begin() + 1, entry.value.Data().end());
            }
        }
        out.push_back(entry);
    }
    return napi_ok;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<DistributedKv::Entry>& in, napi_value& out, bool hasSchema)
{
    ZLOGD("napi_value <- std::vector<DistributedKv::Entry> %{public}d", static_cast<int>(in.size()));
    napi_status status = napi_create_array_with_length(env, in.size(), &out);
    ASSERT(status == napi_ok, "create array failed!", status);
    int index = 0;
    for (const auto& item : in) {
        napi_value entry = nullptr;
        SetValue(env, item, entry, hasSchema);
        napi_set_element(env, out, index++, entry);
    }
    return status;
}

/* napi_value <-> std::vector<DistributedKv::StoreId> */
napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<DistributedKv::StoreId>& out)
{
    out.clear();
    ZLOGD("napi_value -> std::vector<DistributedKv::StoreId> ");
    bool isArray = false;
    napi_is_array(env, in, &isArray);
    ASSERT(isArray, "not array", napi_invalid_arg);

    uint32_t length = 0;
    napi_status status = napi_get_array_length(env, in, &length);
    ASSERT((status == napi_ok) && (length > 0), "get_array failed!", status);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value item = nullptr;
        status = napi_get_element(env, in, i, &item);
        ASSERT((status == napi_ok), "no element", status);
        if ((status != napi_ok) || (item == nullptr)) {
            continue;
        }
        std::string value;
        status = GetValue(env, item, value);
        DistributedKv::StoreId storeId { value };
        out.push_back(storeId);
    }
    return status;
}

napi_status JSUtil::SetValue(napi_env env, const std::vector<DistributedKv::StoreId>& in, napi_value& out)
{
    ZLOGD("napi_value <- std::vector<DistributedKv::StoreId>  %{public}d", static_cast<int>(in.size()));
    napi_status status = napi_create_array_with_length(env, in.size(), &out);
    ASSERT((status == napi_ok), "create_array failed!", status);
    int index = 0;
    for (const auto& item : in) {
        napi_value entry = nullptr;
        SetValue(env, item.storeId, entry);
        napi_set_element(env, out, index++, entry);
    }
    return status;
}

/* napi_value <-> DistributedKv::ChangeNotification */
napi_status JSUtil::GetValue(napi_env env, napi_value in, DistributedKv::ChangeNotification& out, bool hasSchema)
{
    ZLOGD("napi_value -> DistributedKv::ChangeNotification ");
    (void)(env);
    (void)(in);
    (void)(out);
    ASSERT(false, "DistributedKv::ChangeNotification from napi_value, unsupported!", napi_invalid_arg);
    return napi_invalid_arg;
}

napi_status JSUtil::SetValue(napi_env env, const DistributedKv::ChangeNotification& in, napi_value& out, bool hasSchema)
{
    ZLOGD("napi_value <- DistributedKv::ChangeNotification ");
    napi_status status = napi_create_object(env, &out);
    ASSERT((status == napi_ok), "napi_create_object for DistributedKv::ChangeNotification failed!", status);
    napi_value deviceId = nullptr;
    status = SetValue(env, in.GetDeviceId(), deviceId);
    ASSERT((status == napi_ok) || (deviceId == nullptr), "GetDeviceId failed!", status);
    status = napi_set_named_property(env, out, "deviceId", deviceId);
    ASSERT((status == napi_ok), "set_named_property deviceId failed!", status);

    napi_value insertEntries = nullptr;
    status = SetValue(env, in.GetInsertEntries(), insertEntries, hasSchema);
    ASSERT((status == napi_ok) || (insertEntries == nullptr), "GetInsertEntries failed!", status);
    status = napi_set_named_property(env, out, "insertEntries", insertEntries);
    ASSERT((status == napi_ok), "set_named_property insertEntries failed!", status);

    napi_value updateEntries = nullptr;
    status = SetValue(env, in.GetUpdateEntries(), updateEntries, hasSchema);
    ASSERT((status == napi_ok) || (updateEntries == nullptr), "GetUpdateEntries failed!", status);
    status = napi_set_named_property(env, out, "updateEntries", updateEntries);
    ASSERT((status == napi_ok), "set_named_property updateEntries failed!", status);

    napi_value deleteEntries = nullptr;
    status = SetValue(env, in.GetDeleteEntries(), deleteEntries, hasSchema);
    ASSERT((status == napi_ok) || (deleteEntries == nullptr), "GetDeleteEntries failed!", status);
    status = napi_set_named_property(env, out, "deleteEntries", deleteEntries);
    ASSERT((status == napi_ok), "set_named_property deleteEntries failed!", status);
    return status;
}

/* napi_value <-> DistributedKv::Options */
napi_status JSUtil::GetValue(napi_env env, napi_value in, DistributedKv::Options& options)
{
    ZLOGD("napi_value -> DistributedKv::Options ");
    napi_status status = napi_invalid_arg;
    GetNamedProperty(env, in, "createIfMissing", options.createIfMissing);
    GetNamedProperty(env, in, "encrypt", options.encrypt);
    GetNamedProperty(env, in, "backup", options.backup);
    GetNamedProperty(env, in, "autoSync", options.autoSync);

    int32_t kvStoreType = 0;
    GetNamedProperty(env, in, "kvStoreType", kvStoreType);
    options.kvStoreType = static_cast<DistributedKv::KvStoreType>(kvStoreType);

    JsSchema *jsSchema = nullptr;
    status = GetNamedProperty(env, in, "schema", jsSchema);
    if (status == napi_ok) {
        options.schema = jsSchema->Dump();
    }

    int32_t level = 0;
    status = GetNamedProperty(env, in, "securityLevel", level);
    if (status != napi_ok) {
        return status;
    }

    return GetLevel(level, options.securityLevel);
}

napi_status JSUtil::GetLevel(int32_t level, int32_t &out)
{
    switch (level) {
        case SecurityLevel::S1:
        case SecurityLevel::S2:
        case SecurityLevel::S3:
        case SecurityLevel::S4:
            out = level;
            return napi_ok;
        default:
            return napi_invalid_arg;
    }
}

napi_status JSUtil::GetValue(napi_env env, napi_value inner, JsSchema*& out)
{
    return JsSchema::ToJson(env, inner, out);
}

napi_status JSUtil::SetValue(napi_env env, const DistributedKv::Options& in, napi_value& out)
{
    (void)(env);
    (void)(in);
    (void)(out);
    ASSERT(false, "DistributedKv::Options to napi_value, unsupported!", napi_invalid_arg);
    return napi_invalid_arg;
}

napi_value JSUtil::DefineClass(napi_env env, const std::string& name,
    const napi_property_descriptor* properties, size_t count, napi_callback newcb)
{
    // base64("data.distributedkvstore") as rootPropName, i.e. global.<root>
    const std::string rootPropName = "ZGF0YS5kaXN0cmlidXRlZGt2c3RvcmU";
    napi_value root = nullptr;
    bool hasRoot = false;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_has_named_property(env, global, rootPropName.c_str(), &hasRoot);
    if (hasRoot) {
        napi_get_named_property(env, global, rootPropName.c_str(), &root);
    } else {
        napi_create_object(env, &root);
        napi_set_named_property(env, global, rootPropName.c_str(), root);
    }

    std::string propName = "constructor_of_" + name;
    napi_value constructor = nullptr;
    bool hasProp = false;
    napi_has_named_property(env, root, propName.c_str(), &hasProp);
    if (hasProp) {
        napi_get_named_property(env, root, propName.c_str(), &constructor);
        if (constructor != nullptr) {
            ZLOGD("got data.distributeddata.%{public}s as constructor", propName.c_str());
            return constructor;
        }
        hasProp = false; // no constructor.
    }

    NAPI_CALL(env, napi_define_class(env, name.c_str(), name.size(), newcb, nullptr, count, properties, &constructor));
    NAPI_ASSERT(env, constructor != nullptr, "napi_define_class failed!");

    if (!hasProp) {
        napi_set_named_property(env, root, propName.c_str(), constructor);
        ZLOGD("save constructor to data.distributeddata.%{public}s", propName.c_str());
    }
    return constructor;
}

napi_ref JSUtil::NewWithRef(napi_env env, size_t argc, napi_value* argv, void** out, napi_value constructor)
{
    napi_value object = nullptr;
    napi_status status = napi_new_instance(env, constructor, argc, argv, &object);
    ASSERT(status == napi_ok, "napi_new_instance failed", nullptr);
    ASSERT(object != nullptr, "napi_new_instance failed", nullptr);

    status = napi_unwrap(env, object, out);
    ASSERT(status == napi_ok, "napi_unwrap failed", nullptr);
    ASSERT(out != nullptr, "napi_unwrap failed", nullptr);

    napi_ref ref = nullptr;
    status = napi_create_reference(env, object, 1, &ref);
    ASSERT(status == napi_ok, "napi_create_referenc!e failed", nullptr);
    ASSERT(ref != nullptr, "napi_create_referenc!e failed", nullptr);
    return ref;
}

napi_status JSUtil::Unwrap(napi_env env, napi_value in, void** out, napi_value constructor)
{
    if (constructor != nullptr) {
        bool isInstance = false;
        napi_instanceof(env, in, constructor, &isInstance);
        if (!isInstance) {
            ZLOGE("not a instance of *");
            return napi_invalid_arg;
        }
    }
    return napi_unwrap(env, in, out);
}

bool JSUtil::Equals(napi_env env, napi_value value, napi_ref copy)
{
    if (copy == nullptr) {
        return (value == nullptr);
    }

    napi_value copyValue = nullptr;
    napi_get_reference_value(env, copy, &copyValue);

    bool isEquals = false;
    napi_strict_equals(env, value, copyValue, &isEquals);
    return isEquals;
}

napi_status JSUtil::GetValue(napi_env env, napi_value in, std::vector<Blob> &out)
{
    ZLOGD("napi_value -> std::GetValue Blob");
    out.clear();
    napi_valuetype type = napi_undefined;
    napi_status nstatus = napi_typeof(env, in, &type);
    ASSERT((nstatus == napi_ok) && (type == napi_object), "invalid type", napi_invalid_arg);
    PredicatesProxy *predicates = nullptr;
    napi_unwrap(env, in, reinterpret_cast<void **>(&predicates));
    ASSERT((predicates != nullptr), "invalid type", napi_invalid_arg);
    std::vector<Key> keys;
    nstatus = napi_invalid_arg;
    Status status = KvUtils::GetKeys(*(predicates->predicates_), keys);
    if (status == Status::SUCCESS) {
        ZLOGD("napi_value —> GetValue Blob ok");
        out = keys;
        nstatus = napi_ok;
    }
    return nstatus;
}

napi_status JSUtil::GetValue(napi_env env, napi_value in, DataQuery &query)
{
    ZLOGD("napi_value -> std::GetValue DataQuery");
    napi_valuetype type = napi_undefined;
    napi_status nstatus = napi_typeof(env, in, &type);
    ASSERT((nstatus == napi_ok) && (type == napi_object), "invalid type", napi_invalid_arg);
    PredicatesProxy *predicates = nullptr;
    napi_unwrap(env, in, reinterpret_cast<void **>(&predicates));
    ASSERT((predicates != nullptr), "invalid type", napi_invalid_arg);
    Status status = KvUtils::ToQuery(*(predicates->predicates_), query);
    if (status != Status::SUCCESS) {
        ZLOGD("napi_value -> GetValue DataQuery failed ");
    }
    return nstatus;
}

napi_status JSUtil::GetCurrentAbilityParam(napi_env env, ContextParam &param)
{
    auto ability = AbilityRuntime::GetCurrentAbility(env);
    if (ability == nullptr) {
        ZLOGE("GetCurrentAbility -> ability pointer is nullptr");
        return napi_invalid_arg;
    }

    auto context = ability->GetAbilityContext();
    if (context == nullptr) {
        ZLOGE("Get fa context  -> fa context pointer is nullptr");
        return napi_invalid_arg;
    }
    param.area = context->GetArea();
    param.baseDir = context->GetDatabaseDir();
    auto hapInfo = context->GetHapModuleInfo();
    if (hapInfo != nullptr) {
        param.hapName = hapInfo->moduleName;
    }
    ZLOGI("area:%{public}d hapName:%{public}s baseDir:%{public}s", param.area, param.hapName.c_str(),
        param.baseDir.c_str());
    return napi_ok;
}

napi_status JSUtil::GetValue(napi_env env, napi_value in, ContextParam &param)
{
    if (in == nullptr) {
        ZLOGD("hasProp is false -> fa stage");
        return GetCurrentAbilityParam(env, param);
    }

    bool isStageMode = false;
    napi_status status = GetNamedProperty(env, in, "stageMode", isStageMode);
    ASSERT(status == napi_ok, "get stageMode param failed", napi_invalid_arg);
    if (!isStageMode) {
        ZLOGD("isStageMode is false -> fa stage");
        return GetCurrentAbilityParam(env, param);
    }

    ZLOGD("stage mode branch");
    status = GetNamedProperty(env, in, "databaseDir", param.baseDir);
    ASSERT(status == napi_ok, "get databaseDir param failed", napi_invalid_arg);
    status = GetNamedProperty(env, in, "area", param.area);
    ASSERT(status == napi_ok, "get area param failed", napi_invalid_arg);
    napi_value hapInfo = nullptr;
    GetNamedProperty(env, in, "currentHapModuleInfo", hapInfo);
    if (hapInfo != nullptr) {
        status = GetNamedProperty(env, hapInfo, "moduleName", param.hapName);
        ASSERT(status == napi_ok, "get hap name failed", napi_invalid_arg);
    }
    return napi_ok;
}
} // namespace OHOS::DistributedKVStore
