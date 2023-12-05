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
#include "hisysevent_c_test.h"

#include <climits>
#include <securec.h>
#include "def.h"
#include "hisysevent_c.h"

using namespace OHOS::HiviewDFX;
using namespace testing::ext;

namespace {
const char TEST_DOMAIN[] = "TEST_DOMAIN";
const char TEST_NAME[] = "TEST_NAME";
}

void HiSysEventCTest::SetUp()
{}

void HiSysEventCTest::TearDown()
{}

/**
 * @tc.name: HiSysEventCTest001
 * @tc.desc: Test writing of basic type data.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    HiSysEventParam param1 = {
        .name = "KEY_BOOL_F",
        .t = HISYSEVENT_BOOL,
        .v = { .b = false },
        .arraySize = 0,
    };
    HiSysEventParam param2 = {
        .name = "KEY_BOOL_T",
        .t = HISYSEVENT_BOOL,
        .v = { .b = true },
        .arraySize = 0,
    };
    HiSysEventParam param3 = {
        .name = "KEY_INT8_MIN",
        .t = HISYSEVENT_INT8,
        .v = { .i8 = SCHAR_MIN },
        .arraySize = 0,
    };
    HiSysEventParam param4 = {
        .name = "KEY_INT8_MAX",
        .t = HISYSEVENT_INT8,
        .v = { .i8 = SCHAR_MAX },
        .arraySize = 0,
    };
    HiSysEventParam param5 = {
        .name = "KEY_UINT8_MIN",
        .t = HISYSEVENT_UINT8,
        .v = { .ui8 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam param6 = {
        .name = "KEY_UINT8_MAX",
        .t = HISYSEVENT_UINT8,
        .v = { .ui8 = UCHAR_MAX },
        .arraySize = 0,
    };
    HiSysEventParam param7 = {
        .name = "KEY_INT16_MIN",
        .t = HISYSEVENT_INT16,
        .v = { .i16 = SHRT_MIN },
        .arraySize = 0,
    };
    HiSysEventParam param8 = {
        .name = "KEY_INT16_MAX",
        .t = HISYSEVENT_INT16,
        .v = { .i16 = SHRT_MAX },
        .arraySize = 0,
    };
    HiSysEventParam param9 = {
        .name = "KEY_UINT16_MIN",
        .t = HISYSEVENT_UINT16,
        .v = { .ui16 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam param10 = {
        .name = "KEY_UINT16_MAX",
        .t = HISYSEVENT_UINT16,
        .v = { .ui16 = USHRT_MAX },
        .arraySize = 0,
    };
    HiSysEventParam param11 = {
        .name = "KEY_INT32_MIN",
        .t = HISYSEVENT_INT32,
        .v = { .i32 = INT_MIN },
        .arraySize = 0,
    };
    HiSysEventParam param12 = {
        .name = "KEY_INT32_MAX",
        .t = HISYSEVENT_INT32,
        .v = { .i32 = INT_MAX },
        .arraySize = 0,
    };
    HiSysEventParam param13 = {
        .name = "KEY_UINT32_MIN",
        .t = HISYSEVENT_UINT32,
        .v = { .ui32 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam param14 = {
        .name = "KEY_UINT32_MAX",
        .t = HISYSEVENT_UINT32,
        .v = { .ui32 = UINT_MAX },
        .arraySize = 0,
    };
    HiSysEventParam param15 = {
        .name = "KEY_INT64_MIN",
        .t = HISYSEVENT_INT64,
        .v = { .i64 = LLONG_MIN },
        .arraySize = 0,
    };
    HiSysEventParam param16 = {
        .name = "KEY_INT64_MAX",
        .t = HISYSEVENT_INT64,
        .v = { .i64 = LLONG_MAX },
        .arraySize = 0,
    };
    HiSysEventParam param17 = {
        .name = "KEY_UINT64_MIN",
        .t = HISYSEVENT_UINT64,
        .v = { .ui64 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam param18 = {
        .name = "KEY_UINT64_MAX",
        .t = HISYSEVENT_UINT64,
        .v = { .ui64 = ULLONG_MAX },
        .arraySize = 0,
    };
    HiSysEventParam param19 = {
        .name = "KEY_FLOAT",
        .t = HISYSEVENT_FLOAT,
        .v = { .f = 123.456 },
        .arraySize = 0,
    };
    HiSysEventParam param20 = {
        .name = "KEY_DOUBLE",
        .t = HISYSEVENT_DOUBLE,
        .v = { .d = 123.456789 },
        .arraySize = 0,
    };
    char testStr[] = "STR_VALUE";
    HiSysEventParam param21 = {
        .name = "KEY_STRING",
        .t = HISYSEVENT_STRING,
        .v = { .s = testStr },
        .arraySize = 0,
    };

    HiSysEventParam params[] = {
        param1, param2, param3, param4, param5, param6, param7,
        param8, param9, param10, param11, param12, param13, param14,
        param15, param16, param17, param18, param19, param20, param21,
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_FAULT, params, len);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: HiSysEventCTest002
 * @tc.desc: Test writing of array type data.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    bool bArr[] = { true, false };
    HiSysEventParam param1 = {
        .name = "KEY_BOOL_ARR",
        .t = HISYSEVENT_BOOL_ARRAY,
        .v = { .array = bArr },
        .arraySize = sizeof(bArr) / sizeof(bArr[0]),
    };
    int8_t int8Arr[] = { -1, 2, -3 };
    HiSysEventParam param2 = {
        .name = "KEY_INT8_ARR",
        .t = HISYSEVENT_INT8_ARRAY,
        .v = { .array = int8Arr },
        .arraySize = sizeof(int8Arr) / sizeof(int8Arr[0]),
    };
    uint8_t uint8Arr[] = { 4, 5, 6 };
    HiSysEventParam param3 = {
        .name = "KEY_UINT8_ARR",
        .t = HISYSEVENT_UINT8_ARRAY,
        .v = { .array = uint8Arr },
        .arraySize = sizeof(uint8Arr) / sizeof(uint8Arr[0]),
    };
    int16_t int16Arr[] = { -7, 8, -9 };
    HiSysEventParam param4 = {
        .name = "KEY_INT16_ARR",
        .t = HISYSEVENT_INT16_ARRAY,
        .v = { .array = int16Arr },
        .arraySize = sizeof(int16Arr) / sizeof(int16Arr[0]),
    };
    uint16_t uint16Arr[] = { 10, 11, 12 };
    HiSysEventParam param5 = {
        .name = "KEY_UINT16_ARR",
        .t = HISYSEVENT_UINT16_ARRAY,
        .v = { .array = uint16Arr },
        .arraySize = sizeof(uint16Arr) / sizeof(uint16Arr[0]),
    };
    int32_t int32Arr[] = { -13, 14, -15 };
    HiSysEventParam param6 = {
        .name = "KEY_INT32_ARR",
        .t = HISYSEVENT_INT32_ARRAY,
        .v = { .array = int32Arr },
        .arraySize = sizeof(int32Arr) / sizeof(int32Arr[0]),
    };
    uint32_t uint32Arr[] = { 16, 17, 18 };
    HiSysEventParam param7 = {
        .name = "KEY_UINT32_ARR",
        .t = HISYSEVENT_UINT32_ARRAY,
        .v = { .array = uint32Arr },
        .arraySize = sizeof(uint32Arr) / sizeof(uint32Arr[0]),
    };
    int64_t int64Arr[] = { -19, 20, -21 };
    HiSysEventParam param8 = {
        .name = "KEY_INT64_ARR",
        .t = HISYSEVENT_INT64_ARRAY,
        .v = { .array = int64Arr },
        .arraySize = sizeof(int64Arr) / sizeof(int64Arr[0]),
    };
    uint64_t uint64Arr[] = { 22, 23, 24 };
    HiSysEventParam param9 = {
        .name = "KEY_UINT64_ARR",
        .t = HISYSEVENT_UINT64_ARRAY,
        .v = { .array = uint64Arr },
        .arraySize = sizeof(uint64Arr) / sizeof(uint64Arr[0]),
    };
    float fArr[] = { 1.1, 2.2, 3.3 };
    HiSysEventParam param10 = {
        .name = "KEY_FLOAT_ARR",
        .t = HISYSEVENT_FLOAT_ARRAY,
        .v = { .array = fArr },
        .arraySize = sizeof(fArr) / sizeof(fArr[0]),
    };
    double dArr[] = { 4.4, 5.5, 6.6 };
    HiSysEventParam param11 = {
        .name = "KEY_DOUBLE_ARR",
        .t = HISYSEVENT_DOUBLE_ARRAY,
        .v = { .array = dArr },
        .arraySize = sizeof(dArr) / sizeof(dArr[0]),
    };
    char s1[] = "str1";
    char s2[] = "str2";
    char s3[] = "str3";
    char* sArr[] = { s1, s2, s3 };
    HiSysEventParam param12 = {
        .name = "KEY_STRING_ARR",
        .t = HISYSEVENT_STRING_ARRAY,
        .v = { .array = sArr },
        .arraySize = sizeof(sArr) / sizeof(sArr[0]),
    };

    HiSysEventParam params[] = {
        param1, param2, param3, param4, param5, param6,
        param7, param8, param9, param10, param11, param12,
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_STATISTIC, params, len);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: HiSysEventCTest003
 * @tc.desc: Test writing of base data.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, 0);

    HiSysEventParam params1[] = {};
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_SECURITY, params1, 0);
    ASSERT_EQ(res, 0);

    char* sArr[] = {};
    HiSysEventParam param = {
        .name = "KEY_STRING_ARR",
        .t = HISYSEVENT_STRING_ARRAY,
        .v = { .array = sArr },
        .arraySize = sizeof(sArr) / sizeof(sArr[0]),
    };
    HiSysEventParam params2[] = { param };
    size_t len = sizeof(params2) / sizeof(params2[0]);
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params2, len);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: HiSysEventCTest004
 * @tc.desc: Test writing of invalid data.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    int res = OH_HiSysEvent_Write(nullptr, TEST_NAME, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_DOMAIN_NAME_INVALID);

    res = OH_HiSysEvent_Write(TEST_DOMAIN, nullptr, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_EVENT_NAME_INVALID);

    HiSysEventParam param = {
        .name = "KEY_STRING",
        .t = HISYSEVENT_STRING,
        .v = { .s = nullptr },
        .arraySize = 0,
    };
    HiSysEventParam params[] = { param };
    size_t len = sizeof(params) / sizeof(params[0]);
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params, len);
    ASSERT_EQ(res, ERR_VALUE_INVALID);

    HiSysEventParam param1 = {
        .name = "KEY_STRING_ARR",
        .t = HISYSEVENT_STRING_ARRAY,
        .v = { .array = nullptr },
        .arraySize = 0,
    };
    HiSysEventParam params1[] = { param1 };
    len = sizeof(params1) / sizeof(params1[0]);
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params1, len);
    ASSERT_EQ(res, ERR_VALUE_INVALID);

    HiSysEventParam param2 = {
        .name = "KEY_INT32_ARR",
        .t = HISYSEVENT_INT32_ARRAY,
        .v = { .array = nullptr },
        .arraySize = 2,
    };
    HiSysEventParam params2[] = { param2 };
    len = sizeof(params2) / sizeof(params2[0]);
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params2, len);
    ASSERT_EQ(res, ERR_VALUE_INVALID);

    char c1[] = "str1";
    char* strs[] = { nullptr,  c1 };
    HiSysEventParam param3 = {
        .name = "KEY_STRING_ARR",
        .t = HISYSEVENT_STRING_ARRAY,
        .v = { .array = strs },
        .arraySize = 2,
    };
    HiSysEventParam params3[] = { param3 };
    len = sizeof(params3) / sizeof(params3[0]);
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params3, len);
    ASSERT_EQ(res, ERR_VALUE_INVALID);
}

/**
 * @tc.name: HiSysEventCTest005
 * @tc.desc: Test writing of invalid domain.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    const char* domain1 = "123domain";
    int res = OH_HiSysEvent_Write(domain1, TEST_NAME, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_DOMAIN_NAME_INVALID);

    const char* domain2 = "_domain";
    res = OH_HiSysEvent_Write(domain2, TEST_NAME, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_DOMAIN_NAME_INVALID);

    const char* domain3 = "";
    res = OH_HiSysEvent_Write(domain3, TEST_NAME, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_DOMAIN_NAME_INVALID);

    std::string domain4(MAX_DOMAIN_LENGTH + 1, 'a');
    res = OH_HiSysEvent_Write(domain4.c_str(), TEST_NAME, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_DOMAIN_NAME_INVALID);

    const char* domain5 = "domain##5";
    res = OH_HiSysEvent_Write(domain5, TEST_NAME, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_DOMAIN_NAME_INVALID);
}

/**
 * @tc.name: HiSysEventCTest006
 * @tc.desc: Test writing of invalid event name.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    const char* name1 = "123name";
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, name1, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_EVENT_NAME_INVALID);

    const char* name2 = "_name";
    res = OH_HiSysEvent_Write(TEST_DOMAIN, name2, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_EVENT_NAME_INVALID);

    const char* name3 = "";
    res = OH_HiSysEvent_Write(TEST_DOMAIN, name3, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_EVENT_NAME_INVALID);

    std::string name4(MAX_EVENT_NAME_LENGTH + 1, 'a');
    res = OH_HiSysEvent_Write(TEST_DOMAIN, name4.c_str(), HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_EVENT_NAME_INVALID);

    const char* name5 = "name##5";
    res = OH_HiSysEvent_Write(TEST_DOMAIN, name5, HISYSEVENT_SECURITY, nullptr, 0);
    ASSERT_EQ(res, ERR_EVENT_NAME_INVALID);
}

/**
 * @tc.name: HiSysEventCTest007
 * @tc.desc: Test writing of invalid key.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    HiSysEventParam param1 = {
        .name = "",
        .t = HISYSEVENT_INT32,
        .v = { .i32 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam params1[] = { param1 };
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params1, 1);
    ASSERT_EQ(res, ERR_KEY_NAME_INVALID);

    HiSysEventParam param2 = {
        .name = "_key1",
        .t = HISYSEVENT_INT32,
        .v = { .i32 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam params2[] = { param2 };
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params2, 1);
    ASSERT_EQ(res, ERR_KEY_NAME_INVALID);

    HiSysEventParam param3 = {
        .name = "123_key2",
        .t = HISYSEVENT_INT32,
        .v = { .i32 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam params3[] = { param3 };
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params3, 1);
    ASSERT_EQ(res, ERR_KEY_NAME_INVALID);

    HiSysEventParam param4 = {
        .name = "key3**",
        .t = HISYSEVENT_INT32,
        .v = { .i32 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam params4[] = { param4 };
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params4, 1);
    ASSERT_EQ(res, ERR_KEY_NAME_INVALID);

    HiSysEventParam param5 = {
        .name = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        .t = HISYSEVENT_INT32,
        .v = { .i32 = 0 },
        .arraySize = 0,
    };
    HiSysEventParam params5[] = { param5 };
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params5, 1);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: HiSysEventCTest008
 * @tc.desc: Test writing of 129 params.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest008, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    HiSysEventParam params[MAX_PARAM_NUMBER + 1];
    for (size_t i = 0; i <= MAX_PARAM_NUMBER; ++i) {
        HiSysEventParam param = {
            .t = HISYSEVENT_INT32,
            .v = { .i32 = 0 },
            .arraySize = 0,
        };
        std::string key = "key" + std::to_string(i);
        if (strcpy_s(param.name, sizeof(param.name), key.c_str()) != EOK) {
            return;
        }
        params[i] = param;
    }
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params, MAX_PARAM_NUMBER + 1);
    ASSERT_EQ(res, ERR_KEY_NUMBER_TOO_MUCH);
}

/**
 * @tc.name: HiSysEventCTest010
 * @tc.desc: Test writing of long string params.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest010, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    size_t strLen = MAX_STRING_LENGTH + 1;
    char* longStr = new char[strLen + 1];
    for (size_t i = 0; i < strLen; ++i) {
        longStr[i] = 'a';
    }
    longStr[strLen] = '\0';
    HiSysEventParam param = {
        .name = "KEY_STRING",
        .t = HISYSEVENT_STRING,
        .v = { .s = longStr },
        .arraySize = 0,
    };
    HiSysEventParam params[] = { param };
    size_t len = sizeof(params) / sizeof(params[0]);
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params, len);
    ASSERT_EQ(res, ERR_VALUE_LENGTH_TOO_LONG);

    char* srts[] = { longStr };
    HiSysEventParam param2 = {
        .name = "KEY_STRING_ARR",
        .t = HISYSEVENT_STRING_ARRAY,
        .v = { .array = srts },
        .arraySize = 1,
    };
    HiSysEventParam params2[] = { param2 };
    len = sizeof(params2) / sizeof(params2[0]);
    res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params2, len);
    ASSERT_EQ(res, ERR_VALUE_LENGTH_TOO_LONG);

    delete[] longStr;
}

/**
 * @tc.name: HiSysEventCTest011
 * @tc.desc: Test writing of long array params.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest011, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    int32_t int32s[MAX_ARRAY_SIZE + 1] = { 0 };
    for (size_t i = 0; i <= MAX_ARRAY_SIZE; ++i) {
        int32s[i] = 1;
    }
    HiSysEventParam param = {
        .name = "KEY_INT32_ARR",
        .t = HISYSEVENT_INT32_ARRAY,
        .v = { .array = int32s },
        .arraySize = MAX_ARRAY_SIZE + 1,
    };
    HiSysEventParam params[] = { param };
    size_t len = sizeof(params) / sizeof(params[0]);
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params, len);
    ASSERT_EQ(res, ERR_ARRAY_TOO_MUCH);
}

/**
 * @tc.name: HiSysEventCTest012
 * @tc.desc: Test writing of oversize event.
 * @tc.type: FUNC
 * @tc.require: issueI5O9JB
 */
HWTEST_F(HiSysEventCTest, HiSysEventCTest012, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event.
     * @tc.steps: step2. write event.
     * @tc.steps: step3. check the result of writing.
     */
    size_t strLen = MAX_STRING_LENGTH;
    char* longStr = new char[strLen + 1];
    for (size_t i = 0; i < strLen; ++i) {
        longStr[i] = 'a';
    }
    longStr[strLen] = '\0';
    char* srts[] = { longStr, longStr };
    HiSysEventParam param = {
        .name = "KEY_STRING_ARR",
        .t = HISYSEVENT_STRING_ARRAY,
        .v = { .array = srts },
        .arraySize = 2,
    };
    HiSysEventParam params[] = { param };
    size_t len = sizeof(params) / sizeof(params[0]);
    int res = OH_HiSysEvent_Write(TEST_DOMAIN, TEST_NAME, HISYSEVENT_BEHAVIOR, params, len);
    ASSERT_EQ(res, ERR_OVER_SIZE);
    delete[] longStr;
}
