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

#include "query_argument.h"

namespace OHOS {
namespace HiviewDFX {
bool QueryArgument::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteInt64(beginTime)) {
        return false;
    }
    if (!parcel.WriteInt64(endTime)) {
        return false;
    }
    if (!parcel.WriteInt32(maxEvents)) {
        return false;
    }
    if (!parcel.WriteInt64(fromSeq)) {
        return false;
    }
    if (!parcel.WriteInt64(toSeq)) {
        return false;
    }
    return true;
}

QueryArgument* QueryArgument::Unmarshalling(Parcel& parcel)
{
    QueryArgument* ret = new(std::nothrow) QueryArgument();
    if (ret == nullptr) {
        return ret;
    }
    if (!parcel.ReadInt64(ret->beginTime)) {
        goto error;
    }
    if (!parcel.ReadInt64(ret->endTime)) {
        goto error;
    }
    if (!parcel.ReadInt32(ret->maxEvents)) {
        goto error;
    }
    if (!parcel.ReadInt64(ret->fromSeq)) {
        goto error;
    }
    if (!parcel.ReadInt64(ret->toSeq)) {
        goto error;
    }
    return ret;
error:
    delete ret;
    ret = nullptr;
    return nullptr;
}
} // namespace HiviewDFX
} // namespace OHOS
