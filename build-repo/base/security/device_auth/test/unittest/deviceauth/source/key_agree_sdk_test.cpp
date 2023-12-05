/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "key_agree_sdk_test.h"

#include <gtest/gtest.h>
#include "key_agree_sdk.h"
#include "hc_types.h"

using namespace std;
using namespace testing::ext;

static const int32_t g_sessionIdClient = 1001;
static const int32_t g_sessionIdServer = 1002;
static const int32_t g_messageLength = 2048;
static const int32_t g_sharedSecretLength = 7;
static const int32_t g_deviceIdLength = 17;
static const int32_t g_sessionKeyLength = 64;

static KeyAgreeSession *g_session;
static KeyAgreeSession *g_session_s;
static KeyAgreeBlob g_sharedSecret = { NULL, 0 };
static KeyAgreeBlob g_deviceId = { NULL, 0 };
static KeyAgreeBlob g_deviceId_s = { NULL, 0 };
static KeyAgreeBlob g_messageToTransmit1 = { NULL, 0 };
static KeyAgreeBlob g_messageToTransmit2 = { NULL, 0 };
static KeyAgreeBlob g_messageToTransmit3 = { NULL, 0 };
static KeyAgreeBlob g_messageToTransmit4 = { NULL, 0 };
static KeyAgreeBlob g_messageToTransmit5 = { NULL, 0 };
static KeyAgreeBlob g_sessionKey_s = { NULL, 0 };
static KeyAgreeBlob g_sessionKey = { NULL, 0 };

class KeyAgreeInitSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void KeyAgreeInitSessionTest::SetUpTestCase()
{
    g_session = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);
    g_session_s = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);
}
void KeyAgreeInitSessionTest::TearDownTestCase()
{
    HcFree(g_session);
    HcFree(g_session_s);
}
void KeyAgreeInitSessionTest::SetUp() {}
void KeyAgreeInitSessionTest::TearDown()
{
    KeyAgreeFreeSession(g_session);
    KeyAgreeFreeSession(g_session_s);
}

HWTEST_F(KeyAgreeInitSessionTest, KeyAgreeInitSessionTest001, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeInitSessionTest, KeyAgreeInitSessionTest002, TestSize.Level0)
{
    g_session_s->sessionId = g_sessionIdServer;
    KeyAgreeResult res = KeyAgreeInitSession(g_session_s, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_SERVER);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeInitSessionTest, KeyAgreeInitSessionTest003, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_DL_SPEKE_256, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeInitSessionTest, KeyAgreeInitSessionTest004, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_DL_SPEKE_384, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeInitSessionTest, KeyAgreeInitSessionTest005, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_EC_SPEKE_P256, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeInitSessionTest, KeyAgreeInitSessionTest006, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_EC_SPEKE_X25519, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeInitSessionTest, KeyAgreeInitSessionTest007, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(NULL, KEYAGREE_PROTOCOL_EC_SPEKE_X25519, KEYAGREE_TYPE_CLIENT);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

class KeyAgreeStartSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void KeyAgreeStartSessionTest::SetUpTestCase()
{
    g_session = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);
    g_session_s = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);

    g_sharedSecret.length = g_sharedSecretLength;
    g_sharedSecret.data = (uint8_t *)HcMalloc(g_sharedSecret.length, 0);
    (void)memcpy_s(g_sharedSecret.data, g_sharedSecret.length, (uint8_t *)"123456", g_sharedSecretLength);

    g_deviceId.length = g_deviceIdLength;
    g_deviceId.data = (uint8_t *)HcMalloc(g_deviceId.length, 0);
    (void)memcpy_s(g_deviceId.data, g_deviceId.length, (uint8_t *)"clientclientabcd", g_deviceIdLength);

    g_deviceId_s.length = g_deviceIdLength;
    g_deviceId_s.data = (uint8_t *)HcMalloc(g_deviceId_s.length, 0);
    (void)memcpy_s(g_deviceId_s.data, g_deviceId_s.length, (uint8_t *)"serverserverabcd", g_deviceIdLength);
}

void KeyAgreeStartSessionTest::TearDownTestCase()
{
    HcFree(g_session);
    HcFree(g_session_s);
    HcFree(g_sharedSecret.data);
    HcFree(g_deviceId.data);
    HcFree(g_deviceId_s.data);
}
void KeyAgreeStartSessionTest::SetUp() {}
void KeyAgreeStartSessionTest::TearDown()
{
    KeyAgreeFreeSession(g_session);
    KeyAgreeFreeSession(g_session_s);
}

