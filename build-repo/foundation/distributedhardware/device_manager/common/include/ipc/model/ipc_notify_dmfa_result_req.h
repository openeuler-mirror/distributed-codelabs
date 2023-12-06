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

#ifndef OHOS_DEVICE_MANAGER_IPC_NOTIFY_DMFA_RESULT_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_NOTIFY_DMFA_RESULT_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyDMFAResultReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyDMFAResultReq);

public:
    /**
     * @tc.name: IpcNotifyDMFAResultReq::GetJsonParam
     * @tc.desc: IPC notifies device management FA request result to get json parameters
     * @tc.type: FUNC
     */
    std::string GetJsonParam() const
    {
        return JsonParam_;
    }

    /**
     * @tc.name: IpcNotifyDMFAResultReq::SetJsonParam
     * @tc.desc: IPC notifies device management FA request result to set json parameters
     * @tc.type: FUNC
     */
    void SetJsonParam(const std::string &JsonParam)
    {
        JsonParam_ = JsonParam;
    }

private:
    std::string JsonParam_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_NOTIFY_DMFA_RESULT_REQ_H
