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
#ifndef OHOS_JS_UTIL_H
#define OHOS_JS_UTIL_H
#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::ObjectStore {
class JSUtil final {
public:
    /* napi_value <-> bool */
    static napi_status GetValue(napi_env env, napi_value in, bool &out);
    static napi_status SetValue(napi_env env, const bool &in, napi_value &out);

    /* napi_value <-> double */
    static napi_status GetValue(napi_env env, napi_value in, double &out);
    static napi_status SetValue(napi_env env, const double &in, napi_value &out);

    /* napi_value <-> std::string */
    static napi_status GetValue(napi_env env, napi_value in, std::string &out);
    static napi_status SetValue(napi_env env, const std::string &in, napi_value &out);

    /* napi_value <-> std::vector<std::string> */
    static napi_status GetValue(napi_env env, napi_value in, std::vector<std::string> &out);
    static napi_status SetValue(napi_env env, const std::vector<std::string> &in, napi_value &out);

    /* napi_value <-> std::vector<uint8_t> */
    static napi_status GetValue(napi_env env, napi_value in, std::vector<uint8_t> &out);
    static napi_status SetValue(napi_env env, const std::vector<uint8_t> &in, napi_value &out);

    static void GenerateNapiError(napi_env env, int32_t status, int32_t &errCode, std::string &errMessage);
};

#define NAPI_ASSERT_ERRCODE(env, assertion, version, err)                                                       \
    do {                                                                                                        \
        if (!(assertion)) {                                                                                     \
            if ((version) >= 9) {                                                                               \
                napi_throw_error((env), std::to_string((err)->GetCode()).c_str(), (err)->GetMessage().c_str()); \
            }                                                                                                   \
            return nullptr;                                                                                     \
        }                                                                                                       \
    } while (0)

#define CHECH_STATUS_ERRCODE(env, condition, err)                                                           \
    do {                                                                                                    \
        if (!(condition)) {                                                                                 \
            napi_throw_error((env), std::to_string((err)->GetCode()).c_str(), (err)->GetMessage().c_str()); \
            return nullptr;                                                                                 \
        }                                                                                                   \
    } while (0)

#define CHECH_STATUS_RETURN_VOID(env, condition, ctxt, info) \
    do {                                                     \
        if (!(condition)) {                                  \
            LOG_ERROR(info);                                 \
            (ctxt)->status = napi_generic_failure;           \
            (ctxt)->message = std::string(info);             \
            return;                                          \
        }                                                    \
    } while (0)

#define LOG_ERROR_RETURN(condition, message, retVal)             \
    do {                                                         \
        if (!(condition)) {                                      \
            LOG_ERROR("test (" #condition ") failed: " message); \
            return retVal;                                       \
        }                                                        \
    } while (0)

#define LOG_ERROR_RETURN_VOID(condition, message)                \
    do {                                                         \
        if (!(condition)) {                                      \
            LOG_ERROR("test (" #condition ") failed: " message); \
            return;                                              \
        }                                                        \
    } while (0)
} // namespace OHOS::ObjectStore
#endif // OHOS_JS_UTIL_H
