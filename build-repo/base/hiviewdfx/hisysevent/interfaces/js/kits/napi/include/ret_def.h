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

#ifndef NAPI_HISYSEVENT_RET_DEF_H
#define NAPI_HISYSEVENT_RET_DEF_H

namespace OHOS {
namespace HiviewDFX {
namespace NapiError {
// common api error
constexpr int32_t ERR_PERMISSION_CHECK = 201;
constexpr int32_t ERR_PARAM_CHECK = 401;

constexpr int32_t ERR_UNKNOWN = 11200000;

// business error of write function
constexpr int32_t ERR_INVALID_DOMAIN = 11200001;
constexpr int32_t ERR_INVALID_EVENT_NAME = 11200002;
constexpr int32_t ERR_ENV_ABNORMAL = 11200003;
constexpr int32_t ERR_CONTENT_OVER_LIMIT = 11200004;
constexpr int32_t ERR_INVALID_PARAM_NAME = 11200051;
constexpr int32_t ERR_STR_LEN_OVER_LIMIT = 11200052;
constexpr int32_t ERR_PARAM_COUNT_OVER_LIMIT = 11200053;
constexpr int32_t ERR_ARRAY_SIZE_OVER_LIMIT = 11200054;

// business error of addWatcher function
constexpr int32_t ERR_WATCHER_COUNT_OVER_LIMIT = 11200101;
constexpr int32_t ERR_WATCH_RULE_COUNT_OVER_LIMIT = 11200102;

// business error of removeWatcher function
constexpr int32_t ERR_WATCHER_NOT_EXIST = 11200201;

// business error of query function
constexpr int32_t ERR_QUERY_RULE_COUNT_OVER_LIMIT = 11200301;
constexpr int32_t ERR_INVALID_QUERY_RULE = 11200302;
constexpr int32_t ERR_CONCURRENT_QUERY_COUNT_OVER_LIMIT = 11200303;
constexpr int32_t ERR_QUERY_TOO_FREQUENTLY = 11200304;
}

namespace NapiInnerError {
constexpr int32_t ERR_INVALID_DOMAIN_IN_QUERY_RULE = 112003001;
constexpr int32_t ERR_INVALID_EVENT_NAME_IN_QUERY_RULE = 112003002;
}

static constexpr int32_t NAPI_SUCCESS = 0;

static constexpr int32_t ERR_INVALID_PARAM_COUNT = -100;
static constexpr int32_t ERR_NAPI_PARSED_FAILED = -101;
static constexpr int32_t ERR_LISTENER_RULES_TYPE_NOT_ARRAY = -102;
static constexpr int32_t ERR_LISTENER_RULE_TYPE_INVALID = -103;
static constexpr int32_t ERR_QUERY_ARG_TYPE_INVALID = -104;
static constexpr int32_t ERR_QUERY_RULES_TYPE_NOT_ARRAY = -105;
static constexpr int32_t ERR_QUERY_RULE_TYPE_INVALID = -106;
static constexpr int32_t ERR_NAPI_LISTENER_NOT_FOUND = -107;
} // namespace HiviewDFX
} // namespace OHOS

#endif // NAPI_HISYSEVENT_RET_DEF_H

