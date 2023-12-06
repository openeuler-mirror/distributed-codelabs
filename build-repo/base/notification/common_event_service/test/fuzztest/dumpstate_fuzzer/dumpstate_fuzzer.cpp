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

#include "dumpstate_fuzzer.h"
#include "securec.h"
#include "common_event.h"

namespace OHOS {
namespace EventFwk {
class TestSubscriber : public CommonEventSubscriber {
public:
    explicit TestSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~TestSubscriber()
    {}

    void OnReceiveEvent(const CommonEventData &data) override
    {}
};
}  // namespace EventFwk
namespace {
    constexpr size_t U32_AT_SIZE = 4;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint8_t dumpType = *data;
    int32_t code = U32_AT(reinterpret_cast<const uint8_t*>(data));
    std::string stringData(data);
    std::vector<std::string> state;
    state.emplace_back(stringData);
    EventFwk::CommonEvent commonEvent;
    // test PublishCommonEvent function
    AAFwk::Want want;
    EventFwk::CommonEventData commonEventData(want);
    commonEventData.SetCode(code);
    commonEventData.SetData(stringData);
    // make commonEventPublishInfo info
    EventFwk::CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> permissions;
    permissions.emplace_back(stringData);
    commonEventPublishInfo.SetSubscriberPermissions(permissions);
    commonEventPublishInfo.IsSticky();
    commonEventPublishInfo.GetSubscriberPermissions();
    commonEventPublishInfo.SetBundleName(stringData);
    commonEventPublishInfo.GetBundleName();
    // make CommonEventSubscriber info
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(stringData);

    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetDeviceId(stringData);
    std::shared_ptr<EventFwk::TestSubscriber> subscriber =
        std::make_shared<EventFwk::TestSubscriber>(subscribeInfo);
    if (subscriber != nullptr) {
        subscriber->SetCode(code);
        subscriber->GetCode();
        subscriber->SetData(stringData);
        subscriber->GetData();
        subscriber->SetCodeAndData(code, stringData);
        subscriber->AbortCommonEvent();
        subscriber->ClearAbortCommonEvent();
        subscriber->GetAbortCommonEvent();
        subscriber->GoAsyncCommonEvent();
    }
    commonEvent.PublishCommonEvent(commonEventData, commonEventPublishInfo, subscriber);
    // test PublishCommonEvent and four paramter
    commonEvent.PublishCommonEvent(commonEventData, commonEventPublishInfo, subscriber, code, code);
    // test DumpState function
    return commonEvent.DumpState(dumpType, stringData, code, state);
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
