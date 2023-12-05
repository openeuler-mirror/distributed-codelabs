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

#include "dslm_test.h"

#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <iostream>
#include <mutex>
#include <thread>

#include "file_ex.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"

#include "device_security_defines.h"
#include "device_security_info.h"
#include "device_security_level_defines.h"
#include "dslm_core_defines.h"
#include "dslm_core_process.h"
#include "dslm_credential.h"
#include "dslm_crypto.h"
#include "dslm_device_list.h"
#include "dslm_fsm_process.h"
#include "dslm_hievent.h"
#include "dslm_inner_process.h"
#include "dslm_messenger_wrapper.h"
#include "dslm_msg_interface_mock.h"
#include "dslm_msg_serialize.h"
#include "dslm_msg_utils.h"
#include "dslm_ohos_request.h"
#include "dslm_ohos_verify.h"
#include "dslm_request_callback_mock.h"
#include "utils_datetime.h"
#include "utils_mem.h"

using namespace std;
using namespace std::chrono;
using namespace testing;
using namespace testing::ext;

// for testing
extern "C" {
extern bool CheckMessage(const uint8_t *msg, uint32_t length);
extern void DoTimerProcess(TimerProc callback, const void *context);
}

namespace OHOS {
namespace Security {
namespace DslmUnitTest {
void DslmTest::SetUpTestCase()
{
    // modify the device's systime to ensure that the certificate verification passes
    constexpr time_t yearTimeLeast = 1640966400;
    constexpr time_t yearTimeValid = 1648518888;
    struct timeval timeVal = {0};
    gettimeofday(&timeVal, nullptr);
    if (timeVal.tv_sec <= yearTimeLeast) {
        timeVal.tv_sec = yearTimeValid;
        settimeofday(&timeVal, nullptr);
    }

    static const char *acls[] = {"ACCESS_IDS"};
    static const char *perms[] = {
        "ohos.permission.PLACE_CALL",
        "ohos.permission.ACCESS_IDS",
    };
    uint64_t tokenId;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 1,
        .dcaps = nullptr,
        .perms = perms,
        .acls = acls,
        .processName = "dslm_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    SaveStringToFile("/sys/fs/selinux/enforce", "0");
}
void DslmTest::TearDownTestCase()
{
    SaveStringToFile("/sys/fs/selinux/enforce", "1");
}
void DslmTest::SetUp()
{
}
void DslmTest::TearDown()
{
}

static void BlockCheckDeviceStatus(const DeviceIdentify *device, uint32_t status, uint64_t millisec)
{
    static int sleepTick = 10;
    uint64_t cnt = millisec / static_cast<uint64_t>(sleepTick) + 1;
    do {
        const DslmDeviceInfo *info = GetDslmDeviceInfo(device);
        if (info == nullptr) {
            continue;
        }
        if (info->machine.currState == status) {
            break;
        }
        if (cnt == 0) {
            break;
        }
        this_thread::sleep_for(milliseconds(sleepTick));
        cnt--;
    } while (true);
}

HWTEST_F(DslmTest, BuildDeviceSecInfoRequest_case1, TestSize.Level0)
{
    uint64_t random = 0x0807060504030201;
    MessageBuff *msg = nullptr;
    // 0d196608 = 0x030000
    const char *except =
        "{\"message\":1,\"payload\":{\"version\":196608,\"challenge\":\"0102030405060708\",\"support\":[3000,2000]}}";
    int32_t ret = BuildDeviceSecInfoRequest(random, &msg);
    ASSERT_EQ(0, ret);
    EXPECT_STREQ(except, (const char *)msg->buff);
    FreeMessageBuff(msg);
}

HWTEST_F(DslmTest, BuildDeviceSecInfoRequest_case2, TestSize.Level0)
{
    uint64_t random = 0x0807060504030201;
    MessageBuff **msg = nullptr;
    int32_t ret = BuildDeviceSecInfoRequest(random, msg);
    ASSERT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, BuildDeviceSecInfoRequest_case3, TestSize.Level0)
{
    uint64_t random = 0x0807060504030201;
    const char *message = "{\"mege\":1,\"payload\":{\"version\":131072,\"challenge\":\"0102030405060708\"}}";
    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};
    MessageBuff *msg_ptr = &msg;

