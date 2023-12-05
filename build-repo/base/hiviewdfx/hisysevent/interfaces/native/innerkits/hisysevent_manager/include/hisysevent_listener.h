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

#ifndef HISYSEVENT_LISTENER_H
#define HISYSEVENT_LISTENER_H

#include <string>

#include "hisysevent_record.h"

namespace OHOS {
namespace HiviewDFX {
class HiSysEventListener {
public:
    HiSysEventListener() {}
    virtual ~HiSysEventListener() {}

public:
   virtual void OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent) = 0;
   virtual void OnServiceDied() = 0;

private:
    HiSysEventListener(const HiSysEventListener&) = delete;
    HiSysEventListener& operator=(const HiSysEventListener&) = delete;
    HiSysEventListener(const HiSysEventListener&&) = delete;
    HiSysEventListener& operator=(const HiSysEventListener&&) = delete;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HISYSEVENT_LISTENER_H
