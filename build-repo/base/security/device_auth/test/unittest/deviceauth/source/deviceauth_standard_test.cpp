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

#include "deviceauth_standard_test.h"
#include <cinttypes>
#include <unistd.h>
#include <gtest/gtest.h>
#include "account_module_defines.h"
#include "alg_loader.h"
#include "common_defs.h"
#include "device_auth.h"
#include "device_auth_defines.h"
#include "hc_dev_info_mock.h"
#include "json_utils.h"
#include "protocol_task_main_mock.h"
#include "securec.h"

using namespace std;
using namespace testing::ext;

#define TEST_REQ_ID 123
#define TEST_REQ_ID2 321
#define TEST_APP_ID "TestAppId"
#define TEST_GROUP_NAME "TestGroup"
#define TEST_AUTH_ID "TestAuthId"
#define TEST_AUTH_ID2 "TestAuthId2"
#define TEST_AUTH_ID3 "TestAuthId3"
#define TEST_UDID "TestUdid"
#define TEST_UDID2 "TestUdid2"
#define TEST_UDID_CLIENT "5420459D93FE773F9945FD64277FBA2CAB8FB996DDC1D0B97676FBB1242B3930"
#define TEST_UDID_SERVER "52E2706717D5C39D736E134CC1E3BE1BAA2AA52DB7C76A37C749558BD2E6492C"
#define TEST_USER_ID_AUTH "4269DC28B639681698809A67EDAD08E39F207900038F91FEF95DD042FE2874E4"
#define TEST_USER_ID "1234ABCD"
#define TEST_USER_ID2 "DCBA4321"
#define TEST_GROUP_ID "E2EE6F830B176B2C96A9F99BFAE2A61F5D1490B9F4A090E9D8C2874C230C7C21"
#define TEST_GROUP_ID2 "1234ABCD"
#define TEST_GROUP_ID3 "6B7B805962B8EB8275D73128BFDAA7ECD755A2EC304E36543941874A277FA75F"
#define TEST_AUTH_CODE "37364761534f454d33567a73424e794f33573330507069434b31676f7254706b"
#define TEST_AUTH_CODE2 "2f7562744654535564586e665467546b322b4b506b65626373466f48766a4335"
#define TEST_AUTH_CODE3 "1234567812345678123456781234567812345678123456781234567812345678"
#define TEST_AUTH_TOKEN3 "10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92"
#define TEST_QUERY_PARAMS "bac"
#define TEST_PIN_CODE "123456"
#define TEST_DEV_AUTH_TEMP_KEY_PAIR_LEN 32
#define TEST_DEV_AUTH_SLEEP_TIME 10000
static const int32_t TEST_AUTH_OS_ACCOUNT_ID = 100;
static const int TEST_DEV_AUTH_BUFFER_SIZE = 128;
static const char *g_createParams = "{\"groupName\":\"TestGroup\",\"deviceId\":\"TestAuthId\",\"groupType\":256,\"group"
    "Visibility\":-1,\"userType\":0,\"expireTime\":-1}";
static const char *g_createParams3 = "{\"groupType\":1282,\"userId\":\"1234ABCD\",\"peerUserId\":\"DCBA4321\"}";
static const char *g_disbandParams =
    "{\"groupId\":\"E2EE6F830B176B2C96A9F99BFAE2A61F5D1490B9F4A090E9D8C2874C230C7C21\"}";
static const char *g_disbandParams2 = "{\"groupId\":\"1234ABCD\"}";
static const char *g_disbandParams3 =
    "{\"groupId\":\"6B7B805962B8EB8275D73128BFDAA7ECD755A2EC304E36543941874A277FA75F\"}";
static const char *g_addParams =
    "{\"groupId\":\"E2EE6F830B176B2C96A9F99BFAE2A61F5D1490B9F4A090E9D8C2874C230C7C21\","
    "\"groupType\":256,\"pinCode\":\"123456\"}";
static const char *g_addParams2 =
    "{\"groupType\":1,\"groupId\":\"1234ABCD\","
    "\"deviceList\":[{\"deviceId\":\"TestAuthId2\",\"udid\":\"TestUdid\",\"userId\":\"1234ABCD\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"37364761534f454d33567a73424e794f33573330507069434b31676f7254706b\"}},"
    "{\"deviceId\":\"TestAuthId3\",\"udid\":\"TestUdid2\",\"userId\":\"1234ABCD\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"2f7562744654535564586e665467546b322b4b506b65626373466f48766a4335\"}}]}";
static const char *g_addParams3 =
    "{\"groupType\":1282,\"groupId\":\"6B7B805962B8EB8275D73128BFDAA7ECD755A2EC304E36543941874A277FA75F\","
    "\"deviceList\":[{\"deviceId\":\"TestAuthId2\",\"udid\":\"TestUdid\",\"userId\":\"DCBA4321\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"37364761534f454d33567a73424e794f33573330507069434b31676f7254706b\"}},"
    "{\"deviceId\":\"TestAuthId3\",\"udid\":\"TestUdid2\",\"userId\":\"DCBA4321\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"2f7562744654535564586e665467546b322b4b506b65626373466f48766a4335\"}}]}";
static const char *g_addParams4 =
    "{\"groupType\":1,\"groupId\":\"4269DC28B639681698809A67EDAD08E39F207900038F91FEF95DD042FE2874E4\","
    "\"deviceList\":[{\"deviceId\":\"52E2706717D5C39D736E134CC1E3BE1BAA2AA52DB7C76A37C749558BD2E6492C\","
    "\"udid\":\"52E2706717D5C39D736E134CC1E3BE1BAA2AA52DB7C76A37C749558BD2E6492C\","
    "\"userId\":\"4269DC28B639681698809A67EDAD08E39F207900038F91FEF95DD042FE2874E4\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92\"}},"
    "{\"deviceId\":\"TestAuthId3\",\"udid\":\"TestUdid2\","
    "\"userId\":\"4269DC28B639681698809A67EDAD08E39F207900038F91FEF95DD042FE2874E4\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"2f7562744654535564586e665467546b322b4b506b65626373466f48766a4335\"}}]}";
static const char *g_addParams5 =
    "{\"groupType\":1,\"groupId\":\"4269DC28B639681698809A67EDAD08E39F207900038F91FEF95DD042FE2874E4\","
    "\"deviceList\":[{\"deviceId\":\"5420459D93FE773F9945FD64277FBA2CAB8FB996DDC1D0B97676FBB1242B3930\","
    "\"udid\":\"5420459D93FE773F9945FD64277FBA2CAB8FB996DDC1D0B97676FBB1242B3930\","
    "\"userId\":\"4269DC28B639681698809A67EDAD08E39F207900038F91FEF95DD042FE2874E4\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92\"}},"
    "{\"deviceId\":\"TestAuthId3\",\"udid\":\"TestUdid2\","
    "\"userId\":\"4269DC28B639681698809A67EDAD08E39F207900038F91FEF95DD042FE2874E4\","
    "\"credential\":{\"credentialType\":1,"
    "\"authCode\":\"2f7562744654535564586e665467546b322b4b506b65626373466f48766a4335\"}}]}";
static const char *g_deleteParams =
    "{\"groupId\":\"E2EE6F830B176B2C96A9F99BFAE2A61F5D1490B9F4A090E9D8C2874C230C7C21\",\"deleteId\":\"TestAuthId2\"}";
static const char *g_deleteParams2 =
    "{\"groupType\":1,\"groupId\":\"1234ABCD\","
    "\"deviceList\":[{\"deviceId\":\"TestAuthId2\"},{\"deviceId\":\"TestAuthId3\"}]}";
static const char *g_deleteParams3 =
    "{\"groupType\":1282,\"groupId\":\"6B7B805962B8EB8275D73128BFDAA7ECD755A2EC304E36543941874A277FA75F\","
    "\"deviceList\":[{\"deviceId\":\"TestAuthId2\"},{\"deviceId\":\"TestAuthId3\"}]}";
static const char *g_getRegisterInfoParams =
    "{\"version\":\"1.0.0\",\"deviceId\":\"TestAuthId\",\"userId\":\"1234ABCD\"}";
static const char *g_authParams = "{\"peerAuthId\":\"TestAuthId2\",\"servicePkgName\":\"TestAppId\",\"isClient\":true}";

enum AsyncStatus {
    ASYNC_STATUS_WAITING = 0,
    ASYNC_STATUS_TRANSMIT = 1,
    ASYNC_STATUS_FINISH = 2,
    ASYNC_STATUS_ERROR = 3
};

static AsyncStatus volatile g_asyncStatus;
static uint32_t g_transmitDataMaxLen = 2048;
static uint8_t g_transmitData[2048] = { 0 };
static uint32_t g_transmitDataLen = 0;

static bool OnTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    if (memcpy_s(g_transmitData, g_transmitDataMaxLen, data, dataLen) != EOK) {
        return false;
    }
    g_transmitDataLen = dataLen;
    g_asyncStatus = ASYNC_STATUS_TRANSMIT;
    return true;
}

static void OnSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen)
{
    (void)requestId;
    (void)sessionKey;
    (void)sessionKeyLen;
    return;
}

static void OnFinish(int64_t requestId, int operationCode, const char *authReturn)
{
    g_asyncStatus = ASYNC_STATUS_FINISH;
}

static void OnError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    g_asyncStatus = ASYNC_STATUS_ERROR;
}

static char *OnBindRequest(int64_t requestId, int operationCode, const char* reqParam)
{
    CJson *json = CreateJson();
    AddIntToJson(json, FIELD_CONFIRMATION, REQUEST_ACCEPTED);
    AddIntToJson(json, FIELD_OS_ACCOUNT_ID, TEST_AUTH_OS_ACCOUNT_ID);
    AddStringToJson(json, FIELD_PIN_CODE, TEST_PIN_CODE);
    AddStringToJson(json, FIELD_DEVICE_ID, TEST_AUTH_ID2);
    char *returnDataStr = PackJsonToString(json);
    FreeJson(json);
    return returnDataStr;
}

static char *OnAuthRequest(int64_t requestId, int operationCode, const char* reqParam)
{
    CJson *json = CreateJson();
    AddIntToJson(json, FIELD_CONFIRMATION, REQUEST_ACCEPTED);
    AddIntToJson(json, FIELD_OS_ACCOUNT_ID, TEST_AUTH_OS_ACCOUNT_ID);
    AddStringToJson(json, FIELD_PEER_CONN_DEVICE_ID, TEST_UDID_CLIENT);
    AddStringToJson(json, FIELD_SERVICE_PKG_NAME, TEST_APP_ID);
    char *returnDataStr = PackJsonToString(json);
    FreeJson(json);
    return returnDataStr;
}

static DeviceAuthCallback g_gmCallback = {
    .onTransmit = OnTransmit,
    .onSessionKeyReturned = OnSessionKeyReturned,
    .onFinish = OnFinish,
    .onError = OnError,
    .onRequest = OnBindRequest
};

static DeviceAuthCallback g_gaCallback = {
    .onTransmit = OnTransmit,
    .onSessionKeyReturned = OnSessionKeyReturned,
    .onFinish = OnFinish,
    .onError = OnError,
    .onRequest = OnAuthRequest
};

static void RemoveDir(const char *path)
{
    char strBuf[TEST_DEV_AUTH_BUFFER_SIZE] = {0};
    if (path == nullptr) {
        return;
    }
    if (sprintf_s(strBuf, sizeof(strBuf) - 1, "rm -rf %s", path) < 0) {
        return;
    }
    printf("%s\n", strBuf);
    system(strBuf);
    return;
}