HWTEST_F(KeyAgreeStartSessionTest, KeyAgreeStartSessionTest001, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeStartSessionTest, KeyAgreeStartSessionTest002, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, NULL, NULL);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeStartSessionTest, KeyAgreeStartSessionTest003, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, NULL, &g_deviceId, NULL);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeStartSessionTest, KeyAgreeStartSessionTest004, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(NULL, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

class KeyAgreeGenerateNextMessageTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void KeyAgreeGenerateNextMessageTest::SetUpTestCase()
{
    g_session = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);
    g_session_s = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);

    g_sharedSecret.length = g_sharedSecretLength;
    g_sharedSecret.data = (uint8_t *)HcMalloc(g_sharedSecret.length, g_sharedSecretLength);
    (void)memcpy_s(g_sharedSecret.data, g_sharedSecret.length, (uint8_t *)"123456", g_sharedSecretLength);

    g_deviceId.length = g_deviceIdLength;
    g_deviceId.data = (uint8_t *)HcMalloc(g_deviceId.length, 0);
    (void)memcpy_s(g_deviceId.data, g_deviceId.length, (uint8_t *)"clientclientabcd", g_deviceIdLength);

    g_deviceId_s.length = g_deviceIdLength;
    g_deviceId_s.data = (uint8_t *)HcMalloc(g_deviceId_s.length, 0);
    (void)memcpy_s(g_deviceId_s.data, g_deviceId_s.length, (uint8_t *)"serverserverabcd", g_deviceIdLength);
}
void KeyAgreeGenerateNextMessageTest::TearDownTestCase()
{
    HcFree(g_session);
    HcFree(g_session_s);
    HcFree(g_sharedSecret.data);
    HcFree(g_deviceId.data);
    HcFree(g_deviceId_s.data);
}
void KeyAgreeGenerateNextMessageTest::SetUp()
{
    g_messageToTransmit1.length = g_messageLength;
    g_messageToTransmit1.data = (uint8_t *)HcMalloc(g_messageToTransmit1.length, 0);

    g_messageToTransmit2.length = g_messageLength;
    g_messageToTransmit2.data = (uint8_t *)HcMalloc(g_messageToTransmit2.length, 0);

    g_messageToTransmit3.length = g_messageLength;
    g_messageToTransmit3.data = (uint8_t *)HcMalloc(g_messageToTransmit3.length, 0);

    g_messageToTransmit4.length = g_messageLength;
    g_messageToTransmit4.data = (uint8_t *)HcMalloc(g_messageToTransmit4.length, 0);

    g_messageToTransmit5.length = g_messageLength;
    g_messageToTransmit5.data = (uint8_t *)HcMalloc(g_messageToTransmit5.length, 0);

    g_sessionKey_s.length = g_sessionKeyLength;
    g_sessionKey_s.data = (uint8_t *)HcMalloc(g_sessionKey_s.length, 0);

    g_sessionKey.length = g_sessionKeyLength;
    g_sessionKey.data = (uint8_t *)HcMalloc(g_sessionKey.length, 0);
}
void KeyAgreeGenerateNextMessageTest::TearDown()
{
    HcFree(g_messageToTransmit1.data);
    HcFree(g_messageToTransmit2.data);
    HcFree(g_messageToTransmit3.data);
    HcFree(g_messageToTransmit4.data);
    HcFree(g_messageToTransmit5.data);
    HcFree(g_sessionKey_s.data);
    HcFree(g_sessionKey.data);
    KeyAgreeFreeSession(g_session);
    KeyAgreeFreeSession(g_session_s);
}

HWTEST_F(KeyAgreeGenerateNextMessageTest, KeyAgreeGenerateNextMessageTest001, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    g_session_s->sessionId = g_sessionIdServer;
    res = KeyAgreeInitSession(g_session_s, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_SERVER);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session_s, &g_sharedSecret, &g_deviceId_s, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, NULL, &g_messageToTransmit1);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit1, &g_messageToTransmit2);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit2, &g_messageToTransmit3);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit3, &g_messageToTransmit4);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session_s, &g_sessionKey_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit4, &g_messageToTransmit5);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session, &g_sessionKey);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    EXPECT_EQ(*(g_sessionKey.data), *(g_sessionKey_s.data));
}

HWTEST_F(KeyAgreeGenerateNextMessageTest, KeyAgreeGenerateNextMessageTest002, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_DL_SPEKE_256, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    g_session_s->sessionId = g_sessionIdServer;
    res = KeyAgreeInitSession(g_session_s, KEYAGREE_PROTOCOL_DL_SPEKE_256, KEYAGREE_TYPE_SERVER);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session_s, &g_sharedSecret, &g_deviceId_s, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, NULL, &g_messageToTransmit1);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit1, &g_messageToTransmit2);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit2, &g_messageToTransmit3);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session, &g_sessionKey);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit3, &g_messageToTransmit4);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session_s, &g_sessionKey_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    EXPECT_EQ(*(g_sessionKey.data), *(g_sessionKey_s.data));
}

