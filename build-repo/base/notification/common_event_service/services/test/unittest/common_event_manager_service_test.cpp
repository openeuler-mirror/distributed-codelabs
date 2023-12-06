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
#define private public
#define protected public
#include "common_event_manager_service.h"

#include "common_event.h"
#include "common_event_death_recipient.h"
#include "common_event_stub.h"
#include "common_event_proxy.h"
#include "common_event_data.h"
#include "common_event_subscriber.h"
#include "event_receive_proxy.h"
#include "common_event_publish_info.h"
#include "matching_skills.h"

#include "event_receive_stub.h"
#include "event_log_wrapper.h"
#include "datetime_ex.h"

#undef private
#undef protected

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;
using namespace OHOS;

class CommonEventManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

void CommonEventManagerServiceTest::SetUpTestCase()
{}

void CommonEventManagerServiceTest::TearDownTestCase()
{}

void CommonEventManagerServiceTest::SetUp()
{}

void CommonEventManagerServiceTest::TearDown()
{}

/**
 * @tc.name: Init_001
 * @tc.desc: Test CommonEventManagerService_
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventManagerServiceTest, Init_001, Level1)
{
    CommonEventManagerService commonEventManagerService;
    int result = commonEventManagerService.Init();
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: Init_002
 * @tc.desc: Test CommonEventManagerService_
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventManagerServiceTest, Init_002, Level1)
{
    CommonEventManagerService commonEventManagerService;
    int result = commonEventManagerService.Init();
    EXPECT_NE(ERR_INVALID_OPERATION, result);
}

/**
 * @tc.name: PublishCommonEvent_001
 * @tc.desc: Test CommonEventManagerService_
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventManagerServiceTest, PublishCommonEvent_001, Level1)
{
    CommonEventManagerService commonEventManagerService;
    CommonEventData event;
    CommonEventPublishInfo publishInfo;

    int32_t userId = 1;
    const int32_t ERR_NOTIFICATION_CESM_ERROR = 1500008;
    int32_t result = commonEventManagerService.PublishCommonEvent(event, publishInfo, nullptr, userId);
    EXPECT_EQ(ERR_NOTIFICATION_CESM_ERROR, result);
}

/**
 * @tc.name: PublishCommonEvent_002
 * @tc.desc: Test CommonEventManagerService_
 * @tc.type: FUNC
 * @tc.require: I582Y4
 */
HWTEST_F(CommonEventManagerServiceTest, PublishCommonEvent_002, Level1)
{
    CommonEventManagerService commonEventManagerService;
    CommonEventData event;
    CommonEventPublishInfo publishInfo;

    int32_t userId = 1;
    bool result = commonEventManagerService.PublishCommonEvent(event, publishInfo, nullptr, userId);
    EXPECT_EQ(true, result);
}