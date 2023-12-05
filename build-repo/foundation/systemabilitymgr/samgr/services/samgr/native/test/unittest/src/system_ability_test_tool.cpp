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

#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "datetime_ex.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "nativetoken_kit.h"
#include "sam_log.h"
#include "softbus_bus_center.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "system_ability_load_callback_stub.h"
#include "system_ability_status_change_stub.h"
#include "token_setproc.h"

#define private public
#include "system_ability_manager.h"

using namespace OHOS;
using namespace std;

namespace {
    constexpr int32_t PARSE_WAIT_TIME = 1000 * 50; // us
    constexpr int32_t DEFAULT_SA_ID = 1401;
    constexpr int32_t ARGC_DEFAULT_LENTH = 2;
    constexpr int32_t ARGC_EXTEND_LENTH = 3;
    constexpr int32_t ARV_SAID_INDEX = 2;
    constexpr int32_t ARV_DEVICEID_INDEX = 3;

    const string HELP_CONTENT = "usage: samgr test tool <command> <options>\n"
                            "These are common samgr test tool commands list:\n"
                            "  h            list available commands\n"
                            "  ls           list all said\n"
                            "  parse        list all sa xml in profile\n"
                            "  get          get loacl system ability with given said\n"
                            "  getrmt       get remote system ability with given said and deviceid\n"
                            "  add          add mock system ability\n"
                            "  remove       remove mock system ability\n"
                            "  load         load local system ability\n"
                            "  loadrmt      load remote system ability\n"
                            "  subscribe    subscribe sa info with options\n"
                            "  unsubcribe   unsubscribe sa info with options\n"
                            "  device       get remote device networkid";

    class MockLoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
        void OnLoadSACompleteForRemote(const std::string& deviceId,
            int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
    };

    void MockLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
        const sptr<IRemoteObject>& remoteObject)
    {
        cout << "OnLoadSystemAbilitySuccess systemAbilityId:" << systemAbilityId << " IRemoteObject result:" <<
            ((remoteObject != nullptr) ? "succeed" : "failed") << endl;
    }

    void MockLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
    {
        cout << "OnLoadSystemAbilityFail systemAbilityId:" << systemAbilityId << endl;
    }

    void MockLoadCallback::OnLoadSACompleteForRemote(const std::string& deviceId, int32_t systemAbilityId,
        const sptr<IRemoteObject>& remoteObject)
    {
        cout << "OnLoadSACompleteForRemote systemAbilityId:" << systemAbilityId <<  "ret : "<<
            ((remoteObject != nullptr) ? "succeed" : "failed") << endl;
    }

    class MockSaStatusChange : public SystemAbilityStatusChangeStub {
    public:
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    };

    void MockSaStatusChange::OnAddSystemAbility(int32_t systemAbilityId, [[maybe_unused]] const std::string& deviceId)
    {
        cout << "OnAddSystemAbility said : " << systemAbilityId << endl;
    }

    void MockSaStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId,
        [[maybe_unused]] const std::string& deviceId)
    {
        cout << "OnRemoveSystemAbility said : " << systemAbilityId << endl;
    }

    static void DoHelp()
    {
        cout << HELP_CONTENT << endl;
    }

    static void DoList()
    {
        cout << "system ability list" << endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }
        vector<u16string> abilities = sm->ListSystemAbilities();
        cout << "total sa : " << abilities.size() << endl;
        for (u16string said : abilities) {
            cout << Str16ToStr8(said)<< endl;
        }
    }

    static void DoPasre()
    {
        cout << "parse system ability profile" << endl;
        sptr<SystemAbilityManager> manager = SystemAbilityManager::GetInstance();
        if (manager == nullptr) {
            cout << "------------------manager is unavailable-----------------------------------------" << endl;
            return;
        }
        manager->Init();
        usleep(PARSE_WAIT_TIME);
        cout << "init system ability profile size " << manager->saProfileMap_.size() << endl;
        for (const auto& [said, saProfile] : manager->saProfileMap_) {
            cout << "system ability profile said " << said << " process " << Str16ToStr8(saProfile.process) << endl;
        }
    }


    static void DoGet(int32_t said)
    {
        cout << "get system ability " << said <<endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }

        auto ability = sm->GetSystemAbility(said);
        if (ability != nullptr) {
            cout << "----------------- find sa : "<< said << "-----------------------------------------" << endl;
        } else {
            cout << "----------------- not find sa : "<< said << "--------------------------------------" << endl;
        }
    }

    static void DoGetRmt(int32_t said, const string& deviceid)
    {
        cout << "get remote system ability " << said << endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }

        auto ability = sm->GetSystemAbility(said, deviceid);
        if (ability != nullptr) {
            cout << "----------------- find remote sa : "<< said << " remoteobject : "<< ability <<"---------" << endl;
        } else {
            cout << "----------------- not find remote sa : "<< said << "------------------------------------" << endl;
        }
    }

    static void DoLoadRemote(int32_t said, const string& deviceid)
    {
        cout << "DoLoadRemote system ability " << said <<endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }
        sptr<MockLoadCallback> callback = new MockLoadCallback();
        int32_t res = sm->LoadSystemAbility(said, deviceid, callback);
        cout << "load system ability result : " << ((res == 0) ? "succeed" : "failed") << endl;
    }

    static void DoAdd()
    {
        cout << "add mock system ability : " << DISTRIBUTED_SCHED_TEST_TT_ID << endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }
        sptr<IRemoteObject> testAbility = new TestTransactionService();
        int32_t res = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
        cout << "add system ability result : " << ((res == 0) ? "succeed" : "failed") << endl;
    }

    static void DoRemove()
    {
        cout << "remove mock system ability" << endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }
        sptr<IRemoteObject> testAbility = new TestTransactionService();
        sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
        int32_t res = sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
        cout << "remove system ability result : " << ((res == 0) ? "succeed" : "failed") << endl;
    }

    static void DoLoad(int32_t said)
    {
        cout << "load system ability " << said << endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }

        sptr<MockLoadCallback> callback = new MockLoadCallback();
        int32_t res = sm->LoadSystemAbility(said, callback);
        cout << "load system ability result : " << ((res == 0) ? "succeed" : "failed") << endl;
    }

    static void DoGetDevice()
    {
        cout << "get remote deviceid" << endl;
        NodeBasicInfo *info = NULL;
        int32_t infoNum = 0;
        int32_t ret = GetAllNodeDeviceInfo("TestTool", &info, &infoNum);
        if (ret != 0) {
            cout << "get remote deviceid GetAllNodeDeviceInfo failed" << endl;
            return;
        }
        for (int32_t i = 0; i < infoNum; i++) {
            cout << "networkid : " << std::string(info->networkId) << " deviceName : "
                << std::string(info->deviceName) << endl;
            info++;
        }
    }

    static void DoSubscribe(int32_t said)
    {
        cout << "subscribe system ability" << endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }

        sptr<MockSaStatusChange> listen = new MockSaStatusChange();
        int32_t res = sm->SubscribeSystemAbility(said, listen);
        cout << "subscribe system ability result : " << ((res == 0) ? "succeed" : "failed") << endl;
    }

    static void DoUnSubscribe(int32_t said)
    {
        cout << "unsubscribe system ability" << endl;
        sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sm == nullptr)  {
            cout << "------------------samgr is unavailable-----------------------------------------" << endl;
            return;
        }

        sptr<MockSaStatusChange> listen = new MockSaStatusChange();
        int32_t res = sm->UnSubscribeSystemAbility(said, listen);
        cout << "unsubscribe system ability result : " << ((res == 0) ? "succeed" : "failed") << endl;
    }
}

