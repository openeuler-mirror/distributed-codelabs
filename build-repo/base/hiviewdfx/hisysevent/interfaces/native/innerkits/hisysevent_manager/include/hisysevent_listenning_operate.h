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

#ifndef HISYSEVENT_LISTENNING_OPERATE_H
#define HISYSEVENT_LISTENNING_OPERATE_H

#include <memory>
#include <string>
#include <vector>

#include "hisysevent_base_manager.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventListenningOperate {
public:
    HiSysEventListenningOperate() = default;
    virtual ~HiSysEventListenningOperate() {}

public:
    virtual int32_t AddListener(const std::shared_ptr<HiSysEventBaseListener> listener,
        const std::vector<ListenerRule>& rules);
    virtual int32_t RemoveListener(const std::shared_ptr<HiSysEventBaseListener> listener);
    virtual int32_t SetDebugMode(const std::shared_ptr<HiSysEventBaseListener> listener, const bool mode);

private:
    HiSysEventListenningOperate(const HiSysEventListenningOperate&) = delete;
    HiSysEventListenningOperate& operator=(const HiSysEventListenningOperate&) = delete;
    HiSysEventListenningOperate(const HiSysEventListenningOperate&&) = delete;
    HiSysEventListenningOperate& operator=(const HiSysEventListenningOperate&&) = delete;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_LISTENNING_OPERATE_H
