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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_PUBLISH_INFO_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_PUBLISH_INFO_H

#include "parcel.h"

namespace OHOS {
namespace EventFwk {
class CommonEventPublishInfo : public Parcelable {
public:
    CommonEventPublishInfo();

    /**
     * A constructor used to create a CommonEventPublishInfo instance by copying
     * parameters from an existing one.
     *
     * @param commonEventPublishInfo Indicates the publish info.
     */
    explicit CommonEventPublishInfo(const CommonEventPublishInfo &commonEventPublishInfo);

    ~CommonEventPublishInfo();

    /**
     * Sets whether the type of a common event is sticky or not.
     *
     * @param sticky Indicates the type of a common event is sticky or not
     */
    void SetSticky(bool sticky);

    /**
     * Obtains whether it is a sticky common event, which can be set
     * by SetSticky(bool).
     *
     * @return Returns the common event is sticky or not.
     */
    bool IsSticky() const;

    /**
     * Sets permissions for subscribers.
     *
     * @param subscriberPermissions Indicates the permissions for subscribers.
     */
    void SetSubscriberPermissions(const std::vector<std::string> &subscriberPermissions);

    /**
     * Obtains subscriber permissions to a common event, which can be set by
     * setSubscriberPermissions(const std::vector<std::string>&).
     *
     * @return Returns the permissions for subscribers.
     */
    const std::vector<std::string> &GetSubscriberPermissions() const;

    /**
     * Sets whether the type of a common event is ordered or not.
     *
     * @param ordered Indicates the type of a common event is ordered or not.
     */
    void SetOrdered(bool ordered);

    /**
     *
     * Obtains whether it is an ordered common event, which can be set by setOrdered(boolean).
     * set by SetOrdered(bool).
     *
     * @return Returns the type of a common event is ordered or not.
     */
    bool IsOrdered() const;

    /**
     * Sets the bundleName.
     *
     * @param bundleName Indicates the bundleName of a common event.
     */
    void SetBundleName(const std::string &bundleName);

    /**
     * Obtains the bundleName of a common event
     *
     * @return Returns the bundleName of a common event.
     */
    std::string GetBundleName() const;

    /**
     * Marshals a CommonEventData object into a Parcel.
     *
     * @param parcel Indicates specified Parcel object.
     * @return Returns true if success; false otherwise.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * UnMarshals a Parcel object into a CommonEventData.
     *
     * @return Returns the common event data.
     */
    static CommonEventPublishInfo *Unmarshalling(Parcel &parcel);

private:
    /**
     * Reads a CommonEventData object from a Parcel.
     *
     * @param parcel Indicates specified Parcel object.
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    bool sticky_;
    bool ordered_;
    std::string bundleName_;
    std::vector<std::string> subscriberPermissions_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_PUBLISH_INFO_H