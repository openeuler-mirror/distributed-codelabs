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

#include "ondemand_helper.h"

#include <iostream>
#include <memory>
#include <thread>

#include "datetime_ex.h"
#include "errors.h"
#include "if_system_ability_manager.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "isystem_ability_load_callback.h"
#include "nativetoken_kit.h"
#include "sam_mock_permission.h"
#include "softbus_bus_center.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

using namespace OHOS;
using namespace std;

namespace OHOS {
namespace {
constexpr int32_t LOOP_TIME = 1000;
}

OnDemandHelper::OnDemandHelper()
{
    loadCallback_ = new OnDemandLoadCallback();
    loadCallback2_ = new OnDemandLoadCallback();
    loadCallback3_ = new OnDemandLoadCallback();
    loadCallback4_ = new OnDemandLoadCallback();
}

OnDemandHelper& OnDemandHelper::GetInstance()
{
    static OnDemandHelper instance;
    return instance;
}

int32_t OnDemandHelper::OnDemandAbility(int32_t systemAbilityId)
{
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = sm->LoadSystemAbility(systemAbilityId, loadCallback_);
    if (result != ERR_OK) {
        cout << "systemAbilityId:" << systemAbilityId << " load failed, result code:" << result << endl;
        return result;
    }
    cout << "LoadSystemAbility result:" << result << " spend:" << (GetTickCount() - begin) << " ms"
            << " systemAbilityId:" << systemAbilityId << endl;
    return ERR_OK;
}

void OnDemandHelper::GetDeviceList()
{
    NodeBasicInfo *info = NULL;
    int32_t infoNum = 0;
    int32_t ret = GetAllNodeDeviceInfo("ondemand", &info, &infoNum);
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

std::string OnDemandHelper::GetFirstDevice()
{
    NodeBasicInfo *info = NULL;
    int32_t infoNum = 0;
    int32_t ret = GetAllNodeDeviceInfo("ondemand", &info, &infoNum);
    if (ret != 0) {
        cout << "get remote deviceid GetAllNodeDeviceInfo failed" << endl;
        return "";
    }
    if (infoNum <= 0) {
        cout << "get remote deviceid failed, no remote device" << endl;
        return "";
    }
    return std::string(info->networkId);
}

int32_t OnDemandHelper::LoadRemoteAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    cout << "LoadRemoteAbility start"<< endl;
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return ERR_NULL_OBJECT;
    }
    int32_t result = -1;
    if (callback == nullptr) {
        result = sm->LoadSystemAbility(systemAbilityId, deviceId, loadCallback_);
    } else {
        result = sm->LoadSystemAbility(systemAbilityId, deviceId, callback);
    }

    if (result != ERR_OK) {
        cout << "systemAbilityId:" << systemAbilityId << " load failed, result code:" << result << endl;
    }
    cout << "LoadRemoteAbility result:" << result << " spend:" << (GetTickCount() - begin) << " ms"
        << " systemAbilityId:" << systemAbilityId << endl;
    return result;
}

