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

#ifndef OHOS_HIVIEWDFX_HISYSEVENT_RET_H
#define OHOS_HIVIEWDFX_HISYSEVENT_RET_H

namespace OHOS {
namespace HiviewDFX {
static constexpr int32_t IPC_CALL_SUCCEED = 0;

static constexpr int32_t ERR_LISTENER_NOT_EXIST = -10;
static constexpr int32_t ERR_SYS_EVENT_SERVICE_NOT_FOUND = -11;

static constexpr int32_t ERR_PARCEL_DATA_IS_NULL = -12;
static constexpr int32_t ERR_REMOTE_SERVICE_IS_NULL = -13;
static constexpr int32_t ERR_CAN_NOT_WRITE_DESCRIPTOR = -14;
static constexpr int32_t ERR_CAN_NOT_WRITE_PARCEL = -15;
static constexpr int32_t ERR_CAN_NOT_WRITE_REMOTE_OBJECT = -16;
static constexpr int32_t ERR_CAN_NOT_SEND_REQ = -17;
static constexpr int32_t ERR_CAN_NOT_READ_PARCEL = -18;
static constexpr int32_t ERR_NO_PERMISSION = -19;
static constexpr int32_t ERR_LISTENER_STATUS_INVALID = -20;
static constexpr int32_t ERR_LISTENERS_EMPTY = -21;
static constexpr int32_t ERR_ADD_DEATH_RECIPIENT = -22;
static constexpr int32_t ERR_QUERY_RULE_INVALID = -23;
static constexpr int32_t ERR_DEBUG_MODE_SET_REPEAT = -24;

static constexpr int32_t ERR_TOO_MANY_WATCH_RULES = -25;
static constexpr int32_t ERR_TOO_MANY_WATCHERS = -26;
static constexpr int32_t ERR_TOO_MANY_QUERY_RULES = -27;
static constexpr int32_t ERR_TOO_MANY_CONCURRENT_QUERIES = -28;
static constexpr int32_t ERR_QUERY_OVER_TIME = -29;
static constexpr int32_t ERR_QUERY_OVER_LIMIT = -30;
static constexpr int32_t ERR_QUERY_TOO_FREQUENTLY = -31;
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_HISYSEVENT_RET_H
