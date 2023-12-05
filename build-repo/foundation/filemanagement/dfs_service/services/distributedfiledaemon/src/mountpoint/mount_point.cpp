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

#include "mountpoint/mount_point.h"

#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

atomic<uint32_t> MountPoint::idGen_;

MountPoint::MountPoint(const Utils::MountArgument &mountArg) : mountArg_(mountArg)
{
    id_ = idGen_++;
}

void MountPoint::Mount() const
{
    LOGI("mount");
}

void MountPoint::Umount() const
{
    LOGE("umount");
}

bool MountPoint::operator==(const MountPoint &rop) const
{
    return mountArg_.GetFullDst() == rop.mountArg_.GetFullDst();
}

Utils::MountArgument MountPoint::GetMountArgument() const
{
    return mountArg_;
}

string MountPoint::ToString() const
{
    return "";
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
