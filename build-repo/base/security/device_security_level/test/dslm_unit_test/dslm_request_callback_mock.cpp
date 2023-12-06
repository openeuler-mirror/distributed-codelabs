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

#include "dslm_request_callback_mock.h"

#include <gmock/gmock.h>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace DslmUnitTest {
DslmRequestCallbackMock::DslmRequestCallbackMock()
{
    std::lock_guard<std::mutex> lock(mutex_);
    EXPECT_EQ(obj_, nullptr);
    obj_ = this;
}

DslmRequestCallbackMock::~DslmRequestCallbackMock()
{
    std::lock_guard<std::mutex> lock(mutex_);
    EXPECT_EQ(obj_, this);
    obj_ = nullptr;
}

void DslmRequestCallbackMock::MockedCallback(uint32_t owner, uint32_t cookie, uint32_t result,
    const DslmCallbackInfo *info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (obj_ != nullptr) {
        obj_->RequestCallback(cookie, result, info);
    }
}

DslmRequestCallback *DslmRequestCallbackMock::obj_ = nullptr;
std::mutex DslmRequestCallbackMock::mutex_ {};
} // namespace DslmUnitTest
} // namespace Security
} // namespace OHOS
