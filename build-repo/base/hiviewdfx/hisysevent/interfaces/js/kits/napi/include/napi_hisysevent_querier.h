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

#ifndef NAPI_HISYSEVENT_QUERIER_H
#define NAPI_HISYSEVENT_QUERIER_H

#include <functional>
#include <string>
#include <vector>
#include <sys/syscall.h>
#include <unistd.h>

#include "hisysevent_base_query_callback.h"
#include "js_callback_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_callback_context.h"

namespace OHOS {
namespace HiviewDFX {
using ON_COMPLETE_FUNC = std::function<void(const napi_env, const napi_ref)>;
class NapiHiSysEventQuerier : public HiSysEventBaseQueryCallback {
public:
    NapiHiSysEventQuerier(CallbackContext* context, ON_COMPLETE_FUNC handler);
    virtual ~NapiHiSysEventQuerier();

public:
    virtual void OnQuery(const ::std::vector<std::string>& sysEvents,
        const std::vector<int64_t>& seq) override;
    virtual void OnComplete(int32_t reason, int32_t total, int64_t seq) override;

private:
    CallbackContext* callbackContext;
    ON_COMPLETE_FUNC onCompleteHandler;
    std::shared_ptr<JsCallbackManager> jsCallbackManager;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // NAPI_HISYSEVENT_QUERIER_H
