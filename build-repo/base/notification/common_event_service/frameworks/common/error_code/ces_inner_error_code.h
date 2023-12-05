/* Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef BASE_NOTIFICATION_CES_STANDARD_INNERKITS_BASE_INCLUDE_CES_INNER_ERRORS_H
#define BASE_NOTIFICATION_CES_STANDARD_INNERKITS_BASE_INCLUDE_CES_INNER_ERRORS_H

#include "errors.h"

namespace OHOS {
namespace Notification {

    // The application dose not have permission to call the interface
    const int32_t ERR_NOTIFICATION_CES_COMMON_PERMISSION_DENIED = 201;

    // The param input is invalid
    const int32_t ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID = 401;

    // The SystemCapacity name was not found
    const int32_t ERR_NOTIFICATION_CES_COMMON_SYSTEMCAP_NOT_SUPPORT = 801;

    // The want action send by event is null
    const int32_t ERR_NOTIFICATION_CES_WANT_ACTION_IS_NULL = 1500001;

    // The sandbox application can not send common event
    const int32_t ERR_NOTIFICATION_CES_SANDBOX_NOT_SUPPORT = 1500002;

    // The common event send frequency too high
    const int32_t ERR_NOTIFICATION_CES_EVENT_FREQ_TOO_HIGH = 1500003;

    // The application can not send common event, it may be not SA app or System app
    const int32_t ERR_NOTIFICATION_CES_NOT_SA_SYSTEM_APP = 1500004;

    // The subscriber can not found
    const int32_t ERR_NOTIFICATION_CES_NO_SUBSCRIBER = 1500005;

    // The usreId is invalid
    const int32_t ERR_NOTIFICATION_CES_USERID_INVALID = 1500006;

    // The message send error
    const int32_t ERR_NOTIFICATION_SEND_ERROR = 1500007;

    // The CEMS error
    const int32_t ERR_NOTIFICATION_CESM_ERROR = 1500008;

    // The system error
    const int32_t ERR_NOTIFICATION_SYS_ERROR = 1500009;
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_CES_STANDARD_INNERKITS_BASE_INCLUDE_CES_INNER_ERRORS_H