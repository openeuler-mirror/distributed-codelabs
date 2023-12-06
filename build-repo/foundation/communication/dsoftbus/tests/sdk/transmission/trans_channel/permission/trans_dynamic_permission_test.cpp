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

#include "ISessionService.h"
#include "permission_entry.h"
#include "softbus_errcode.h"
#include "softbus_permission.h"

using namespace testing::ext;

namespace OHOS {
using Communication::SoftBus::ISessionService;

int g_permUid = 0;
int g_permPid = 0;
std::string g_permSessionName;

class TransDynamicPermissionTest : public testing::Test {
public:
    TransDynamicPermissionTest()
    {}
    ~TransDynamicPermissionTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void TransDynamicPermissionTest::SetUpTestCase(void)
{
    g_permUid = (int)getuid();
    ASSERT_TRUE(g_permUid >= 0);
    g_permPid = (int)getpid();
    ASSERT_TRUE(g_permPid > 0);
    g_permSessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(g_permPid);
    ASSERT_EQ(TransPermissionInit(), SOFTBUS_OK);
}

void TransDynamicPermissionTest::TearDownTestCase(void)
{}

/**
 * @tc.name: DynamicPermissionTest001
 * @tc.desc: Grant dynamic permission, use invalid params.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransDynamicPermissionTest, DynamicPermissionTest001, TestSize.Level0)
{
    std::shared_ptr<ISessionService> softbusManager = ISessionService::GetInstance();
    ASSERT_NE(softbusManager, nullptr);

    int ret = softbusManager->GrantPermission(g_permUid, g_permPid, g_permSessionName);
    ASSERT_NE(ret, SOFTBUS_OK);
}

/**
 * @tc.name: DynamicPermissionTest002
 * @tc.desc: Remove dynamic permission, use invalid params.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransDynamicPermissionTest, DynamicPermissionTest002, TestSize.Level0)
{
    std::shared_ptr<ISessionService> softbusManager = ISessionService::GetInstance();
    ASSERT_NE(softbusManager, nullptr);

    int ret = softbusManager->RemovePermission(g_permSessionName);
    ASSERT_NE(ret, SOFTBUS_OK);
}

/**
 * @tc.name: DynamicPermissionTest003
 * @tc.desc: AddDynamicPermission success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransDynamicPermissionTest, DynamicPermissionTest003, TestSize.Level0)
{
    int ret = AddDynamicPermission(g_permUid, g_permPid, g_permSessionName.c_str());
    ASSERT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: DynamicPermissionTest004
 * @tc.desc: DeleteDynamicPermission success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransDynamicPermissionTest, DynamicPermissionTest004, TestSize.Level0)
{
    int ret = DeleteDynamicPermission(g_permSessionName.c_str());
    ASSERT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: DynamicPermissionTest005
 * @tc.desc: Dynamic permission reach the upper limit.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransDynamicPermissionTest, DynamicPermissionTest005, TestSize.Level0)
{
    int testPid = 10000;
    int ret = 0;
    int expectRet = SOFTBUS_OK;
    int testNum = 99;

    std::string sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(g_permPid);
    ret = AddDynamicPermission(g_permUid, g_permPid, sessionName.c_str());
    ASSERT_EQ(ret, SOFTBUS_OK);

    for (int i = 0; i < testNum; i++) {
        sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(testPid);
        ret = AddDynamicPermission(g_permUid, testPid, sessionName.c_str());
        ASSERT_EQ(ret, expectRet);
        testPid++;
    }

    expectRet = SOFTBUS_NO_ENOUGH_DATA;
    for (int i = 0; i < testNum; i++) {
        sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(testPid);
        ret = AddDynamicPermission(g_permUid, testPid, sessionName.c_str());
        ASSERT_EQ(ret, expectRet);
    }

    sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(g_permPid);
    ret = DeleteDynamicPermission(sessionName.c_str());
    ASSERT_EQ(ret, SOFTBUS_OK);

    sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(testPid);
    ret = AddDynamicPermission(g_permUid, testPid, sessionName.c_str());
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = DeleteDynamicPermission(sessionName.c_str());
    ASSERT_EQ(ret, SOFTBUS_OK);
    testPid--;
    for (int i = 0; i < testNum; i++) {
        sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(testPid);
        testPid--;
        ret = DeleteDynamicPermission(sessionName.c_str());
        ASSERT_EQ(ret, SOFTBUS_OK) << sessionName;
    }
}

/**
 * @tc.name: DynamicPermissionTest006
 * @tc.desc: Check trans open permission.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransDynamicPermissionTest, DynamicPermissionTest006, TestSize.Level0)
{
    int testPid = 10000;
    int ret = 0;
    int testNum = 100;

    for (int i = 0; i < testNum; i++) {
        std::string sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(testPid);
        ret = CheckTransPermission(g_permUid, testPid, "DBinderBus", sessionName.c_str(), ACTION_OPEN);
        ASSERT_NE(ret, SOFTBUS_OK);
        ret = AddDynamicPermission(g_permUid, testPid, sessionName.c_str());
        ASSERT_EQ(ret, SOFTBUS_OK);

        ret = CheckTransPermission(g_permUid, testPid, "DBinderBus", sessionName.c_str(), ACTION_OPEN);
        ASSERT_EQ(ret, SOFTBUS_OK) << "sessionName: " << sessionName.c_str();
        ret = CheckTransSecLevel(sessionName.c_str(), sessionName.c_str());
        ASSERT_EQ(ret, SOFTBUS_OK);

        testPid++;
    }
    testPid--;
    for (int i = 0; i < testNum; i++) {
        std::string sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(testPid);
        ret = DeleteDynamicPermission(sessionName.c_str());
        ASSERT_EQ(ret, SOFTBUS_OK);

        ret = CheckTransPermission(g_permUid, testPid, "DBinderBus", sessionName.c_str(), ACTION_OPEN);
        ASSERT_NE(ret, SOFTBUS_OK);

        testPid--;
    }
}

/**
 * @tc.name: DynamicPermissionTest007
 * @tc.desc: Repeated call AddDynamicPermission and DeleteDynamicPermission.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransDynamicPermissionTest, DynamicPermissionTest007, TestSize.Level0)
{
    int testPid = 17258;
    int ret = 0;
    int testNum = 100;
    std::string sessionName = "DBinder" + std::to_string(g_permUid) + std::string("_") + std::to_string(testPid);

    for (int i = 0; i < testNum; i++) {
        ret = CheckTransPermission(g_permUid, testPid, "DBinderBus", sessionName.c_str(), ACTION_OPEN);
        ASSERT_NE(ret, SOFTBUS_OK);
        ret = AddDynamicPermission(g_permUid, testPid, sessionName.c_str());
        ASSERT_EQ(ret, SOFTBUS_OK);

        ret = CheckTransPermission(g_permUid, testPid, "DBinderBus", sessionName.c_str(), ACTION_OPEN);
        ASSERT_EQ(ret, SOFTBUS_OK) << "sessionName: " << sessionName.c_str();
        ret = CheckTransSecLevel(sessionName.c_str(), sessionName.c_str());
        ASSERT_EQ(ret, SOFTBUS_OK);

        ret = DeleteDynamicPermission(sessionName.c_str());
        ASSERT_EQ(ret, SOFTBUS_OK);

        ret = CheckTransPermission(g_permUid, testPid, "DBinderBus", sessionName.c_str(), ACTION_OPEN);
        ASSERT_NE(ret, SOFTBUS_OK);
    }
}
} // namespace OHOS
