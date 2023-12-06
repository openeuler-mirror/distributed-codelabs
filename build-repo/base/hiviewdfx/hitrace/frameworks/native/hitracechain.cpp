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

#include "hitrace/hitracechain.h"
#include "hitracechain_inner.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {

HiTraceId HiTraceChain::Begin(const string& name, int flags)
{
    return HiTraceId(::HiTraceChainBegin(name.c_str(), flags));
}

void HiTraceChain::End(const HiTraceId& id)
{
    ::HiTraceChainEnd(&(id.id_));
    return;
}

HiTraceId HiTraceChain::GetId()
{
    return HiTraceId(::HiTraceChainGetId());
}

void HiTraceChain::SetId(const HiTraceId& id)
{
    ::HiTraceChainSetId(&(id.id_));
    return;
}

void HiTraceChain::ClearId()
{
    ::HiTraceChainClearId();
    return;
}

HiTraceId HiTraceChain::CreateSpan()
{
    return HiTraceId(::HiTraceChainCreateSpan());
}

void HiTraceChain::Tracepoint(HiTraceTracepointType type, const HiTraceId& id, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ::HiTraceChainTracepointInner(HITRACE_CM_DEFAULT, type, &(id.id_), fmt, args);
    va_end(args);

    return;
}

void HiTraceChain::Tracepoint(HiTraceCommunicationMode mode, HiTraceTracepointType type, const HiTraceId& id,
    const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ::HiTraceChainTracepointInner(mode, type, &(id.id_), fmt, args);
    va_end(args);

    return;
}
} // namespace HiviewDFX
} // namespace OHOS
