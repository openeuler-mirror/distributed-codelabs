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

#include <gtest/gtest.h>

#include "common_event_support.h"
#include "want.h"
#define private public
#define protected public
#include "static_subscriber_manager.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;

extern void MockVerifyAccessToken(bool mockRet, int8_t times = 0);
extern void MockGetHapTokenID(Security::AccessToken::AccessTokenID mockRet);
extern void MockQueryExtensionInfos(bool mockRet, uint8_t mockCase);
extern void MockGetResConfigFile(bool mockRet, uint8_t mockCase);
extern void MockQueryActiveOsAccountIds(bool mockRet, uint8_t mockCase);
extern void MockGetOsAccountLocalIdFromUid(bool mockRet, uint8_t mockCase = 0);
extern bool IsConnectAbilityCalled();
extern void ResetAbilityManagerHelperState();
extern void ResetAccountMock();
extern void ResetBundleManagerHelperMock();
extern void ResetAccessTokenHelperMock();

namespace {
    constexpr uint8_t TEST_ALLOWLIST_SIZE = 3;
    constexpr uint8_t TEST_MUL_SIZE = 2;
    constexpr uint8_t MOCK_CASE_2 = 2;
    constexpr uint8_t MOCK_CASE_3 = 3;
    constexpr uint8_t MOCK_CASE_4 = 4;
    constexpr uint8_t MOCK_CASE_5 = 5;
}

class StaticSubscriberManagerUnitTest : public testing::Test {
public:
    StaticSubscriberManagerUnitTest() {}

    virtual ~StaticSubscriberManagerUnitTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void StaticSubscriberManagerUnitTest::SetUpTestCase() {}

void StaticSubscriberManagerUnitTest::TearDownTestCase() {}

void StaticSubscriberManagerUnitTest::SetUp() {}

void StaticSubscriberManagerUnitTest::TearDown()
{
    ResetBundleManagerHelperMock();
    ResetAccountMock();
    ResetAccessTokenHelperMock();
}

/*
 * @tc.name: InitAllowListTest_0100
 * @tc.desc: test if StaticSubscriberManager's InitAllowList function executed as expected in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitAllowListTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitAllowListTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // Init
    bool initResult = manager->InitAllowList();
    // expect init success
    EXPECT_TRUE(initResult);
    // expect allow list has apps
    EXPECT_EQ(TEST_ALLOWLIST_SIZE, manager->subscriberList_.size());
    // expect hasInit true
    EXPECT_TRUE(manager->hasInitAllowList_);
}

/*
 * @tc.name: ParseEventsTest_0100
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           valid params when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is not empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect map content is correct
    string expectEventName = "usual.event.TIME_TICK";
    auto it = manager->validSubscribers_.find(expectEventName);
    ASSERT_NE(manager->validSubscribers_.end(), it);
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> infos = it->second;
    ASSERT_EQ(1, infos.size());
    auto info = infos[0];
    EXPECT_EQ(testExtensionName, info.name);
    EXPECT_EQ(testExtensionBundleName, info.bundleName);
    EXPECT_EQ(testExtensionUserId, info.userId);
}

/*
 * @tc.name: ParseEventsTest_0200
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid empty profile when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile ="";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is not empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_0300
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid non-object profile when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile = "test";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_0400
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid empty 'commonEvents' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_0500
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid non-object 'commonEvents' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":true"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_0600
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid empty 'name' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0600, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK\"],"
        "            \"name\":\"\","
        "            \"permission\":\"\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_0700
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid non-string 'name' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0700, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK\"],"
        "            \"name\":true,"
        "            \"permission\":\"\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_0800
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid non-string 'name' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0800, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK\"],"
        "            \"name\":\"fakeName\","
        "            \"permission\":\"\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_0900
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid json string when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_0900, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_1000
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid nonm-string 'permission' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1000, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":true"
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_1100
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid null type 'events' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":null,"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_1200
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid non-array 'events' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":123,"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":true"
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_1300
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid empty 'events' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":true"
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_1400
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid null 'event' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[null],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":true"
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_1500
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           invalid non-string 'event' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[123],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":true"
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_1600
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected
 *            when profile has two elements.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1600, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK0\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"\""
        "        },"
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK1\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"permission1\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is not empty
    EXPECT_EQ(TEST_MUL_SIZE, manager->validSubscribers_.size());
    // expect map content is correct
    string expectEventName0 = "usual.event.TIME_TICK0";
    auto it0 = manager->validSubscribers_.find(expectEventName0);
    ASSERT_NE(manager->validSubscribers_.end(), it0);
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> infos0 = it0->second;
    ASSERT_EQ(1, infos0.size());
    auto info0 = infos0[0];
    EXPECT_EQ(testExtensionName, info0.name);
    EXPECT_EQ(testExtensionBundleName, info0.bundleName);
    EXPECT_EQ(testExtensionUserId, info0.userId);
    EXPECT_EQ("", info0.permission);
    // expect map content is correct
    string expectEventName1 = "usual.event.TIME_TICK1";
    auto it1 = manager->validSubscribers_.find(expectEventName1);
    ASSERT_NE(manager->validSubscribers_.end(), it1);
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> infos1 = it1->second;
    ASSERT_EQ(1, infos1.size());
    auto info1 = infos1[0];
    EXPECT_EQ(testExtensionName, info1.name);
    EXPECT_EQ(testExtensionBundleName, info1.bundleName);
    EXPECT_EQ(testExtensionUserId, info1.userId);
    string expectPermission = "permission1";
    EXPECT_EQ(expectPermission, info1.permission);
}

/*
 * @tc.name: ParseEventsTest_1700
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           one invalid element when profile has two elements.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1700, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":true,"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"permission1\""
        "        },"
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK1\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"permission1\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is not empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect map content is correct
    string expectEventName0 = "usual.event.TIME_TICK0";
    auto it0 = manager->validSubscribers_.find(expectEventName0);
    EXPECT_EQ(manager->validSubscribers_.end(), it0);
    // expect map content is correct
    string expectEventName1 = "usual.event.TIME_TICK1";
    auto it1 = manager->validSubscribers_.find(expectEventName1);
    ASSERT_NE(manager->validSubscribers_.end(), it1);
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> infos = it1->second;
    ASSERT_EQ(1, infos.size());
    auto info = infos[0];
    EXPECT_EQ(testExtensionName, info.name);
    EXPECT_EQ(testExtensionBundleName, info.bundleName);
    EXPECT_EQ(testExtensionUserId, info.userId);
    string expectPermission = "permission1";
    EXPECT_EQ(expectPermission, info.permission);
}

/*
 * @tc.name: ParseEventsTest_1800
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           two invalid elements when profile has two elements.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1800, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[0\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"\""
        "        },"
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK1\"],"
        "            \"name\"icSubscriber\","
        "            \"permission\":\"permission1\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is not empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // expect map content is correct
    string expectEventName0 = "usual.event.TIME_TICK0";
    auto it0 = manager->validSubscribers_.find(expectEventName0);
    EXPECT_EQ(manager->validSubscribers_.end(), it0);
    // expect map content is correct
    string expectEventName1 = "usual.event.TIME_TICK1";
    auto it1 = manager->validSubscribers_.find(expectEventName1);
    EXPECT_EQ(manager->validSubscribers_.end(), it1);
}

/*
 * @tc.name: ParseEventsTest_1900
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 * duplicated elements when profile has two elements.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_1900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_1900, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK0\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"\""
        "        },"
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK0\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is not empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect map content is correct
    string expectEventName0 = "usual.event.TIME_TICK0";
    auto it = manager->validSubscribers_.find(expectEventName0);
    ASSERT_NE(manager->validSubscribers_.end(), it);
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> infos = it->second;
    ASSERT_EQ(1, infos.size());
    auto info = infos[0];
    EXPECT_EQ(testExtensionName, info.name);
    EXPECT_EQ(testExtensionBundleName, info.bundleName);
    EXPECT_EQ(testExtensionUserId, info.userId);
    EXPECT_EQ("", info.permission);
}

/*
 * @tc.name: ParseEventsTest_2000
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 * almost duplicated elements when profile has two elements.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_2000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_2000, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK0\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"\""
        "        },"
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK0\"],"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"permission1\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is not empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect map content is correct
    string expectEventName0 = "usual.event.TIME_TICK0";
    auto it = manager->validSubscribers_.find(expectEventName0);
    ASSERT_NE(manager->validSubscribers_.end(), it);
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> infos = it->second;
    ASSERT_EQ(1, infos.size());
    auto info = infos[0];
    EXPECT_EQ(testExtensionName, info.name);
    EXPECT_EQ(testExtensionBundleName, info.bundleName);
    EXPECT_EQ(testExtensionUserId, info.userId);
    EXPECT_EQ("", info.permission);
}

/*
 * @tc.name: ParseEventsTest_2100
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           no 'name' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_2100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_2100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"events\":[\"usual.event.TIME_TICK\"],"
        "            \"permission\":\"\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: ParseEventsTest_2200
 * @tc.desc: test if StaticSubscriberManager's ParseEvents function executed as expected with
 *           no 'events' element when profile only has one element.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, ParseEventsTest_2200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, ParseEventsTest_2200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    std::string testExtensionName = "StaticSubscriber";
    std::string testExtensionBundleName = "default";
    int testExtensionUserId = 100;
    std::string testProfile =
        "{"
        "    \"commonEvents\":["
        "        {"
        "            \"name\":\"StaticSubscriber\","
        "            \"permission\":\"\""
        "        }"
        "    ]"
        "}";
    // Init
    manager->ParseEvents(testExtensionName, testExtensionBundleName, testExtensionUserId, testProfile);
    // expect valid subscribers map is empty
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddToValidSubsribersTest_0100
 * @tc.desc: test if StaticSubscriberManager's AddToValidSubsribers function executed as expected in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName = "testEvent";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName, testInfo);
    // expect valid subscribers map is empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddToValidSubsribersTest_0200
 * @tc.desc: test if StaticSubscriberManager's AddToValidSubsribers function executed as expected when add 
 *           duplicated subscriber with same event.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName = "testEvent";
    StaticSubscriberManager::StaticSubscriberInfo testInfo0 = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName, testInfo0);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    StaticSubscriberManager::StaticSubscriberInfo testInfo1 = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName, testInfo1);
    EXPECT_EQ(1, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddToValidSubsribersTest_0300
 * @tc.desc: test if StaticSubscriberManager's AddToValidSubsribers function executed as expected when add 
 *           different subscribers with same event.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName = "testEvent";
    StaticSubscriberManager::StaticSubscriberInfo testInfo0 = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName, testInfo0);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    StaticSubscriberManager::StaticSubscriberInfo testInfo1 = {
        .name = "testName1",
        .bundleName = "testBundleName1",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName, testInfo1);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    auto subcriberList = manager->validSubscribers_[testEventName];
    EXPECT_EQ(TEST_MUL_SIZE, subcriberList.size());
}

/*
 * @tc.name: AddToValidSubsribersTest_0400
 * @tc.desc: test if StaticSubscriberManager's AddToValidSubsribers function executed as expected when add 
 *           same subscriber with diffrernt events.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName0 = "testEvent0";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName0, testInfo);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testEventName1 = "testEvent1";
    manager->AddToValidSubscribers(testEventName1, testInfo);
    EXPECT_EQ(TEST_MUL_SIZE, manager->validSubscribers_.size());
    auto subcriberList1 = manager->validSubscribers_[testEventName0];
    EXPECT_EQ(1, subcriberList1.size());
    auto subcriberList2 = manager->validSubscribers_[testEventName1];
    EXPECT_EQ(1, subcriberList2.size());
}

/*
 * @tc.name: AddToValidSubsribersTest_0500
 * @tc.desc: test if StaticSubscriberManager's AddToValidSubsribers function executed as expected when add 
 *           different subscribers with diffrernt events.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddToValidSubsribersTest_0500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName0 = "testEvent0";
    StaticSubscriberManager::StaticSubscriberInfo testInfo0 = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName0, testInfo0);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testEventName1 = "testEvent1";
    StaticSubscriberManager::StaticSubscriberInfo testInfo1 = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName1, testInfo1);
    EXPECT_EQ(TEST_MUL_SIZE, manager->validSubscribers_.size());
}

/*
 * @tc.name: RemoveSubscriberWithBundleNameTest_0100
 * @tc.desc: test if StaticSubscriberManager's RemoveSubscriberWithBundleName function executed as expected
 *           in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, RemoveSubscriberWithBundleNameTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, RemoveSubscriberWithBundleNameTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName = "testEvent";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName, testInfo);
    // expect valid subscribers map is empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testBundleName = "testBundleName";
    int testUserId = 100;
    manager->RemoveSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: RemoveSubscriberWithBundleNameTest_0200
 * @tc.desc: test if StaticSubscriberManager's RemoveSubscriberWithBundleName function executed as expected
 *           when userId not match.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, RemoveSubscriberWithBundleNameTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, RemoveSubscriberWithBundleNameTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName = "testEvent";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName, testInfo);
    // expect valid subscribers map is empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testBundleName = "testBundleName";
    int testUserId = 101;
    manager->RemoveSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(1, manager->validSubscribers_.size());
}

/*
 * @tc.name: RemoveSubscriberWithBundleNameTest_0300
 * @tc.desc: test if StaticSubscriberManager's RemoveSubscriberWithBundleName function executed as expected
 *           when bundleName not match.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, RemoveSubscriberWithBundleNameTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, RemoveSubscriberWithBundleNameTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName = "testEvent";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName, testInfo);
    // expect valid subscribers map is empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testBundleName = "testBundleName1";
    int testUserId = 100;
    manager->RemoveSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(1, manager->validSubscribers_.size());
}

/*
 * @tc.name: RemoveSubscriberWithBundleNameTest_0400
 * @tc.desc: test if StaticSubscriberManager's RemoveSubscriberWithBundleName function executed as expected
 *           when multiple bundlename and userid matched.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, RemoveSubscriberWithBundleNameTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, RemoveSubscriberWithBundleNameTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string testEventName0 = "testEvent0";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "testName",
        .bundleName = "testBundleName",
        .userId = 100,
        .permission = ""
    };
    manager->AddToValidSubscribers(testEventName0, testInfo);
    // expect valid subscribers map is empty
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testEventName1 = "testEvent1";
    manager->AddToValidSubscribers(testEventName1, testInfo);
    // expect valid subscribers map is empty
    EXPECT_EQ(TEST_MUL_SIZE, manager->validSubscribers_.size());
    std::string testBundleName = "testBundleName";
    int testUserId = 100;
    manager->RemoveSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: VerifyPublisherPermissionTest_0100
 * @tc.desc: test if StaticSubscriberManager's VerifyPublisherPermission function executed as expected
 *           when in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, VerifyPublisherPermissionTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, VerifyPublisherPermissionTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    Security::AccessToken::AccessTokenID id = 100;
    std::string permission = "permission";
    MockVerifyAccessToken(true);
    EXPECT_TRUE(manager->VerifyPublisherPermission(id, permission));
}

/*
 * @tc.name: VerifyPublisherPermissionTest_0200
 * @tc.desc: test if StaticSubscriberManager's VerifyPublisherPermission function executed as expected
 *           when permission is null.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, VerifyPublisherPermissionTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, VerifyPublisherPermissionTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    Security::AccessToken::AccessTokenID id = 100;
    std::string permission = "";
    EXPECT_TRUE(manager->VerifyPublisherPermission(id, permission));
}

/*
 * @tc.name: VerifyPublisherPermissionTest_0300
 * @tc.desc: test if StaticSubscriberManager's VerifyPublisherPermission function executed as expected
 *           when permission verify failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, VerifyPublisherPermissionTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, VerifyPublisherPermissionTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    Security::AccessToken::AccessTokenID id = 100;
    std::string permission = "permission";
    MockVerifyAccessToken(false);
    EXPECT_FALSE(manager->VerifyPublisherPermission(id, permission));
}

/*
 * @tc.name: InitValidSubscribersTest_0100
 * @tc.desc: test if StaticSubscriberManager's InitValidSubscribers function executed as expected
 *           in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    EXPECT_TRUE(manager->InitValidSubscribers());
    EXPECT_TRUE(manager->hasInitValidSubscribers_);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testEventName = "usual.event.TIME_TICK";
    auto validSubscribers = manager->validSubscribers_[testEventName];
    EXPECT_EQ(1, validSubscribers.size());
}

/*
 * @tc.name: InitValidSubscribersTest_0200
 * @tc.desc: test if StaticSubscriberManager's InitValidSubscribers function executed as expected
 *           when valid list is not empty.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    std::string eventName = "eventName0";
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> info0;
    manager->validSubscribers_.insert(make_pair(eventName, info0));
    EXPECT_EQ(1, manager->validSubscribers_.size());
    EXPECT_TRUE(manager->InitValidSubscribers());
    EXPECT_TRUE(manager->hasInitValidSubscribers_);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testEventName = "usual.event.TIME_TICK";
    auto validSubscribers = manager->validSubscribers_[testEventName];
    EXPECT_EQ(1, validSubscribers.size());
}

/*
 * @tc.name: InitValidSubscribersTest_0300
 * @tc.desc: test if StaticSubscriberManager's InitValidSubscribers function executed as expected
 *           when query extension info failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    MockQueryExtensionInfos(false, 0);
    std::string eventName = "eventName0";
    EXPECT_FALSE(manager->InitValidSubscribers());
}

/*
 * @tc.name: InitValidSubscribersTest_0400
 * @tc.desc: test if StaticSubscriberManager's InitValidSubscribers function executed as expected
 *           when one invalid extensions were found.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    MockQueryExtensionInfos(true, MOCK_CASE_2);
    MockGetResConfigFile(true, 1);
    EXPECT_TRUE(manager->InitValidSubscribers());
    EXPECT_TRUE(manager->hasInitValidSubscribers_);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: InitValidSubscribersTest_0500
 * @tc.desc: test if StaticSubscriberManager's InitValidSubscribers function executed as expected
 *           when two valid extensions were found, profile different.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    manager->subscriberList_.emplace_back("com.ohos.systemui1");
    MockQueryExtensionInfos(true, MOCK_CASE_3);
    MockGetResConfigFile(true, MOCK_CASE_2);
    EXPECT_TRUE(manager->InitValidSubscribers());
    EXPECT_TRUE(manager->hasInitValidSubscribers_);
    EXPECT_EQ(TEST_MUL_SIZE, manager->validSubscribers_.size());
    std::string testEventName0 = "usual.event.TIME_TICK";
    auto validSubscribers0 = manager->validSubscribers_[testEventName0];
    EXPECT_EQ(TEST_MUL_SIZE, validSubscribers0.size());
    std::string testEventName1 = "usual.event.TIME_TICK1";
    auto validSubscribers1 = manager->validSubscribers_[testEventName1];
    EXPECT_EQ(TEST_MUL_SIZE, validSubscribers1.size());
}

/*
 * @tc.name: InitValidSubscribersTest_0600
 * @tc.desc: test if StaticSubscriberManager's InitValidSubscribers function executed as expected
 *           when two valid extensions were found, profile same.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0600, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    manager->subscriberList_.emplace_back("com.ohos.systemui1");
    MockQueryExtensionInfos(true, MOCK_CASE_3);
    MockGetResConfigFile(true, MOCK_CASE_3);
    EXPECT_TRUE(manager->InitValidSubscribers());
    EXPECT_TRUE(manager->hasInitValidSubscribers_);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testEventName = "usual.event.TIME_TICK";
    auto validSubscribers = manager->validSubscribers_[testEventName];
    EXPECT_EQ(TEST_MUL_SIZE, validSubscribers.size());
}

/*
 * @tc.name: InitValidSubscribersTest_0700
 * @tc.desc: test if StaticSubscriberManager's InitValidSubscribers function executed as expected
 *           when one valid extension and one invalid extension were found.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0700, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    MockQueryExtensionInfos(true, MOCK_CASE_3);
    MockGetResConfigFile(true, MOCK_CASE_3);
    EXPECT_TRUE(manager->InitValidSubscribers());
    EXPECT_TRUE(manager->hasInitValidSubscribers_);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string testEventName = "usual.event.TIME_TICK";
    auto validSubscribers = manager->validSubscribers_[testEventName];
    EXPECT_EQ(1, validSubscribers.size());
}

/*
 * @tc.name: InitValidSubscribersTest_0800
 * @tc.desc: test if StaticSubscriberManager's InitValidSubscribers function executed as expected
 *           when two invalid extension were found.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, InitValidSubscribersTest_0800, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    manager->subscriberList_.emplace_back("com.ohos.systemui2");
    MockQueryExtensionInfos(true, MOCK_CASE_3);
    MockGetResConfigFile(true, MOCK_CASE_3);
    EXPECT_TRUE(manager->InitValidSubscribers());
    EXPECT_TRUE(manager->hasInitValidSubscribers_);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: UpdateSubscriberTest_0100
 * @tc.desc: test if StaticSubscriberManager's UpdateSubscriber function executed as expected
 *           in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    Want testWant;
    std::string testNormalEventName = "testEventName";
    testWant.SetAction(testNormalEventName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    manager->UpdateSubscriber(testEventData);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: UpdateSubscriberTest_0200
 * @tc.desc: test if StaticSubscriberManager's UpdateSubscriber function executed as expected
 *           when add bundle event was sent.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    MockGetOsAccountLocalIdFromUid(true);
    MockQueryActiveOsAccountIds(true, 0);
    manager->UpdateSubscriber(testEventData);
    EXPECT_EQ(1, manager->validSubscribers_.size());
}

/*
 * @tc.name: UpdateSubscriberTest_0300
 * @tc.desc: test if StaticSubscriberManager's UpdateSubscriber function executed as expected
 *           when change bundle event was sent.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    std::string eventName = "usual.event.TIME_TICK";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "StaticSubscriber",
        .bundleName = "com.ohos.systemui",
        .userId = 100,
        .permission = "permission1"
    };
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> info0;
    info0.emplace_back(testInfo);
    manager->validSubscribers_.insert(make_pair(eventName, info0));
    auto validSubscribers0 = manager->validSubscribers_[eventName];
    EXPECT_EQ(1, validSubscribers0.size());
    std::string expectPermissionBefore = "permission1";
    EXPECT_EQ(expectPermissionBefore, validSubscribers0[0].permission);
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    manager->UpdateSubscriber(testEventData);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    std::string expectPermissionAfter = "permission0";
    auto validSubscribers1 = manager->validSubscribers_[eventName];
    EXPECT_EQ(1, validSubscribers1.size());
    EXPECT_EQ(expectPermissionAfter, validSubscribers1[0].permission);
}

/*
 * @tc.name: UpdateSubscriberTest_0400
 * @tc.desc: test if StaticSubscriberManager's UpdateSubscriber function executed as expected
 *           when remove bundle event was sent.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    std::string eventName = "usual.event.TIME_TICK";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "StaticSubscriber",
        .bundleName = "com.ohos.systemui",
        .userId = 100,
        .permission = "permission1"
    };
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> info0;
    info0.emplace_back(testInfo);
    manager->validSubscribers_.insert(make_pair(eventName, info0));
    auto validSubscribers0 = manager->validSubscribers_[eventName];
    EXPECT_EQ(1, validSubscribers0.size());
    std::string expectPermissionBefore = "permission1";
    EXPECT_EQ(expectPermissionBefore, validSubscribers0[0].permission);
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    manager->UpdateSubscriber(testEventData);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: UpdateSubscriberTest_0500
 * @tc.desc: test if StaticSubscriberManager's UpdateSubscriber function executed as expected
 *           when GetOsAccountLocalIdFromUid failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    MockGetOsAccountLocalIdFromUid(false);
    manager->UpdateSubscriber(testEventData);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: UpdateSubscriberTest_0600
 * @tc.desc: test if StaticSubscriberManager's UpdateSubscriber function executed as expected
 *           when QueryActiveOsAccountIds failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0600, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    MockQueryActiveOsAccountIds(false, 0);
    manager->UpdateSubscriber(testEventData);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: UpdateSubscriberTest_0700
 * @tc.desc: test if StaticSubscriberManager's UpdateSubscriber function executed as expected
 *           when userId not match.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, UpdateSubscriberTest_0700, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    MockGetOsAccountLocalIdFromUid(true);
    MockQueryActiveOsAccountIds(true, 1);
    manager->UpdateSubscriber(testEventData);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberTest_0100
 * @tc.desc: test if StaticSubscriberManager's AddSubscriber function executed as expected
 *           in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    AppExecFwk::ExtensionAbilityInfo info0;
    info0.bundleName = "com.ohos.systemui";
    info0.name = "StaticSubscriber";
    MockGetResConfigFile(true, 1);
    manager->AddSubscriber(info0);
    EXPECT_EQ(1, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberTest_0200
 * @tc.desc: test if StaticSubscriberManager's AddSubscriber function executed as expected
 *           when GetResConfigFile failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    AppExecFwk::ExtensionAbilityInfo info0;
    info0.bundleName = "com.ohos.systemui";
    info0.name = "StaticSubscriber";
    MockGetResConfigFile(false, 0);
    manager->AddSubscriber(info0);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberTest_0300
 * @tc.desc: test if StaticSubscriberManager's AddSubscriber function executed as expected
 *           when GetOsAccountLocalIdFromUid failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    AppExecFwk::ExtensionAbilityInfo info0;
    info0.bundleName = "com.ohos.systemui";
    info0.name = "StaticSubscriber";
    MockGetResConfigFile(true, 1);
    MockGetOsAccountLocalIdFromUid(false);
    manager->AddSubscriber(info0);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberTest_0400
 * @tc.desc: test if StaticSubscriberManager's AddSubscriber function executed as expected
 *           when profiles has more than one profile.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    AppExecFwk::ExtensionAbilityInfo info0;
    info0.bundleName = "com.ohos.systemui";
    info0.name = "StaticSubscriber";
    MockGetResConfigFile(true, MOCK_CASE_2);
    MockGetOsAccountLocalIdFromUid(true);
    manager->AddSubscriber(info0);
    EXPECT_EQ(TEST_MUL_SIZE, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberWithBundleNameTest_0100
 * @tc.desc: test if StaticSubscriberManager's AddSubscriberWithBundleName function executed as expected
 *           in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    // mock that cet config file with one profile
    MockGetResConfigFile(true, 1);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    manager->AddSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(1, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberWithBundleNameTest_0200
 * @tc.desc: test if StaticSubscriberManager's AddSubscriberWithBundleName function executed as expected
 *           when QueryExtensionInfos failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    // mock that query extension info failed
    MockQueryExtensionInfos(false, 0);
    MockGetResConfigFile(true, 1);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    manager->AddSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberWithBundleNameTest_0300
 * @tc.desc: test if StaticSubscriberManager's AddSubscriberWithBundleName function executed as expected
 *           when bundleName not match.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    std::string testBundleName = "com.ohos.systemui1";
    int32_t testUserId = 100;
    manager->AddSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberWithBundleNameTest_0400
 * @tc.desc: test if StaticSubscriberManager's AddSubscriberWithBundleName function executed as expected
 *           when extension is not in allow list.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui0");
    // mock that query extension info with one extension
    MockQueryExtensionInfos(true, 1);
    MockGetResConfigFile(true, 1);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    manager->AddSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(0, manager->validSubscribers_.size());
}

/*
 * @tc.name: AddSubscriberWithBundleNameTest_0500
 * @tc.desc: test if StaticSubscriberManager's AddSubscriberWithBundleName function executed as expected
 *           when more than one extensions were queried.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, AddSubscriberWithBundleNameTest_0500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // init subscriberList
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    MockQueryExtensionInfos(true, MOCK_CASE_4);
    MockGetResConfigFile(true, MOCK_CASE_4);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    manager->AddSubscriberWithBundleName(testBundleName, testUserId);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    std::string testEventName = "usual.event.TIME_TICK";
    auto subscribers = manager->validSubscribers_[testEventName];
    EXPECT_EQ(TEST_MUL_SIZE, subscribers.size());
}

/*
 * @tc.name: VerifySubscriberPermissionTest_0100
 * @tc.desc: test if StaticSubscriberManager's VerifySubscriberPermission function executed as expected
 *           in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    MockGetHapTokenID(100);
    // mock that verify success
    MockVerifyAccessToken(true);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    std::string permission0 = "permission0";
    std::string permission1 = "permission1";
    std::vector<std::string> permissions;
    permissions.emplace_back(permission0);
    permissions.emplace_back(permission1);
    EXPECT_TRUE(manager->VerifySubscriberPermission(testBundleName, testUserId, permissions));
}

/*
 * @tc.name: VerifySubscriberPermissionTest_0200
 * @tc.desc: test if StaticSubscriberManager's VerifySubscriberPermission function executed as expected
 *           when permissions list is empty.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    MockGetHapTokenID(100);
    // mock that verify success
    MockVerifyAccessToken(true);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    std::vector<std::string> permissions;
    EXPECT_TRUE(manager->VerifySubscriberPermission(testBundleName, testUserId, permissions));
}

/*
 * @tc.name: VerifySubscriberPermissionTest_0300
 * @tc.desc: test if StaticSubscriberManager's VerifySubscriberPermission function executed as expected
 *           when permissions list has empty permission.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    MockGetHapTokenID(100);
    // mock that verify success
    MockVerifyAccessToken(true);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    std::string permission0 = "";
    std::string permission1 = "permission1";
    std::vector<std::string> permissions;
    permissions.emplace_back(permission0);
    permissions.emplace_back(permission1);
    EXPECT_TRUE(manager->VerifySubscriberPermission(testBundleName, testUserId, permissions));
}

/*
 * @tc.name: VerifySubscriberPermissionTest_0400
 * @tc.desc: test if StaticSubscriberManager's VerifySubscriberPermission function executed as expected
 *           when permissions partly verify failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    MockGetHapTokenID(100);
    // mock that first time verify success, second tile verify failed
    MockVerifyAccessToken(false, 1);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    std::string permission0 = "permission0";
    std::string permission1 = "permission1";
    std::vector<std::string> permissions;
    permissions.emplace_back(permission0);
    permissions.emplace_back(permission1);
    EXPECT_FALSE(manager->VerifySubscriberPermission(testBundleName, testUserId, permissions));
}

/*
 * @tc.name: VerifySubscriberPermissionTest_0500
 * @tc.desc: test if StaticSubscriberManager's VerifySubscriberPermission function executed as expected
 *           when permissions all verify failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, VerifySubscriberPermissionTest_0500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    MockGetHapTokenID(100);
    // mock that verify failed
    MockVerifyAccessToken(false);
    std::string testBundleName = "com.ohos.systemui";
    int32_t testUserId = 100;
    std::string permission0 = "permission0";
    std::string permission1 = "permission1";
    std::vector<std::string> permissions;
    permissions.emplace_back(permission0);
    permissions.emplace_back(permission1);
    EXPECT_FALSE(manager->VerifySubscriberPermission(testBundleName, testUserId, permissions));
}

/*
 * @tc.name: PublishCommonEventTest_0100
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when first time publish normal event.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    auto subscribers = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers.size());
    EXPECT_TRUE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_0200
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when second time publish normal event.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // mock that allowlist has already inited
    manager->hasInitAllowList_ = true;
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    // mock that valid subscribers has already inited
    manager->hasInitValidSubscribers_ = true;
    std::string eventName = "usual.event.TIME_TICK";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "StaticSubscriber",
        .bundleName = "com.ohos.systemui",
        .userId = 100,
        .permission = "permission1"
    };
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> info0;
    info0.emplace_back(testInfo);
    manager->validSubscribers_.insert(make_pair(eventName, info0));
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    auto subscribers = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers.size());
    EXPECT_TRUE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_0300
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when first time publish and InitValidSubscribers failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos failed
    MockQueryExtensionInfos(false, 0);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_0400
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_BOOT_COMPLETED and InitValidSubscriber success.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_0500
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_BOOT_COMPLETED and InitValidSubscriber failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos failed
    MockQueryExtensionInfos(false, 0);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_0600
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_LOCKED_BOOT_COMPLETED and InitValidSubscriber success.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0600, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_0700
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_LOCKED_BOOT_COMPLETED and InitValidSubscriber failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0700, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_LOCKED_BOOT_COMPLETED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos failed
    MockQueryExtensionInfos(false, 0);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_0800
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_USER_SWITCHED and InitValidSubscriber success.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0800, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_0900
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_USER_SWITCHED and InitValidSubscriber failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_0900, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos failed
    MockQueryExtensionInfos(false, 0);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1000
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_UID_REMOVED and InitValidSubscriber success.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1000, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_UID_REMOVED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1100
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_UID_REMOVED and InitValidSubscriber failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_UID_REMOVED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos failed
    MockQueryExtensionInfos(false, 0);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1200
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_USER_STARTED and InitValidSubscriber success.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_USER_STARTED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1300
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_USER_STARTED and InitValidSubscriber failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1300, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_USER_STARTED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos failed
    MockQueryExtensionInfos(false, 0);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(0, manager->validSubscribers_.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1400
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_PACKAGE_ADDED.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1400, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // mock that allowlist has already inited
    manager->hasInitAllowList_ = true;
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    manager->subscriberList_.emplace_back("com.ohos.systemui1");
    // mock that valid subscribers has already inited
    manager->hasInitValidSubscribers_ = true;
    std::string eventName = "usual.event.TIME_TICK";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "StaticSubscriber",
        .bundleName = "com.ohos.systemui",
        .userId = 100,
        .permission = "permission1"
    };
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> info0;
    info0.emplace_back(testInfo);
    manager->validSubscribers_.insert(make_pair(eventName, info0));
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui1";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);

    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, MOCK_CASE_2);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, MOCK_CASE_5);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    // expect time_tick and add event both have subscriber
    EXPECT_EQ(TEST_MUL_SIZE, manager->validSubscribers_.size());
    auto subscribers0 = manager->validSubscribers_[eventName];
    EXPECT_EQ(TEST_MUL_SIZE, subscribers0.size());
    auto subscribers1 = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers1.size());
    EXPECT_TRUE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1500
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_PACKAGE_CHANGED.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1500, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // mock that allowlist has already inited
    manager->hasInitAllowList_ = true;
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    manager->subscriberList_.emplace_back("com.ohos.systemui1");
    // mock that valid subscribers has already inited
    manager->hasInitValidSubscribers_ = true;
    std::string eventName = "usual.event.TIME_TICK";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "StaticSubscriber",
        .bundleName = "com.ohos.systemui",
        .userId = 100,
        .permission = "permission1"
    };
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> info0;
    info0.emplace_back(testInfo);
    manager->validSubscribers_.insert(make_pair(eventName, info0));
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui1";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, MOCK_CASE_2);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, MOCK_CASE_5);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    // expect time_tick and add event both have subscriber
    EXPECT_EQ(TEST_MUL_SIZE, manager->validSubscribers_.size());
    auto subscribers0 = manager->validSubscribers_[eventName];
    EXPECT_EQ(TEST_MUL_SIZE, subscribers0.size());
    auto subscribers1 = manager->validSubscribers_[CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED];
    EXPECT_EQ(1, subscribers1.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1600
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publish a COMMON_EVENT_PACKAGE_REMOVED.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1600, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // mock that allowlist has already inited
    manager->hasInitAllowList_ = true;
    manager->subscriberList_.emplace_back("com.ohos.systemui");
    manager->subscriberList_.emplace_back("com.ohos.systemui1");
    // mock that valid subscribers has already inited
    manager->hasInitValidSubscribers_ = true;
    std::string eventName = "usual.event.TIME_TICK";
    StaticSubscriberManager::StaticSubscriberInfo testInfo = {
        .name = "StaticSubscriber",
        .bundleName = "com.ohos.systemui",
        .userId = 100,
        .permission = "permission1"
    };
    std::vector<StaticSubscriberManager::StaticSubscriberInfo> info0;
    info0.emplace_back(testInfo);
    manager->validSubscribers_.insert(make_pair(eventName, info0));
    // construct common event data
    Want testWant;
    std::string testNormalEventName = CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED;
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui1";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, MOCK_CASE_2);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, MOCK_CASE_5);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    // expect time_tick and add event both have subscriber
    EXPECT_EQ(1, manager->validSubscribers_.size());
    auto subscribers0 = manager->validSubscribers_[eventName];
    EXPECT_EQ(1, subscribers0.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1700
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when subscriber userId smaller than min of valid userId.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1700, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    int32_t testUserId = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, 1);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    auto subscribers = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1800
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when userId param is ALL_USER.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1800, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct ALL_USER userId
    int32_t testUserId = -1;
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    auto subscribers = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers.size());
    EXPECT_TRUE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_1900
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when subscriber userid dismatch param userId.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_1900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_1900, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct not match userId
    int32_t testUserId = 101;
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that all VerifyAccessToken operation success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    auto subscribers = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_2000
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when publisher permission verify failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_2000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_2000, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct not match userId
    int32_t testUserId = 100;
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that first time verify permission failed, second time success
    MockVerifyAccessToken(true, 1);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    auto subscribers = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_2100
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when subscriber permission verify failed.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_2100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_2100, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // assume that publisher require subscriber need has some permission
    std::vector<std::string> permissionsRequieredForSub;
    std::string permission1RequieredForSub = "permission1RequieredForSub";
    std::string permission2RequieredForSub = "permission2RequieredForSub";
    permissionsRequieredForSub.emplace_back(permission1RequieredForSub);
    permissionsRequieredForSub.emplace_back(permission2RequieredForSub);
    testPublishInfo.SetSubscriberPermissions(permissionsRequieredForSub);
    // construct not match userId
    int32_t testUserId = 100;
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that first time verify permission failed, second time success
    MockVerifyAccessToken(false, 1);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    auto subscribers = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

/*
 * @tc.name: PublishCommonEventTest_2200
 * @tc.desc: test if StaticSubscriberManager's PublishCommonEvent function executed as expected
 *           in when subscriber bundleName and publishInfo bundleName dismatch.
 * @tc.type: FUNC
 * @tc.require: #I5RLKK
 * 
 */
HWTEST_F(StaticSubscriberManagerUnitTest, PublishCommonEventTest_2200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "StaticSubscriberManagerUnitTest, PublishCommonEventTest_2200, TestSize.Level1";
    std::shared_ptr<StaticSubscriberManager> manager = std::make_shared<StaticSubscriberManager>();
    ASSERT_NE(nullptr, manager);
    // construct common event data
    Want testWant;
    std::string testNormalEventName = "usual.event.TIME_TICK";
    testWant.SetAction(testNormalEventName);
    std::string testBundleName = "com.ohos.systemui1";
    std::string testAbilityName = "StaticSubscriber";
    testWant.SetElementName(testBundleName, testAbilityName);
    CommonEventData testEventData;
    testEventData.SetWant(testWant);
    // construct common event publish info,mock that publishinfo's bundlename is the same with subscriber
    // and permission verify successfully.
    CommonEventPublishInfo testPublishInfo;
    testPublishInfo.SetBundleName(testBundleName);
    // construct not match userId
    int32_t testUserId = 100;
    // construct other params, not important
    Security::AccessToken::AccessTokenID testCallerToken = 100;
    sptr<IRemoteObject> service = nullptr;
    std::string bundleName = "com.ohos.systemui";
    // mock that QueryExtensionInfos success
    MockQueryExtensionInfos(true, 1);
    // mock that GetResConfigFile success
    MockGetResConfigFile(true, 1);
    // mock that verify permission success
    MockVerifyAccessToken(true);
    // mock that GetOsAccountLocalIdFromUid success
    MockGetOsAccountLocalIdFromUid(true, MOCK_CASE_2);
    manager->PublishCommonEvent(testEventData, testPublishInfo, testCallerToken, testUserId, service, bundleName);
    EXPECT_EQ(1, manager->validSubscribers_.size());
    // expect that targer event has two subscribers
    auto subscribers = manager->validSubscribers_[testNormalEventName];
    EXPECT_EQ(1, subscribers.size());
    EXPECT_FALSE(IsConnectAbilityCalled());
    ResetAbilityManagerHelperState();
}

