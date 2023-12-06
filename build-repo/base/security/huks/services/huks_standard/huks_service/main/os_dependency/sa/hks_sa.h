/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef HKS_SA_H
#define HKS_SA_H

#include "iremote_broker.h"
#include "iremote_stub.h"
#include "nocopyable.h"
#include "system_ability.h"

namespace OHOS {
namespace Security {
namespace Hks {
enum ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};
enum ResponseCode {
    HW_NO_ERROR =  0,
    HW_SYSTEM_ERROR = -1,
    HW_PERMISSION_DENIED = -2,
};

enum HksMessage {
    HKS_MSG_BASE = 0x3a400,

    HKS_MSG_GEN_KEY = HKS_MSG_BASE,
    HKS_MSG_IMPORT_KEY,
    HKS_MSG_EXPORT_PUBLIC_KEY,
    HKS_MSG_IMPORT_WRAPPED_KEY,
    HKS_MSG_DELETE_KEY,
    HKS_MSG_GET_KEY_PARAMSET,
    HKS_MSG_KEY_EXIST,
    HKS_MSG_GENERATE_RANDOM,
    HKS_MSG_SIGN,
    HKS_MSG_VERIFY,
    HKS_MSG_ENCRYPT,
    HKS_MSG_DECRYPT,
    HKS_MSG_AGREE_KEY,
    HKS_MSG_DERIVE_KEY,
    HKS_MSG_MAC,
    HKS_MSG_GET_KEY_INFO_LIST,
    HKS_MSG_ATTEST_KEY,
    HKS_MSG_GET_CERTIFICATE_CHAIN,
    HKS_MSG_INIT,
    HKS_MSG_UPDATE,
    HKS_MSG_FINISH,
    HKS_MSG_ABORT,

    /* new cmd type must be added before HKS_MSG_MAX */
    HKS_MSG_MAX,
};

constexpr int SA_ID_KEYSTORE_SERVICE = 3510;

class IHksService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.security.hks.service");
    // message code
    uint32_t MSG_CODE_BASE = HKS_MSG_BASE;
    uint32_t MSG_CODE_INIT = HKS_MSG_INIT;
    uint32_t MSG_CODE_UPDATE = HKS_MSG_UPDATE;
    uint32_t MSG_CODE_FINISH = HKS_MSG_FINISH;
    uint32_t MSG_CODE_ABORT = HKS_MSG_ABORT;
    uint32_t MSG_CODE_GEN_KEY = HKS_MSG_GEN_KEY;
    uint32_t MSG_CODE_IMPORT_KEY = HKS_MSG_IMPORT_KEY;
    uint32_t MSG_CODE_EXPORT_PUBLIC_KEY = HKS_MSG_EXPORT_PUBLIC_KEY;
    uint32_t MSG_CODE_IMPORT_WRAPPED_KEY = HKS_MSG_IMPORT_WRAPPED_KEY;
    uint32_t MSG_CODE_DELETE_KEY = HKS_MSG_DELETE_KEY;
    uint32_t MSG_CODE_GET_KEY_PARAMSET = HKS_MSG_GET_KEY_PARAMSET;
    uint32_t MSG_CODE_KEY_EXIST = HKS_MSG_KEY_EXIST;
    uint32_t MSG_CODE_GENERATE_RANDOM = HKS_MSG_GENERATE_RANDOM;
    uint32_t MSG_CODE_SIGN = HKS_MSG_SIGN;
    uint32_t MSG_CODE_VERIFY = HKS_MSG_VERIFY;
    uint32_t MSG_CODE_ENCRYPT = HKS_MSG_ENCRYPT;
    uint32_t MSG_CODE_DECRYPT = HKS_MSG_DECRYPT;
    uint32_t MSG_CODE_AGREE_KEY = HKS_MSG_AGREE_KEY;
    uint32_t MSG_CODE_DERIVE_KEY = HKS_MSG_DERIVE_KEY;
    uint32_t MSG_CODE_MAC = HKS_MSG_MAC;
    uint32_t MSG_CODE_GET_KEY_INFO_LIST = HKS_MSG_GET_KEY_INFO_LIST;
    uint32_t MSG_CODE_ATTEST_KEY = HKS_MSG_ATTEST_KEY;
    uint32_t MSG_CODE_GET_CERTIFICATE_CHAIN = HKS_MSG_GET_CERTIFICATE_CHAIN;
    /* new message code must be added before MSG_CODE_MAX */
    uint32_t MSG_CODE_MAX = HKS_MSG_MAX;
};

class HksService : public SystemAbility, public IRemoteStub<IHksService> {
    DECLEAR_SYSTEM_ABILITY(HksService)

public:
    DISALLOW_COPY_AND_MOVE(HksService);
    HksService(int saId, bool runOnCreate);
    virtual ~HksService();

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    static sptr<HksService> GetInstance();

protected:
    void OnStart() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnStop() override;

private:
    HksService();
    bool Init();

    bool registerToService_;
    ServiceRunningState runningState_;
    static std::mutex instanceLock;
    static sptr<HksService> instance;
};
} // namespace Hks
} // namespace Security
} // namespace OHOS

#endif // HKS_SA_H
