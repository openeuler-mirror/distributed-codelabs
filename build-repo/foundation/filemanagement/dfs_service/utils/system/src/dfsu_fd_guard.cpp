/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dfsu_fd_guard.h"

#include <unistd.h>

#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
DfsuFDGuard::DfsuFDGuard(int fd) : fd_(fd) {}

DfsuFDGuard::DfsuFDGuard(int fd, bool autoClose) : fd_(fd), autoClose_(autoClose) {}

DfsuFDGuard::~DfsuFDGuard()
{
    if (fd_ >= 0 && autoClose_) {
        LOGD("DfsuFDGuard Deconstruction, fd_ = %{public}d", fd_);
        close(fd_);
    }
}

DfsuFDGuard::operator bool() const
{
    return fd_ >= 0;
}

int DfsuFDGuard::GetFD() const
{
    return fd_;
}

void DfsuFDGuard::SetFD(int fd, bool autoClose)
{
    fd_ = fd;
    autoClose_ = autoClose;
}

void DfsuFDGuard::ClearFD()
{
    fd_ = -1;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
