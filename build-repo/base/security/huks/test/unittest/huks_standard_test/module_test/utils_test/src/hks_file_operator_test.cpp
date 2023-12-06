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

#include "hks_file_operator_test.h"

#include <gtest/gtest.h>
#include <unistd.h>

#include "hks_file_operator.h"

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_type.h"

#include "securec.h"

using namespace testing::ext;
namespace Unittest::HksFileOperatorTest {
class HksFileOperatorTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksFileOperatorTest::SetUpTestCase(void)
{
}

void HksFileOperatorTest::TearDownTestCase(void)
{
}

void HksFileOperatorTest::SetUp()
{
}

void HksFileOperatorTest::TearDown()
{
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest001
 * @tc.desc: tdd HksFileSize, with NULL input, expect 0
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest001, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest001");
    int32_t ret = HksFileSize(nullptr, nullptr);
    ASSERT_EQ(ret, 0) << "HksFileSize failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest002
 * @tc.desc: tdd HksFileWrite, with NULL input, expect HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest002, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest002");
    int32_t ret = HksFileWrite(nullptr, nullptr, 0, nullptr, 0);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksFileWrite failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest003
 * @tc.desc: tdd HksFileRead, with NULL input, expect 0
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest003, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest003");
    int32_t ret = HksFileRead(nullptr, nullptr, 0, nullptr, 0);
    ASSERT_EQ(ret, 0) << "HksFileRead failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest006
 * @tc.desc: tdd HksRemoveDir, with NULL input, expect HKS_FAILURE
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest006, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest006");
    int32_t ret = HksRemoveDir(nullptr);
    ASSERT_EQ(ret, HKS_FAILURE) << "HksRemoveDir failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest007
 * @tc.desc: tdd HksGetStoragePath, with NULL input, expect HKS_ERROR_INVALID_ARGUMENT
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest007, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest007");
    int32_t ret = HksGetStoragePath(HKS_STORAGE_MAIN_PATH, nullptr, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksGetStoragePath failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest008
 * @tc.desc: tdd HksMakeDir, with NULL input, expect HKS_ERROR_MAKE_DIR_FAIL
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest008, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest008");
    int32_t ret = HksMakeDir(nullptr);
    ASSERT_EQ(ret, HKS_ERROR_MAKE_DIR_FAIL) << "HksMakeDir failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest009
 * @tc.desc: tdd HksIsDirExist, with NULL input, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest009, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest009");
    int32_t ret = HksIsDirExist(nullptr);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER) << "HksIsDirExist failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest010
 * @tc.desc: tdd HksIsFileExist, with NULL input, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest010, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest010");
    int32_t ret = HksIsFileExist(nullptr, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_NULL_POINTER) << "HksIsFileExist failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest011
 * @tc.desc: tdd HksIsFileExist, with NULL input, expect HKS_ERROR_INTERNAL_ERROR
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest011, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest011");
    const char *path = "1";
    const uint32_t fileNameSize = 600;
    char *fileName = (char *)HksMalloc(fileNameSize);
    (void)memset_s(fileName, fileNameSize, 1, fileNameSize);
    int32_t ret = HksIsFileExist(path, fileName);
    ASSERT_EQ(ret, HKS_ERROR_INTERNAL_ERROR) << "HksIsFileExist failed, ret = " << ret;
    HksFree(fileName);
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest012
 * @tc.desc: tdd HksFileRemove, with NULL input, expect HKS_ERROR_NULL_POINTER
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest012, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest012");
    int32_t ret = HksFileRemove(nullptr, nullptr);
    ASSERT_EQ(ret, HKS_ERROR_INVALID_ARGUMENT) << "HksFileRemove failed, ret = " << ret;
}

/**
 * @tc.name: HksFileOperatorTest.HksFileOperatorTest013
 * @tc.desc: tdd HksFileRemove, with NULL input, expect HKS_ERROR_INTERNAL_ERROR
 * @tc.type: FUNC
 */
HWTEST_F(HksFileOperatorTest, HksFileOperatorTest013, TestSize.Level0)
{
    HKS_LOG_I("enter HksFileOperatorTest013");
    const char *path = "1";
    const uint32_t fileNameSize = 600;
    char *fileName = (char *)HksMalloc(fileNameSize);
    (void)memset_s(fileName, fileNameSize, 1, fileNameSize);
    int32_t ret = HksFileRemove(path, fileName);
    ASSERT_EQ(ret, HKS_ERROR_INTERNAL_ERROR) << "HksFileRemove failed, ret = " << ret;
    HksFree(fileName);
}
}
