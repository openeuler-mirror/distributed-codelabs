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

#ifndef PERMISSION_VALIDATOR_H
#define PERMISSION_VALIDATOR_H
#include <string>
#include "types.h"
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
class PermissionValidator {
public:
    API_EXPORT static PermissionValidator &GetInstance();
    // check whether the client process have enough privilege to share data with the other devices.
    // tokenId: client process tokenId
    API_EXPORT  bool CheckSyncPermission(uint32_t tokenId);

private:
    static constexpr const char *DISTRIBUTED_DATASYNC = "ohos.permission.DISTRIBUTED_DATASYNC";
};
} // namespace DistributedKv
} // namespace OHOS
#endif // PERMISSION_VALIDATOR_H
