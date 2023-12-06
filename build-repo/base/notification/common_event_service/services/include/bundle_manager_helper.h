/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_BUNDLE_MANAGER_HELPER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_BUNDLE_MANAGER_HELPER_H

#include <string>
#include <vector>

#include "bms_death_recipient.h"
#include "bundle_mgr_interface.h"
#include "extension_ability_info.h"
#include "iremote_object.h"
#include "refbase.h"
#include "singleton.h"
#include "want.h"

namespace OHOS {
namespace EventFwk {
class BundleManagerHelper : public DelayedSingleton<BundleManagerHelper> {
public:
    using IBundleMgr = OHOS::AppExecFwk::IBundleMgr;

    BundleManagerHelper();

    virtual ~BundleManagerHelper();

    /**
     * Checks whether it is system application.
     *
     * @param uid Indicates the uid of the application.
     * @return Returns true if successful; false otherwise.
     */
    bool CheckIsSystemAppByUid(uid_t uid);

    bool CheckIsSystemAppByBundleName(const std::string &bundleName, const int32_t &userId);

    /**
     * Gets bundle name.
     *
     * @param uid Indicates the uid of the application.
     * @return Returns the bundle name.
     */
    std::string GetBundleName(uid_t uid);

    /**
     * Queries extension information.
     *
     * @param extensionInfo Indicates the extension information.
     * @return Returns true if successful; false otherwise.
     */
    bool QueryExtensionInfos(std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfo);

    /**
     * Queries extension information by user.
     *
     * @param extensionInfos Indicates the extension information.
     * @param userId Indicates the ID of user.
     * @return Returns true if successful; false otherwise.
     */
    bool QueryExtensionInfos(std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos, const int32_t &userId);

    /**
     * Gets res config file.
     *
     * @param extension Indicates the extension information.
     * @param profileInfos Indicates the profile information.
     * @return Returns true if successful; false otherwise.
     */
    bool GetResConfigFile(const AppExecFwk::ExtensionAbilityInfo &extension, std::vector<std::string> &profileInfos);

    /**
     * Clears bundle manager helper.
     *
     */
    void ClearBundleManagerHelper();

private:
    bool GetBundleMgrProxy();

private:
    sptr<IBundleMgr> sptrBundleMgr_;
    std::mutex mutex_;
    sptr<BMSDeathRecipient> bmsDeath_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_BUNDLE_MANAGER_HELPER_H