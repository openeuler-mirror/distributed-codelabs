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

#include "common_event_publish_info.h"
#include "event_log_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace EventFwk {
CommonEventPublishInfo::CommonEventPublishInfo() : sticky_(false), ordered_(false)
{
}

CommonEventPublishInfo::CommonEventPublishInfo(const CommonEventPublishInfo &commonEventPublishInfo)
{
    sticky_ = commonEventPublishInfo.sticky_;
    ordered_ = commonEventPublishInfo.ordered_;
    bundleName_ = commonEventPublishInfo.bundleName_;
    subscriberPermissions_ = commonEventPublishInfo.subscriberPermissions_;
}

CommonEventPublishInfo::~CommonEventPublishInfo()
{
}

void CommonEventPublishInfo::SetSticky(bool sticky)
{
    sticky_ = sticky;
}

bool CommonEventPublishInfo::IsSticky() const
{
    return sticky_;
}

void CommonEventPublishInfo::SetSubscriberPermissions(const std::vector<std::string> &subscriberPermissions)
{
    subscriberPermissions_ = subscriberPermissions;
}

const std::vector<std::string> &CommonEventPublishInfo::GetSubscriberPermissions() const
{
    return subscriberPermissions_;
}

void CommonEventPublishInfo::SetOrdered(bool ordered)
{
    ordered_ = ordered;
}

bool CommonEventPublishInfo::IsOrdered() const
{
    return ordered_;
}

void CommonEventPublishInfo::SetBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

std::string CommonEventPublishInfo::GetBundleName() const
{
    return bundleName_;
}

bool CommonEventPublishInfo::Marshalling(Parcel &parcel) const
{
    EVENT_LOGD("enter");

    // write subscriber permissions
    std::vector<std::u16string> permissionVec_;
    for (std::vector<std::string>::size_type i = 0; i < subscriberPermissions_.size(); ++i) {
        permissionVec_.emplace_back(Str8ToStr16(subscriberPermissions_[i]));
    }
    if (!parcel.WriteString16Vector(permissionVec_)) {
        EVENT_LOGE("common event Publish Info write permission failed");
        return false;
    }

    // write ordered
    if (!parcel.WriteBool(ordered_)) {
        EVENT_LOGE("common event Publish Info write ordered failed");
        return false;
    }

    // write sticky
    if (!parcel.WriteBool(sticky_)) {
        EVENT_LOGE("common event Publish Info write sticky failed");
        return false;
    }
    // write bundleName
    if (!parcel.WriteString16(Str8ToStr16(bundleName_))) {
        EVENT_LOGE("common event Publish Info  write bundleName failed");
        return false;
    }
    return true;
}

bool CommonEventPublishInfo::ReadFromParcel(Parcel &parcel)
{
    EVENT_LOGD("enter");

    // read subscriber permissions
    std::vector<std::u16string> permissionVec_;
    if (!parcel.ReadString16Vector(&permissionVec_)) {
        EVENT_LOGE("ReadFromParcel read permission error");
        return false;
    }
    subscriberPermissions_.clear();
    for (std::vector<std::u16string>::size_type i = 0; i < permissionVec_.size(); i++) {
        subscriberPermissions_.emplace_back(Str16ToStr8(permissionVec_[i]));
    }
    // read ordered
    ordered_ = parcel.ReadBool();
    // read sticky
    sticky_ = parcel.ReadBool();
    // read bundleName
    bundleName_ = Str16ToStr8(parcel.ReadString16());

    return true;
}

CommonEventPublishInfo *CommonEventPublishInfo::Unmarshalling(Parcel &parcel)
{
    CommonEventPublishInfo *commonEventPublishInfo = new (std::nothrow) CommonEventPublishInfo();

    if (commonEventPublishInfo == nullptr) {
        EVENT_LOGE("commonEventPublishInfo == nullptr");
        return nullptr;
    }

    if (!commonEventPublishInfo->ReadFromParcel(parcel)) {
        EVENT_LOGE("failed to ReadFromParcel");
        delete commonEventPublishInfo;
        commonEventPublishInfo = nullptr;
    }

    return commonEventPublishInfo;
}
}  // namespace EventFwk
}  // namespace OHOS