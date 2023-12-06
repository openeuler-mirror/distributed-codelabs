/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_HIVIEWDFX_ASH_MEM_UTILS_H
#define OHOS_HIVIEWDFX_ASH_MEM_UTILS_H

#include "ashmem.h"
#include "message_parcel.h"
#include "refbase.h"

namespace OHOS {
namespace HiviewDFX {
class AshMemUtils {
public:
    static sptr<Ashmem> WriteBulkData(MessageParcel& parcel, const std::vector<std::u16string>& src);
    static bool ReadBulkData(MessageParcel& parcel, std::vector<std::u16string>& dest);
    static void CloseAshmem(sptr<Ashmem> ashmem);

private:
    static sptr<Ashmem> GetAshmem();
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_ASH_MEM_UTILS_H