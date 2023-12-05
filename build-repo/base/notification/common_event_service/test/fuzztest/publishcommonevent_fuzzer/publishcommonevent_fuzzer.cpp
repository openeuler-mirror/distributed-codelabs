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

#include "publishcommonevent_fuzzer.h"
#include "securec.h"
#include "common_event_manager.h"
#include "common_event_support.h"

constexpr int8_t FUZZ_DATA_LEN = 3;

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_TEST_ACTION1);
    EventFwk::CommonEventData commonEventData;
    commonEventData.SetWant(want);
    int32_t code = U32_AT(reinterpret_cast<const uint8_t*>(data));
    commonEventData.SetCode(code);
    std::string stringData(data);
    commonEventData.SetData(stringData);

    EventFwk::CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> permissions;
    permissions.emplace_back(stringData);
    commonEventPublishInfo.SetSubscriberPermissions(permissions);

    std::shared_ptr<EventFwk::CommonEventSubscriber> subscriber = nullptr;

    // test pubilshCommonEvent function and two paramter
    EventFwk::CommonEventManager::PublishCommonEvent(commonEventData, commonEventPublishInfo);

    if (size < FUZZ_DATA_LEN) {
        return EventFwk::CommonEventManager::PublishCommonEvent(
            commonEventData, commonEventPublishInfo, subscriber);
    } else {
        int32_t uid = U32_AT(reinterpret_cast<const uint8_t*>(data));
        return EventFwk::CommonEventManager::PublishCommonEvent(
            commonEventData, commonEventPublishInfo, subscriber, uid, code);
    }
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char *)malloc(size + 1);
    if (ch == nullptr) {
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
