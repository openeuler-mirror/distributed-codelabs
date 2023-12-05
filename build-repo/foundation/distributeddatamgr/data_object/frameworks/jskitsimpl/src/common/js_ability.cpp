/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "js_ability.h"

#include "extension_context.h"
#include "logger.h"

namespace OHOS {
namespace ObjectStore {
Context::Context(std::shared_ptr<AbilityRuntime::Context> stageContext)
{
    bundleName_ = stageContext->GetBundleName();
    LOG_DEBUG("Stage: bundle:%{public}s", bundleName_.c_str());
}

Context::Context(std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext)
{
    bundleName_ = abilityContext->GetBundleName();
    LOG_DEBUG("FA:  bundle:%{public}s", bundleName_.c_str());
}

std::string Context::GetBundleName()
{
    return bundleName_;
}

std::shared_ptr<Context> JSAbility::GetContext(napi_env env, napi_value value)
{
    bool mode = false;
    AbilityRuntime::IsStageContext(env, value, mode);
    if (mode) {
        LOG_DEBUG("Get context as stage mode.");
        auto stageContext = AbilityRuntime::GetStageModeContext(env, value);
        if (stageContext == nullptr) {
            LOG_ERROR("GetStageModeContext failed.");
            return nullptr;
        }
        return std::make_shared<Context>(stageContext);
    }

    LOG_DEBUG("Get context as feature ability mode.");
    auto ability = AbilityRuntime::GetCurrentAbility(env);
    if (ability == nullptr) {
        LOG_ERROR("GetCurrentAbility failed.");
        return nullptr;
    }
    auto abilityContext = ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        LOG_ERROR("GetAbilityContext failed.");
        return nullptr;
    }
    return std::make_shared<Context>(abilityContext);
}
} // namespace ObjectStore
} // namespace OHOS