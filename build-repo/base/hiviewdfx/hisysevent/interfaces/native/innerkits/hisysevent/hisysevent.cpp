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

#include "hisysevent.h"

#include <chrono>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>

#include "def.h"
#include "hilog/log.h"
#include "hitrace/trace.h"
#include "stringfilter.h"
#include "transport.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT" };
}

WriteController HiSysEvent::controller;

static inline uint64_t GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

static std::string GetTimeZone()
{
    struct timeval tv;
    if (gettimeofday(&tv, nullptr) != 0) {
        HiLog::Error(LABEL, "can not get tz");
        return "";
    }
    time_t sysSec = tv.tv_sec;
    struct tm tmLocal;
    if (localtime_r(&sysSec, &tmLocal) == nullptr) {
        HiLog::Error(LABEL, "failed to get local time.");
        return "";
    }
    int timeZoneBufSize = 20;
    char timeZone[timeZoneBufSize];
    auto ret = strftime(timeZone, timeZoneBufSize, "%z", &tmLocal);
    if (ret > 0) {
        return std::string(timeZone);
    }
    return std::string("+0000");
}

int HiSysEvent::CheckKey(const std::string &key)
{
    if (!StringFilter::GetInstance().IsValidName(key, MAX_PARAM_NAME_LENGTH)) {
        return ERR_KEY_NAME_INVALID;
    }
    return SUCCESS;
}

int HiSysEvent::CheckValue(const std::string &value)
{
    if (value.length() > MAX_STRING_LENGTH) {
        return ERR_VALUE_LENGTH_TOO_LONG;
    }
    return SUCCESS;
}

int HiSysEvent::CheckArraySize(unsigned long size)
{
    if (size > MAX_ARRAY_SIZE) {
        return ERR_ARRAY_TOO_MUCH;
    }
    return SUCCESS;
}

unsigned int HiSysEvent::GetArrayMax()
{
    return MAX_ARRAY_SIZE;
}

void HiSysEvent::ExplainRetCode(HiSysEvent::EventBase &eventBase)
{
    if (eventBase.retCode_ > SUCCESS) {
        HiLog::Warn(LABEL, "some value of param discard as invalid data, error=%{public}d, message=%{public}s",
            eventBase.retCode_, ERR_MSG_LEVEL1[eventBase.retCode_ - 1]);
    } else if (eventBase.retCode_ < SUCCESS) {
        HiLog::Error(LABEL, "discard data, error=%{public}d, message=%{public}s",
            eventBase.retCode_, ERR_MSG_LEVEL0[-eventBase.retCode_ - 1]);
    }
}

bool HiSysEvent::IsError(HiSysEvent::EventBase &eventBase)
{
    return (eventBase.retCode_ < SUCCESS);
}

bool HiSysEvent::IsErrorAndUpdate(int retCode, HiSysEvent::EventBase &eventBase)
{
    if (retCode < SUCCESS) {
        eventBase.retCode_ = retCode;
        return true;
    }
    return false;
}

bool HiSysEvent::IsWarnAndUpdate(int retCode, EventBase &eventBase)
{
    if (retCode != SUCCESS) {
        eventBase.retCode_ = retCode;
        return true;
    }
    return false;
}

bool HiSysEvent::UpdateAndCheckKeyNumIsOver(HiSysEvent::EventBase &eventBase)
{
    eventBase.keyCnt_++;
    if (eventBase.keyCnt_ > MAX_PARAM_NUMBER) {
        eventBase.retCode_ = ERR_KEY_NUMBER_TOO_MUCH;
        return true;
    }
    return false;
}

void HiSysEvent::AppendValue(HiSysEvent::EventBase &eventBase, const std::string &item)
{
    std::string text = item;
    if (item.length() > MAX_STRING_LENGTH) {
        text = item.substr(0, MAX_STRING_LENGTH);
        eventBase.retCode_ = ERR_VALUE_LENGTH_TOO_LONG;
    }
    eventBase.jsonStr_ << "\"" << StringFilter::GetInstance().EscapeToRaw(text) << "\"";
}

void HiSysEvent::AppendValue(HiSysEvent::EventBase &eventBase, const char item)
{
    eventBase.jsonStr_ << static_cast<short>(item);
}

