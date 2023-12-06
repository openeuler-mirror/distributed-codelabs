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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_RECORD_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_RECORD_H

#include "accesstoken_kit.h"
#include "common_event_constant.h"
#include "common_event_data.h"
#include "common_event_publish_info.h"

namespace OHOS {
namespace EventFwk {
struct EventRecordInfo {
    pid_t pid;
    uid_t uid;
    Security::AccessToken::AccessTokenID callerToken;
    std::string bundleName;
    bool isSubsystem;
    bool isSystemApp;
    bool isProxy;

    EventRecordInfo() : pid(0), uid(0), callerToken(0), isSubsystem(false), isSystemApp(false), isProxy(false) {}
};

struct CommonEventRecord {
    std::shared_ptr<CommonEventData> commonEventData;
    std::shared_ptr<CommonEventPublishInfo> publishInfo;
    struct tm recordTime {};
    EventRecordInfo eventRecordInfo;
    int32_t userId;
    bool isSystemEvent;

    CommonEventRecord()
        : commonEventData(nullptr),
          publishInfo(nullptr),
          userId(UNDEFINED_USER),
          isSystemEvent(false)
    {}
};
}  // namespace EventFwk
}  // namespace OHOS
#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_RECORD_H