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

#ifndef MOUNT_POINT_H
#define MOUNT_POINT_H

#include <atomic>
#include <functional>

#include "dfsu_mount_argument_descriptors.h"
#include "nocopyable.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class MountPoint final : public NoCopyable {
public:
    MountPoint(const Utils::MountArgument &mountArg);
    ~MountPoint() = default;

    uint32_t GetID() const
    {
        return id_;
    };

    bool isAccountLess() const
    {
        return mountArg_.accountless_;
    };

    std::string ToString() const;
    Utils::MountArgument GetMountArgument() const;
    bool operator==(const MountPoint &rop) const;

private:
    friend class OsAccountObserver;
    Utils::MountArgument mountArg_;
    void Mount() const;
    void Umount() const;
    static std::atomic<uint32_t> idGen_;
    uint32_t id_ {0};
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // MOUNT_POINT_H