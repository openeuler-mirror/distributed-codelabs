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

#include "mountpoint/mount_manager.h"

#include <memory>

#include "mountpoint/mount_point.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

MountManager::MountManager()
{
}

MountManager::~MountManager()
{
}

void MountManager::Mount(unique_ptr<MountPoint> mp)
{
    (void)mp;
}

void MountManager::Umount(weak_ptr<MountPoint> wmp)
{
    (void)wmp;
}

void MountManager::Umount(const std::string &groupId)
{
    (void)groupId;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
