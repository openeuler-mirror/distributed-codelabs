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

#ifndef OHOS_DISTRIBUTED_DATA_SERVICES_CONFIG_MODEL_CHECKER_CONFIG_H
#define OHOS_DISTRIBUTED_DATA_SERVICES_CONFIG_MODEL_CHECKER_CONFIG_H
#include "serializable/serializable.h"
#include "checker/checker_manager.h"
namespace OHOS {
namespace DistributedData {
class CheckerConfig final : public Serializable {
public:
    struct Trust final : public Serializable, public CheckerManager::Trust {
        bool Marshal(json &node) const override;
        bool Unmarshal(const json &node) override;
    };
    std::vector<std::string> checkers;
    std::vector<Trust> trusts;
    bool Marshal(json &node) const override;
    bool Unmarshal(const json &node) override;
};
} // namespace DistributedData
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_DATA_SERVICES_CONFIG_MODEL_CHECKER_CONFIG_H
