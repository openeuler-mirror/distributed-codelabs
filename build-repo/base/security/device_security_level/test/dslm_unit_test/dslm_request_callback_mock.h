/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DSLM_REQUEST_CALLBACK_MOCK_H
#define DSLM_REQUEST_CALLBACK_MOCK_H

#include <cstdint>
#include <gmock/gmock.h>
#include <mutex>
#include <new>

#include "dslm_callback_info.h"

namespace OHOS {
namespace Security {
namespace DslmUnitTest {
class DslmRequestCallback {
public:
    DslmRequestCallback() = default;
    virtual ~DslmRequestCallback() = default;
    virtual void RequestCallback(uint32_t cookie, uint32_t result, const DslmCallbackInfo *info) = 0;
};

class DslmRequestCallbackMock : public DslmRequestCallback {
public:
    DslmRequestCallbackMock();
    ~DslmRequestCallbackMock() override;
    MOCK_METHOD3(RequestCallback, void(uint32_t cookie, uint32_t result, const DslmCallbackInfo *info));

    static void MockedCallback(uint32_t owner, uint32_t cookie, uint32_t result, const DslmCallbackInfo *info);

private:
    static DslmRequestCallback *obj_;
    static std::mutex mutex_;
};
} // namespace DslmUnitTest
} // namespace Security
} // namespace OHOS

#endif // DSLM_REQUEST_CALLBACK_MOCK_H
