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

#ifndef DFSU_EXCEPTION_H
#define DFSU_EXCEPTION_H

#include <exception>
#include <sstream>
#include <string>

#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
enum {
    ERR_DEFAULT,
    ERR_UTILS_ACTOR_QUEUE_STOP,
    ERR_UTILS_ACTOR_INVALID_CMD,
    ERR_NETWORK_AGENT_TEMPLATE_OPEN_SESSION_FAIL,
    ERR_SOFTBUS_AGENT_ON_SESSION_OPENED_FAIL,
    ERR_DEVICE_CID_UN_INIT,
};

class DfsuException : public std::exception {
public:
    DfsuException(int code, const std::string &msg) : code_(code), msg_(msg) {};

    uint32_t code() const noexcept
    {
        return code_;
    }

    virtual const char *what() const noexcept
    {
        return msg_.c_str();
    }

private:
    int code_ {ERR_DEFAULT};
    std::string msg_;
};

#define ThrowException(code, msg)                           \
    do {                                                    \
        std::stringstream __ss;                             \
        __ss << '[' << (code) << ']' << (msg) << std::endl; \
        LOGE("%{public}s", __ss.str().c_str());             \
        throw DfsuException((code), __ss.str());            \
    } while (0)
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFSU_EXCEPTION_H