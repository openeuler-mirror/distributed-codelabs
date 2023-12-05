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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_TEST_UNITTEST_MOCK_INCLUDE_MOCK_BUNDLE_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_TEST_UNITTEST_MOCK_INCLUDE_MOCK_BUNDLE_MANAGER_H

#include <vector>

#include "ability_info.h"
#include "application_info.h"
#include "bundle_mgr_host.h"
#include "bundle_mgr_interface.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class MockBundleMgrService : public BundleMgrHost {
public:
    MockBundleMgrService()
    {}

    ~MockBundleMgrService()
    {}

    /**
     * @brief Check whether the app is system app by it's UID.
     * @param uid Indicates the uid.
     * @return Returns true if the bundle is a system application; returns false otherwise.
     */
    virtual bool CheckIsSystemAppByUid(const int uid) override;

    void MockSetIsSystemApp(bool isSystemApp);

private:
    bool isSystemApp_ = false;
    bool isSystemAppMock_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_TEST_UNITTEST_MOCK_INCLUDE_MOCK_BUNDLE_MANAGER_H
