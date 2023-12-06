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

#ifndef OHOS_DM_CREDENTIAL_MANAGER_H
#define OHOS_DM_CREDENTIAL_MANAGER_H

#include "device_auth.h"
#include "idevice_manager_service_listener.h"
#include "dm_timer.h"
#include "hichain_connector.h"
namespace OHOS {
namespace DistributedHardware {
typedef struct {
    int32_t credentialType;
    std::string credentialId;
    std::string serverPk;
    std::string pkInfoSignature;
    std::string pkInfo;
    std::string authCode;
    std::string peerDeviceId;
} CredentialData;
class DmCredentialManager final : public IDmGroupResCallback,
                                  public std::enable_shared_from_this<DmCredentialManager> {
public:
    DmCredentialManager(std::shared_ptr<HiChainConnector> hiChainConnector,
                        std::shared_ptr<IDeviceManagerServiceListener> listener);
    ~DmCredentialManager();

    /**
     * @tc.name: HiChainConnector::RegisterCredentialCallback
     * @tc.desc: Register Credential Callback Info of the DmCredential Manager
     * @tc.type: FUNC
     */
    int32_t RegisterCredentialCallback(const std::string &pkgName);

    /**
     * @tc.name: HiChainConnector::UnRegisterCredentialCallback
     * @tc.desc: UnRegister Credential Callback Info of the DmCredential Manager
     * @tc.type: FUNC
     */
    int32_t UnRegisterCredentialCallback(const std::string &pkgName);

    /**
     * @tc.name: HiChainConnector::RequestCredential
     * @tc.desc: Request Credential Info of the DmCredential Manager
     * @tc.type: FUNC
     */
    int32_t RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr);
    
    /**
     * @tc.name: HiChainConnector::ImportCredential
     * @tc.desc: Import Credential Info of the DmCredential Manager
     * @tc.type: FUNC
     */
    int32_t ImportCredential(const std::string &pkgName, const std::string &credentialInfo);

    /**
     * @tc.name: HiChainConnector::ImportLocalCredential
     * @tc.desc: Import Nonsymmetry Credential Info of the DmCredential Manager
     * @tc.type: FUNC
     */
    int32_t ImportLocalCredential(const std::string &credentialInfo);

    /**
     * @tc.name: DmCredentialManager::ImportRemoteCredential
     * @tc.desc: Import Symmetry Credential Info of the DmCredential Manager
     * @tc.type: FUNC
     */
    int32_t ImportRemoteCredential(const std::string &credentialInfo);

    /**
     * @tc.name: DmCredentialManager::DeleteRemoteCredential
     * @tc.desc: delete Symmetry Credential Info of the DmCredential Manager
     * @tc.type: FUNC
     */
    int32_t DeleteRemoteCredential(const std::string &credentialInfo);

     /**
     * @tc.name: HiChainConnector::DeleteCredential
     * @tc.desc: Delete Credential Info of the DmCredential Manager
     * @tc.type: FUNC
     */
    int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo);

     /**
     * @tc.name: HiChainConnector::OnCredentialResult
     * @tc.desc: Credential Result of the DmCredential Manager
     * @tc.type: FUNC
     */
    void OnGroupResult(int64_t requestId, int32_t action, const std::string &resultInfo);
private:
    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::vector<std::string> credentialVec_;
    int64_t requestId_;
    std::mutex locks_;
    std::string pkgName_;
private:
    int32_t GetCredentialData(const std::string &credentialInfo, const CredentialData &inputCreData,
        nlohmann::json &jsonOutObj);
    int32_t GetAddDeviceList(const nlohmann::json &jsonObject, nlohmann::json &jsonDeviceList);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CREDENTIAL_MANAGER_H