    int32_t ret = BuildDeviceSecInfoRequest(random, &msg_ptr);
    ASSERT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, BuildDeviceSecInfoResponse_case1, TestSize.Level0)
{
    uint64_t random = 0x0807060504030201;
    uint8_t info[] = {'a', 'b', 'c', 'd', 1, 3, 5, 7, 9};
    DslmCredBuff cred = {(CredType)3, 9, info};

    // 0d196608 = 0x030000
    const char *except = "{\"message\":2,\"payload\":{\"version\":196608,\"type\":3,\"challenge\":\"0102030405060708\","
                         "\"info\":\"YWJjZAEDBQcJ\"}}";

    MessageBuff *msg = nullptr;
    int32_t ret = BuildDeviceSecInfoResponse(random, (DslmCredBuff *)&cred, &msg);
    ASSERT_EQ(0, ret);

    EXPECT_STREQ(except, (const char *)msg->buff);
    FreeMessageBuff(msg);
}

HWTEST_F(DslmTest, BuildDeviceSecInfoResponse_case2, TestSize.Level0)
{
    uint64_t random = 0x0807060504030201;
    uint8_t info[] = {'a', 'b', 'c', 'd', 1, 3, 5, 7, 9};
    DslmCredBuff cred = {(CredType)3, 9, info};

    {
        MessageBuff **msg = nullptr;

        int32_t ret = BuildDeviceSecInfoResponse(random, (DslmCredBuff *)&cred, msg);
        ASSERT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        MessageBuff msg;
        memset_s(&msg, sizeof(MessageBuff), 0, sizeof(MessageBuff));
        MessageBuff *msgPtr = &msg;

        int32_t ret = BuildDeviceSecInfoResponse(random, (DslmCredBuff *)&cred, &msgPtr);
        ASSERT_EQ(ERR_INVALID_PARA, ret);
    }
}

HWTEST_F(DslmTest, ParseMessage_case1, TestSize.Level0)
{
    const char *message = "{\"message\":1,\"payload\":{\"version\":131072,\"challenge\":\"0102030405060708\"}}";
    const char *except = "{\"version\":131072,\"challenge\":\"0102030405060708\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    MessagePacket *packet = ParseMessage(&msg);
    ASSERT_NE(nullptr, packet);

    EXPECT_EQ(1U, packet->type);
    EXPECT_STREQ(except, (const char *)packet->payload);

    FreeMessagePacket(packet);
}

HWTEST_F(DslmTest, ParseMessage_case2, TestSize.Level0)
{
    const char *message = "{\"mege\":1,\"payload\":{\"version\":131072,\"challenge\":\"0102030405060708\"}}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    MessagePacket *packet = ParseMessage(&msg);
    EXPECT_EQ(nullptr, packet);
    FreeMessagePacket(packet);
}

HWTEST_F(DslmTest, ParseMessage_case3, TestSize.Level0)
{
    const char *message = "{\"message\":1,\"pay\":{\"version\":131072,\"challenge\":\"0102030405060708\"}}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    MessagePacket *packet = ParseMessage(&msg);
    EXPECT_EQ(nullptr, packet);
    FreeMessagePacket(packet);
}

HWTEST_F(DslmTest, ParseMessage_case4, TestSize.Level0)
{
    const MessageBuff *buff = nullptr;
    EXPECT_EQ(nullptr, ParseMessage(buff));
}

HWTEST_F(DslmTest, ParseMessage_case5, TestSize.Level0)
{
    uint8_t *message = nullptr;
    uint32_t messageLen = 0;
    MessageBuff msg = {.length = messageLen, .buff = message};

    EXPECT_EQ(nullptr, ParseMessage(&msg));
}

HWTEST_F(DslmTest, ParseMessage_case6, TestSize.Level0)
{
    uint8_t message[] = {'1', '2'};
    uint32_t messageLen = 2;
    MessageBuff msg = {.length = messageLen, .buff = message};
    EXPECT_EQ(nullptr, ParseMessage(&msg));
}

HWTEST_F(DslmTest, ParseMessage_case7, TestSize.Level0)
{
    uint8_t message[] = {1, 2, 0};
    uint32_t messageLen = 3;
    MessageBuff msg = {.length = messageLen, .buff = message};
    EXPECT_EQ(nullptr, ParseMessage(&msg));
}

HWTEST_F(DslmTest, ParseDeviceSecInfoRequest_case1, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challenge\":\"010203040a0b0c0d\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    RequestObject obj;
    (void)memset_s(&obj, sizeof(RequestObject), 0, sizeof(RequestObject));

    // 3351057 = 0x332211
    int32_t ret = ParseDeviceSecInfoRequest(&msg, &obj);
    EXPECT_EQ(0, ret);

    EXPECT_EQ(0x332211U, obj.version);
    EXPECT_EQ(0x0d0c0b0a04030201UL, obj.challenge);
    EXPECT_EQ(0U, obj.arraySize);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoRequest_case2, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challenge\":\"z\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    RequestObject obj;
    (void)memset_s(&obj, sizeof(RequestObject), 0, sizeof(RequestObject));

    int32_t ret = ParseDeviceSecInfoRequest(&msg, &obj);
    EXPECT_EQ(ERR_NO_CHALLENGE, ret);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoRequest_case3, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challenge\":1}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    RequestObject obj;
    (void)memset_s(&obj, sizeof(RequestObject), 0, sizeof(RequestObject));
    int32_t ret = ParseDeviceSecInfoRequest(&msg, &obj);
    EXPECT_EQ(ERR_NO_CHALLENGE, ret);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoRequest_case4, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challssenge\":\"z\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    RequestObject obj;
    (void)memset_s(&obj, sizeof(RequestObject), 0, sizeof(RequestObject));
    int32_t ret = ParseDeviceSecInfoRequest(&msg, &obj);
    EXPECT_EQ(ERR_NO_CHALLENGE, ret);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoRequest_case5, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challenge\":\"010203040a0b0c0d\",\"support\":[33,44,55]}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    RequestObject obj;
    (void)memset_s(&obj, sizeof(RequestObject), 0, sizeof(RequestObject));

    // 3351057 = 0x332211
    int32_t ret = ParseDeviceSecInfoRequest(&msg, &obj);
    EXPECT_EQ(static_cast<int32_t>(0), ret);
    EXPECT_EQ(0x332211U, obj.version);
    EXPECT_EQ(0x0d0c0b0a04030201U, obj.challenge);
    // add support
    EXPECT_EQ(3U, obj.arraySize);
    EXPECT_EQ(33U, obj.credArray[0]);
    EXPECT_EQ(44U, obj.credArray[1]);
    EXPECT_EQ(55U, obj.credArray[2]);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoRequest_case6, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challenge\":\"010203040a0b0c0d\",\"support\":[]}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    RequestObject obj;
    (void)memset_s(&obj, sizeof(RequestObject), 0, sizeof(RequestObject));

    // 3351057 = 0x332211
    int32_t ret = ParseDeviceSecInfoRequest(&msg, &obj);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0x332211U, obj.version);
    EXPECT_EQ(0x0d0c0b0a04030201U, obj.challenge);
    // add support
    EXPECT_EQ(0U, obj.arraySize);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoRequest_case7, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challenge\":\"010203040a0b0c0d\",\"support\":[]}";
    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    RequestObject obj;
    (void)memset_s(&obj, sizeof(RequestObject), 0, sizeof(RequestObject));

    {
        int32_t ret = ParseDeviceSecInfoRequest(nullptr, &obj);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        int32_t ret = ParseDeviceSecInfoRequest(&msg, nullptr);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        int32_t ret = ParseDeviceSecInfoRequest(nullptr, &obj);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        msg.buff = nullptr;
        int32_t ret = ParseDeviceSecInfoRequest(&msg, &obj);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }
}

HWTEST_F(DslmTest, ParseDeviceSecInfoRequest_case8, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challenge}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    RequestObject obj;
    (void)memset_s(&obj, sizeof(RequestObject), 0, sizeof(RequestObject));

