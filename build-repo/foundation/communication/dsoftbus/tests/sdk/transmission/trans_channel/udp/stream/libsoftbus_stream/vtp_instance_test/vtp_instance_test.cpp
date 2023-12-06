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
#include <securec.h>

#define private public
#include "vtp_instance.h"
#include "vtp_instance.cpp"
#undef private

using namespace testing::ext;
namespace OHOS {
#define DEVICE_ID "DEVICE_ID"

class VtpInstanceTest : public testing::Test {
public:
    VtpInstanceTest()
    {}
    ~VtpInstanceTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void VtpInstanceTest::SetUpTestCase(void)
{}

void VtpInstanceTest::TearDownTestCase(void)
{}

/**
 * @tc.name: UpdateVtpLogLevel001
 * @tc.desc: UpdateVtpLogLevel001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VtpInstanceTest, UpdateVtpLogLevel001, TestSize.Level1)
{
    int ret = Communication::SoftBus::UpdateVtpLogLevel();
    EXPECT_EQ(FILLP_DBG_LVL_DEBUG, ret);
}

/**
 * @tc.name: GetVersion001
 * @tc.desc: GetVersion001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VtpInstanceTest, GetVersion001, TestSize.Level1)
{
    std::shared_ptr<Communication::SoftBus::VtpInstance> vtpInstance =
        std::make_shared<Communication::SoftBus::VtpInstance>();
    std::string tmpStr = vtpInstance->GetVersion();

    EXPECT_TRUE(tmpStr == "VTP_V1.0");
}

/**
 * @tc.name: CryptoRand001
 * @tc.desc: CryptoRand001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VtpInstanceTest, CryptoRand001, TestSize.Level1)
{
    std::shared_ptr<Communication::SoftBus::VtpInstance> vtpInstance =
        std::make_shared<Communication::SoftBus::VtpInstance>();

    int res = (int)vtpInstance->CryptoRand();
    EXPECT_NE(0, res);
}

/**
 * @tc.name: PreSetFillpCoreParams001
 * @tc.desc: PreSetFillpCoreParams001, use the wrong parameter.
 * @tc.desc: InitVtp, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VtpInstanceTest, PreSetFillpCoreParams001, TestSize.Level1)
{
    std::shared_ptr<Communication::SoftBus::VtpInstance> vtpInstance =
        std::make_shared<Communication::SoftBus::VtpInstance>();

    vtpInstance->PreSetFillpCoreParams();

    std::string pkgName = "CryptoRandTest";
    bool ret = vtpInstance->InitVtp(pkgName);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: WaitForDestroy001
 * @tc.desc: WaitForDestroy001, use the wrong parameter.
 * @tc.desc: DestroyVtp, use the wrong parameter.
 * @tc.desc: UpdateSocketStreamCount, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VtpInstanceTest, WaitForDestroy001, TestSize.Level1)
{
    std::shared_ptr<Communication::SoftBus::VtpInstance> vtpInstance =
        std::make_shared<Communication::SoftBus::VtpInstance>();

    vtpInstance->PreSetFillpCoreParams();

    std::string pkgName = "CryptoRandTest";
    const int delayTimes =  1;

    vtpInstance->WaitForDestroy(delayTimes);
    EXPECT_TRUE(vtpInstance->isDestroyed_);

    vtpInstance->isDestroyed_ = true;
    vtpInstance->DestroyVtp(pkgName);

    vtpInstance->isDestroyed_ = false;
    vtpInstance->DestroyVtp(pkgName);

    bool add = true;
    vtpInstance->UpdateSocketStreamCount(add);

    add = false;
    vtpInstance->socketStreamCount_ = 0;
    vtpInstance->UpdateSocketStreamCount(add);

    vtpInstance->socketStreamCount_ = 1;
    vtpInstance->UpdateSocketStreamCount(add);

    vtpInstance->isDestroyed_ = false;
    bool ret = vtpInstance->InitVtp(pkgName);
    EXPECT_TRUE(ret);

    vtpInstance->isDestroyed_ = true;
    ret = vtpInstance->InitVtp(pkgName);
    EXPECT_TRUE(ret);
    EXPECT_EQ(true, ret);
}
} // OHOS
