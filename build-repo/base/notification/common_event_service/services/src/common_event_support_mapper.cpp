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

#include "common_event_support_mapper.h"
#include "common_event_support.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
CommonEventSupportMapper::CommonEventSupportMapper()
{
    Init();
}

void CommonEventSupportMapper::Init()
{
    EVENT_LOGI("enter");

    map_.insert(std::make_pair(
        CommonEventSupport::COMMON_EVENT_TEST_ACTION1, CommonEventSupport::COMMON_EVENT_TEST_ACTION2));
    map_.insert(std::make_pair(
        CommonEventSupport::COMMON_EVENT_TEST_ACTION2, CommonEventSupport::COMMON_EVENT_TEST_ACTION1));
}

bool CommonEventSupportMapper::GetMappedSupport(const std::string &support, std::string &mappedSupport)
{
    EVENT_LOGI("enter");

    auto iter = map_.begin();
    for (; iter != map_.end(); ++iter) {
        if (support == iter->first) {
            mappedSupport = iter->second;
            break;
        }
    }

    if (iter == map_.end()) {
        return false;
    }

    return true;
}
}  // namespace EventFwk
}  // namespace OHOS