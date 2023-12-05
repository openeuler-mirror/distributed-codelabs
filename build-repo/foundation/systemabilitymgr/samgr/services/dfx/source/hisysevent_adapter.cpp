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
#include "hisysevent_adapter.h"

#include <string>

// #include "def.h"
// #include "hisysevent.h"
#include "sam_log.h"

namespace OHOS {
using namespace OHOS::HiviewDFX;
namespace {
// const std::string DOMAIN_NAME = std::string(HiSysEvent::Domain::SAMGR);
const std::string ADD_SYSTEMABILITY_FAIL = "SAMGR_ADD_SYSTEMABILITY_FAIL";
const std::string CALLER_PID = "CALLER_PID";
const std::string SAID = "SAID";
const std::string COUNT = "COUNT";
const std::string FILE_NAME = "FILE_NAME";
const std::string GETSA__TAG = "SAMGR_GETSA_FREQUENCY";
}

void ReportAddSystemAbilityFailed(int32_t said, const std::string& filaName)
{
    HILOGE("Skip this method");
    // int ret = HiSysEvent::Write(DOMAIN_NAME,
    //     ADD_SYSTEMABILITY_FAIL,
    //     HiSysEvent::EventType::FAULT,
    //     SAID, said,
    //     FILE_NAME, filaName);
    // if (ret != 0) {
    //     HILOGE("hisysevent report add system ability event failed! ret %{public}d.", ret);
    // }
}

void ReportGetSAFrequency(uint32_t callerPid, uint32_t said, int32_t count)
{
    HILOGE("Skip this method");
    // int ret = HiSysEvent::Write(DOMAIN_NAME,
    //     GETSA__TAG,
    //     HiSysEvent::EventType::STATISTIC,
    //     CALLER_PID, callerPid,
    //     SAID, said,
    //     COUNT, count);
    // if (ret != 0) {
    //     HILOGE("hisysevent report get sa frequency failed! ret %{public}d.", ret);
    // }
}
} // OHOS
