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

#include <iostream>
#include <memory>

#include "gtest/gtest.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "multiuser/os_account_observer.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using Want = OHOS::AAFwk::Want;
constexpr int32_t USER_ID = 101;
std::shared_ptr<OsAccountObserver> g_subScriber = nullptr;

class OsAccountObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void OsAccountObserverTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    if (g_subScriber == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        g_subScriber = std::make_shared<OsAccountObserver>(subscribeInfo);
        if (g_subScriber == nullptr) {
            LOGE("subscriber is nullptr");
            return;
        }

        bool subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(g_subScriber);
        if (!subRet) {
            LOGE("Subscribe common event failed");
        }
    }
}

void OsAccountObserverTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
    bool subRet = EventFwk::CommonEventManager::UnSubscribeCommonEvent(g_subScriber);
    if (!subRet) {
        LOGE("UnSubscribe common event failed");
    }
    g_subScriber = nullptr;
}

void OsAccountObserverTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void OsAccountObserverTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: OsAccountObserverTest_OnReceiveEvent_0100
 * @tc.desc: Verify the OnReceiveEvent function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(OsAccountObserverTest, OsAccountObserverTest_OnReceiveEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OsAccountObserverTest_OnReceiveEvent_0100 start";
    bool res = true;
    try {
        if (g_subScriber != nullptr) {
            Want want;
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
            int32_t code = USER_ID;
            std::string data(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
            EventFwk::CommonEventData eventData(want, code, data);
            g_subScriber->OnReceiveEvent(eventData);
        }
    } catch (const std::exception &e) {
        res = false;
        LOGE("OsAccountObserverTest_OnReceiveEvent_0100 : %{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "OsAccountObserverTest_OnReceiveEvent_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS