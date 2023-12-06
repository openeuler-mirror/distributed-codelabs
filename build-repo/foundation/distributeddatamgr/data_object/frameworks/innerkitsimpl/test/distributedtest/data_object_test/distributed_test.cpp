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

#include <string>
#include <thread>
#include "distributed_object.h"
#include "distributed_objectstore.h"
#include "objectstore_errors.h"
#include "distributed_major.h"
#include "softbus_adapter.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "hilog/log.h"
#include "ipc_skeleton.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
using namespace OHOS::DistributeSystemTest;
using namespace OHOS::ObjectStore;
using namespace OHOS::Security::AccessToken;

namespace  {
constexpr int MAX_RETRY_TIMES = 100;
constexpr int INTERVAL = 200;
constexpr HiLogLabel LABEL = {LOG_CORE, 0, "DistributedTest"};
const std::string DISTRIBUTED_DATASYNC = "ohos.permission.DISTRIBUTED_DATASYNC";
const std::string BUNDLENAME = "com.example.myapplication";
const std::string SESSIONID = "123456";

void GrantPermissionNative()
{
    const char **perms = new const char *[2];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[1] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "distributed_object",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    AccessTokenKit::ReloadNativeTokenInfo();
}
    
class DistributedTest : public DistributeTest {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class ObjectWatcherImpl : public ObjectWatcher {
public:
    bool GetDataStatus();
    void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) override;
    virtual ~ObjectWatcherImpl();
private:
    bool dataChanged_ = false;
};

class StatusNotifierImpl : public StatusNotifier {
public:
    std::string GetOnlineStatus();
    void OnChanged(
        const std::string &sessionId, const std::string &networkId, const std::string &onlineStatus) override;
    virtual ~StatusNotifierImpl();
private:
    std::string onlineStatus_ = "offline";
};

ObjectWatcherImpl::~ObjectWatcherImpl()
{
}

bool ObjectWatcherImpl::GetDataStatus()
{
    return dataChanged_;
}

void ObjectWatcherImpl::OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData)
{
    if (changedData.empty()) {
        HiLog::Info(LABEL, "empty change");
        return;
    }
    HiLog::Info(LABEL, "start %{public}s, %{public}s", sessionid.c_str(), changedData.at(0).c_str());
    dataChanged_ = true;
}

StatusNotifierImpl::~StatusNotifierImpl()
{
}

std::string StatusNotifierImpl::GetOnlineStatus()
{
    return onlineStatus_;
}

void StatusNotifierImpl::OnChanged(const std::string &sessionId,
                                   const std::string &networkId,
                                   const std::string &onlineStatus)
{
    onlineStatus_ = onlineStatus;
}

void DistributedTest::SetUpTestCase(void)
{
}

void DistributedTest::TearDownTestCase(void)
{
}

void DistributedTest::SetUp(void)
{
    GrantPermissionNative();
}

void DistributedTest::TearDown(void)
{
}

void WaitStatus(std::shared_ptr<StatusNotifierImpl> notifierPtr, const std::string &status)
{
    int times = 0;
    while (times < MAX_RETRY_TIMES && notifierPtr->GetOnlineStatus() != status) {
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL));
        times++;
    }
}

void WaitDataChangeStatus(std::shared_ptr<ObjectWatcherImpl> watcherPtr)
{
    int times = 0;
    while (times < MAX_RETRY_TIMES && !watcherPtr->GetDataStatus()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL));
        times++;
    }
}

/**
 * @tc.name: SendMessage
 * @tc.desc: normal testcase of DistributedTest
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, SendMessageCase, TestSize.Level1)
{
    std::string returnValue;
    std::string msgBuf = "recall";
    int ret = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });
    
    EXPECT_TRUE(ret > 0);
    EXPECT_EQ(returnValue, "recall Message");
}

/**
 * @tc.name: DistributedTest_Put_Get_001
 * @tc.desc: test DistributedTest Put & Get.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, Put_Get_001, TestSize.Level1)
{
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
    EXPECT_NE(nullptr, objectStore);
    
    DistributedObject *object = objectStore->CreateObject(SESSIONID);
    EXPECT_NE(nullptr, object);
    
    uint32_t ret = object->PutString("name", "zhangsan");
    EXPECT_EQ(SUCCESS, ret);
    
    std::string newvalue = "";
    ret = object->GetString("name", newvalue);
    EXPECT_EQ(SUCCESS, ret);
    
    std::string msgBuf = "GetItem";
    std::string returnValue;
    int result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });
    EXPECT_TRUE(result > 0);
    EXPECT_EQ(returnValue, "zhangsan");
    
    ret = objectStore->DeleteObject(SESSIONID);
    EXPECT_EQ(SUCCESS, ret);
    msgBuf = "DestroyObject";
    result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        return true;
    });
    EXPECT_TRUE(result > 0);
}

/**
 * @tc.name: DistributedTest_Put_Get_002
 * @tc.desc: test DistributedTest Put & Get.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, Put_Get_002, TestSize.Level1)
{
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(SESSIONID);
    EXPECT_NE(nullptr, object);
    
    auto watcherPtr = std::make_shared<ObjectWatcherImpl>();
    uint32_t ret = objectStore->Watch(object, watcherPtr);
    EXPECT_EQ(SUCCESS, ret);
    
    std::string msgBuf = "PutItem";
    std::string returnValue;
    int result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });
    EXPECT_TRUE(result > 0);
    EXPECT_EQ(returnValue, "PutSuccsess");
    
    WaitDataChangeStatus(watcherPtr);
    EXPECT_TRUE(watcherPtr->GetDataStatus());

    double value = 0.0;
    object->GetDouble("salary", value);
    EXPECT_EQ(value, 100.5);

    ret = objectStore->UnWatch(object);
    EXPECT_EQ(SUCCESS, ret);
    ret = objectStore->DeleteObject(SESSIONID);
    EXPECT_EQ(SUCCESS, ret);
    
    msgBuf = "DestroyObject";
    result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        return true;
    });
    EXPECT_TRUE(result > 0);
}

/**
 * @tc.name: DistributedTest_save_001
 * @tc.desc: test DistributedTest saveRemote.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, save_001, TestSize.Level1)
{
    std::string sessionId = SESSIONID;
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
    EXPECT_NE(nullptr, objectStore);
    
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);
    uint32_t ret = object->PutString("name", "zhangsan");
    EXPECT_EQ(SUCCESS, ret);
    
    std::vector<ObjectStore::DeviceInfo> devices = SoftBusAdapter::GetInstance()->GetDeviceList();
    std::vector<std::string> deviceIds;
    for (auto item : devices) {
    deviceIds.push_back(item.deviceId);
    }
    std::string networkId =SoftBusAdapter::GetInstance()->ToNodeID(deviceIds[0]);
    ret = object->Save(networkId);
    EXPECT_EQ(SUCCESS, ret);
    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
    
    std::string msgBuf = "GetItem";
    std::string returnValue;
    int result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });
    EXPECT_TRUE(result > 0);
    EXPECT_EQ(returnValue, "zhangsan");

    msgBuf = "DestroyObject";
    result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        return true;
    });
    EXPECT_TRUE(result > 0);
}

/**
 * @tc.name: DistributedTest_RevokeSave_001
 * @tc.desc: test DistributedTest RevokeSave.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, RevokeSave_001, TestSize.Level1)
{
    std::string msgBuf = "RevokeSave";
    std::string returnValue;
    int result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
            returnValue = szreturnbuf;
        return true;
    });
    EXPECT_TRUE(result > 0);

    std::string sessionId = SESSIONID;
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(sessionId);
    EXPECT_NE(nullptr, object);
    std::string getValue = "GetItem";
    uint32_t ret = object->GetString("name", getValue);
    EXPECT_NE(SUCCESS, ret);
    EXPECT_NE(getValue, returnValue);

    ret = objectStore->DeleteObject(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DistributedTest_Watch_001
 * @tc.desc: test DistributedTest Watch
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, Watch_001, TestSize.Level1)
{
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(SESSIONID);
    EXPECT_NE(nullptr, object);

    auto watcherPtr = std::make_shared<ObjectWatcherImpl>();
    uint32_t ret = objectStore->Watch(object, watcherPtr);
    EXPECT_EQ(SUCCESS, ret);

    std::string msgBuf = "PutItem";
    std::string returnValue;
    int result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });
    EXPECT_TRUE(result > 0);

    WaitDataChangeStatus(watcherPtr);
    EXPECT_TRUE(watcherPtr->GetDataStatus());
    ret = objectStore->UnWatch(object);
    EXPECT_EQ(SUCCESS, ret);
    ret = objectStore->DeleteObject(SESSIONID);
    EXPECT_EQ(SUCCESS, ret);

    msgBuf = "DestroyObject";
    result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        return true;
    });
    EXPECT_TRUE(result > 0);
}

/**
 * @tc.name: DistributedTest_UnWatch_001
 * @tc.desc: test DistributedTest UnWatch.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, UnWatch_001, TestSize.Level1)
{
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(SESSIONID);
    EXPECT_NE(nullptr, object);

    auto watcherPtr = std::make_shared<ObjectWatcherImpl>();
    uint32_t ret = objectStore->Watch(object, watcherPtr);
    EXPECT_EQ(SUCCESS, ret);

    ret = objectStore->UnWatch(object);
    EXPECT_EQ(SUCCESS, ret);

    std::string msgBuf = "PutItem";
    std::string returnValue;
    int result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });
    EXPECT_TRUE(result > 0);

    EXPECT_NE(nullptr, watcherPtr);
    EXPECT_FALSE(watcherPtr->GetDataStatus());
    ret = objectStore->DeleteObject(SESSIONID);
    EXPECT_EQ(SUCCESS, ret);

    msgBuf = "DestroyObject";
    result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        return true;
    });
    EXPECT_TRUE(result > 0);
}

/**
 * @tc.name: DistributedTest_SetStatusNotifier_001
 * @tc.desc: test DistributedTest SetStatusNotifier.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, SetStatusNotifier_001, TestSize.Level1)
{
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(SESSIONID);
    EXPECT_NE(nullptr, object);

    auto notifierPtr = std::make_shared<StatusNotifierImpl>();
    uint32_t ret = objectStore->SetStatusNotifier(notifierPtr);
    EXPECT_EQ(ret, SUCCESS);

    std::string msgBuf = "PutItem";
    std::string returnValue;
    int result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });

    EXPECT_TRUE(result > 0);
    
    WaitStatus(notifierPtr, "online");
    EXPECT_EQ(notifierPtr->GetOnlineStatus(), "online");
    
    ret = objectStore->DeleteObject(SESSIONID);
    EXPECT_EQ(SUCCESS, ret);

    msgBuf = "DestroyObject";
    result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        return true;
    });
    EXPECT_TRUE(result > 0);
}

/**
 * @tc.name: DistributedTest_SetStatusNotifier_002
 * @tc.desc: test DistributedTest SetStatusNotifier.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedTest, SetStatusNotifier_002, TestSize.Level1)
{
    std::string msgBuf = "PutItem";
    std::string returnValue;
    int result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });
    EXPECT_TRUE(result > 0);
    
    DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
    EXPECT_NE(nullptr, objectStore);
    DistributedObject *object = objectStore->CreateObject(SESSIONID);
    EXPECT_NE(nullptr, object);
    
    auto notifierPtr = std::make_shared<StatusNotifierImpl>();
    uint32_t ret = objectStore->SetStatusNotifier(notifierPtr);
    EXPECT_EQ(ret, SUCCESS);
    auto watcherPtr = std::make_shared<ObjectWatcherImpl>();
    ret = objectStore->Watch(object, watcherPtr);
    
    WaitStatus(notifierPtr, "online");
    EXPECT_EQ(notifierPtr->GetOnlineStatus(), "online");
    
    WaitDataChangeStatus(watcherPtr);
    EXPECT_TRUE(watcherPtr->GetDataStatus());

    msgBuf = "SaveRemote";
    result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        returnValue = szreturnbuf;
        return true;
    });
    EXPECT_TRUE(result > 0);
    
    WaitStatus(notifierPtr, "restored");
    EXPECT_EQ(notifierPtr->GetOnlineStatus(), "restored");
    
    ret = objectStore->DeleteObject(SESSIONID);
    EXPECT_EQ(SUCCESS, ret);

    msgBuf = "DestroyObject";
    result = SendMessage(AGENT_NO::ONE, msgBuf, msgBuf.size(),
        [&](const std::string &szreturnbuf, int rlen)->bool {
        return true;
    });
    EXPECT_TRUE(result > 0);
}
}

int main(int argc, char *argv[])
{
    HiLog::Info(LABEL, "begin");
    g_pDistributetestEnv = new DistributeTestEnvironment("major.desc");
    testing::AddGlobalTestEnvironment(g_pDistributetestEnv);
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}