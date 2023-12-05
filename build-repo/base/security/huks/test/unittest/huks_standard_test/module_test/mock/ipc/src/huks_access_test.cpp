/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "huks_access_test.h"

#include <gtest/gtest.h>

#include "huks_access.h"
#include "huks_core_hal_mock.h"

#include "hks_log.h"

using namespace testing::ext;
namespace Unittest::HuksAccessTest {
class HuksAccessTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HuksAccessTest::SetUpTestCase(void)
{
}

void HuksAccessTest::TearDownTestCase(void)
{
}

void HuksAccessTest::SetUp()
{
}

void HuksAccessTest::TearDown()
{
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest001
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest001, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessModuleInit();
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest002
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest002, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessModuleInit();
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest003
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest003, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessRefresh();
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest004
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest004, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessRefresh();
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest005
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest005, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessGenerateKey(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest006
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest006, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessGenerateKey(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest007
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest007, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessImportKey(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest008
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest008, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessImportKey(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest009
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest009, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessImportWrappedKey(nullptr, nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest010
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest010, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessImportWrappedKey(nullptr, nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest011
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest011, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessExportPublicKey(nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest012
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest012, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessExportPublicKey(nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest013
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest013, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessInit(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest014
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest014, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessInit(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest015
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest015, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessUpdate(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest016
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest016, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessUpdate(nullptr, nullptr, nullptr, nullptr);
}

/**
* @tc.name: HuksAccessTest.HuksAccessTest017
* @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
* @tc.type: FUNC
*/
HWTEST_F(HuksAccessTest, HuksAccessTest017, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessFinish(nullptr, nullptr, nullptr, nullptr);
}

/**
* @tc.name: HuksAccessTest.HuksAccessTest018
* @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
* @tc.type: FUNC
*/
HWTEST_F(HuksAccessTest, HuksAccessTest018, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessFinish(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest019
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest019, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessAbort(nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest020
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest020, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessAbort(nullptr, nullptr);
}

/**
* @tc.name: HuksAccessTest.HuksAccessTest021
* @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
* @tc.type: FUNC
*/
HWTEST_F(HuksAccessTest, HuksAccessTest021, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessGetKeyProperties(nullptr, nullptr);
}

/**
* @tc.name: HuksAccessTest.HuksAccessTest022
* @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
* @tc.type: FUNC
*/
HWTEST_F(HuksAccessTest, HuksAccessTest022, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessGetKeyProperties(nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest023
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest023, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessGetAbility(0);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest024
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest024, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessGetAbility(0);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest025
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest025, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessGetHardwareInfo();
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest026
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest026, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessGetHardwareInfo();
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest027
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest027, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessSign(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest028
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest028, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessSign(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest029
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest029, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessVerify(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest030
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest030, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessVerify(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest031
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest031, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessEncrypt(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest032
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest032, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessEncrypt(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest033
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest033, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessDecrypt(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest034
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest034, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessDecrypt(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest035
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest035, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessAgreeKey(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest036
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest036, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessAgreeKey(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest037
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest037, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessDeriveKey(nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest038
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest038, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessDeriveKey(nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest039
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest039, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessMac(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest040
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest040, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessMac(nullptr, nullptr, nullptr, nullptr);
}

#ifdef _STORAGE_LITE_

/**
 * @tc.name: HuksAccessTest.HuksAccessTest041
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest041, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessCalcHeaderMac(nullptr, nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest042
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest042, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessCalcHeaderMac(nullptr, nullptr, nullptr, nullptr);
}

#endif

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA

/**
 * @tc.name: HuksAccessTest.HuksAccessTest043
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest043, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessUpgradeKeyInfo(nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest044
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest044, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessUpgradeKeyInfo(nullptr, nullptr, nullptr);
}

#endif

/**
 * @tc.name: HuksAccessTest.HuksAccessTest045
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest045, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessAttestKey(nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest046
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest046, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessAttestKey(nullptr, nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest047
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest047, TestSize.Level0)
{
    HksEnableCreateOrDestroy(false);
    HksEnableSetHid(false);
    (void)HuksAccessGenerateRandom(nullptr, nullptr);
}

/**
 * @tc.name: HuksAccessTest.HuksAccessTest048
 * @tc.desc: tdd HksCreateHuksHdiDevice, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HuksAccessTest, HuksAccessTest048, TestSize.Level0)
{
    HksEnableCreateOrDestroy(true);
    HksEnableSetHid(false);
    (void)HuksAccessGenerateRandom(nullptr, nullptr);
}
}
