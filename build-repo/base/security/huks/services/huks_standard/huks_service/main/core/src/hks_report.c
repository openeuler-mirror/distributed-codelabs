/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hks_report.h"

#include "hks_log.h"
#include "hks_template.h"

#ifdef L2_STANDARD
#include "hks_report_wrapper.h"
#endif

void HksReport(const char *funcName, const struct HksProcessInfo *processInfo,
    const struct HksParamSet *paramSetIn, int32_t errorCode)
{
#ifdef L2_STANDARD
    int32_t ret = ReportFaultEvent(funcName, processInfo, paramSetIn, errorCode);
    HKS_IF_NOT_SUCC_LOGE(ret, "report fault event failed, ret = %" LOG_PUBLIC "d", ret)
#else
    (void)funcName;
    (void)processInfo;
    (void)paramSetIn;
    (void)errorCode;
#endif
}
