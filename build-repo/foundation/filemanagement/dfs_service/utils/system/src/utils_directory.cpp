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

#include "utils_directory.h"

#include <sys/types.h>
#include <unistd.h>
#include <system_error>
#include <functional>

#include "directory_ex.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
using namespace std;

void ForceCreateDirectory(const string &path, function<void(const string &)> onSubDirCreated)
{
    string::size_type index = 0;
    do {
        string subPath;
        index = path.find('/', index + 1);
        if (index == string::npos) {
            subPath = path;
        } else {
            subPath = path.substr(0, index);
        }

        if (access(subPath.c_str(), F_OK) != 0) {
            if (mkdir(subPath.c_str(), (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) != 0 && errno != EEXIST) {
                throw system_error(errno, system_category());
            }
            onSubDirCreated(subPath);
        }
    } while (index != string::npos);
}

void ForceCreateDirectory(const string &path)
{
    ForceCreateDirectory(path, nullptr);
}

void ForceCreateDirectory(const string &path, mode_t mode)
{
    ForceCreateDirectory(path, [mode](const string &subPath) {
        if (chmod(subPath.c_str(), mode) == -1) {
            throw system_error(errno, system_category());
        }
    });
}

void ForceCreateDirectory(const string &path, mode_t mode, uid_t uid, gid_t gid)
{
    ForceCreateDirectory(path, [mode, uid, gid](const string &subPath) {
        if (chmod(subPath.c_str(), mode) == -1 || chown(subPath.c_str(), uid, gid) == -1) {
            throw system_error(errno, system_category());
        }
    });
}

void ForceRemoveDirectory(const string &path)
{
    if (!OHOS::ForceRemoveDirectory(path)) {
        throw system_error(errno, system_category());
    }
}
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
