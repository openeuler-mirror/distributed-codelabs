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

#ifndef SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_STUB_H_
#define SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_STUB_H_

#include "ilisten_ability.h"
#include "iremote_stub.h"
#include "refbase.h"

namespace OHOS {
class ListenAbilityStub : public IRemoteStub<IListenAbility> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel &reply, MessageOption &option) override;
};
} // namespace OHOS
#endif /* SAFWK_TEST_SERVICES_SAFWK_UNITTEST_LISTEN_ABILITY_INCLUDE_LISTEN_ABILITY_STUB_H_ */
