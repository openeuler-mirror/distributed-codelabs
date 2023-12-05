/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CONFIG_POLICY_NAPI_H
#define CONFIG_POLICY_NAPI_H

#include <string>

#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi/native_api.h"

namespace OHOS {
namespace Customization {
namespace ConfigPolicy {
struct ConfigAsyncContext {
    napi_async_work work_;
    napi_deferred deferred_;
    napi_ref callbackRef_;
    typedef napi_value (*CreateNapiValue)(napi_env env, ConfigAsyncContext &context);
    CreateNapiValue createValueFunc_;

    std::string relPath_;
    std::string pathValue_;
    std::vector<std::string> paths_;
};

class ConfigPolicyNapi {
public:
    ConfigPolicyNapi();
    ~ConfigPolicyNapi() = default;
    static napi_value Init(napi_env env, napi_value exports);

private:
    static napi_value NAPIGetOneCfgFile(napi_env env, napi_callback_info info);
    static napi_value NAPIGetCfgFiles(napi_env env, napi_callback_info info);
    static napi_value NAPIGetCfgDirList(napi_env env, napi_callback_info info);
    static napi_value CreateUndefined(napi_env env);
    static std::string GetStringFromNAPI(napi_env env, napi_value value);
    static napi_value HandleAsyncWork(napi_env env, ConfigAsyncContext *context, std::string workName,
        napi_async_execute_callback execute, napi_async_complete_callback complete);
    static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType);
    static void NativeGetOneCfgFile(napi_env env, void *data);
    static void NativeGetCfgFiles(napi_env env, void *data);
    static void NativeGetCfgDirList(napi_env env, void *data);
    static void NativeCallbackComplete(napi_env env, napi_status status, void *data);
    static napi_value ParseRelPath(napi_env env, std::string &param, napi_value args);
    static void CreateArraysValueFunc(ConfigAsyncContext &context);
    static napi_value ThrowNapiError(napi_env env, int32_t errCode, const std::string &errMessage);
};
} // namespace ConfigPolicy
} // namespace Customization
} // namespace OHOS
#endif