void HiSysEvent::AppendValue(EventBase &eventBase, const signed char item)
{
    eventBase.jsonStr_ << static_cast<short>(item);
}

void HiSysEvent::AppendValue(HiSysEvent::EventBase &eventBase, const unsigned char item)
{
    eventBase.jsonStr_ << static_cast<unsigned short>(item);
}

void HiSysEvent::InnerWrite(HiSysEvent::EventBase &eventBase)
{
    if (eventBase.jsonStr_.tellp() != 0) {
        eventBase.jsonStr_.seekp(-1, std::ios_base::end);
    }
}

void HiSysEvent::InnerWrite(EventBase &eventBase, HiSysEventParam params[], size_t size)
{
    if (params == nullptr || size == 0) {
        InnerWrite(eventBase);
        return;
    }

    for (size_t i = 0; i < size; ++i) {
        AppendParam(eventBase, params[i]);
    }
    InnerWrite(eventBase);
}

void HiSysEvent::SendSysEvent(HiSysEvent::EventBase &eventBase)
{
    int r = Transport::GetInstance().SendData(eventBase.jsonStr_.str());
    if (r != SUCCESS) {
        eventBase.retCode_ = r;
        ExplainRetCode(eventBase);
    }
}

void HiSysEvent::AppendHexData(HiSysEvent::EventBase &eventBase, const std::string &key, uint64_t value)
{
    eventBase.jsonStr_ << "\"" << key << "\":\"" << std::hex << value << "\"," << std::dec;
}

void HiSysEvent::WritebaseInfo(HiSysEvent::EventBase &eventBase)
{
    if (!StringFilter::GetInstance().IsValidName(eventBase.domain_, MAX_DOMAIN_LENGTH)) {
        eventBase.retCode_ = ERR_DOMAIN_NAME_INVALID;
        return;
    }
    if (!StringFilter::GetInstance().IsValidName(eventBase.eventName_, MAX_EVENT_NAME_LENGTH)) {
        eventBase.retCode_ = ERR_EVENT_NAME_INVALID;
        return;
    }
    AppendData(eventBase, "domain_", eventBase.domain_);
    AppendData(eventBase, "name_", eventBase.eventName_);
    AppendData(eventBase, "type_", eventBase.type_);
    AppendData(eventBase, "time_", GetMilliseconds());
    AppendData(eventBase, "tz_", GetTimeZone());
    AppendData(eventBase, "pid_", getpid());
    AppendData(eventBase, "tid_", gettid());
    AppendData(eventBase, "uid_", getuid());
    HiTraceId hitraceId = HiTraceChain::GetId();
    if (!hitraceId.IsValid()) {
        eventBase.keyCnt_ = 0;
        return;
    }
    AppendHexData(eventBase, "traceid_", hitraceId.GetChainId());
    AppendHexData(eventBase, "spanid_", hitraceId.GetSpanId());
    AppendHexData(eventBase, "pspanid_", hitraceId.GetParentSpanId());
    AppendData(eventBase, "trace_flag_", hitraceId.GetFlags());
    eventBase.keyCnt_ = 0;
}

void HiSysEvent::AppendInvalidParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    eventBase.retCode_ = ERR_VALUE_INVALID;
}

void HiSysEvent::AppendBoolParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<bool>(eventBase, param.name, param.v.b);
}

void HiSysEvent::AppendInt8Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<int8_t>(eventBase, param.name, param.v.i8);
}

void HiSysEvent::AppendUint8Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<uint8_t>(eventBase, param.name, param.v.ui8);
}

void HiSysEvent::AppendInt16Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<int16_t>(eventBase, param.name, param.v.i16);
}

void HiSysEvent::AppendUint16Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<uint16_t>(eventBase, param.name, param.v.ui16);
}

void HiSysEvent::AppendInt32Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<int32_t>(eventBase, param.name, param.v.i32);
}

void HiSysEvent::AppendUint32Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<uint32_t>(eventBase, param.name, param.v.ui32);
}

void HiSysEvent::AppendInt64Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<int64_t>(eventBase, param.name, param.v.i64);
}

void HiSysEvent::AppendUint64Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<uint64_t>(eventBase, param.name, param.v.ui64);
}

void HiSysEvent::AppendFloatParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<float>(eventBase, param.name, param.v.f);
}