void OnDemandHelper::LoadRemoteAbilityMuti(int32_t systemAbilityId, const std::string& deviceId)
{
    std::thread thread1([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread1" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread2([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread2" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread3([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread3" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    thread1.detach();
    thread2.detach();
    thread3.detach();
}

void OnDemandHelper::LoadRemoteAbilityMutiCb(int32_t systemAbilityId, const std::string& deviceId)
{
    std::thread thread1([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread1" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread2([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread2" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback2_);
    });
    std::thread thread3([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread3" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback3_);
    });
    thread1.detach();
    thread2.detach();
    thread3.detach();
}

void OnDemandHelper::LoadRemoteAbilityMutiSA(int32_t systemAbilityId, const std::string& deviceId)
{
    std::thread thread1([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread1" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread2([deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread2" << endl;
        LoadRemoteAbility(DISTRIBUTED_SCHED_TEST_LISTEN_ID, deviceId, loadCallback_);
    });
    std::thread thread3([deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread3" << endl;
        LoadRemoteAbility(DISTRIBUTED_SCHED_TEST_MEDIA_ID, deviceId, loadCallback_);
    });
    thread1.detach();
    thread2.detach();
    thread3.detach();
}

void OnDemandHelper::LoadRemoteAbilityMutiSACb(int32_t systemAbilityId, const std::string& deviceId)
{
    std::thread thread1([systemAbilityId, deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread1" << endl;
        LoadRemoteAbility(systemAbilityId, deviceId, loadCallback_);
    });
    std::thread thread2([deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread2" << endl;
        LoadRemoteAbility(DISTRIBUTED_SCHED_SA_ID, deviceId, loadCallback2_);
    });
    std::thread thread3([deviceId, this]() {
        cout << "LoadRemoteAbilityMuti thread3" << endl;
        LoadRemoteAbility(DISTRIBUTED_SCHED_TEST_MEDIA_ID, deviceId, loadCallback3_);
    });
    thread1.detach();
    thread2.detach();
    thread3.detach();
}

void OnDemandHelper::LoadRemoteAbilityPressure(int32_t systemAbilityId, const std::string& deviceId)
{
    for (int i = 0 ; i < LOOP_TIME; ++i) {
        LoadRemoteAbility(systemAbilityId, deviceId, nullptr);
    }
}

sptr<IRemoteObject> OnDemandHelper::GetSystemAbility(int32_t systemAbilityId)
{
    int64_t begin = GetTickCount();
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        cout << "GetSystemAbilityManager samgr object null!" << endl;
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = sm->GetSystemAbility(systemAbilityId);
    if (remoteObject == nullptr) {
        cout << "GetSystemAbility systemAbilityId:" << systemAbilityId << " failed !" << endl;
        return nullptr;
    }
    cout << "GetSystemAbility result: success "<< " spend:"
        << (GetTickCount() - begin) << " ms" << " systemAbilityId:" << systemAbilityId << endl;
    return remoteObject;
}

void OnDemandHelper::OnLoadSystemAbility(int32_t systemAbilityId)
{
}

void OnDemandHelper::OnDemandLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject>& remoteObject)
{
    cout << "OnLoadSystemAbilitySuccess systemAbilityId:" << systemAbilityId << " IRemoteObject result:" <<
        ((remoteObject != nullptr) ? "succeed" : "failed") << endl;
    OnDemandHelper::GetInstance().OnLoadSystemAbility(systemAbilityId);
}

void OnDemandHelper::OnDemandLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    cout << "OnLoadSystemAbilityFail systemAbilityId:" << systemAbilityId << endl;
}

void OnDemandHelper::OnDemandLoadCallback::OnLoadSACompleteForRemote(const std::string& deviceId,
    int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject)
{
    cout << "OnLoadSACompleteForRemote systemAbilityId:" << systemAbilityId << " IRemoteObject result:" <<
        ((remoteObject != nullptr) ? "succeed" : "failed") << endl;
}
}

int main(int argc, char* argv[])
{
    SamMockPermission::MockPermission();
    OHOS::OnDemandHelper& ondemandHelper = OnDemandHelper::GetInstance();
    string cmd = "load";
    do {
        cout << "please input operation " << endl;
        cin >> cmd;
        int32_t systemAbilityId = 0;
        std::string deviceId = ondemandHelper.GetFirstDevice();
        cout << "please input systemAbilityId for " << cmd << " operation" << endl;
        cin >> systemAbilityId;
        if (cmd == "get") {
            ondemandHelper.GetSystemAbility(systemAbilityId);
        } else if (cmd == "load") {
            ondemandHelper.OnDemandAbility(systemAbilityId);
        } else if (cmd == "device") { // get remote networkid
            ondemandHelper.GetDeviceList();
        } else if (cmd == "loadrmt1") { // single thread with one device, one system ability, one callback
            ondemandHelper.LoadRemoteAbility(systemAbilityId, deviceId, nullptr);
        } else if (cmd == "loadrmt2") { // one device, one system ability, one callback, three threads
            ondemandHelper.LoadRemoteAbilityMuti(systemAbilityId, deviceId);
        } else if (cmd == "loadrmt3") { // one device, one system ability, three callbacks, three threads
            ondemandHelper.LoadRemoteAbilityMutiCb(systemAbilityId, deviceId);
        } else if (cmd == "loadrmt4") { // one device, three system abilities, one callback, three threads
            ondemandHelper.LoadRemoteAbilityMutiSA(systemAbilityId, deviceId);
        } else if (cmd == "loadrmt5") { // one device, three system abilities, three callbacks, three threads
            ondemandHelper.LoadRemoteAbilityMutiSACb(systemAbilityId, deviceId);
        } else if (cmd == "loadrmt6") { // two devices
            int32_t otherSystemAbilityId = 0;
            cout << "please input another systemabilityId for " << cmd << " operation" << endl;
            cin >> otherSystemAbilityId;
            cout << "please input another deviceId for " << cmd << " operation" << endl;
            std::string otherDevice;
            cin >> otherDevice;
            ondemandHelper.LoadRemoteAbility(systemAbilityId, deviceId, nullptr);
            ondemandHelper.LoadRemoteAbility(otherSystemAbilityId, otherDevice, nullptr);
        } else if (cmd == "loadmuti") {
            ondemandHelper.LoadRemoteAbilityPressure(systemAbilityId, deviceId);
        }
        cout << "-----Input q or Q to quit, [load] for LoadSystemAbility, [get] for GetSystemAbility-----" << endl;
        cmd.clear();
        cin.clear();
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
    } while (cmd[0] != 'q' && cmd[0] != 'Q');
    return 0;
}
