/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DAEMON_H
#define DAEMON_H

#include <memory>
#include <mutex>
#include <string>

#include "daemon_stub.h"
#include "i_daemon.h"
#include "iremote_stub.h"
#include "multiuser/os_account_observer.h"
#include "nocopyable.h"
#include "refbase.h"
#include "system_ability.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class Daemon final : public SystemAbility, public DaemonStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(Daemon);

public:
    explicit Daemon(int32_t saID, bool runOnCreate = true) : SystemAbility(saID, runOnCreate) {};
    virtual ~Daemon() = default;

    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }

    int32_t EchoServerDemo(const std::string &echoStr) override;

private:
    Daemon();
    ServiceRunningState state_ { ServiceRunningState::STATE_NOT_START };
    static sptr<Daemon> instance_;
    static std::mutex instanceLock_;
    bool registerToService_ { false };
    std::shared_ptr<OsAccountObserver> subScriber_;
    void PublishSA();
    void RegisterOsAccount();
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DAEMON_H