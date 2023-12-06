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

#include "hisysevent_query_callback_c.h"

#include "hilog/log.h"
#include "hisysevent_record_c.h"
#include "string_util.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT_C_QUERY" };
using HiSysEventRecordCls = OHOS::HiviewDFX::HiSysEventRecord;
using OHOS::HiviewDFX::HiLog;

int ConvertDomain(const HiSysEventRecordCls& recordObj, HiSysEventRecord& recordStruct)
{
    constexpr size_t maxLen = 16;
    return OHOS::HiviewDFX::StringUtil::CopyCString(recordStruct.domain, recordObj.GetDomain(), maxLen);
}

int ConvertEventName(const HiSysEventRecordCls& recordObj, HiSysEventRecord& recordStruct)
{
    constexpr size_t maxLen = 32;
    return OHOS::HiviewDFX::StringUtil::CopyCString(recordStruct.eventName, recordObj.GetEventName(), maxLen);
}

int ConvertTimeZone(const HiSysEventRecordCls& recordObj, HiSysEventRecord& recordStruct)
{
    constexpr size_t maxLen = 5;
    return OHOS::HiviewDFX::StringUtil::CopyCString(recordStruct.tz, recordObj.GetTimeZone(), maxLen);
}

int ConvertLevel(const HiSysEventRecordCls& recordObj, HiSysEventRecord& recordStruct)
{
    return OHOS::HiviewDFX::StringUtil::CreateCString(&recordStruct.level, recordObj.GetLevel());
}

int ConvertTag(const HiSysEventRecordCls& recordObj, HiSysEventRecord& recordStruct)
{
    return OHOS::HiviewDFX::StringUtil::CreateCString(&recordStruct.tag, recordObj.GetTag());
}

int ConvertJsonStr(const HiSysEventRecordCls& recordObj, HiSysEventRecord& recordStruct)
{
    constexpr size_t maxLen = 384 * 1024; // max length of the event is 384KB
    return OHOS::HiviewDFX::StringUtil::CreateCString(&recordStruct.jsonStr, recordObj.AsJson(), maxLen);
}

void InitRecord(HiSysEventRecord& record)
{
    OHOS::HiviewDFX::StringUtil::MemsetSafe(&record, sizeof(struct HiSysEventRecord));
}

void DeleteRecord(HiSysEventRecord& record)
{
    OHOS::HiviewDFX::StringUtil::DeletePointer<char>(&record.level);
    OHOS::HiviewDFX::StringUtil::DeletePointer<char>(&record.tag);
    OHOS::HiviewDFX::StringUtil::DeletePointer<char>(&record.jsonStr);
}

void DeleteRecords(HiSysEventRecord** records, size_t len)
{
    if (records == nullptr || *records == nullptr) {
        return;
    }
    auto realRs = *records;
    for (size_t i = 0; i < len; i++) {
        DeleteRecord(realRs[i]);
    }
    delete[] realRs;
    realRs = nullptr;
}

int ConvertRecord(const HiSysEventRecordCls& recordObj, HiSysEventRecord& recordStruct)
{
    if (int res = ConvertDomain(recordObj, recordStruct); res != 0) {
        HiLog::Error(LABEL, "failed to covert domain=%{public}s",  recordObj.GetDomain().c_str());
        return res;
    }
    if (int res = ConvertEventName(recordObj, recordStruct); res != 0) {
        HiLog::Error(LABEL, "failed to covert name=%{public}s",  recordObj.GetEventName().c_str());
        return res;
    }
    recordStruct.type = HiSysEventEventType(recordObj.GetEventType());
    recordStruct.time = recordObj.GetTime();
    if (int res = ConvertTimeZone(recordObj, recordStruct); res != 0) {
        HiLog::Error(LABEL, "failed to covert tz=%{public}s",  recordObj.GetTimeZone().c_str());
        return res;
    }
    recordStruct.pid = recordObj.GetPid();
    recordStruct.tid = recordObj.GetTid();
    recordStruct.uid = recordObj.GetUid();
    recordStruct.traceId = recordObj.GetTraceId();
    recordStruct.spandId = recordObj.GetSpanId();
    recordStruct.pspanId = recordObj.GetPspanId();
    recordStruct.traceFlag = recordObj.GetTraceFlag();
    if (int res = ConvertLevel(recordObj, recordStruct); res != 0) {
        HiLog::Error(LABEL, "failed to covert level=%{public}s",  recordObj.GetLevel().c_str());
        return res;
    }
    if (int res = ConvertTag(recordObj, recordStruct); res != 0) {
        HiLog::Error(LABEL, "failed to covert tag=%{public}s",  recordObj.GetTag().c_str());
        return res;
    }
    if (int res = ConvertJsonStr(recordObj, recordStruct); res != 0) {
        HiLog::Error(LABEL, "failed to covert jsonStr=%{public}s",  recordObj.AsJson().c_str());
        return res;
    }
    return 0;
}
}
void HiSysEventQueryCallbackC::OnQuery(std::shared_ptr<std::vector<OHOS::HiviewDFX::HiSysEventRecord>> sysEvents)
{
    if (onQuery_ == nullptr) {
        HiLog::Error(LABEL, "the OnQuery function of the callback is null");
        return;
    }
    if (sysEvents == nullptr || sysEvents->empty()) {
        onQuery_(nullptr, 0);
        return;
    }
    size_t size = sysEvents->size();
    auto records = new(std::nothrow) HiSysEventRecord[size];
    for (size_t i = 0; i < size; i++) {
        InitRecord(records[i]);
        if (ConvertRecord(sysEvents->at(i), records[i]) != 0) {
            HiLog::Error(LABEL, "failed to covert record, index=%{public}zu, size=%{public}zu",  i, size);
            DeleteRecords(&records, i + 1); // +1 for release the current record
            return;
        }
    }
    onQuery_(records, size);
    DeleteRecords(&records, size);
}

void HiSysEventQueryCallbackC::OnComplete(int32_t reason, int32_t total)
{
    if (onComplete_ == nullptr) {
        HiLog::Error(LABEL, "the OnComplete function of the callback is null");
        return;
    }
    onComplete_(reason, total);
}