    // 3351057 = 0x332211
    int32_t ret = ParseDeviceSecInfoRequest(&msg, &obj);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoResponse_case1, TestSize.Level0)
{
    const char *message = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"info\":"
                          "\"SkFERS1BTDAwOjg3QUQyOEQzQjFCLi4u\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    uint64_t challenge;
    uint32_t version;
    DslmCredBuff *cred = nullptr;

    // 131072 = 0x020000
    int32_t ret = ParseDeviceSecInfoResponse(&msg, &challenge, &version, &cred);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0x020000U, version);

    EXPECT_EQ(0xE2C4D353EE211F3CUL, challenge);

    const char *except = "JADE-AL00:87AD28D3B1B...";
    EXPECT_NE(nullptr, cred);
    EXPECT_EQ(2U, cred->type);
    EXPECT_EQ(strlen(except), cred->credLen);
    EXPECT_EQ(0, strncmp(except, (const char *)cred->credVal, cred->credLen));
    DestroyDslmCred(cred);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoResponse_case2, TestSize.Level0)
{
    const char *message = "{\"version\":3351057,\"challssenge\":\"z\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    uint64_t challenge;
    uint32_t ver;
    DslmCredBuff *cred = nullptr;

    int32_t ret = ParseDeviceSecInfoResponse(&msg, &challenge, &ver, &cred);
    EXPECT_EQ(ERR_NO_CHALLENGE, ret);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoResponse_case3, TestSize.Level0)
{
    const char *message =
        "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"infod\":\"JADE-AL00:87AD28D3B1B...\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    uint64_t challenge;
    uint32_t ver;
    DslmCredBuff *cred = nullptr;

    int32_t ret = ParseDeviceSecInfoResponse(&msg, &challenge, &ver, &cred);
    EXPECT_EQ(ERR_NO_CRED, ret);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoResponse_case4, TestSize.Level0)
{
    const char *message =
        "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"infod\":\"JADE-AL00:87AD28D3B1B...\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    uint64_t challenge;
    uint32_t ver;
    DslmCredBuff **cred = nullptr;

    int32_t ret = ParseDeviceSecInfoResponse(&msg, &challenge, &ver, cred);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoResponse_case5, TestSize.Level0)
{
    const char *message = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"info\":"
                          "\"SkFERS1BTDAwOjg3QUQyOEQzQjFCLi4u\"}";

    uint32_t messageLen = strlen(message) + 1;
    // msg has null buff
    MessageBuff msg = {.length = messageLen, .buff = nullptr};

    uint64_t challenge;
    uint32_t version;
    DslmCredBuff *cred = nullptr;

    // 131072 = 0x020000
    int32_t ret = ParseDeviceSecInfoResponse(&msg, &challenge, &version, &cred);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, ParseDeviceSecInfoResponse_case6, TestSize.Level0)
{
    int32_t ret;
    const char *message = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"info\":"
                          "\"SkFERS1BTDAwOjg3QUQyOEQzQjFCLi4u\"}";
    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    uint64_t challenge;
    uint32_t ver;
    DslmCredBuff cred;
    memset_s(&cred, sizeof(DslmCredBuff), 0, sizeof(DslmCredBuff));
    DslmCredBuff *credPtr = &cred;

    {
        // malformed inputs
        ret = ParseDeviceSecInfoResponse(&msg, &challenge, &ver, nullptr);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        // malformed inputs, credPtr != nullptr
        ret = ParseDeviceSecInfoResponse(&msg, &challenge, &ver, &credPtr);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }
}

HWTEST_F(DslmTest, ParseDeviceSecInfoResponse_case7, TestSize.Level0)
{
    int32_t ret;
    uint64_t challenge;
    uint32_t version;
    DslmCredBuff *cred = nullptr;

    {
        // malformed challenge
        const char *message = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2A\",\"type\":2,\"info\":"
                              "\"SkFERS1BTDAwOjg3QUQyOEQzQjFCLi4u\"}";
        uint32_t messageLen = strlen(message) + 1;
        MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

        // 131072 = 0x020000
        ret = ParseDeviceSecInfoResponse(&msg, &challenge, &version, &cred);
        EXPECT_EQ(ERR_NO_CHALLENGE, ret);
    }

    {
        // malformed json
        const char *message = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"infod}";
        uint32_t messageLen = strlen(message) + 1;
        MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

        ret = ParseDeviceSecInfoResponse(&msg, &challenge, &version, &cred);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }
}

HWTEST_F(DslmTest, ParseDeviceSecInfoResponse_case8, TestSize.Level0)
{
    // malformed info field
    const char *message = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"info\":"
                          "\"SkFERS1BTDAwOjg3QUQyOEQzQjFCLi4ux\"}";

    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    uint64_t challenge;
    uint32_t version;
    DslmCredBuff *cred = nullptr;

    // 131072 = 0x020000
    int32_t ret = ParseDeviceSecInfoResponse(&msg, &challenge, &version, &cred);
    EXPECT_EQ(ERR_NO_CRED, ret);
}

HWTEST_F(DslmTest, RandomValue_case1, TestSize.Level0)
{
    RandomValue rand1 = {0, {0}};
    (void)memset_s(&rand1, sizeof(RandomValue), 0, sizeof(RandomValue));
    GenerateRandom(&rand1, sizeof(uint64_t));

    RandomValue rand2 = {0, {0}};
    (void)memset_s(&rand2, sizeof(RandomValue), 0, sizeof(RandomValue));
    GenerateRandom(&rand2, sizeof(uint64_t));

    EXPECT_EQ(sizeof(uint64_t), rand1.length);
    EXPECT_EQ(sizeof(uint64_t), rand2.length);

    EXPECT_GT(rand1.value[0] + rand1.value[1] + rand1.value[2] + rand1.value[3], 0);
    EXPECT_EQ(rand1.value[31] + rand1.value[30] + rand1.value[29] + rand1.value[28], 0);
    EXPECT_NE(0, memcmp(rand1.value, rand2.value, sizeof(uint64_t)));
}

HWTEST_F(DslmTest, RandomValue_case2, TestSize.Level0)
{
    RandomValue rand = {0, {0}};
    (void)memset_s(&rand, sizeof(RandomValue), 0, sizeof(RandomValue));

    GenerateRandom(&rand, 1024);
    EXPECT_EQ(static_cast<uint32_t>(RANDOM_MAX_LEN), rand.length);

    GenerateRandom(nullptr, 1024);
}

HWTEST_F(DslmTest, GetMillisecondSinceBoot_case1, TestSize.Level0)
{
    uint64_t tick = 100;
    uint64_t start = GetMillisecondSinceBoot();
    EXPECT_GT(start, 0U);
    this_thread::sleep_for(milliseconds(tick));
    uint64_t end = GetMillisecondSinceBoot();
    EXPECT_GT(end, 0U);

    EXPECT_GT(end - start, tick - 10);
    EXPECT_LT(end - start, tick + 10);
}

HWTEST_F(DslmTest, GetMillisecondSince1970_case1, TestSize.Level0)
{
    uint64_t tick = 100;
    uint64_t start = GetMillisecondSince1970();
    EXPECT_GT(start, 0U);
    this_thread::sleep_for(milliseconds(tick));
    uint64_t end = GetMillisecondSince1970();
    EXPECT_GT(end, 0U);

    EXPECT_GT(end - start, tick - 10);
    EXPECT_LT(end - start, tick + 10);
}

HWTEST_F(DslmTest, GetDateTime_case1, TestSize.Level0)
{
    {
        DateTime date;
        EXPECT_TRUE(GetDateTimeByMillisecondSince1970(GetMillisecondSince1970(), &date));
    }
    {
        DateTime date;
        EXPECT_TRUE(GetDateTimeByMillisecondSinceBoot(GetMillisecondSinceBoot(), &date));
    }
}

