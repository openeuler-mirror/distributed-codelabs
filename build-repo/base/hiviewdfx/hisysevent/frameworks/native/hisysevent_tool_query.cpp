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

#include "hisysevent_tool_query.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

#include "hisysevent_json_decorator.h"

namespace OHOS {
namespace HiviewDFX {
void HiSysEventToolQuery::OnQuery(std::shared_ptr<std::vector<HiSysEventRecord>> sysEvents)
{
    if (sysEvents == nullptr) {
        return;
    }
    for_each((*sysEvents).cbegin(), (*sysEvents).cend(), [this] (const HiSysEventRecord& sysEventRecord) {
        if (this->checkValidEvent && this->eventJsonDecorator != nullptr) {
            std::cout << this->eventJsonDecorator->DecorateEventJsonStr(sysEventRecord) << std::endl;
            return;
        }
        std::cout << sysEventRecord.AsJson() << std::endl;
    });
}

void HiSysEventToolQuery::OnComplete(int32_t reason, int32_t total)
{
    if (!autoExit) {
        return;
    }
    exit(0);
}
} // namespace HiviewDFX
} // namespace OHOS
