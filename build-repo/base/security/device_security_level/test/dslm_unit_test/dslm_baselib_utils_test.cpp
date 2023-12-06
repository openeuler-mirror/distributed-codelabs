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

#include "dslm_baselib_utils_test.h"

#include <gtest/gtest.h>

#include "file_ex.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"

#include "utils_base64.h"
#include "utils_datetime.h"
#include "utils_hexstring.h"
#include "utils_json.h"
#include "utils_mem.h"
#include "utils_state_machine.h"
#include "utils_timer.h"
#include "utils_tlv.h"

#define MAX_ENTRY 8
#define MAX_MALLOC_LEN (1 * 1024 * 1024)

using namespace std;
using namespace std::chrono;
using namespace testing;
using namespace testing::ext;

// for testing
extern "C" {
extern void DoTimerProcess(TimerProc callback, const void *context);
}

namespace OHOS {
namespace Security {
namespace DslmUnitTest {
void DslmBaselibUtilsTest::SetUpTestCase()
{
}
void DslmBaselibUtilsTest::TearDownTestCase()
{
}
void DslmBaselibUtilsTest::SetUp()
{
}
void DslmBaselibUtilsTest::TearDown()
{
}

/**
 * @tc.name: Deserialize_case1
 * @tc.desc: function Deserialize with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, Deserialize_case1, TestSize.Level0)
{
    uint32_t cnt = 0;
    TlvCommon tlvs[MAX_ENTRY];
    // every entry has a sizeof(void *)-byte value
    uint8_t buff[MAX_ENTRY * sizeof(TlvCommon)] = {0};

    {
        cnt = 0;
        (void)memset_s(&buff[0], sizeof(buff), 0, sizeof(buff));
        (void)memset_s(&tlvs[0], sizeof(tlvs), 0, sizeof(tlvs));

        uint32_t ret = Deserialize(nullptr, sizeof(buff), &tlvs[0], MAX_ENTRY, &cnt);
        EXPECT_EQ((uint32_t)TLV_ERR_INVALID_PARA, ret);
    }

    {
        // buff contains 10 entries which greater than MAX_ENTRY
        int i;
        // every entry has a sizeof(void *)-byte value
        uint8_t buff[(MAX_ENTRY + 2) * sizeof(TlvCommon)] = {0};

        cnt = 0;
        (void)memset_s(&tlvs[0], sizeof(tlvs), 0, sizeof(tlvs));

        for (i = 0; i < (MAX_ENTRY + 2); i++) {
            TlvCommon *ptr = (TlvCommon *)buff + i;
            ptr->tag = 0x105;
            ptr->len = 4;
        }

        uint32_t ret = Deserialize(buff, sizeof(buff), &tlvs[0], MAX_ENTRY, &cnt);
        EXPECT_EQ((uint32_t)TLV_ERR_BUFF_NO_ENOUGH, ret);
        EXPECT_EQ(0U, cnt);
    }
}

/**
 * @tc.name: Deserialize_case2
 * @tc.desc: function Deserialize with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, Deserialize_case2, TestSize.Level0)
{
    int i;
    uint32_t cnt = 0;
    TlvCommon tlvs[MAX_ENTRY];
    // every entry has a sizeof(void *)-byte value
    uint8_t buff[MAX_ENTRY * sizeof(TlvCommon)] = {0};

    // malformed tlv entry's len
    cnt = 0;
    (void)memset_s(&buff[0], sizeof(buff), 0, sizeof(buff));
    (void)memset_s(&tlvs[0], sizeof(tlvs), 0, sizeof(tlvs));

    for (i = 0; i < MAX_ENTRY; i++) {
        TlvCommon *ptr = (TlvCommon *)buff + i;
        ptr->tag = 0x105;
        ptr->len = 0x100;
    }

    uint32_t ret = Deserialize(buff, sizeof(buff), &tlvs[0], MAX_ENTRY, &cnt);
    EXPECT_EQ(0U, ret);
    EXPECT_EQ(0U, cnt);
}

/**
 * @tc.name: Serialize_case1
 * @tc.desc: function Serialize with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, Serialize_case1, TestSize.Level0)
{
    uint32_t size = 0;
    int i = 0;
    TlvCommon tlvs[MAX_ENTRY];
    // every entry has a sizeof(void *)-byte value
    uint8_t buff[MAX_ENTRY * sizeof(TlvCommon)] = {0};

    {
        size = 0;
        uint32_t ret = Serialize(nullptr, MAX_ENTRY, buff, sizeof(buff), &size);
        EXPECT_EQ((uint32_t)TLV_ERR_INVALID_PARA, ret);
    }

    {
        // malformed max buffer size

        size = 0;
        (void)memset_s(&buff[0], sizeof(buff), 0, sizeof(buff));
        (void)memset_s(&tlvs[0], sizeof(tlvs), 0, sizeof(tlvs));

        for (i = 0; i < MAX_ENTRY; i++) {
            TlvCommon *ptr = (TlvCommon *)tlvs + i;
            ptr->tag = 0x105;
            ptr->len = 4;
        }

        uint32_t ret = Serialize(tlvs, MAX_ENTRY, buff, 1, &size);
        EXPECT_EQ((uint32_t)TLV_ERR_BUFF_NO_ENOUGH, ret);
        EXPECT_EQ(0U, size);
    }
}

/**
 * @tc.name: Serialize_case1
 * @tc.desc: function Serialize with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, Serialize_case2, TestSize.Level0)
{
    uint32_t size = 0;
    int i = 0;
    TlvCommon tlvs[MAX_ENTRY];
    // every entry has a sizeof(void *)-byte value
    uint8_t buff[MAX_ENTRY * sizeof(TlvCommon)] = {0};
    {
        // malformed tvl.len
        size = 0;
        (void)memset_s(&buff[0], sizeof(buff), 0, sizeof(buff));
        (void)memset_s(&tlvs[0], sizeof(tlvs), 0, sizeof(tlvs));

        for (i = 0; i < MAX_ENTRY; i++) {
            TlvCommon *ptr = (TlvCommon *)tlvs + i;
            ptr->tag = 0x105;
            ptr->len = 0;
        }

        uint32_t ret = Serialize(tlvs, MAX_ENTRY, buff, sizeof(buff), &size);
        EXPECT_EQ(0U, ret);
        // (TLV_TLV_HEAD_LEN + ptr->len) * MAX_ENTRY
        EXPECT_EQ(32U, size);
    }

    {
        // malformed tvl.value
        size = 0;
        (void)memset_s(&buff[0], sizeof(buff), 0, sizeof(buff));
        (void)memset_s(&tlvs[0], sizeof(tlvs), 0, sizeof(tlvs));

        for (i = 0; i < MAX_ENTRY; i++) {
            TlvCommon *ptr = (TlvCommon *)tlvs + i;
            ptr->tag = 0x105;
            ptr->len = 4;
            ptr->value = nullptr;
        }

        uint32_t ret = Serialize(tlvs, MAX_ENTRY, buff, sizeof(buff), &size);
        EXPECT_EQ(0U, ret);
        // (TLV_TLV_HEAD_LEN + ptr->len) * MAX_ENTRY
        EXPECT_EQ(64U, size);
    }
}

/**
 * @tc.name: GetDateTimeByMillisecondSince1970_case1
 * @tc.desc: function GetDateTimeByMillisecondSince1970 with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, GetDateTimeByMillisecondSince1970_case1, TestSize.Level0)
{
    uint64_t input = 0;

    {
        EXPECT_EQ(false, GetDateTimeByMillisecondSince1970(input, nullptr));
        EXPECT_EQ(false, GetDateTimeByMillisecondSinceBoot(input, nullptr));
    }

    {
        DoTimerProcess(nullptr, nullptr);
    }
}

/**
 * @tc.name: Base64UrlDecodeApp_case1
 * @tc.desc: function Base64UrlDecodeApp with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, Base64UrlDecodeApp_case1, TestSize.Level0)
{
    uint8_t src[] = {'a', '-', '_', 'd', '\0'};
    uint8_t *to = nullptr;

    {
        int32_t ret = Base64UrlDecodeApp(nullptr, &to);
        EXPECT_EQ(0, ret);
    }

    {
        int32_t ret = Base64UrlDecodeApp(src, &to);
        EXPECT_EQ(3, ret);
        EXPECT_EQ('\xEF', to[1]);
        EXPECT_EQ('\xDD', to[2]);
        FREE(to);
    }
}

/**
 * @tc.name: Base64DecodeApp_case1
 * @tc.desc: function Base64DecodeApp with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, Base64DecodeApp_case1, TestSize.Level0)
{
    uint8_t *to = nullptr;

    {
        int32_t ret = Base64DecodeApp(nullptr, &to);
        EXPECT_EQ(0, ret);
    }

    {
        uint32_t maxStrLen = MAX_MALLOC_LEN / 3 * 4 + 10;
        uint8_t *maxStr = (uint8_t *)MALLOC(sizeof(uint8_t) * maxStrLen);
        ASSERT_NE(nullptr, maxStr);

        memset_s(maxStr, maxStrLen - 1, 'c', maxStrLen - 1);

        int32_t ret = Base64DecodeApp(maxStr, &to);
        EXPECT_EQ(0, ret);
        FREE(maxStr);
    }
}

/**
 * @tc.name: Base64EncodeApp_case1
 * @tc.desc: function Base64EncodeApp with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, Base64EncodeApp_case1, TestSize.Level0)
{
    uint8_t src[] = {'a', 'b', 'c', 'd', '\0'};

    EXPECT_EQ(nullptr, Base64EncodeApp(nullptr, sizeof(src)));
    EXPECT_EQ(nullptr, Base64EncodeApp(src, 0));

    {
        uint32_t maxStrLen = MAX_MALLOC_LEN / 4 * 3;
        EXPECT_EQ(nullptr, Base64EncodeApp(src, maxStrLen));
    }
}

/**
 * @tc.name: InitStateMachine_case1
 * @tc.desc: function InitStateMachine with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, InitStateMachine_case1, TestSize.Level0)
{
    {
        StateMachine *machine = nullptr;
        uint32_t machineId = 0;
        uint32_t initState = 0;

        InitStateMachine(machine, machineId, initState);
    }

    {
        StateNode *node = nullptr;
        uint32_t nodeCnt = 0;
        StateMachine machine;
        uint32_t event = 0;

        ScheduleMachine(node, nodeCnt, &machine, event, nullptr);
    }
}

/**
 * @tc.name: DestroyJson_case1
 * @tc.desc: function DestroyJson with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, DestroyJson_case1, TestSize.Level0)
{
    int32_t arr[10] = {0};
    const char *field = "test";
    const char *value = "add";

    const char *str = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2}";
    JsonHandle handle = CreateJson(str);

    DestroyJson(nullptr);
    AddFieldBoolToJson(nullptr, field, true);
    AddFieldIntToJson(nullptr, field, 0);
    AddFieldIntArrayToJson(nullptr, field, arr, sizeof(arr));
    AddFieldStringToJson(nullptr, field, value);
    AddFieldJsonToJson(handle, field, nullptr);

    {
        int32_t ret = GetJsonFieldInt(nullptr, str);
        EXPECT_EQ(-1, ret);

        ret = GetJsonFieldInt(handle, nullptr);
        EXPECT_EQ(0, ret);
    }

    {
        uint32_t ret = GetJsonFieldIntArray(nullptr, str, arr, sizeof(arr));
        EXPECT_EQ(0U, ret);
    }

    {
        const char *ret = GetJsonFieldString(nullptr, str);
        EXPECT_EQ(nullptr, ret);
    }

    {
        const char *ret = GetJsonFieldString(handle, nullptr);
        EXPECT_EQ(nullptr, ret);
    }

    EXPECT_EQ(false, CompareJsonData(handle, nullptr, true));

    FREE(handle);
}

/**
 * @tc.name: GetJsonFieldInt_case1
 * @tc.desc: function GetJsonFieldInt with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, GetJsonFieldInt_case1, TestSize.Level0)
{
    const char *str = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2}";
    JsonHandle handle = CreateJson(str);

    int32_t ret = GetJsonFieldInt(handle, "challenge");
    EXPECT_EQ(-1, ret);
    FREE(handle);
}

/**
 * @tc.name: GetJsonFieldIntArray_case1
 * @tc.desc: function GetJsonFieldIntArray with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, GetJsonFieldIntArray_case1, TestSize.Level0)
{
    int32_t arr[5] = {0};
    int32_t arrLen = sizeof(arr) / sizeof(arr[0]);
    const char *str = "{\"version\":131072,\"challenge\":\"test challenge\",\"arr\":[\"3C1F21EE53D3C4E2\", \"elem2\", "
                      "3, 4, 5],\"type\":2}";
    JsonHandle handle = CreateJson(str);

    {
        int32_t ret = GetJsonFieldIntArray(handle, "challenge", arr, arrLen);
        EXPECT_EQ(0U, ret);
    }

    {
        int32_t ret = GetJsonFieldIntArray(handle, "arr", arr, arrLen - 2);
        EXPECT_EQ(1U, ret);
    }

    FREE(handle);
}

/**
 * @tc.name: ByteToHexString_case1
 * @tc.desc: function ByteToHexString with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, ByteToHexString_case1, TestSize.Level0)
{
    const uint8_t hex[] = {0x1, 0xF, 0xE, 0x8, 0xA};
    uint32_t hexLen = sizeof(hex);
    uint8_t str[10] = {0};
    uint32_t strLen = 10;

    ByteToHexString(nullptr, 0, str, strLen);
    EXPECT_EQ(0U, str[0]);

    ByteToHexString(hex, hexLen, nullptr, 0);
    EXPECT_EQ(0U, str[0]);

    ByteToHexString(hex, hexLen, str, strLen - 1);
    EXPECT_EQ(0U, str[0]);
}

/**
 * @tc.name: HexStringToByte_case1
 * @tc.desc: function HexStringToByte with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmBaselibUtilsTest, HexStringToByte_case1, TestSize.Level0)
{
    uint32_t hexLen = 5;
    uint32_t strLen = 10;
    uint8_t hex[5] = {0};
    char str[10] = {0};

    {
        int32_t ret = HexStringToByte(nullptr, 0, hex, hexLen);
        EXPECT_EQ(-1, ret);
        EXPECT_EQ(0U, str[0]);
    }

    {
        int32_t ret = HexStringToByte(str, strLen, nullptr, 0);
        EXPECT_EQ(-1, ret);
        EXPECT_EQ(0U, str[0]);
    }

    {
        int32_t ret = HexStringToByte(str, strLen, hex, hexLen - 2);
        EXPECT_EQ(-1, ret);
        EXPECT_EQ(0U, str[0]);
    }
}
} // namespace DslmUnitTest
} // namespace Security
} // namespace OHOS