static void RemoveHuks(void)
{
    RemoveDir("/data/service/el1/public/huks_service/maindata");
    cout << "[Clear] clear huks:maindata done: " << endl;
    return;
}

static void DeleteDatabase()
{
    const char *groupPath = "/data/service/el1/public/deviceauth";
    RemoveDir(groupPath);
    cout << "[Clear] clear db: done: " << endl;
    RemoveHuks();
    return;
}

static bool GenerateTempKeyPair(Uint8Buff *keyAlias)
{
    int ret = GetLoaderInstance()->checkKeyExist(keyAlias);
    if (ret != HC_SUCCESS) {
        printf("Key pair not exist, start to generate\n");
        int32_t authId = 0;
        Uint8Buff authIdBuff = { reinterpret_cast<uint8_t *>(&authId), sizeof(int32_t)};
        ExtraInfo extInfo = {authIdBuff, -1, -1};
        ret = GetLoaderInstance()->generateKeyPairWithStorage(keyAlias, TEST_DEV_AUTH_TEMP_KEY_PAIR_LEN, P256,
            KEY_PURPOSE_SIGN_VERIFY, &extInfo);
    } else {
        printf("Server key pair already exists\n");
    }

    if (ret != HC_SUCCESS) {
        printf("Generate key pair failed\n");
        return false;
    } else {
        printf("Generate key pair for server success\n");
    }
    return true;
}

static CJson *GetAsyCredentialJson(char *registerInfoStr)
{
    char *keyAliasValue = const_cast<char *>("TestServerKeyPair");
    int32_t keyAliasLen = 18;
    Uint8Buff keyAlias = {
        .val = reinterpret_cast<uint8_t *>(keyAliasValue),
        .length = keyAliasLen
    };
    if (!GenerateTempKeyPair(&keyAlias)) {
        return nullptr;
    }
    uint8_t *serverPkVal = reinterpret_cast<uint8_t *>(HcMalloc(SERVER_PK_SIZE, 0));
    Uint8Buff serverPk = {
        .val = serverPkVal,
        .length = SERVER_PK_SIZE
    };

    int32_t ret = GetLoaderInstance()->exportPublicKey(&keyAlias, &serverPk);
    if (ret != HC_SUCCESS) {
        printf("export PublicKey failed\n");
        HcFree(serverPkVal);
        return nullptr;
    }

    Uint8Buff messageBuff = {
        .val = reinterpret_cast<uint8_t *>(registerInfoStr),
        .length = strlen(registerInfoStr) + 1
    };
    uint8_t *signatureValue = reinterpret_cast<uint8_t *>(HcMalloc(SIGNATURE_SIZE, 0));
    Uint8Buff signature = {
        .val = signatureValue,
        .length = SIGNATURE_SIZE
    };
    ret = GetLoaderInstance()->sign(&keyAlias, &messageBuff, P256, &signature, true);
    if (ret != HC_SUCCESS) {
        printf("Sign pkInfo failed.\n");
        HcFree(serverPkVal);
        HcFree(signatureValue);
        return nullptr;
    }

    CJson *pkInfoJson = CreateJsonFromString(registerInfoStr);
    CJson *credentialJson = CreateJson();
    (void)AddIntToJson(credentialJson, FIELD_CREDENTIAL_TYPE, ASYMMETRIC_CRED);
    (void)AddByteToJson(credentialJson, FIELD_SERVER_PK, serverPkVal, serverPk.length);
    (void)AddByteToJson(credentialJson, FIELD_PK_INFO_SIGNATURE, signatureValue, signature.length);
    (void)AddObjToJson(credentialJson, FIELD_PK_INFO, pkInfoJson);
    FreeJson(pkInfoJson);
    return credentialJson;
}

static void CreateDemoGroup(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->createGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_createParams);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void CreateDemoIdenticalAccountGroup(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getRegisterInfo(g_getRegisterInfoParams, &returnData);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);

    CJson *credJson = GetAsyCredentialJson(returnData);
    ASSERT_NE(credJson, nullptr);
    CJson *json = CreateJson();
    AddIntToJson(json, FIELD_GROUP_TYPE, IDENTICAL_ACCOUNT_GROUP);
    AddStringToJson(json, FIELD_USER_ID, TEST_USER_ID);
    AddObjToJson(json, FIELD_CREDENTIAL, credJson);
    char *jsonStr = PackJsonToString(json);
    FreeJson(credJson);
    FreeJson(json);
    gm->destroyInfo(&returnData);
    if (jsonStr == nullptr) {
        return;
    }
    cout << "jsonstr: " << jsonStr << endl;

    ret = gm->createGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, jsonStr);
    FreeJsonString(jsonStr);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void CreateDemoSymClientIdenticalAccountGroup(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);

    CJson *credJson = CreateJson();
    (void)AddIntToJson(credJson, FIELD_CREDENTIAL_TYPE, SYMMETRIC_CRED);
    (void)AddStringToJson(credJson, FIELD_AUTH_CODE, TEST_AUTH_CODE3);
    CJson *json = CreateJson();
    AddIntToJson(json, FIELD_GROUP_TYPE, IDENTICAL_ACCOUNT_GROUP);
    AddStringToJson(json, FIELD_USER_ID, TEST_USER_ID_AUTH);
    AddObjToJson(json, FIELD_CREDENTIAL, credJson);
    char *jsonStr = PackJsonToString(json);
    FreeJson(credJson);
    FreeJson(json);
    if (jsonStr == nullptr) {
        return;
    }
    cout << "jsonstr: " << jsonStr << endl;
    int32_t ret = gm->createGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, jsonStr);
    FreeJsonString(jsonStr);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void CreateDemoSymServerIdenticalAccountGroup(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);

    CJson *credJson = CreateJson();
    (void)AddIntToJson(credJson, FIELD_CREDENTIAL_TYPE, SYMMETRIC_CRED);
    (void)AddStringToJson(credJson, FIELD_AUTH_CODE, TEST_AUTH_CODE3);
    CJson *json = CreateJson();
    AddIntToJson(json, FIELD_GROUP_TYPE, IDENTICAL_ACCOUNT_GROUP);
    AddStringToJson(json, FIELD_USER_ID, TEST_USER_ID_AUTH);
    AddObjToJson(json, FIELD_CREDENTIAL, credJson);
    char *jsonStr = PackJsonToString(json);
    FreeJson(credJson);
    FreeJson(json);
    if (jsonStr == nullptr) {
        return;
    }
    cout << "jsonstr: " << jsonStr << endl;

    int32_t ret = gm->createGroup(TEST_AUTH_OS_ACCOUNT_ID, TEST_REQ_ID, TEST_APP_ID, jsonStr);
    FreeJsonString(jsonStr);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void CreateDemoAcrossAccountGroup(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->createGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_createParams3);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void DeleteDemoGroup(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_disbandParams);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void DeleteDemoIdenticalAccountGroup(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_disbandParams2);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void DeleteDemoAcrossAccountGroup(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_disbandParams3);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void AddDemoMember(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    bool isClient = true;
    SetDeviceStatus(isClient);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->addMemberToGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_addParams);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    while (g_asyncStatus == ASYNC_STATUS_TRANSMIT) {
        isClient = !isClient;
        SetDeviceStatus(isClient);
        g_asyncStatus = ASYNC_STATUS_WAITING;
        if (isClient) {
            ret = gm->processData(TEST_REQ_ID, g_transmitData, g_transmitDataLen);
        } else {
            ret = gm->processData(TEST_REQ_ID2, g_transmitData, g_transmitDataLen);
        }
        (void)memset_s(g_transmitData, g_transmitDataMaxLen, 0, g_transmitDataMaxLen);
        g_transmitDataLen = 0;
        ASSERT_EQ(ret, HC_SUCCESS);
        while (g_asyncStatus == ASYNC_STATUS_WAITING) {
            usleep(TEST_DEV_AUTH_SLEEP_TIME);
        }
        if (g_asyncStatus == ASYNC_STATUS_ERROR) {
            break;
        }
        if (g_transmitDataLen > 0) {
            g_asyncStatus = ASYNC_STATUS_TRANSMIT;
        }
    }
    SetDeviceStatus(true);
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

static void DeleteDemoMember(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    bool isClient = true;
    SetDeviceStatus(isClient);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteMemberFromGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, g_deleteParams);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    while (g_asyncStatus == ASYNC_STATUS_TRANSMIT) {
        isClient = !isClient;
        SetDeviceStatus(isClient);
        g_asyncStatus = ASYNC_STATUS_WAITING;
        if (isClient) {
            ret = gm->processData(TEST_REQ_ID, g_transmitData, g_transmitDataLen);
        } else {
            ret = gm->processData(TEST_REQ_ID2, g_transmitData, g_transmitDataLen);
        }
        (void)memset_s(g_transmitData, g_transmitDataMaxLen, 0, g_transmitDataMaxLen);
        g_transmitDataLen = 0;
        ASSERT_EQ(ret, HC_SUCCESS);
        while (g_asyncStatus == ASYNC_STATUS_WAITING) {
            usleep(TEST_DEV_AUTH_SLEEP_TIME);
        }
        if (g_asyncStatus == ASYNC_STATUS_ERROR) {
            break;
        }
        if (g_transmitDataLen > 0) {
            g_asyncStatus = ASYNC_STATUS_TRANSMIT;
        }
    }
    SetDeviceStatus(true);
}

static void AuthDemoMember(void)
{
    g_asyncStatus = ASYNC_STATUS_WAITING;
    bool isClient = true;
    SetDeviceStatus(isClient);
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    CJson *authParam = CreateJson();
    AddStringToJson(authParam, FIELD_PEER_CONN_DEVICE_ID, TEST_UDID_SERVER);
    AddStringToJson(authParam, FIELD_SERVICE_PKG_NAME, TEST_APP_ID);
    AddBoolToJson(authParam, FIELD_IS_CLIENT, isClient);
    char *authParamStr = PackJsonToString(authParam);
    FreeJson(authParam);
    if (authParamStr == nullptr) {
        return;
    }
    printf("jsonStr: %s\n", authParamStr);
    int32_t ret = ga->authDevice(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, authParamStr, &g_gaCallback);
    FreeJsonString(authParamStr);
    ASSERT_EQ(ret, HC_SUCCESS);
    while (g_asyncStatus == ASYNC_STATUS_WAITING) {
        usleep(TEST_DEV_AUTH_SLEEP_TIME);
    }
    while (g_asyncStatus == ASYNC_STATUS_TRANSMIT) {
        isClient = !isClient;
        SetDeviceStatus(isClient);
        g_asyncStatus = ASYNC_STATUS_WAITING;
        if (isClient) {
            ret = ga->processData(TEST_REQ_ID, g_transmitData, g_transmitDataLen, &g_gaCallback);
        } else {
            ret = ga->processData(TEST_REQ_ID2, g_transmitData, g_transmitDataLen, &g_gaCallback);
        }
        (void)memset_s(g_transmitData, g_transmitDataMaxLen, 0, g_transmitDataMaxLen);
        g_transmitDataLen = 0;
        ASSERT_EQ(ret, HC_SUCCESS);
        while (g_asyncStatus == ASYNC_STATUS_WAITING) {
            usleep(TEST_DEV_AUTH_SLEEP_TIME);
        }
        if (g_asyncStatus == ASYNC_STATUS_ERROR) {
            break;
        }
        if (g_transmitDataLen > 0) {
            g_asyncStatus = ASYNC_STATUS_TRANSMIT;
        }
    }
    SetDeviceStatus(true);
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
}

class InitDeviceAuthServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void InitDeviceAuthServiceTest::SetUpTestCase() {}
void InitDeviceAuthServiceTest::TearDownTestCase() {}
void InitDeviceAuthServiceTest::SetUp() {}
void InitDeviceAuthServiceTest::TearDown() {}

HWTEST_F(InitDeviceAuthServiceTest, InitDeviceAuthServiceTest001, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

class DestroyDeviceAuthServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DestroyDeviceAuthServiceTest::SetUpTestCase() {}
void DestroyDeviceAuthServiceTest::TearDownTestCase() {}
void DestroyDeviceAuthServiceTest::SetUp() {}
void DestroyDeviceAuthServiceTest::TearDown() {}

HWTEST_F(DestroyDeviceAuthServiceTest, DestroyDeviceAuthServiceTest001, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
    DestroyDeviceAuthService();
}

class GetGmInstanceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GetGmInstanceTest::SetUpTestCase() {}
void GetGmInstanceTest::TearDownTestCase() {}

void GetGmInstanceTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GetGmInstanceTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GetGmInstanceTest, GetGmInstanceTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    EXPECT_NE(gm, nullptr);
}

class GetGaInstanceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GetGaInstanceTest::SetUpTestCase() {}
void GetGaInstanceTest::TearDownTestCase() {}

void GetGaInstanceTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GetGaInstanceTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GetGaInstanceTest, GetGaInstanceTest001, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    EXPECT_NE(ga, nullptr);
}

class GmRegCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmRegCallbackTest::SetUpTestCase() {}
void GmRegCallbackTest::TearDownTestCase() {}

void GmRegCallbackTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmRegCallbackTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmRegCallbackTest, GmRegCallbackTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DeviceAuthCallback callback;
    int32_t ret = gm->regCallback(TEST_APP_ID, &callback);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmRegCallbackTest, GmRegCallbackTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DeviceAuthCallback callback;
    int32_t ret = gm->regCallback(TEST_APP_ID, &callback);
    ASSERT_EQ(ret, HC_SUCCESS);
    DeviceAuthCallback callback2;
    ret = gm->regCallback(TEST_APP_ID, &callback2);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmRegCallbackTest, GmRegCallbackTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DeviceAuthCallback callback;
    int32_t ret = gm->regCallback(nullptr, &callback);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmRegCallbackTest, GmRegCallbackTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

class GmUnRegCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmUnRegCallbackTest::SetUpTestCase() {}
void GmUnRegCallbackTest::TearDownTestCase() {}

void GmUnRegCallbackTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmUnRegCallbackTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmUnRegCallbackTest, GmUnRegCallbackTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DeviceAuthCallback callback;
    int32_t ret = gm->regCallback(TEST_APP_ID, &callback);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmUnRegCallbackTest, GmUnRegCallbackTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->unRegCallback(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmUnRegCallbackTest, GmUnRegCallbackTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->unRegCallback(nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

class GmRegDataChangeListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmRegDataChangeListenerTest::SetUpTestCase() {}
void GmRegDataChangeListenerTest::TearDownTestCase() {}

void GmRegDataChangeListenerTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmRegDataChangeListenerTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmRegDataChangeListenerTest, GmRegDataChangeListenerTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DataChangeListener listener;
    int32_t ret = gm->regDataChangeListener(TEST_APP_ID, &listener);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmRegDataChangeListenerTest, GmRegDataChangeListenerTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DataChangeListener listener;
    int32_t ret = gm->regDataChangeListener(TEST_APP_ID, &listener);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->regDataChangeListener(TEST_APP_ID, &listener);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmRegDataChangeListenerTest, GmRegDataChangeListenerTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DataChangeListener listener;
    int32_t ret = gm->regDataChangeListener(nullptr, &listener);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmRegDataChangeListenerTest, GmRegDataChangeListenerTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regDataChangeListener(TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

class GmUnRegDataChangeListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmUnRegDataChangeListenerTest::SetUpTestCase() {}
void GmUnRegDataChangeListenerTest::TearDownTestCase() {}

void GmUnRegDataChangeListenerTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmUnRegDataChangeListenerTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmUnRegDataChangeListenerTest, GmUnRegDataChangeListenerTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    DataChangeListener listener;
    int32_t ret = gm->regDataChangeListener(TEST_APP_ID, &listener);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->unRegDataChangeListener(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmUnRegDataChangeListenerTest, GmUnRegDataChangeListenerTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->unRegDataChangeListener(TEST_APP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmUnRegDataChangeListenerTest, GmUnRegDataChangeListenerTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->unRegDataChangeListener(nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

class GmCreateGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmCreateGroupTest::SetUpTestCase() {}
void GmCreateGroupTest::TearDownTestCase() {}

void GmCreateGroupTest::SetUp()
{
    DeleteDatabase();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmCreateGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmCreateGroupTest, GmCreateGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->createGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmCreateGroupTest, GmCreateGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoIdenticalAccountGroup();
    CreateDemoAcrossAccountGroup();
}

HWTEST_F(GmCreateGroupTest, GmCreateGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoGroup();
}

class GmCheckAccessToGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmCheckAccessToGroupTest::SetUpTestCase() {}
void GmCheckAccessToGroupTest::TearDownTestCase() {}

void GmCheckAccessToGroupTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmCheckAccessToGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmCheckAccessToGroupTest, GmCheckAccessToGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->checkAccessToGroup(DEFAULT_OS_ACCOUNT, nullptr, TEST_GROUP_ID);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmCheckAccessToGroupTest, GmCheckAccessToGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->checkAccessToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmCheckAccessToGroupTest, GmCheckAccessToGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->checkAccessToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID);
    ASSERT_EQ(ret, HC_SUCCESS);
}

class GmGetPkInfoListTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetPkInfoListTest::SetUpTestCase() {}
void GmGetPkInfoListTest::TearDownTestCase() {}

void GmGetPkInfoListTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetPkInfoListTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, nullptr, TEST_QUERY_PARAMS, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_QUERY_PARAMS, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_QUERY_PARAMS, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetPkInfoListTest, GmGetPkInfoListTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    EXPECT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    char selfUdid[INPUT_UDID_LEN] = { 0 };
    (void)HcGetUdid(reinterpret_cast<uint8_t *>(selfUdid), INPUT_UDID_LEN);
    CJson *json = CreateJson();
    AddStringToJson(json, FIELD_UDID, selfUdid);
    AddBoolToJson(json, FIELD_IS_SELF_PK, true);
    char *jsonStr = PackJsonToString(json);
    FreeJson(json);
    printf("jsonStr: %s\n", jsonStr);
    int32_t ret = gm->getPkInfoList(DEFAULT_OS_ACCOUNT, TEST_APP_ID, jsonStr, &returnData, &returnNum);
    FreeJsonString(jsonStr);
    ASSERT_EQ(ret, HC_SUCCESS);
    printf("returnData: %s\n", returnData);
    ASSERT_NE(returnData, nullptr);
    ASSERT_NE(returnNum, 0);
    gm->destroyInfo(&returnData);
}

class GmGetGroupInfoByIdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetGroupInfoByIdTest::SetUpTestCase() {}
void GmGetGroupInfoByIdTest::TearDownTestCase() {}

void GmGetGroupInfoByIdTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetGroupInfoByIdTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, nullptr, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoByIdTest, GmGetGroupInfoByIdTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_SUCCESS);
    printf("returnData: %s\n", returnData);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

class GmGetGroupInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetGroupInfoTest::SetUpTestCase() {}
void GmGetGroupInfoTest::TearDownTestCase() {}

void GmGetGroupInfoTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetGroupInfoTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, nullptr, TEST_QUERY_PARAMS, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_QUERY_PARAMS, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_QUERY_PARAMS, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetGroupInfoTest, GmGetGroupInfoTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    CJson *json = CreateJson();
    AddStringToJson(json, FIELD_GROUP_OWNER, TEST_APP_ID);
    char *jsonStr = PackJsonToString(json);
    FreeJson(json);
    printf("jsonStr: %s\n", jsonStr);
    int32_t ret = gm->getGroupInfo(DEFAULT_OS_ACCOUNT, TEST_APP_ID, jsonStr, &returnData, &returnNum);
    FreeJsonString(jsonStr);
    ASSERT_EQ(ret, HC_SUCCESS);
    printf("returnData: %s\n", returnData);
    ASSERT_NE(returnData, nullptr);
    ASSERT_NE(returnNum, 0);
    gm->destroyInfo(&returnData);
}

class GmGetJoinedGroupsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetJoinedGroupsTest::SetUpTestCase() {}
void GmGetJoinedGroupsTest::TearDownTestCase() {}

void GmGetJoinedGroupsTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetJoinedGroupsTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, nullptr, PEER_TO_PEER_GROUP, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, PEER_TO_PEER_GROUP, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, nullptr, PEER_TO_PEER_GROUP, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetJoinedGroupsTest, GmGetJoinedGroupsTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getJoinedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, PEER_TO_PEER_GROUP, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    printf("returnData: %s\n", returnData);
    ASSERT_NE(returnData, nullptr);
    ASSERT_NE(returnNum, 0);
    gm->destroyInfo(&returnData);
}

class GmGetRelatedGroupsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetRelatedGroupsTest::SetUpTestCase() {}
void GmGetRelatedGroupsTest::TearDownTestCase() {}

void GmGetRelatedGroupsTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetRelatedGroupsTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, nullptr, TEST_AUTH_ID, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRelatedGroupsTest, GmGetRelatedGroupsTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getRelatedGroups(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    printf("returnData: %s\n", returnData);
    ASSERT_NE(returnData, nullptr);
    ASSERT_NE(returnNum, 0);
    gm->destroyInfo(&returnData);
}

class GmGetDeviceInfoByIdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetDeviceInfoByIdTest::SetUpTestCase() {}
void GmGetDeviceInfoByIdTest::TearDownTestCase() {}

void GmGetDeviceInfoByIdTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetDeviceInfoByIdTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, nullptr, TEST_AUTH_ID, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, nullptr, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, TEST_GROUP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetDeviceInfoByIdTest, GmGetDeviceInfoByIdTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getDeviceInfoById(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_AUTH_ID, TEST_GROUP_ID, &returnData);
    ASSERT_EQ(ret, HC_SUCCESS);
    printf("returnData: %s\n", returnData);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

class GmGetTrustedDevicesTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetTrustedDevicesTest::SetUpTestCase() {}
void GmGetTrustedDevicesTest::TearDownTestCase() {}

void GmGetTrustedDevicesTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetTrustedDevicesTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, nullptr, TEST_GROUP_ID, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, nullptr, &returnNum);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, &returnData, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetTrustedDevicesTest, GmGetTrustedDevicesTest005, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    uint32_t returnNum = 0;
    int32_t ret = gm->getTrustedDevices(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, &returnData, &returnNum);
    ASSERT_EQ(ret, HC_SUCCESS);
    printf("returnData: %s\n", returnData);
    ASSERT_NE(returnData, nullptr);
    ASSERT_NE(returnNum, 0);
    gm->destroyInfo(&returnData);
}

class GmIsDeviceInGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmIsDeviceInGroupTest::SetUpTestCase() {}
void GmIsDeviceInGroupTest::TearDownTestCase() {}

void GmIsDeviceInGroupTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmIsDeviceInGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, nullptr, TEST_GROUP_ID, TEST_AUTH_ID);
    ASSERT_EQ(ret, false);
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr, TEST_AUTH_ID);
    ASSERT_EQ(ret, false);
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, nullptr);
    ASSERT_EQ(ret, false);
}

HWTEST_F(GmIsDeviceInGroupTest, GmIsDeviceInGroupTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    bool ret = gm->isDeviceInGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, TEST_GROUP_ID, TEST_AUTH_ID);
    ASSERT_EQ(ret, true);
}

class GmAddMemberToGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmAddMemberToGroupTest::SetUpTestCase() {}
void GmAddMemberToGroupTest::TearDownTestCase() {}

void GmAddMemberToGroupTest::SetUp()
{
    DeleteDatabase();
}

void GmAddMemberToGroupTest::TearDown() {}

HWTEST_F(GmAddMemberToGroupTest, GmAddMemberToGroupTest001, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->addMemberToGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, nullptr, TEST_QUERY_PARAMS);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
    DestroyDeviceAuthService();
}

HWTEST_F(GmAddMemberToGroupTest, GmAddMemberToGroupTest002, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->addMemberToGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
    DestroyDeviceAuthService();
}

HWTEST_F(GmAddMemberToGroupTest, GmAddMemberToGroupTest003, TestSize.Level0)
{
    SetIsoSupported(true);
    SetPakeV1Supported(false);
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoGroup();
    AddDemoMember();
    DestroyDeviceAuthService();
}

HWTEST_F(GmAddMemberToGroupTest, GmAddMemberToGroupTest004, TestSize.Level0)
{
    SetIsoSupported(false);
    SetPakeV1Supported(true);
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoGroup();
    AddDemoMember();
    DestroyDeviceAuthService();
}

class GmDeleteMemberFromGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmDeleteMemberFromGroupTest::SetUpTestCase() {}
void GmDeleteMemberFromGroupTest::TearDownTestCase() {}

void GmDeleteMemberFromGroupTest::SetUp()
{
    DeleteDatabase();
}

void GmDeleteMemberFromGroupTest::TearDown() {}

HWTEST_F(GmDeleteMemberFromGroupTest, GmDeleteMemberFromGroupTest001, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->deleteMemberFromGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, nullptr, TEST_QUERY_PARAMS);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
    DestroyDeviceAuthService();
}

HWTEST_F(GmDeleteMemberFromGroupTest, GmDeleteMemberFromGroupTest002, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->deleteMemberFromGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
    DestroyDeviceAuthService();
}

HWTEST_F(GmDeleteMemberFromGroupTest, GmDeleteMemberFromGroupTest003, TestSize.Level0)
{
    SetIsoSupported(true);
    SetPakeV1Supported(false);
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoGroup();
    AddDemoMember();
    DeleteDemoMember();
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_FINISH);
    DestroyDeviceAuthService();
}

HWTEST_F(GmDeleteMemberFromGroupTest, GmDeleteMemberFromGroupTest004, TestSize.Level0)
{
    SetIsoSupported(false);
    SetPakeV1Supported(true);
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoGroup();
    AddDemoMember();
    DeleteDemoMember();
    ASSERT_EQ(g_asyncStatus, ASYNC_STATUS_ERROR);
    DestroyDeviceAuthService();
}

class GmProcessDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmProcessDataTest::SetUpTestCase() {}
void GmProcessDataTest::TearDownTestCase() {}

void GmProcessDataTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmProcessDataTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmProcessDataTest, GmProcessDataTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    EXPECT_NE(gm, nullptr);
    int32_t ret = gm->processData(TEST_REQ_ID, nullptr, 0);
    EXPECT_NE(ret, HC_SUCCESS);
}

class GmAddMultiMembersToGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmAddMultiMembersToGroupTest::SetUpTestCase() {}
void GmAddMultiMembersToGroupTest::TearDownTestCase() {}

void GmAddMultiMembersToGroupTest::SetUp()
{
    DeleteDatabase();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmAddMultiMembersToGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmAddMultiMembersToGroupTest, GmAddMultiMembersToGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, nullptr, TEST_QUERY_PARAMS);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmAddMultiMembersToGroupTest, GmAddMultiMembersToGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmAddMultiMembersToGroupTest, GmAddMultiMembersToGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoIdenticalAccountGroup();
    ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_addParams2);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmAddMultiMembersToGroupTest, GmAddMultiMembersToGroupTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoIdenticalAccountGroup();
    CreateDemoAcrossAccountGroup();
    ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_addParams3);
    ASSERT_EQ(ret, HC_SUCCESS);
}

class GmDelMultiMembersFromGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmDelMultiMembersFromGroupTest::SetUpTestCase() {}
void GmDelMultiMembersFromGroupTest::TearDownTestCase() {}

void GmDelMultiMembersFromGroupTest::SetUp()
{
    DeleteDatabase();
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmDelMultiMembersFromGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmDelMultiMembersFromGroupTest, GmDelMultiMembersFromGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->delMultiMembersFromGroup(DEFAULT_OS_ACCOUNT, nullptr, TEST_QUERY_PARAMS);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmDelMultiMembersFromGroupTest, GmDelMultiMembersFromGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->delMultiMembersFromGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmDelMultiMembersFromGroupTest, GmDelMultiMembersFromGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoIdenticalAccountGroup();
    ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_addParams2);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->delMultiMembersFromGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_deleteParams2);
    ASSERT_EQ(ret, HC_SUCCESS);
}

HWTEST_F(GmDelMultiMembersFromGroupTest, GmDelMultiMembersFromGroupTest004, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoIdenticalAccountGroup();
    CreateDemoAcrossAccountGroup();
    ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_addParams3);
    ASSERT_EQ(ret, HC_SUCCESS);
    ret = gm->delMultiMembersFromGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_deleteParams3);
    ASSERT_EQ(ret, HC_SUCCESS);
}

class GmGetRegisterInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmGetRegisterInfoTest::SetUpTestCase() {}
void GmGetRegisterInfoTest::TearDownTestCase() {}

void GmGetRegisterInfoTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GmGetRegisterInfoTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmGetRegisterInfoTest, GmGetRegisterInfoTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getRegisterInfo(nullptr, &returnData);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRegisterInfoTest, GmGetRegisterInfoTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->getRegisterInfo(TEST_QUERY_PARAMS, nullptr);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
}

HWTEST_F(GmGetRegisterInfoTest, GmGetRegisterInfoTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    char *returnData = nullptr;
    int32_t ret = gm->getRegisterInfo(g_getRegisterInfoParams, &returnData);
    ASSERT_EQ(ret, HC_SUCCESS);
    ASSERT_NE(returnData, nullptr);
    gm->destroyInfo(&returnData);
}

class GmDeleteGroupTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GmDeleteGroupTest::SetUpTestCase() {}
void GmDeleteGroupTest::TearDownTestCase() {}

void GmDeleteGroupTest::SetUp()
{
    DeleteDatabase();
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
}

void GmDeleteGroupTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GmDeleteGroupTest, GmDeleteGroupTest001, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->deleteGroup(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, TEST_APP_ID, nullptr);
    ASSERT_NE(ret, HC_SUCCESS);
}

HWTEST_F(GmDeleteGroupTest, GmDeleteGroupTest002, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoGroup();
    DeleteDemoGroup();
}

HWTEST_F(GmDeleteGroupTest, GmDeleteGroupTest003, TestSize.Level0)
{
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    int32_t ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoIdenticalAccountGroup();
    CreateDemoAcrossAccountGroup();
    DeleteDemoAcrossAccountGroup();
    DeleteDemoIdenticalAccountGroup();
}

class GaAuthDeviceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GaAuthDeviceTest::SetUpTestCase() {}
void GaAuthDeviceTest::TearDownTestCase() {}

void GaAuthDeviceTest::SetUp()
{
    DeleteDatabase();
}

void GaAuthDeviceTest::TearDown() {}

HWTEST_F(GaAuthDeviceTest, GaAuthDeviceTest001, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    ret = ga->authDevice(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, nullptr, &g_gmCallback);
    ASSERT_EQ(ret, HC_ERR_INVALID_PARAMS);
    DestroyDeviceAuthService();
}

HWTEST_F(GaAuthDeviceTest, GaAuthDeviceTest002, TestSize.Level0)
{
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const GroupAuthManager *ga = GetGaInstance();
    ASSERT_NE(ga, nullptr);
    ret = ga->authDevice(DEFAULT_OS_ACCOUNT, TEST_REQ_ID, g_authParams, nullptr);
    ASSERT_NE(ret, HC_SUCCESS);
    DestroyDeviceAuthService();
}

HWTEST_F(GaAuthDeviceTest, GaAuthDeviceTest003, TestSize.Level0)
{
    SetIsoSupported(true);
    SetPakeV1Supported(false);
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoGroup();
    AddDemoMember();
    AuthDemoMember();
    DestroyDeviceAuthService();
}

HWTEST_F(GaAuthDeviceTest, GaAuthDeviceTest004, TestSize.Level0)
{
    SetIsoSupported(true);
    SetPakeV1Supported(false);
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoSymClientIdenticalAccountGroup();
    ASSERT_EQ(ret, HC_SUCCESS);
    SetDeviceStatus(false);
    CreateDemoSymServerIdenticalAccountGroup();
    SetDeviceStatus(true);
    AuthDemoMember();
    DestroyDeviceAuthService();
}

HWTEST_F(GaAuthDeviceTest, GaAuthDeviceTest005, TestSize.Level0)
{
    SetIsoSupported(true);
    SetPakeV1Supported(false);
    int32_t ret = InitDeviceAuthService();
    ASSERT_EQ(ret, HC_SUCCESS);
    const DeviceGroupManager *gm = GetGmInstance();
    ASSERT_NE(gm, nullptr);
    ret = gm->regCallback(TEST_APP_ID, &g_gmCallback);
    ASSERT_EQ(ret, HC_SUCCESS);
    CreateDemoSymClientIdenticalAccountGroup();
    SetDeviceStatus(false);
    CreateDemoSymServerIdenticalAccountGroup();
    SetDeviceStatus(true);
    ret = gm->addMultiMembersToGroup(DEFAULT_OS_ACCOUNT, TEST_APP_ID, g_addParams4);
    ASSERT_EQ(ret, HC_SUCCESS);
    SetDeviceStatus(false);
    ret = gm->addMultiMembersToGroup(TEST_AUTH_OS_ACCOUNT_ID, TEST_APP_ID, g_addParams5);
    ASSERT_EQ(ret, HC_SUCCESS);
    SetDeviceStatus(true);
    AuthDemoMember();
    DestroyDeviceAuthService();
}

class GaProcessDataTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void GaProcessDataTest::SetUpTestCase() {}
void GaProcessDataTest::TearDownTestCase() {}

void GaProcessDataTest::SetUp()
{
    int ret = InitDeviceAuthService();
    EXPECT_EQ(ret, HC_SUCCESS);
}

void GaProcessDataTest::TearDown()
{
    DestroyDeviceAuthService();
}

HWTEST_F(GaProcessDataTest, GaProcessDataTest002, TestSize.Level0)
{
    const GroupAuthManager *ga = GetGaInstance();
    EXPECT_NE(ga, nullptr);
    int32_t ret = ga->processData(TEST_REQ_ID, nullptr, 0, nullptr);
    EXPECT_NE(ret, HC_SUCCESS);
}
