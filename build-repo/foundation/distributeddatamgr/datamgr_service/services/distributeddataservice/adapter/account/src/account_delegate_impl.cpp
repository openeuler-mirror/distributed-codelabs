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

#define LOG_TAG "EVENT_HANDLER"

#include "account_delegate_impl.h"
#include <algorithm>
#include <thread>
#include <unistd.h>
#include <vector>
#include "constant.h"

namespace OHOS {
namespace DistributedKv {

using namespace OHOS::DistributedData;

AccountDelegateImpl::~AccountDelegateImpl()
{
    observerMap_.Clear();
}

void AccountDelegateImpl::NotifyAccountChanged(const AccountEventInfo &accountEventInfo)
{
    observerMap_.ForEach([&accountEventInfo] (const auto& key, const auto& val) {
        if (val->GetLevel() == AccountDelegate::Observer::LevelType::HIGH) {
            val->OnAccountChanged(accountEventInfo);
        }
        return false;
    });
    observerMap_.ForEach([&accountEventInfo] (const auto& key, const auto& val) {
        if (val->GetLevel() == AccountDelegate::Observer::LevelType::LOW) {
            val->OnAccountChanged(accountEventInfo);
        }
        return false;
    });
}

Status AccountDelegateImpl::Subscribe(std::shared_ptr<Observer> observer)
{
    ZLOGD("start");
    if (observer == nullptr || observer->Name().empty()) {
        return Status::INVALID_ARGUMENT;
    }
    if (observerMap_.Contains(observer->Name())) {
        return Status::INVALID_ARGUMENT;
    }

    auto ret = observerMap_.Insert(observer->Name(), observer);
    if (ret) {
        ZLOGD("end");
        return Status::SUCCESS;
    }
    ZLOGE("fail");
    return Status::ERROR;
}

Status AccountDelegateImpl::Unsubscribe(std::shared_ptr<Observer> observer)
{
    ZLOGD("start");
    if (observer == nullptr || observer->Name().empty()) {
        return Status::INVALID_ARGUMENT;
    }
    if (!observerMap_.Contains(observer->Name())) {
        return Status::INVALID_ARGUMENT;
    }

    auto ret = observerMap_.Erase(observer->Name());
    if (ret) {
        ZLOGD("end");
        return Status::SUCCESS;
    }
    ZLOGD("fail");
    return Status::ERROR;
}

bool AccountDelegateImpl::RegisterHashFunc(HashFunc hash)
{
    hash_ = hash;
    return true;
}

std::string AccountDelegateImpl::DoHash(const void *data, size_t size, bool isUpper) const
{
    if (hash_ == nullptr) {
        return std::string(static_cast<const char *>(data), size);
    }
    return hash_(data, size, isUpper);
}
}  // namespace DistributedKv
}  // namespace OHOS
