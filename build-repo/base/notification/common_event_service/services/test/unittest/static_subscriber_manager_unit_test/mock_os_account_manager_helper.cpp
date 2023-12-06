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

#include "os_account_manager_helper.h"

namespace {
int32_t g_mockId = 100; // default id when there is no os_account part
bool g_mockQueryActiveOsAccountIdsRet = true;
bool g_mockGetOsAccountLocalIdFromUidRet = true;
int32_t g_mockIdForGetOsAccountLocalIdFromUid = 100;
}

void MockQueryActiveOsAccountIds(bool mockRet, uint8_t mockCase)
{
    g_mockQueryActiveOsAccountIdsRet = mockRet;
    switch (mockCase) {
        case 1: {
            g_mockId = 101; // 101 mockcase1
            break;
        }
        default: {
            g_mockId = 100; // 100 mockdefault
            break;
        }
    }
}

void ResetAccountMock()
{
    g_mockId = 100; // 100 mockId
    g_mockQueryActiveOsAccountIdsRet = true;
    g_mockGetOsAccountLocalIdFromUidRet = true;
    g_mockIdForGetOsAccountLocalIdFromUid = 100;// 100 MockIdForGetOsAccountLocalIdFromUid
}

void MockGetOsAccountLocalIdFromUid(bool mockRet, uint8_t mockCase = 0)
{
    g_mockGetOsAccountLocalIdFromUidRet = mockRet;
    switch (mockCase) {
        case 1: { // mock for invalid id
            g_mockIdForGetOsAccountLocalIdFromUid = -2; // -2 mock for invalid id
            break;
        }
        case 2: { // mock for system id
            g_mockIdForGetOsAccountLocalIdFromUid = 88; // 88 mock for system id
            break;
        }
        default: {
            g_mockIdForGetOsAccountLocalIdFromUid = 100; // 100 mock for system id
            break;
        }
    }
}

namespace OHOS {
namespace EventFwk {
ErrCode OsAccountManagerHelper::QueryActiveOsAccountIds(std::vector<int32_t>& ids)
{
    ids.emplace_back(g_mockId);
    return g_mockQueryActiveOsAccountIdsRet ? ERR_OK : ERR_INVALID_OPERATION;
}

ErrCode OsAccountManagerHelper::GetOsAccountLocalIdFromUid(const int32_t uid, int32_t &id)
{
    id = g_mockIdForGetOsAccountLocalIdFromUid;
    return g_mockGetOsAccountLocalIdFromUidRet ? ERR_OK : ERR_INVALID_OPERATION;
}
}  // namespace EventFwk
}  // namespace OHOS