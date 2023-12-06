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

#ifndef OHOS_HIVIEWDFX_QUERY_ARGUMENT_H
#define OHOS_HIVIEWDFX_QUERY_ARGUMENT_H

#include "parcel.h"

namespace OHOS {
namespace HiviewDFX {
class QueryArgument : public Parcelable {
public:
    QueryArgument() {}
    QueryArgument(int64_t beginTime, int64_t endTime, int32_t maxEvents = 0,
        int64_t fromSeq = -1, int64_t toSeq = -1)
        : beginTime(beginTime), endTime(endTime), maxEvents(maxEvents), fromSeq(fromSeq), toSeq(toSeq) {}
    ~QueryArgument() {}

    bool Marshalling(Parcel& parcel) const override;
    static QueryArgument* Unmarshalling(Parcel& parcel);

    int64_t beginTime = -1;
    int64_t endTime = -1;
    int32_t maxEvents = 0;
    int64_t fromSeq = -1;
    int64_t toSeq = -1;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_QUERY_ARGUMENT_H