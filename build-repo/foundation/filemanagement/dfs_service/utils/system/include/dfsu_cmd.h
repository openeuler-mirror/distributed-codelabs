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

#ifndef DFSU_CMD_H
#define DFSU_CMD_H

#include <functional>

#include "dfsu_exception.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
enum class CmdImportance {
    // If it fails(even has tried multiple times), shutdown the actor.
    VITAL,

    SUBVITAL,
    // If it fails(even has tried multiple times), reboot the actor.
    NORMAL,

    // If it fails(may also try multiple times), just do nothing.
    TRIVIAL,
};

struct CmdOptions {
    CmdImportance importance_ { CmdImportance::TRIVIAL };
    uint32_t tryTimes_ {1};
};

template<typename T>
class DfsuActor;

template<typename Ctx>
class VirtualCmd {
    friend class DfsuActor<Ctx>;

public:
    VirtualCmd() = default;
    virtual ~VirtualCmd() = default;
    virtual void operator()(Ctx *ctx) = 0;

    void UpdateOption(CmdOptions op)
    {
        option_ = op;
    }

protected:
    CmdOptions option_;
};

template<typename Ctx, typename... Args>
class DfsuCmd : public VirtualCmd<Ctx> {
    friend class DfsuActor<Ctx>;

public:
    DfsuCmd(void (Ctx::*f)(Args...), Args... args) : f_(f), args_(args...) {}
    ~DfsuCmd() override = default;

private:
    void (Ctx::*f_)(Args...);
    std::tuple<Args...> args_;

    void operator()(Ctx *ctx) override
    {
        if (!VirtualCmd<Ctx>::option_.tryTimes_) {
            ThrowException(ERR_UTILS_ACTOR_INVALID_CMD, "Cannot execute a command that has 0 try times");
        }

        VirtualCmd<Ctx>::option_.tryTimes_--;
        std::apply(f_, std::tuple_cat(std::make_tuple(ctx), args_));
    }
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFSU_CMD_H