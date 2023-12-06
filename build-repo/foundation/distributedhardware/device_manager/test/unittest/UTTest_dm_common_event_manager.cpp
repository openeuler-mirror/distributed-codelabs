/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thread>
#include "dm_common_event_manager.h"
#include "dm_constants.h"
#include "hichain_connector.h"
#include "UTTest_dm_common_event_manager.h"
#include "matching_skills.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
void DmCommonEventManagerTest::SetUp()
{
}

void DmCommonEventManagerTest::TearDown()
{
}

void DmCommonEventManagerTest::SetUpTestCase()
{
}

void DmCommonEventManagerTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: DmCommonEventManager::~DmCommonEventManager_001
 * @tc.desc: Destructor of DmCommonEventManager
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCommonEventManagerTest, DeleteDmCommonEventManager_001, testing::ext::TestSize.Level0)
{
    auto commonEventManager = std::make_shared<DmCommonEventManager>();
}

/**
 * @tc.name: DmCommonEventManager::SubscribeServiceEvent_001
 * @tc.desc: call SubscribeServiceEvent(), pass parameters： strEvent and callbackNode.callback_
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCommonEventManagerTest, SubscribeServiceEvent_001, testing::ext::TestSize.Level0)
{
    std::string strEvent;
    CommomEventCallback callback = nullptr;
    auto commonEventManager = std::make_shared<DmCommonEventManager>();
    bool result = commonEventManager->SubscribeServiceEvent(strEvent, callback);
    EXPECT_EQ(false, result);
}

/**
 * @tc.name: DmCommonEventManager::SubscribeServiceEvent_002
 * @tc.desc: call SubscribeServiceEvent(), pass parameters： strEvent and callbackNode.callback_
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCommonEventManagerTest, SubscribeServiceEvent_002, testing::ext::TestSize.Level0)
{
    std::string strEvent = "test";
    CommomEventCallback callback = nullptr;
    auto commonEventManager = std::make_shared<DmCommonEventManager>();
    bool result = commonEventManager->SubscribeServiceEvent(strEvent, callback);
    EXPECT_EQ(false, result);
}

/**
 * @tc.name: DmCommonEventManager::UnsubscribeServiceEvent_001
 * @tc.desc: call UnsubscribeServiceEvent(), pass parameters： strEvent
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCommonEventManagerTest, UnsubscribeServiceEvent_001, testing::ext::TestSize.Level0)
{
    auto commonEventManager = std::make_shared<DmCommonEventManager>();
    bool result = commonEventManager->UnsubscribeServiceEvent();
    EXPECT_EQ(false, result);
}

/**
 * @tc.name: DmCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility_001
 * @tc.desc: call OnAddSystemAbility()
 * @tc.type: FUNC
 */
HWTEST_F(DmCommonEventManagerTest, OnAddSystemAbility_001, testing::ext::TestSize.Level0)
{
    int32_t systemAbilityId = COMMON_EVENT_SERVICE_ID;
    std::string deviceId;
    auto commonEventManager = std::make_shared<DmCommonEventManager>();
    auto systemAbilityStatusChangeListener =
        std::make_shared<DmCommonEventManager::SystemAbilityStatusChangeListener>(commonEventManager->subscriber_);
    systemAbilityStatusChangeListener->OnAddSystemAbility(systemAbilityId, deviceId);
}

/**
 * @tc.name: DmCommonEventManager::SystemAbilityStatusChangeListener::OnAddSystemAbility_001
 * @tc.desc: call OnAddSystemAbility()
 * @tc.type: FUNC
 */
HWTEST_F(DmCommonEventManagerTest, OnAddSystemAbility_002, testing::ext::TestSize.Level0)
{
    int32_t systemAbilityId = 0;
    std::string deviceId;
    auto commonEventManager = std::make_shared<DmCommonEventManager>();
    auto systemAbilityStatusChangeListener =
        std::make_shared<DmCommonEventManager::SystemAbilityStatusChangeListener>(commonEventManager->subscriber_);
    systemAbilityStatusChangeListener->OnAddSystemAbility(systemAbilityId, deviceId);
}

/**
 * @tc.name: DmCommonEventManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility_001
 * @tc.desc: call OnRemoveSystemAbility()
 * @tc.type: FUNC
 */
HWTEST_F(DmCommonEventManagerTest, OnRemoveSystemAbility_002, testing::ext::TestSize.Level0)
{
    int32_t systemAbilityId = COMMON_EVENT_SERVICE_ID;
    std::string deviceId;
    auto commonEventManager = std::make_shared<DmCommonEventManager>();
    auto systemAbilityStatusChangeListener =
        std::make_shared<DmCommonEventManager::SystemAbilityStatusChangeListener>(commonEventManager->subscriber_);
    systemAbilityStatusChangeListener->OnRemoveSystemAbility(systemAbilityId, deviceId);
}

/**
 * @tc.name: DmCommonEventManager::SystemAbilityStatusChangeListener::OnReceiveEvent_001
 * @tc.desc: call OnReceiveEvent()
 * @tc.type: FUNC
 */
HWTEST_F(DmCommonEventManagerTest, OnReceiveEvent_001, testing::ext::TestSize.Level0)
{
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction("changeEvent");
    data.SetWant(want);
    data.SetCode(0);

    std::string strEvent = "test";
    CommomEventCallback callback = nullptr;
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(strEvent);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    auto commonEventManager = std::make_shared<DmCommonEventManager>();
    commonEventManager->subscriber_ = std::make_shared<DmEventSubscriber>(subscriberInfo, callback, strEvent);
    commonEventManager->subscriber_->OnReceiveEvent(data);
}

/**
 * @tc.name: DmCommonEventManager::SystemAbilityStatusChangeListener::OnReceiveEvent_002
 * @tc.desc: call OnReceiveEvent()
 * @tc.type: FUNC
 */
HWTEST_F(DmCommonEventManagerTest, OnReceiveEvent_002, testing::ext::TestSize.Level0)
{
    AAFwk::Want want;
    EventFwk::CommonEventData data;
    want.SetAction("test");
    data.SetWant(want);
    data.SetCode(0);
    CommomEventCallback callback = nullptr;
    EventFwk::MatchingSkills matchingSkills;
    std::string strEvent = "test";
    matchingSkills.AddEvent(strEvent);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    auto commonEventManager = std::make_shared<DmCommonEventManager>();
    commonEventManager->subscriber_ = std::make_shared<DmEventSubscriber>(subscriberInfo, callback, strEvent);
    commonEventManager->subscriber_->OnReceiveEvent(data);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
