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

#ifndef SAMGR_SERVICES_DFX_INCLUDE__HISYSEVENT_ADAPTER_H
#define SAMGR_SERVICES_DFX_INCLUDE__HISYSEVENT_ADAPTER_H

#include <string>
namespace OHOS {
void ReportAddSystemAbilityFailed(int32_t said, const std::string& filaName);

void ReportGetSAFrequency(uint32_t callerPid, uint32_t said, int32_t count);
} // OHOS
#endif // SAMGR_SERVICES_DFX_INCLUDE__HISYSEVENT_ADAPTER_H