void HiSysEvent::AppendDoubleParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendData<double>(eventBase, param.name, param.v.d);
}

void HiSysEvent::AppendStringParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    if (param.v.s == nullptr) {
        eventBase.retCode_ = ERR_VALUE_INVALID;
        return;
    }
    AppendData(eventBase, param.name, std::string(param.v.s));
}

template<typename T>
void HiSysEvent::AppendArrayParam(HiSysEvent::EventBase &eventBase, const std::string &key,
    const T *array, size_t arraySize)
{
    if (array == nullptr) {
        eventBase.retCode_ = ERR_VALUE_INVALID;
        return;
    }
    std::vector<T> value(array, array + arraySize);
    HiSysEvent::AppendArrayData<T>(eventBase, key, value);
}

void HiSysEvent::AppendBoolArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<bool>(eventBase, param.name, reinterpret_cast<bool*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendInt8ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<int8_t>(eventBase, param.name, reinterpret_cast<int8_t*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendUint8ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<uint8_t>(eventBase, param.name, reinterpret_cast<uint8_t*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendInt16ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<int16_t>(eventBase, param.name, reinterpret_cast<int16_t*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendUint16ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<uint16_t>(eventBase, param.name, reinterpret_cast<uint16_t*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendInt32ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<int32_t>(eventBase, param.name, reinterpret_cast<int32_t*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendUint32ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<uint32_t>(eventBase, param.name, reinterpret_cast<uint32_t*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendInt64ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<int64_t>(eventBase, param.name, reinterpret_cast<int64_t*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendUint64ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<uint64_t>(eventBase, param.name, reinterpret_cast<uint64_t*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendFloatArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<float>(eventBase, param.name, reinterpret_cast<float*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendDoubleArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    AppendArrayParam<double>(eventBase, param.name, reinterpret_cast<double*>(param.v.array), param.arraySize);
}

void HiSysEvent::AppendStringArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    auto array = reinterpret_cast<char**>(param.v.array);
    if (array == nullptr) {
        eventBase.retCode_ = ERR_VALUE_INVALID;
        return;
    }
    for (size_t i = 0; i < param.arraySize; ++i) {
        if (auto temp = array + i; *temp == nullptr) {
            eventBase.retCode_ = ERR_VALUE_INVALID;
            return;
        }
    }
    std::vector<std::string> value(array, array + param.arraySize);
    HiSysEvent::AppendArrayData<std::string>(eventBase, param.name, value);
}

void HiSysEvent::AppendParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param)
{
    using AppendParamFunc = void (*)(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    constexpr int totalAppendFuncSize = 25;
    const AppendParamFunc appendFuncs[totalAppendFuncSize] = {
        &HiSysEvent::AppendInvalidParam,
        &HiSysEvent::AppendBoolParam,
        &HiSysEvent::AppendInt8Param,
        &HiSysEvent::AppendUint8Param,
        &HiSysEvent::AppendInt16Param,
        &HiSysEvent::AppendUint16Param,
        &HiSysEvent::AppendInt32Param,
        &HiSysEvent::AppendUint32Param,
        &HiSysEvent::AppendInt64Param,
        &HiSysEvent::AppendUint64Param,
        &HiSysEvent::AppendFloatParam,
        &HiSysEvent::AppendDoubleParam,
        &HiSysEvent::AppendStringParam,
        &HiSysEvent::AppendBoolArrayParam,
        &HiSysEvent::AppendInt8ArrayParam,
        &HiSysEvent::AppendUint8ArrayParam,
        &HiSysEvent::AppendInt16ArrayParam,
        &HiSysEvent::AppendUint16ArrayParam,
        &HiSysEvent::AppendInt32ArrayParam,
        &HiSysEvent::AppendUint32ArrayParam,
        &HiSysEvent::AppendInt64ArrayParam,
        &HiSysEvent::AppendUint64ArrayParam,
        &HiSysEvent::AppendFloatArrayParam,
        &HiSysEvent::AppendDoubleArrayParam,
        &HiSysEvent::AppendStringArrayParam,
    };
    if (size_t paramType = param.t; paramType < totalAppendFuncSize) {
        appendFuncs[paramType](eventBase, param);
    } else {
        eventBase.retCode_ = ERR_VALUE_INVALID;
    }
}
} // namespace HiviewDFX
} // namespace OHOS