static void DoDefault(char* argv[])
{
    if (strcmp(argv[1], "h") == 0) {
        DoHelp();
        return;
    }
    if (strcmp(argv[1], "ls") == 0) {
        DoList();
        return;
    }
    if (strcmp(argv[1], "parse") == 0) {
        DoPasre();
        return;
    }
    if (strcmp(argv[1], "add") == 0) {
        DoAdd();
        return;
    }
    if (strcmp(argv[1], "remove") == 0) {
        DoRemove();
        return;
    }
    if (strcmp(argv[1], "device") == 0) {
        DoGetDevice();
        return;
    }
    cout << "DoDefault failed, get help with arg 'h'"<< endl;
}

static void DoExtend(char* argv[])
{
    if (strcmp(argv[1], "get") == 0) {
        int32_t said = DEFAULT_SA_ID;
        StrToInt(argv[ARV_SAID_INDEX], said);
        DoGet(said);
        return;
    }
    if (strcmp(argv[1], "load") == 0) {
        int32_t said = DEFAULT_SA_ID;
        StrToInt(argv[ARV_SAID_INDEX], said);
        DoLoad(said);
        return;
    }

    if (strcmp(argv[1], "subscribe") == 0) {
        int32_t said = DEFAULT_SA_ID;
        StrToInt(argv[ARV_SAID_INDEX], said);
        DoSubscribe(said);
        return;
    }

    if (strcmp(argv[1], "unsubscribe") == 0) {
        int32_t said = DEFAULT_SA_ID;
        StrToInt(argv[ARV_SAID_INDEX], said);
        DoUnSubscribe(said);
        return;
    }
    cout << "DoExtend failed, get help with arg 'h'"<< endl;
}

static void DoRemote(char* argv[])
{
    if (strcmp(argv[1], "getrmt") == 0) {
        int32_t said = DEFAULT_SA_ID;
        StrToInt(argv[ARV_SAID_INDEX], said);
        string deviceid = argv[ARV_DEVICEID_INDEX];
        DoGetRmt(said, deviceid);
        return;
    }
    if (strcmp(argv[1], "loadrmt") == 0) {
        int32_t said = DEFAULT_SA_ID;
        StrToInt(argv[ARV_SAID_INDEX], said);
        string deviceid = argv[ARV_DEVICEID_INDEX];
        DoLoadRemote(said, deviceid);
        return;
    }
    cout << "DoRemote failed, get help with arg 'h'"<< endl;
}

int main(int argc, char* argv[])
{
    static const char *PERMS[] = {
        "ohos.permission.DISTRIBUTED_DATASYNC"
    };
    uint64_t tokenId;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = PERMS,
        .acls = nullptr,
        .processName = "distributedsched",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    if (argc == ARGC_DEFAULT_LENTH) {
        DoDefault(argv);
    } else if (argc == ARGC_EXTEND_LENTH) {
        DoExtend(argv);
    } else if (argc > ARGC_EXTEND_LENTH) {
        DoRemote(argv);
    } else {
        cout << "invalid command, get help with arg 'h'"<< endl;
    }
    return 0;
}