HWTEST_F(DslmTest, InitDslmCredentialFunctions_case1, TestSize.Level0)
{
    bool ret = InitDslmCredentialFunctions(NULL);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DslmTest, OhosDslmCred_case1, TestSize.Level0)
{
    const DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {0}};
    RequestObject object;

    object.arraySize = 1;
    object.credArray[0] = CRED_TYPE_STANDARD;
    object.challenge = 0x1234567812345678;
    object.version = 0x112234;

    DslmCredBuff *cred = nullptr;

    int32_t ret = DefaultRequestDslmCred(&identify, &object, &cred);
    ASSERT_EQ(SUCCESS, static_cast<int32_t>(ret));

    DslmCredInfo info;
    (void)memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));

    ret = DefaultVerifyDslmCred(&identify, object.challenge, cred, &info);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_GE(info.credLevel, 1U);

    DestroyDslmCred(cred);
}

HWTEST_F(DslmTest, OnRequestDeviceSecLevelInfo_case1, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'g'}};

    const RequestOption option = {
        .challenge = 0xffffffffffffffff,
        .timeout = 2,
    };

    {
        uint32_t cookie = 1234;
        DslmRequestCallbackMock mockCallback;
        EXPECT_CALL(mockCallback, RequestCallback(_, _, _)).Times(Exactly(0));
        int32_t ret = OnRequestDeviceSecLevelInfo(&device, &option, 0, cookie, DslmRequestCallbackMock::MockedCallback);
        EXPECT_EQ(static_cast<int32_t>(ret), ERR_MSG_NOT_INIT);
    }

    {
        uint32_t cookie = 5678;
        DslmMsgInterfaceMock mockMsg;
        DslmRequestCallbackMock mockCallback;
        EXPECT_CALL(mockMsg, IsMessengerReady(_)).Times(AtLeast(1));
        EXPECT_CALL(mockMsg, GetSelfDeviceIdentify(_, _, _)).Times(AtLeast(1));
        EXPECT_CALL(mockMsg, GetDeviceOnlineStatus(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(false));
        EXPECT_CALL(mockCallback, RequestCallback(_, _, _)).Times(Exactly(0));
        int32_t ret = OnRequestDeviceSecLevelInfo(&device, &option, 0, cookie, DslmRequestCallbackMock::MockedCallback);
        EXPECT_EQ(static_cast<int32_t>(ret), ERR_NOEXIST_DEVICE);

        EXPECT_CALL(mockMsg, SendMsgTo(_, _, _, _, _)).Times(AtLeast(2));
        mockMsg.MakeMsgLoopback();
        mockMsg.MakeDeviceOnline(&device);
        BlockCheckDeviceStatus(&device, STATE_SUCCESS, 10000);
        mockMsg.MakeDeviceOffline(&device);
    }
}

HWTEST_F(DslmTest, OnRequestDeviceSecLevelInfo_case2, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'g'}};

    const RequestOption option = {
        .challenge = 0xffffffffffffffff,
        .timeout = 2,
    };

    {
        uint32_t cookie = 0xabcd;
        DslmMsgInterfaceMock mockMsg;
        EXPECT_CALL(mockMsg, IsMessengerReady(_)).Times(AtLeast(1));
        EXPECT_CALL(mockMsg, GetSelfDeviceIdentify(_, _, _)).Times(AtLeast(1));
        EXPECT_CALL(mockMsg, GetDeviceOnlineStatus(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(true));
        EXPECT_CALL(mockMsg, SendMsgTo(_, _, _, _, _)).Times(Exactly(0));
        DslmRequestCallbackMock mockCallback;
        auto isRightLevel = [](const DslmCallbackInfo *info) { return info->level >= 1; };
        EXPECT_CALL(mockCallback, RequestCallback(cookie, 0, Truly(isRightLevel))).Times(Exactly(1));

        int32_t ret = OnRequestDeviceSecLevelInfo(&device, &option, 0, cookie, DslmRequestCallbackMock::MockedCallback);
        EXPECT_EQ(ret, 0);
        mockMsg.MakeDeviceOffline(&device);
    }

    {
        const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {0, 'b', 'c', 'd', 'e', 'f', 'g'}};
        uint32_t cookie = 0xabcd;
        DslmMsgInterfaceMock mockMsg;
        EXPECT_CALL(mockMsg, IsMessengerReady(_)).Times(AtLeast(1));
        EXPECT_CALL(mockMsg, GetSelfDeviceIdentify(_, _, _)).Times(AtLeast(1));
        EXPECT_CALL(mockMsg, SendMsgTo(_, _, _, _, _)).Times(Exactly(0));
        DslmRequestCallbackMock mockCallback;
        auto isRightLevel = [](const DslmCallbackInfo *info) { return info->level >= 1; };
        EXPECT_CALL(mockCallback, RequestCallback(cookie, 0, Truly(isRightLevel))).Times(Exactly(1));

        int32_t ret = OnRequestDeviceSecLevelInfo(&device, &option, 0, cookie, DslmRequestCallbackMock::MockedCallback);
        EXPECT_EQ(ret, 0);
        mockMsg.MakeDeviceOffline(&device);
    }
}

