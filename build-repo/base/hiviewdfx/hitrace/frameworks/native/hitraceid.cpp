/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hitrace/hitraceid.h"

#include <cstdint>

#include "hitrace/hitracechainc.h"

namespace OHOS {
namespace HiviewDFX {

HiTraceId::HiTraceId()
{
    id_.valid = HITRACE_ID_INVALID;
    id_.ver = 0;
    id_.chainId = 0;
    id_.flags = 0;
    id_.spanId = 0;
    id_.parentSpanId = 0;
}

HiTraceId::HiTraceId(const HiTraceIdStruct& id) : id_(id)
{}

HiTraceId::HiTraceId(const uint8_t* pIdArray, int len)
{
    id_ = HiTraceChainBytesToId(pIdArray, len);
}

bool HiTraceId::IsValid() const
{
    return HiTraceChainIsValid(&id_);
}

bool HiTraceId::IsFlagEnabled(HiTraceFlag flag) const
{
    return HiTraceChainIsFlagEnabled(&id_, flag);
}

void HiTraceId::EnableFlag(HiTraceFlag flag)
{
    HiTraceChainEnableFlag(&id_, flag);
    return;
}

int HiTraceId::GetFlags() const
{
    return HiTraceChainGetFlags(&id_);
}

void HiTraceId::SetFlags(int flags)
{
    HiTraceChainSetFlags(&id_, flags);
    return;
}

uint64_t HiTraceId::GetChainId() const
{
    return HiTraceChainGetChainId(&id_);
}

void HiTraceId::SetChainId(uint64_t chainId)
{
    HiTraceChainSetChainId(&id_, chainId);
    return;
}

uint64_t HiTraceId::GetSpanId() const
{
    return HiTraceChainGetSpanId(&id_);
}

void HiTraceId::SetSpanId(uint64_t spanId)
{
    HiTraceChainSetSpanId(&id_, spanId);
    return;
}

uint64_t HiTraceId::GetParentSpanId() const
{
    return HiTraceChainGetParentSpanId(&id_);
}

void HiTraceId::SetParentSpanId(uint64_t parentSpanId)
{
    HiTraceChainSetParentSpanId(&id_, parentSpanId);
    return;
}

int HiTraceId::ToBytes(uint8_t* pIdArray, int len) const
{
    return HiTraceChainIdToBytes(&id_, pIdArray, len);
}

} // namespace HiviewDFX
} // namespace OHOS
