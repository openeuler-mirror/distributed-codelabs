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

#ifndef JS_COMMON_H
#define JS_COMMON_H
#include "hilog/log.h"
namespace OHOS::ObjectStore {
#define CHECK_EQUAL_WITH_RETURN_NULL(status, value)                   \
    {                                                                 \
        if (status != value) {                                        \
            LOG_ERROR("error! %{public}d %{public}d", status, value); \
            return nullptr;                                           \
        }                                                             \
    }

#define CHECK_EQUAL_WITH_RETURN_VOID(status, value)                   \
    {                                                                 \
        if (status != value) {                                        \
            LOG_ERROR("error! %{public}d %{public}d", status, value); \
            return;                                                   \
        }                                                             \
    }

#define CHECK_EQUAL_WITH_RETURN_FALSE(status, value)                  \
    {                                                                 \
        if (status != value) {                                        \
            LOG_ERROR("error! %{public}d %{public}d", status, value); \
            return false;                                             \
        }                                                             \
    }

#define ASSERT_MATCH_ELSE_RETURN_VOID(condition)        \
    {                                                   \
        if (!(condition)) {                             \
            LOG_ERROR("error! %{public}s", #condition); \
            return;                                     \
        }                                               \
    }

#define ASSERT_MATCH_ELSE_RETURN_NULL(condition)        \
    {                                                   \
        if (!(condition)) {                             \
            LOG_ERROR("error! %{public}s", #condition); \
            return nullptr;                             \
        }                                               \
    }

#define ASSERT_MATCH_ELSE_GOTO_ERROR(condition)         \
    {                                                   \
        if (!(condition)) {                             \
            LOG_ERROR("error! %{public}s", #condition); \
            goto ERROR;                                 \
        }                                               \
    }

#define CHECK_API_VALID(assertion)                                                                           \
    do {                                                                                                     \
        if (!(assertion)) {                                                                                  \
            std::shared_ptr<DeviceNotSupportedError> apiError = std::make_shared<DeviceNotSupportedError>(); \
            ctxt->SetError(apiError);                                                                        \
            ctxt->status = napi_generic_failure;                                                             \
            return;                                                                                          \
        }                                                                                                    \
    } while (0)

#define CHECK_VALID(assertion, msg)                                                  \
    do {                                                                             \
        if (!(assertion)) {                                                          \
            std::shared_ptr<InnerError> innerError = std::make_shared<InnerError>(); \
            ctxt->SetError(innerError);                                              \
            ctxt->status = napi_generic_failure;                                     \
            ctxt->message = msg;                                                     \
            return;                                                                  \
        }                                                                            \
    } while (0)
} // namespace OHOS::ObjectStore
static const char *CHANGE = "change";
static const char *STATUS = "status";
#endif // JS_COMMON_H
