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

#include <cstring>
#include <thread>
#include <vector>
#include <sys/time.h>
#include <regex>

#include <hilog_common.h>
#include <flow_control.h>
#include <log_timestamp.h>
#include <properties.h>
#include <log_utils.h>

#include "log_buffer.h"

namespace OHOS {
namespace HiviewDFX {
using namespace std;

static size_t g_maxBufferSizeByType[LOG_TYPE_MAX] = {262144, 262144, 262144, 262144, 262144};
static int GenerateHilogMsgInside(HilogMsg& hilogMsg, const string& msg, uint16_t logType);

HilogBuffer::HilogBuffer()
{
    for (int i = 0; i < LOG_TYPE_MAX; i++) {
        sizeByType[i] = 0;
    }
    InitBuffLen();
    InitBuffHead();
}

void HilogBuffer::InitBuffLen()
{
    size_t global_size = GetBufferSize(LOG_TYPE_MAX, false);
    size_t persist_global_size = GetBufferSize(LOG_TYPE_MAX, true);
    for (int i = 0; i < LOG_TYPE_MAX; i++) {
        size_t size = GetBufferSize(i, false);
        size_t persist_size = GetBufferSize(i, true);
        SetBuffLen(i, global_size);
        SetBuffLen(i, persist_global_size);
        SetBuffLen(i, size);
        SetBuffLen(i, persist_size);
    }
}

void HilogBuffer::InitBuffHead()
{
    const string msg = "========Zeroth log of type: ";
    std::vector<char> buf(MAX_LOG_LEN, 0);
    HilogMsg *headMsg = reinterpret_cast<HilogMsg *>(buf.data());

    for (uint16_t i = 0; i < LOG_TYPE_MAX; i++) {
        string typeStr = LogType2Str(i);
        if (typeStr == "invalid") {
            continue;
        }
        string tmpStr = msg + typeStr;
        if (GenerateHilogMsgInside(*headMsg, tmpStr, i) == RET_SUCCESS) {
            Insert(*headMsg);
        }
    }
}

HilogBuffer::~HilogBuffer() {}

size_t HilogBuffer::Insert(const HilogMsg& msg)
{
    size_t elemSize = CONTENT_LEN((&msg)); /* include '\0' */

    if (unlikely(msg.tag_len > MAX_TAG_LEN || msg.tag_len == 0 || elemSize > MAX_LOG_LEN || elemSize <= 0)) {
        return 0;
    }

    LogMsgContainer &msgList = (msg.type == LOG_KMSG) ? hilogKlogList : hilogDataList;
    {
        std::lock_guard<decltype(hilogBufferMutex)> lock(hilogBufferMutex);

        // Delete old entries when full
        if (elemSize + sizeByType[msg.type] >= g_maxBufferSizeByType[msg.type]) {
            // Drop 5% of maximum log when full
            std::list<HilogData>::iterator it = msgList.begin();
            static const float DROP_RATIO = 0.05;
            while (sizeByType[msg.type] > g_maxBufferSizeByType[msg.type] * (1 - DROP_RATIO) &&
                it != msgList.end()) {
                if ((*it).type != msg.type) {    // Only remove old logs of the same type
                    ++it;
                    continue;
                }
                OnDeleteItem(it, DeleteReason::BUFF_OVERFLOW);
                size_t cLen = it->len - it->tag_len;
                sizeByType[(*it).type] -= cLen;
                it = msgList.erase(it);
            }

            // Re-confirm if enough elements has been removed
            if (sizeByType[msg.type] >= g_maxBufferSizeByType[msg.type]) {
                std::cout << "Failed to clean old logs." << std::endl;
            }
        }

        // Append new log into HilogBuffer
        msgList.emplace_back(msg);
        OnPushBackedItem(msgList);
    }

    // Update current size of HilogBuffer
    sizeByType[msg.type] += elemSize;

    // Notify readers about new element added
    OnNewItem(msgList);
    return elemSize;
}

static bool LogMatchFilter(const LogFilter& filter, const HilogData& logData)
{
    // types & levels match
    if (((static_cast<uint16_t>(0b01 << logData.type)) & filter.types) == 0) {
        return false;
    }
    if (((static_cast<uint16_t>(0b01 << logData.level)) & filter.levels) == 0) {
        return false;
    }

    int i = 0;
    // domain match
    static constexpr uint32_t LOW_BYTE = 0xFF;
    static constexpr uint32_t LOW_BYTE_REVERSE = ~LOW_BYTE;
    /* 1) domain id equals exactly: (0xd012345 == 0xd012345)
       2) last 8 bits is sub domain id, if it's 0xFF, compare high 24 bits:
       (0xd0123ff & 0xffffff00 == 0xd012345 & 0xffffff00) */
    bool match = false;
    for (i = 0; i < filter.domainCount; i++) {
        if ((logData.domain == filter.domains[i])
        || ((static_cast<uint8_t>(filter.domains[i]) == LOW_BYTE)
             && ((logData.domain & LOW_BYTE_REVERSE) == (filter.domains[i] & LOW_BYTE_REVERSE)))) {
            match = true;
            break;
        }
    }
    if (filter.domainCount && match == filter.blackDomain) {
        return false;
    }
    match = false;
    // tag match
    for (i = 0; i < filter.tagCount; i++) {
        if (strcmp(logData.tag, filter.tags[i]) == 0) {
            match = true;
            break;
        }
    }
    if (filter.tagCount && match == filter.blackTag) {
        return false;
    }
    match = false;
    // pid match
    for (i = 0; i < filter.pidCount; i++) {
        if (logData.pid == filter.pids[i]) {
            match = true;
            break;
        }
    }
    if (filter.pidCount && match == filter.blackPid) {
        return false;
    }
    // regular expression match
    if (filter.regex[0] != 0) {
        std::regex regExpress(filter.regex);
        if (std::regex_search(logData.content, regExpress) == false) {
            return false;
        }
    }
    return true;
}

std::optional<HilogData> HilogBuffer::Query(const LogFilter& filter, const ReaderId& id, int tailCount)
{
    auto reader = GetReader(id);
    if (!reader) {
        std::cerr << "Reader not registered!\n";
        return std::nullopt;
    }
    uint16_t qTypes = filter.types;
    LogMsgContainer &msgList = (qTypes == (0b01 << LOG_KMSG)) ? hilogKlogList : hilogDataList;

    std::shared_lock<decltype(hilogBufferMutex)> lock(hilogBufferMutex);

    if (reader->m_msgList != &msgList) {
        reader->m_msgList = &msgList;
        if (tailCount == 0) {
            reader->m_pos = msgList.begin();
        } else {
            reader->m_pos = msgList.end();
            reader->m_pos--;
        }
        for (int i = 0; (i < tailCount) && (reader->m_pos != msgList.begin());) {
            if (LogMatchFilter(filter, (*reader->m_pos))) {
                i++;
            }
            reader->m_pos--;
        }
    }

    if (reader->skipped) {
        const string msg = "========Slow reader missed log lines: ";
        const string tmpStr = msg + to_string(reader->skipped);
        std::vector<char> buf(MAX_LOG_LEN, 0);
        HilogMsg *headMsg = reinterpret_cast<HilogMsg *>(buf.data());
        if (GenerateHilogMsgInside(*headMsg, tmpStr, LOG_CORE) == RET_SUCCESS) {
            const HilogData logData(*headMsg);
            reader->skipped = 0;
            return logData;
        }
    }

    while (reader->m_pos != msgList.end()) {
        const HilogData& logData = *reader->m_pos;
        reader->m_pos++;
        if (LogMatchFilter(filter, logData)) {
            return logData;
        }
    }
    return std::nullopt;
}

int32_t HilogBuffer::Delete(uint16_t logType)
{
    std::list<HilogData> &msgList = (logType == LOG_KMSG) ? hilogKlogList : hilogDataList;
    if (logType >= LOG_TYPE_MAX) {
        return ERR_LOG_TYPE_INVALID;
    }
    size_t sum = 0;
    std::unique_lock<decltype(hilogBufferMutex)> lock(hilogBufferMutex);
    std::list<HilogData>::iterator it = msgList.begin();

    // Delete logs corresponding to queryCondition
    while (it != msgList.end()) {
        // Only remove old logs of the same type
        if ((*it).type != logType) {
            ++it;
            continue;
        }
        // Delete corresponding logs
        OnDeleteItem(it, DeleteReason::CMD_CLEAR);

        size_t cLen = it->len - it->tag_len;
        sum += cLen;
        sizeByType[(*it).type] -= cLen;
        it = msgList.erase(it);
    }
    return sum;
}

HilogBuffer::ReaderId HilogBuffer::CreateBufReader(std::function<void()> onNewDataCallback)
{
    std::unique_lock<decltype(m_logReaderMtx)> lock(m_logReaderMtx);
    auto reader = std::make_shared<BufferReader>();
    if (reader != nullptr) {
        reader->skipped = 0;
        reader->m_onNewDataCallback = onNewDataCallback;
    }
    ReaderId id = reinterpret_cast<ReaderId>(reader.get());
    m_logReaders.insert(std::make_pair(id, reader));
    return id;
}

void HilogBuffer::RemoveBufReader(const ReaderId& id)
{
    std::unique_lock<decltype(m_logReaderMtx)> lock(m_logReaderMtx);
    auto it = m_logReaders.find(id);
    if (it != m_logReaders.end()) {
        m_logReaders.erase(it);
    }
}

void HilogBuffer::OnDeleteItem(LogMsgContainer::iterator itemPos, DeleteReason reason)
{
    std::shared_lock<decltype(m_logReaderMtx)> lock(m_logReaderMtx);
    for (auto& [id, readerPtr] : m_logReaders) {
        if (readerPtr->m_pos == itemPos) {
            readerPtr->m_pos = std::next(itemPos);
            if (reason == DeleteReason::BUFF_OVERFLOW) {
                readerPtr->skipped++;
            }
        }
    }
}

void HilogBuffer::OnPushBackedItem(LogMsgContainer& msgList)
{
    std::shared_lock<decltype(m_logReaderMtx)> lock(m_logReaderMtx);
    for (auto& [id, readerPtr] : m_logReaders) {
        if (readerPtr->m_pos == msgList.end()) {
            readerPtr->m_pos = std::prev(msgList.end());
        }
    }
}

void HilogBuffer::OnNewItem(LogMsgContainer& msgList)
{
    std::shared_lock<decltype(m_logReaderMtx)> lock(m_logReaderMtx);
    for (auto& [id, readerPtr] : m_logReaders) {
        if (readerPtr->m_msgList == &msgList && readerPtr->m_onNewDataCallback) {
            readerPtr->m_onNewDataCallback();
        }
    }
}

std::shared_ptr<HilogBuffer::BufferReader> HilogBuffer::GetReader(const ReaderId& id)
{
    std::shared_lock<decltype(m_logReaderMtx)> lock(m_logReaderMtx);
    auto it = m_logReaders.find(id);
    if (it != m_logReaders.end()) {
        return it->second;
    }
    return std::shared_ptr<HilogBuffer::BufferReader>();
}

int64_t HilogBuffer::GetBuffLen(uint16_t logType)
{
    if (logType >= LOG_TYPE_MAX) {
        return ERR_LOG_TYPE_INVALID;
    }
    uint64_t buffSize = g_maxBufferSizeByType[logType];
    return buffSize;
}

int32_t HilogBuffer::SetBuffLen(uint16_t logType, uint64_t buffSize)
{
    if (logType >= LOG_TYPE_MAX) {
        return ERR_LOG_TYPE_INVALID;
    }
    if (buffSize < MIN_BUFFER_SIZE || buffSize > MAX_BUFFER_SIZE) {
        return ERR_BUFF_SIZE_INVALID;
    }
    std::unique_lock<decltype(hilogBufferMutex)> lock(hilogBufferMutex);
    g_maxBufferSizeByType[logType] = buffSize;
    return RET_SUCCESS;
}

void HilogBuffer::CountLog(const StatsInfo &info)
{
    stats.Count(info);
}

void HilogBuffer::ResetStats()
{
    stats.Reset();
}

LogStats& HilogBuffer::GetStatsInfo()
{
    return stats;
}

static int GenerateHilogMsgInside(HilogMsg& hilogMsg, const string& msg, uint16_t logType)
{
    const string tag = "HiLog";
    size_t contentLen =  tag.length() + 1 + msg.length() + 1;
    hilogMsg.len = static_cast<uint16_t>(sizeof(HilogMsg) + contentLen);
    hilogMsg.len = hilogMsg.len > MAX_LOG_LEN ? MAX_LOG_LEN : hilogMsg.len;
    contentLen = hilogMsg.len - static_cast<uint16_t>(sizeof(HilogMsg));

    struct timespec ts = {0};
    (void)clock_gettime(CLOCK_REALTIME, &ts);
    struct timespec ts_mono = {0};
    (void)clock_gettime(CLOCK_MONOTONIC, &ts_mono);
    hilogMsg.tv_sec = static_cast<uint32_t>(ts.tv_sec);
    hilogMsg.tv_nsec = static_cast<uint32_t>(ts.tv_nsec);
    hilogMsg.mono_sec = static_cast<uint32_t>(ts_mono.tv_nsec);
    hilogMsg.type = logType;
    hilogMsg.level = LOG_INFO;
    hilogMsg.pid = 0;
    hilogMsg.tid = 0;
    hilogMsg.domain = 0;
    hilogMsg.tag_len = tag.length() + 1;
    if (memcpy_s(hilogMsg.tag, contentLen, tag.c_str(), hilogMsg.tag_len) != 0) {
        return RET_FAIL;
    }
    if (memcpy_s(hilogMsg.tag + hilogMsg.tag_len, contentLen - hilogMsg.tag_len, msg.c_str(), msg.length() + 1) != 0) {
        return RET_FAIL;
    }

    return RET_SUCCESS;
}
} // namespace HiviewDFX
} // namespace OHOS
