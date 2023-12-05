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

#include "dslm_service.h"

#include <thread>

#include "iremote_object.h"
#include "singleton.h"
#include "utils_log.h"

#include "device_security_defines.h"
#include "dslm_hidumper.h"
#include "dslm_ipc_process.h"
#include "dslm_rpc_process.h"

namespace OHOS {
namespace Security {
namespace DeviceSecurityLevel {
REGISTER_SYSTEM_ABILITY_BY_ID(DslmService, DEVICE_SECURITY_LEVEL_MANAGER_SA_ID, true);

DslmService::DslmService(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
    SECURITY_LOG_INFO("object initialization");
}

void DslmService::OnStart()
{
    SECURITY_LOG_INFO("start");

    std::thread thread([this]() {
        if (InitService() == SUCCESS) {
            SECURITY_LOG_INFO("init service success");
        }
        if (!Publish(this)) {
            SECURITY_LOG_ERROR("publish service failed");
        }
    });
    thread.detach();
}

void DslmService::OnStop()
{
    UnInitService();
    SECURITY_LOG_INFO("stop service");
}

int32_t DslmService::Dump(int fd, const std::vector<std::u16string> &args)
{
    DslmDumper(fd);
    return 0;
}

int32_t DslmService::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    do {
        if (IDeviceSecurityLevel::GetDescriptor() != data.ReadInterfaceToken()) {
            SECURITY_LOG_ERROR("local descriptor is not equal remote");
            break;
        }
        switch (code) {
            case CMD_GET_DEVICE_SECURITY_LEVEL:
                return ProcessGetDeviceSecurityLevel(data, reply);
            default:
                return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    } while (false);

    return ERR_REQUEST_CODE_ERR;
}

int32_t DslmService::ProcessGetDeviceSecurityLevel(MessageParcel &data, MessageParcel &reply)
{
    return Singleton<DslmIpcProcess>::GetInstance().DslmProcessGetDeviceSecurityLevel(data, reply);
}
} // namespace DeviceSecurityLevel
} // namespace Security
} // namespace OHOS