HWTEST_F(DslmTest, OnRequestDeviceSecLevelInfo_case3, TestSize.Level0)
{
    constexpr uint32_t maxNotifySize = 64;

    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a'}};
    const RequestOption option = {
        .challenge = 0xffabcdffffffffee,
        .timeout = 2,
        .extra = 0,
    };

    DslmMsgInterfaceMock mockMsg;
    EXPECT_CALL(mockMsg, IsMessengerReady(_)).Times(AtLeast(1));
    EXPECT_CALL(mockMsg, GetSelfDeviceIdentify(_, _, _)).Times(AtLeast(1));
    EXPECT_CALL(mockMsg, GetDeviceOnlineStatus(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(true));
    auto isSendRequestOut = [](const uint8_t *message) {
        const char *prefix = "{\"message\":1,\"payload\":{\"version\":196608,\"challenge\":\"";
        const string msg = string(static_cast<const char *>(static_cast<const void *>(message)));
        EXPECT_EQ(msg.rfind(prefix, 0), 0U);
        return true;
    };

    uint32_t cookie = 0x4567;
    EXPECT_CALL(mockMsg, SendMsgTo(_, _, _, Truly(isSendRequestOut), _)).Times(Exactly(1)).WillRepeatedly(Return(true));

    DslmDeviceInfo *info = CreatOrGetDslmDeviceInfo(&device);
    ASSERT_NE(info, nullptr);

    EXPECT_EQ(info->notifyListSize, 0U);

    DslmRequestCallbackMock callback;
    EXPECT_CALL(callback, RequestCallback(cookie, Ne(0U), Ne(nullptr))).Times(Exactly(maxNotifySize));
    for (uint32_t i = 1; i <= maxNotifySize; i++) {
        int32_t ret = OnRequestDeviceSecLevelInfo(&device, &option, 0, cookie, DslmRequestCallbackMock::MockedCallback);
        EXPECT_EQ(static_cast<uint32_t>(ret), 0U);
        EXPECT_EQ(info->notifyListSize, i);
        EXPECT_EQ(info->historyListSize, 0U);
    }
    for (uint32_t i = 1; i <= maxNotifySize; i++) {
        int32_t ret = OnRequestDeviceSecLevelInfo(&device, &option, 0, cookie, DslmRequestCallbackMock::MockedCallback);
        EXPECT_EQ(static_cast<uint32_t>(ret), ERR_SA_BUSY);
        EXPECT_EQ(info->notifyListSize, maxNotifySize);
        EXPECT_EQ(info->historyListSize, 0U);
    }
    mockMsg.MakeDeviceOffline(&device);

    EXPECT_EQ(info->notifyListSize, 0U);
    EXPECT_EQ(info->historyListSize, 30U); // 30 is the max history list size
}

HWTEST_F(DslmTest, OnRequestDeviceSecLevelInfo_case4, TestSize.Level0)
{
    DslmMsgInterfaceMock mockMsg;
    DslmRequestCallbackMock mockCallback;

    EXPECT_CALL(mockMsg, IsMessengerReady(_)).Times(AtLeast(1));
    EXPECT_CALL(mockMsg, GetDeviceOnlineStatus(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(mockMsg, SendMsgTo(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(mockMsg, GetSelfDeviceIdentify(_, _, _)).Times(AtLeast(1));

    mutex mtx;
    condition_variable cv;
    int32_t cnt = 0;
    const time_point<system_clock> start = system_clock::now();
    const int32_t reqTimes = 3;

    uint32_t cookies[] = {0, 0x1234, 0x5678, 0xabcd};
    uint32_t timeouts[] = {0, 1, 3, 5};

    auto checkCookie = [&mtx, &cv, &cnt, &start, &cookies, &timeouts](uint32_t cookie) {
        unique_lock<mutex> lck(mtx);
        cnt++;
        cv.notify_one();
        time_point<system_clock> curr = system_clock::now();
        auto cost = duration_cast<seconds>(curr - start).count();
        EXPECT_EQ(cookie, cookies[cnt]);
        EXPECT_EQ(cost, timeouts[cnt]);
        return true;
    };

    EXPECT_CALL(mockCallback, RequestCallback(Truly(checkCookie), ERR_TIMEOUT, _)).Times(Exactly(3));

    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}};
    RequestOption option;
    for (int i = 1; i <= reqTimes; i++) {
        option.timeout = timeouts[i];
        int32_t ret =
            OnRequestDeviceSecLevelInfo(&device, &option, i, cookies[i], DslmRequestCallbackMock::MockedCallback);
        EXPECT_EQ(static_cast<uint32_t>(ret), 0U);
    }

    unique_lock<mutex> lck(mtx);
    cv.wait(lck, [&cnt]() { return (cnt == reqTimes); });
    mockMsg.MakeDeviceOffline(&device);
}

HWTEST_F(DslmTest, OnRequestDeviceSecLevelInfo_case5, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}};
    const RequestOption option = {
        .challenge = 0xffabcdffffffffee,
        .timeout = 2,
        .extra = 0,
    };
    uint32_t cookie = 1234;

    int32_t ret = OnRequestDeviceSecLevelInfo(&device, &option, 0, cookie, nullptr);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, OnPeerMsgRequestInfoReceived_case1, TestSize.Level0)
{
    const char *input = "{\"version\":65536,\"challenge\":\"0102030405060708\"}";
    uint32_t len = strlen(input) + 1;

    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'}};

    DslmMsgInterfaceMock mockMsg;

    auto isSendResponseOut = [](const uint8_t *message) {
        const string msg = string(static_cast<const char *>(static_cast<const void *>(message)));
        EXPECT_EQ(msg.find("{\"message\":2,\"payload\":{"), 0U);
        EXPECT_GT(msg.find("\"version\":"), 0U);
        EXPECT_GT(msg.find("\"challenge\":"), 0U);
        EXPECT_GT(msg.find("\"type\":"), 0U);
        EXPECT_GT(msg.find("\"info\":"), 0U);
        return true;
    };

    EXPECT_CALL(mockMsg, SendMsgTo(_, _, _, Truly(isSendResponseOut), _)).Times(Exactly(1));

    int32_t ret = OnPeerMsgRequestInfoReceived(&device, (const uint8_t *)input, len);
    EXPECT_EQ(0, static_cast<int32_t>(ret));
}

HWTEST_F(DslmTest, OnPeerMsgRequestInfoReceived_case2, TestSize.Level0)
{
    const DeviceIdentify *device = nullptr;
    const char *input = "{\"version\":65536,\"challenge\":\"0102030405060708\"}";
    uint32_t len = strlen(input) + 1;

    int32_t ret = OnPeerMsgRequestInfoReceived(device, (const uint8_t *)input, len);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, OnPeerMsgResponseInfoReceived_case1, TestSize.Level0)
{
    const char *input = "{\"version\":65536,\"type\":0,\"challenge\":\"EEFFFFFFFFCDABFF\",\"info\":"
                        "\"MDAwMTAyMDMwNDA1MDYwNzA4MDkwQTBCMEMwRDBFMEYxMDExMTIxMzE0MTUxNkFBQkJDQ0RE\"}";
    uint32_t len = strlen(input) + 1;

    DeviceIdentify device = {8, {'a', 'b', 'c', 'd', 'e', 'f', 'g'}};

    int32_t ret = OnPeerMsgResponseInfoReceived(&device, (const uint8_t *)input, len);
    EXPECT_EQ(ERR_NOEXIST_DEVICE, static_cast<int32_t>(ret));
}

HWTEST_F(DslmTest, OnPeerMsgResponseInfoReceived_case2, TestSize.Level0)
{
    const DeviceIdentify *device = nullptr;
    const char *input = "{\"version\":65536,\"challenge\":\"0102030405060708\"}";
    uint32_t len = strlen(input) + 1;

    int32_t ret = OnPeerMsgResponseInfoReceived(device, (const uint8_t *)input, len);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, InitSelfDeviceSecureLevel_case1, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'}};
    DslmDeviceInfo *info = GetDslmDeviceInfo(&device);
    EXPECT_EQ(nullptr, info);

    DslmMsgInterfaceMock mockMsg;
    mockMsg.MakeSelfDeviceId(&device);
    mockMsg.MakeMsgLoopback();
    EXPECT_CALL(mockMsg, GetSelfDeviceIdentify(_, _, _)).Times(AtLeast(1));
    InitSelfDeviceSecureLevel();

    info = GetDslmDeviceInfo(&device);
    ASSERT_NE(nullptr, info);
    EXPECT_GE(info->credInfo.credLevel, 1U);
    mockMsg.MakeDeviceOffline(&device);
}

HWTEST_F(DslmTest, InitSelfDeviceSecureLevel_case2, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'}};

    DslmDeviceInfo *info = GetDslmDeviceInfo(&device);
    EXPECT_EQ(nullptr, info);

    DslmMsgInterfaceMock mockMsg;
    EXPECT_CALL(mockMsg, SendMsgTo(_, _, _, _, _)).Times(Exactly(6));
    mockMsg.MakeDeviceOnline(&device);

    info = GetDslmDeviceInfo(&device);
    ASSERT_NE(nullptr, info);
    EXPECT_EQ(1U, info->queryTimes);
    EXPECT_EQ(STATE_WAITING_CRED_RSP, info->machine.currState);

    BlockCheckDeviceStatus(&device, STATE_SUCCESS, 5000);
    EXPECT_EQ(STATE_FAILED, info->machine.currState);
    EXPECT_LT(5U, info->queryTimes);
    mockMsg.MakeDeviceOffline(&device);
}

HWTEST_F(DslmTest, InnerKitsTest_case1, TestSize.Level0)
{
    DeviceIdentify device = {DEVICE_ID_MAX_LEN, {0}};

    DeviceSecurityInfo *info = nullptr;
    int32_t ret = RequestDeviceSecurityInfo(&device, nullptr, &info);
    EXPECT_EQ(ret, 0);
    int32_t level = 0;
    ret = GetDeviceSecurityLevelValue(info, &level);
    FreeDeviceSecurityInfo(info);
    EXPECT_EQ(ret, 0);
    EXPECT_GE(level, 1);
}

static int32_t g_cnt = 0;
static mutex g_mtx;
static condition_variable g_cv;

void TestDeviceSecurityInfoCallback(const DeviceIdentify *identify, struct DeviceSecurityInfo *info)
{
    unique_lock<mutex> lck(g_mtx);
    int32_t level = 0;
    int32_t ret = GetDeviceSecurityLevelValue(info, &level);
    FreeDeviceSecurityInfo(info);
    EXPECT_EQ(ret, 0);
    EXPECT_GE(level, 1);
    g_cnt++;
    g_cv.notify_one();
}

HWTEST_F(DslmTest, InnerKitsTest_case2, TestSize.Level0)
{
    DeviceIdentify device = {DEVICE_ID_MAX_LEN, {0}};

    g_cnt = 0;
    int ret = RequestDeviceSecurityInfoAsync(&device, nullptr, TestDeviceSecurityInfoCallback);
    EXPECT_EQ(ret, 0);

    ret = RequestDeviceSecurityInfoAsync(&device, nullptr, TestDeviceSecurityInfoCallback);
    EXPECT_EQ(ret, 0);

    ret = RequestDeviceSecurityInfoAsync(&device, nullptr, TestDeviceSecurityInfoCallback);
    EXPECT_EQ(ret, 0);

    unique_lock<mutex> lck(g_mtx);
    g_cv.wait_for(lck, std::chrono::milliseconds(2000), []() { return (g_cnt == 3); });
    EXPECT_EQ(g_cnt, 3);
}

HWTEST_F(DslmTest, InnerKitsTest_case3, TestSize.Level0)
{
    DeviceIdentify device = {DEVICE_ID_MAX_LEN, {0}};
    (void)memset_s(device.identity, DEVICE_ID_MAX_LEN, 'F', DEVICE_ID_MAX_LEN);
    DeviceSecurityInfo *info = nullptr;
    int32_t ret = RequestDeviceSecurityInfo(&device, nullptr, &info);
    EXPECT_EQ(ret, ERR_NOEXIST_DEVICE);
}

HWTEST_F(DslmTest, GetSupportedCredTypes_case1, TestSize.Level0)
{
    int32_t ret = GetSupportedCredTypes(nullptr, 0);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: GetSupportedCredTypes_case2
 * @tc.desc: function GetSupportedCredTypes with malformed inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, GetSupportedCredTypes_case2, TestSize.Level0)
{
    int32_t ret;
    CredType list[] = {CRED_TYPE_MINI, CRED_TYPE_SMALL, CRED_TYPE_STANDARD, CRED_TYPE_LARGE};
    ret = GetSupportedCredTypes(list, 2);
    EXPECT_EQ(2, ret);
}

HWTEST_F(DslmTest, CreateDslmCred_case1, TestSize.Level0)
{
    CredType type = CRED_TYPE_STANDARD;

    EXPECT_EQ(nullptr, CreateDslmCred(type, 0, nullptr));
}

HWTEST_F(DslmTest, CheckAndGenerateChallenge_case1, TestSize.Level0)
{
    DslmDeviceInfo device;
    (void)memset_s(&device, sizeof(DslmDeviceInfo), 0, sizeof(DslmDeviceInfo));

    {
        int32_t ret = CheckAndGenerateChallenge(nullptr);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        int32_t ret = CheckAndGenerateChallenge(&device);
        EXPECT_EQ(SUCCESS, ret);
    }
}

HWTEST_F(DslmTest, CheckAndGenerateChallenge_case2, TestSize.Level0)
{
    DslmDeviceInfo device;
    (void)memset_s(&device, sizeof(DslmDeviceInfo), 0, sizeof(DslmDeviceInfo));

    {
        device.nonceTimeStamp = 0xFFFFFFFFFFFFFFFF;
        int32_t ret = CheckAndGenerateChallenge(&device);
        EXPECT_EQ(SUCCESS, ret);
    }

    {
        device.nonceTimeStamp = GetMillisecondSinceBoot();
        int32_t ret = CheckAndGenerateChallenge(&device);
        EXPECT_EQ(SUCCESS, ret);
    }

    {
        device.nonceTimeStamp = GetMillisecondSinceBoot();
        device.nonce = 1;
        int32_t ret = CheckAndGenerateChallenge(&device);
        EXPECT_EQ(SUCCESS, ret);
    }
}

HWTEST_F(DslmTest, SendDeviceInfoRequest_case1, TestSize.Level0)
{
    DslmDeviceInfo *device = nullptr;

    int32_t ret = SendDeviceInfoRequest(device);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, VerifyDeviceInfoResponse_case1, TestSize.Level0)
{
    DslmDeviceInfo *device = nullptr;

    int32_t ret = VerifyDeviceInfoResponse(device, NULL);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, VerifyDeviceInfoResponse_case2, TestSize.Level0)
{
    MessageBuff msg = {.length = 0, .buff = nullptr};
    DslmDeviceInfo device;
    (void)memset_s(&device, sizeof(device), 0, sizeof(device));

    int32_t ret = VerifyDeviceInfoResponse(&device, &msg);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, VerifyDeviceInfoResponse_case3, TestSize.Level0)
{
    const char *message = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"info\":"
                          "\"SkFERS1BTDAwOjg3QUQyOEQzQjFCLi4u\"}";
    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    DslmDeviceInfo device;
    (void)memset_s(&device, sizeof(device), 0, sizeof(device));

    int32_t ret = VerifyDeviceInfoResponse(&device, &msg);
    EXPECT_EQ(ERR_CHALLENGE_ERR, ret);
}

HWTEST_F(DslmTest, VerifyDeviceInfoResponse_case4, TestSize.Level0)
{
    DslmDeviceInfo device;
    memset_s(&device, sizeof(DslmDeviceInfo), 0, sizeof(DslmDeviceInfo));
    device.nonce = 0xE2C4D353EE211F3C;
    const char *message = "{\"version\":131072,\"challenge\":\"3C1F21EE53D3C4E2\",\"type\":2,\"info\":"
                          "\"SkFERS1BTDAwOjg3QUQyOEQzQjFCLi4u\"}";
    uint32_t messageLen = strlen(message) + 1;
    MessageBuff msg = {.length = messageLen, .buff = (uint8_t *)message};

    {
        int32_t ret = VerifyDeviceInfoResponse(&device, &msg);
        EXPECT_EQ(ERR_CHALLENGE_ERR, ret);
    }

    {
        device.nonceTimeStamp = 0xFFFFFFFFFFFFFFFF;
        int32_t ret = VerifyDeviceInfoResponse(&device, &msg);
        EXPECT_EQ(ERR_CHALLENGE_ERR, ret);
    }
}

HWTEST_F(DslmTest, GetDslmDeviceInfo_case1, TestSize.Level0)
{
    DeviceIdentify *device = nullptr;

    EXPECT_EQ(nullptr, GetDslmDeviceInfo(device));
}

HWTEST_F(DslmTest, CreatOrGetDslmDeviceInfo_case1, TestSize.Level0)
{
    DeviceIdentify *device = nullptr;

    EXPECT_EQ(nullptr, CreatOrGetDslmDeviceInfo(device));
}

HWTEST_F(DslmTest, CreatOrGetDslmDeviceInfo_case2, TestSize.Level0)
{
    DeviceIdentify device = {.length = DEVICE_ID_MAX_LEN - 1};

    EXPECT_EQ(nullptr, CreatOrGetDslmDeviceInfo(&device));
}

HWTEST_F(DslmTest, IsSameDevice_case1, TestSize.Level0)
{
    DeviceIdentify *device_first = nullptr;
    DeviceIdentify device_second;
    (void)memset_s(&device_second, sizeof(device_second), 0, sizeof(device_second));

    EXPECT_EQ(false, IsSameDevice(device_first, &device_second));
}

HWTEST_F(DslmTest, GetCurrentMachineState_case1, TestSize.Level0)
{
    DslmDeviceInfo *info = nullptr;
    uint32_t ret = GetCurrentMachineState(info);
    EXPECT_EQ(STATE_FAILED, ret);
}

HWTEST_F(DslmTest, OnMsgSendResultNotifier_case1, TestSize.Level0)
{
    DeviceIdentify identify = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}};
    uint64_t transNo = 0;
    uint32_t result = ERR_DEFAULT;

    {
        uint32_t ret = OnMsgSendResultNotifier(&identify, transNo, result);
        EXPECT_EQ(SUCCESS, ret);
    }

    {
        uint32_t ret = OnMsgSendResultNotifier(nullptr, transNo, result);
        EXPECT_EQ(SUCCESS, ret);
    }
}

