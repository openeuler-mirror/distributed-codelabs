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

#ifndef SESSION_POOL_H
#define SESSION_POOL_H

#include <list>
#include <memory>
#include <mutex>
#include <set>

#include "network/base_session.h"
#include "network/kernel_talker.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SessionPool final : protected NoCopyable {
public:
    explicit SessionPool(std::shared_ptr<KernelTalker> &talker) : talker_(talker) {}
    ~SessionPool() = default;
    void HoldSession(std::shared_ptr<BaseSession> session);
    void ReleaseSession(const int32_t fd);
    void ReleaseSession(const std::string &cid);
    void ReleaseAllSession();

private:
    std::recursive_mutex sessionPoolLock_;
    std::list<std::shared_ptr<BaseSession>> usrSpaceSessionPool_;
    std::shared_ptr<KernelTalker> &talker_;

    void AddSessionToPool(std::shared_ptr<BaseSession> session);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // SESSION_POOL_H