HWTEST_F(KeyAgreeGenerateNextMessageTest, KeyAgreeGenerateNextMessageTest003, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_DL_SPEKE_384, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    g_session_s->sessionId = g_sessionIdServer;
    res = KeyAgreeInitSession(g_session_s, KEYAGREE_PROTOCOL_DL_SPEKE_384, KEYAGREE_TYPE_SERVER);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session_s, &g_sharedSecret, &g_deviceId_s, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, NULL, &g_messageToTransmit1);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit1, &g_messageToTransmit2);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit2, &g_messageToTransmit3);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session, &g_sessionKey);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit3, &g_messageToTransmit4);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session_s, &g_sessionKey_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    EXPECT_EQ(*(g_sessionKey.data), *(g_sessionKey_s.data));
}

HWTEST_F(KeyAgreeGenerateNextMessageTest, KeyAgreeGenerateNextMessageTest004, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_EC_SPEKE_P256, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    g_session_s->sessionId = g_sessionIdServer;
    res = KeyAgreeInitSession(g_session_s, KEYAGREE_PROTOCOL_EC_SPEKE_P256, KEYAGREE_TYPE_SERVER);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session_s, &g_sharedSecret, &g_deviceId_s, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, NULL, &g_messageToTransmit1);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit1, &g_messageToTransmit2);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit2, &g_messageToTransmit3);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session, &g_sessionKey);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit3, &g_messageToTransmit4);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session_s, &g_sessionKey_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    EXPECT_EQ(*(g_sessionKey.data), *(g_sessionKey_s.data));
}

HWTEST_F(KeyAgreeGenerateNextMessageTest, KeyAgreeGenerateNextMessageTest005, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_EC_SPEKE_X25519, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    g_session_s->sessionId = g_sessionIdServer;
    res = KeyAgreeInitSession(g_session_s, KEYAGREE_PROTOCOL_EC_SPEKE_X25519, KEYAGREE_TYPE_SERVER);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session_s, &g_sharedSecret, &g_deviceId_s, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, NULL, &g_messageToTransmit1);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit1, &g_messageToTransmit2);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit2, &g_messageToTransmit3);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session, &g_sessionKey);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit3, &g_messageToTransmit4);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session_s, &g_sessionKey_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    EXPECT_EQ(*(g_sessionKey.data), *(g_sessionKey_s.data));
}

HWTEST_F(KeyAgreeGenerateNextMessageTest, KeyAgreeGenerateNextMessageTest006, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_EC_SPEKE_X25519, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    g_session_s->sessionId = g_sessionIdServer;
    res = KeyAgreeInitSession(g_session_s, KEYAGREE_PROTOCOL_EC_SPEKE_P256, KEYAGREE_TYPE_SERVER);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session_s, &g_sharedSecret, &g_deviceId_s, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, NULL, &g_messageToTransmit1);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit1, &g_messageToTransmit2);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit2, &g_messageToTransmit3);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

class KeyAgreeIsFinishTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void KeyAgreeIsFinishTest::SetUpTestCase()
{
    g_session = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);
    g_session_s = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);
}
void KeyAgreeIsFinishTest::TearDownTestCase()
{
    HcFree(g_session);
    HcFree(g_session_s);
}
void KeyAgreeIsFinishTest::SetUp() {}
void KeyAgreeIsFinishTest::TearDown()
{
    KeyAgreeFreeSession(g_session);
    KeyAgreeFreeSession(g_session_s);
}

HWTEST_F(KeyAgreeIsFinishTest, KeyAgreeIsFinishTest001, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeIsFinishTest, KeyAgreeIsFinishTest002, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(NULL);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

class KeyAgreeGetResultTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void KeyAgreeGetResultTest::SetUpTestCase()
{
    g_session = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);
    g_session_s = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);

    g_sessionKey_s.length = g_sessionKeyLength;
    g_sessionKey_s.data = (uint8_t *)HcMalloc(g_sessionKey_s.length, 0);

    g_sessionKey.length = g_sessionKeyLength;
    g_sessionKey.data = (uint8_t *)HcMalloc(g_sessionKey.length, 0);
}
void KeyAgreeGetResultTest::TearDownTestCase()
{
    HcFree(g_session);
    HcFree(g_session_s);
    HcFree(g_sessionKey_s.data);
    HcFree(g_sessionKey.data);
}
void KeyAgreeGetResultTest::SetUp() {}
void KeyAgreeGetResultTest::TearDown()
{
    KeyAgreeFreeSession(g_session);
    KeyAgreeFreeSession(g_session_s);
}

HWTEST_F(KeyAgreeGetResultTest, KeyAgreeGetResultTest001, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session, NULL);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

HWTEST_F(KeyAgreeGetResultTest, KeyAgreeGetResultTest002, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session, &g_sessionKey);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}

class KeyAgreeFreeSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void KeyAgreeFreeSessionTest::SetUpTestCase()
{
    g_session = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);
    g_session_s = (KeyAgreeSession *)HcMalloc(sizeof(KeyAgreeSession), 0);

    g_sharedSecret.length = g_sharedSecretLength;
    g_sharedSecret.data = (uint8_t *)HcMalloc(g_sharedSecret.length, 0);
    (void)memcpy_s(g_sharedSecret.data, g_sharedSecret.length, (uint8_t *)"123456", g_sharedSecretLength);

    g_deviceId.length = g_deviceIdLength;
    g_deviceId.data = (uint8_t *)HcMalloc(g_deviceId.length, 0);
    (void)memcpy_s(g_deviceId.data, g_deviceId.length, (uint8_t *)"clientclientabcd", g_deviceIdLength);

    g_deviceId_s.length = g_deviceIdLength;
    g_deviceId_s.data = (uint8_t *)HcMalloc(g_deviceId_s.length, 0);
    (void)memcpy_s(g_deviceId_s.data, g_deviceId_s.length, (uint8_t *)"serverserverabcd", g_deviceIdLength);

    g_messageToTransmit1.length = g_messageLength;
    g_messageToTransmit1.data = (uint8_t *)HcMalloc(g_messageToTransmit1.length, 0);

    g_messageToTransmit2.length = g_messageLength;
    g_messageToTransmit2.data = (uint8_t *)HcMalloc(g_messageToTransmit2.length, 0);

    g_messageToTransmit3.length = g_messageLength;
    g_messageToTransmit3.data = (uint8_t *)HcMalloc(g_messageToTransmit3.length, 0);

    g_messageToTransmit4.length = g_messageLength;
    g_messageToTransmit4.data = (uint8_t *)HcMalloc(g_messageToTransmit4.length, 0);

    g_messageToTransmit5.length = g_messageLength;
    g_messageToTransmit5.data = (uint8_t *)HcMalloc(g_messageToTransmit5.length, 0);

    g_sessionKey_s.length = g_sessionKeyLength;
    g_sessionKey_s.data = (uint8_t *)HcMalloc(g_sessionKey_s.length, 0);

    g_sessionKey.length = g_sessionKeyLength;
    g_sessionKey.data = (uint8_t *)HcMalloc(g_sessionKey.length, 0);
}
void KeyAgreeFreeSessionTest::TearDownTestCase()
{
    HcFree(g_session);
    HcFree(g_session_s);
    HcFree(g_sharedSecret.data);
    HcFree(g_deviceId.data);
    HcFree(g_deviceId_s.data);
    HcFree(g_messageToTransmit1.data);
    HcFree(g_messageToTransmit2.data);
    HcFree(g_messageToTransmit3.data);
    HcFree(g_messageToTransmit4.data);
    HcFree(g_messageToTransmit5.data);
    HcFree(g_sessionKey_s.data);
    HcFree(g_sessionKey.data);
}
void KeyAgreeFreeSessionTest::SetUp() {}
void KeyAgreeFreeSessionTest::TearDown()
{
    KeyAgreeFreeSession(g_session);
    KeyAgreeFreeSession(g_session_s);
}

HWTEST_F(KeyAgreeFreeSessionTest, KeyAgreeFreeSessionTest001, TestSize.Level0)
{
    g_session->sessionId = g_sessionIdClient;
    KeyAgreeResult res = KeyAgreeInitSession(g_session, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_CLIENT);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    g_session_s->sessionId = g_sessionIdServer;
    res = KeyAgreeInitSession(g_session_s, KEYAGREE_PROTOCOL_ANY, KEYAGREE_TYPE_SERVER);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session, &g_sharedSecret, &g_deviceId, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeStartSession(g_session_s, &g_sharedSecret, &g_deviceId_s, NULL);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, NULL, &g_messageToTransmit1);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit1, &g_messageToTransmit2);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit2, &g_messageToTransmit3);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session_s, &g_messageToTransmit3, &g_messageToTransmit4);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session_s, &g_sessionKey_s);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGenerateNextMessage(g_session, &g_messageToTransmit4, &g_messageToTransmit5);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeIsFinish(g_session);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session, &g_sessionKey);
    EXPECT_EQ(res, KEYAGREE_SUCCESS);

    EXPECT_EQ(*(g_sessionKey.data), *(g_sessionKey_s.data));

    KeyAgreeFreeSession(g_session);
    KeyAgreeFreeSession(g_session_s);

    res = KeyAgreeGetResult(g_session, &g_sessionKey);
    EXPECT_NE(res, KEYAGREE_SUCCESS);

    res = KeyAgreeGetResult(g_session_s, &g_sessionKey_s);
    EXPECT_NE(res, KEYAGREE_SUCCESS);
}