HWTEST_F(DslmTest, OnPeerStatusReceiver_case1, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}};
    uint32_t status = 1234;
    uint32_t devType = 0;

    int32_t ret = OnPeerStatusReceiver(&device, status, devType);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DslmTest, InitDslmProcess_case1, TestSize.Level0)
{
    EXPECT_EQ(false, InitDslmProcess());
}

HWTEST_F(DslmTest, DeinitDslmProcess_case1, TestSize.Level0)
{
    EXPECT_EQ(true, DeinitDslmProcess());
}

// dslm_ohos_verify.c
HWTEST_F(DslmTest, VerifyOhosDslmCred_case1, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}};
    uint64_t challenge = 0x1234;
    uint8_t info[] = {'a', 'b', 'c', 'd', 1, 3, 5, 7, 9};
    DslmCredBuff cred = {CRED_TYPE_STANDARD, 9, info};
    DslmCredInfo *credInfo = nullptr;

    int32_t ret = VerifyOhosDslmCred(&device, challenge, &cred, credInfo);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, VerifyOhosDslmCred_case2, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}};
    uint64_t challenge = 0x1234;
    uint8_t info[] = {'a', 'b', 'c', 'd', 1, 3, 5, 7, 9};
    DslmCredBuff cred = {CRED_TYPE_LARGE, 9, info};
    DslmCredInfo credInfo;
    (void)memset_s(&credInfo, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));

    int32_t ret = VerifyOhosDslmCred(&device, challenge, &cred, &credInfo);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmTest, VerifyOhosDslmCred_case3, TestSize.Level0)
{
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}};
    uint64_t challenge = 0x1234;
    uint8_t info[] = {'a', 'b', 'c', 'd', 1, 3, 5, 7, 9};
    DslmCredBuff cred = {CRED_TYPE_SMALL, 9, info};
    DslmCredInfo credInfo;
    (void)memset_s(&credInfo, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));

    int32_t ret = VerifyOhosDslmCred(&device, challenge, &cred, &credInfo);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

// dslm_ohos_request.c

// 2nd param of GetCredFromCurrentDevice() is 0
HWTEST_F(DslmTest, GetCredFromCurrentDevice_case1, TestSize.Level0)
{
    char cred[] = "test";
    uint32_t len = 0;

    int32_t ret = GetCredFromCurrentDevice(cred, len);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

/**
 * @tc.name: GetPeerDeviceOnlineStatus_case1
 * @tc.desc: function GetPeerDeviceOnlineStatus when g_messenger is NULL
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, GetPeerDeviceOnlineStatus_case1, TestSize.Level0)
{
    EXPECT_EQ(false, GetPeerDeviceOnlineStatus(nullptr, nullptr));
}

/**
 * @tc.name: GetPeerDeviceOnlineStatus_case2
 * @tc.desc: function GetPeerDeviceOnlineStatus with null input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, GetPeerDeviceOnlineStatus_case2, TestSize.Level0)
{
    DslmMsgInterfaceMock mockMsg;
    EXPECT_EQ(false, GetPeerDeviceOnlineStatus(nullptr, nullptr));
}

/**
 * @tc.name: GetSelfDevice_case1
 * @tc.desc: function GetSelfDevice with null input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, GetSelfDevice_case1, TestSize.Level0)
{
    (void)GetSelfDevice(nullptr);
}

/**
 * @tc.name: DeinitMessenger_case1
 * @tc.desc: function DeinitMessenger when g_messenger is NULL
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, DeinitMessenger_case1, TestSize.Level0)
{
    DslmMsgInterfaceMock mockMsg;

    uint32_t ret = DeinitMessenger();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DeinitMessenger_case2
 * @tc.desc: function DeinitMessenger when g_messenger is not NULL
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, DeinitMessenger_case2, TestSize.Level0)
{
    uint32_t ret = DeinitMessenger();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: SendMsgToDevice_case1
 * @tc.desc: function SendMsgToDevice when g_messenger is NULL
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, SendMsgToDevice_case1, TestSize.Level0)
{
    DslmMsgInterfaceMock mockMsg;
    uint64_t transNo = 0xfe;
    const DeviceIdentify devId = {DEVICE_ID_MAX_LEN, {0}};
    const uint8_t msg[] = {'1', '2'};
    uint32_t msgLen = 2;

    mockMsg.~DslmMsgInterfaceMock();
    EXPECT_CALL(mockMsg, SendMsgTo(_, _, _, _, _)).Times(Exactly(0));

    SendMsgToDevice(transNo, &devId, msg, msgLen);
}

/**
 * @tc.name: CheckMessage_case1
 * @tc.desc: function CheckMessage when malformed input,
 *           msg contains non ASCII item.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, CheckMessage_case1, TestSize.Level0)
{
    const uint8_t msg[] = {'1', 0x8f, '\0'};
    uint32_t msgLen = 3;

    EXPECT_EQ(false, CheckMessage(msg, msgLen));
}

// just for coverage
/**
 * @tc.name: FreeMessagePacket_case1
 * @tc.desc: function FreeMessagePacket when packet->payload is NULL
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, FreeMessagePacket_case1, TestSize.Level0)
{
    MessagePacket *packet = (MessagePacket *)MALLOC(sizeof(MessagePacket));
    ASSERT_NE(nullptr, packet);
    (void)memset_s(packet, sizeof(MessagePacket), 0, sizeof(MessagePacket));

    FreeMessagePacket(packet);
}

// just for coverage
/**
 * @tc.name: FreeMessageBuff_case1
 * @tc.desc: function FreeMessageBuff with null input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, FreeMessageBuff_case1, TestSize.Level0)
{
    MessageBuff *buff = nullptr;

    FreeMessageBuff(buff);
}

// just for coverage
/**
 * @tc.name: FreeMessageBuff_case2
 * @tc.desc: function FreeMessageBuff when buff->buff is NULL
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, FreeMessageBuff_case2, TestSize.Level0)
{
    MessageBuff *buff = (MessageBuff *)MALLOC(sizeof(MessageBuff));
    ASSERT_NE(nullptr, buff);
    memset_s(buff, sizeof(MessageBuff), 0, sizeof(MessageBuff));

    FreeMessageBuff(buff);
}

static void dummyDump(const DslmDeviceInfo *info, int32_t fd)
{
    (void)info;
    (void)fd;
}

HWTEST_F(DslmTest, ForEachDeviceDump_case1, TestSize.Level0)
{
    ForEachDeviceDump(nullptr, 0);
    ForEachDeviceDump(dummyDump, 0);
    InitDslmStateMachine(nullptr);
    ScheduleDslmStateMachine(nullptr, 0, nullptr);
}

/**
 * @tc.name: DestroyDslmCred_case1
 * @tc.desc: function DestroyDslmCred with malformed inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, DestroyDslmCred_case1, TestSize.Level0)
{
    DslmCredBuff *cred = (DslmCredBuff *)MALLOC(sizeof(DslmCredBuff));
    cred->type = (CredType)3;
    cred->credLen = 9;
    cred->credVal = nullptr;

    DestroyDslmCred(nullptr);
    DestroyDslmCred(cred);
}

/**
 * @tc.name: ReportHiEventAppInvoke_case1
 * @tc.desc: function ReportHiEventAppInvoke with malformed inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, ReportHiEventAppInvoke_case1, TestSize.Level0)
{
    DslmDeviceInfo info;
    (void)memset_s(&info, sizeof(DslmDeviceInfo), 0, sizeof(DslmDeviceInfo));
    info.lastRequestTime = 10U;
    ReportHiEventInfoSync(nullptr);
    ReportHiEventInfoSync(&info);
    ReportHiEventAppInvoke(nullptr);
}

/**
 * @tc.name: GetDeviceSecurityLevelValue_case1
 * @tc.desc: function GetDeviceSecurityLevelValue with malformed inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, GetDeviceSecurityLevelValue_case1, TestSize.Level0)
{
    int32_t ret;
    int32_t level = 0;
    DeviceSecurityInfo info = {.magicNum = 0xcd, .result = 0, .level = 0};

    ret = GetDeviceSecurityLevelValue(nullptr, &level);
    EXPECT_EQ(ERR_INVALID_PARA, ret);

    ret = GetDeviceSecurityLevelValue(&info, nullptr);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

/**
 * @tc.name: RequestDeviceSecurityInfoAsync_case1
 * @tc.desc: function RequestDeviceSecurityInfoAsync with malformed inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmTest, RequestDeviceSecurityInfoAsync_case1, TestSize.Level0)
{
    int32_t ret;
    const DeviceIdentify device = {DEVICE_ID_MAX_LEN, {'a', 'b', 'c', 'd', 'e', 'f', 'g'}};
    RequestOption opt = {.challenge = 0xcd, .timeout = 400, .extra = 0};
    auto callback = [](const DeviceIdentify *identify, struct DeviceSecurityInfo *info) { return; };

    ret = RequestDeviceSecurityInfoAsync(nullptr, &opt, callback);
    EXPECT_EQ(ERR_INVALID_PARA, ret);

    ret = RequestDeviceSecurityInfoAsync(&device, &opt, nullptr);
    EXPECT_EQ(ERR_INVALID_PARA, ret);

    // malformed option->timeout > MAX_KEEP_LEN
    ret = RequestDeviceSecurityInfoAsync(&device, &opt, callback);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}
} // namespace DslmUnitTest
} // namespace Security
} // namespace OHOS