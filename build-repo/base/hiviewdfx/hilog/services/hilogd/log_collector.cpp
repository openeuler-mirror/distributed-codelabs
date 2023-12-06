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

#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <iostream>
#include <string>
#include <thread>
#include <string_view>

#include "log_kmsg.h"
#include "flow_control.h"
#include "log_domains.h"
#include "log_collector.h"

namespace OHOS {
namespace HiviewDFX {
using namespace std;

void LogCollector::InsertDropInfo(const HilogMsg &msg, int droppedCount)
{
    string dropLog = to_string(droppedCount) + " line(s) dropped!";
    constexpr auto tag = "LOGLIMITD"sv;
    std::vector<char> buffer(sizeof(HilogMsg) + tag.size() + dropLog.size() + 1, '\0');
    HilogMsg *dropMsg = reinterpret_cast<HilogMsg *>(buffer.data());
    if (dropMsg != nullptr) {
        dropMsg->len     = buffer.size();
        dropMsg->version = msg.version;
        dropMsg->type    = msg.type;
        dropMsg->level   = msg.level;
        dropMsg->tag_len = tag.size();
        dropMsg->tv_sec  = msg.tv_sec;
        dropMsg->tv_nsec = msg.tv_nsec;
        dropMsg->mono_sec = msg.mono_sec;
        dropMsg->pid     = msg.pid;
        dropMsg->tid     = msg.tid;
        dropMsg->domain  = msg.domain;

        auto remainSize = buffer.size() - sizeof(HilogMsg);
        if (memcpy_s(dropMsg->tag, remainSize, tag.data(), tag.size())) {
            std::cerr << "Can't copy tag info of Drop Info message\n";
        }

        remainSize -= tag.size();
        auto logTextPtr = dropMsg->tag + tag.size(); // log text is behind tag data
        if (memcpy_s(logTextPtr, remainSize, dropLog.data(), dropLog.size() + 1)) {
            std::cerr << "Can't copy log text info of Drop Info message\n";
        }

        InsertLogToBuffer(*dropMsg);
    }
}
#ifndef __RECV_MSG_WITH_UCRED_
void LogCollector::onDataRecv(std::vector<char>& data)
#else
void LogCollector::onDataRecv(const ucred& cred, std::vector<char>& data)
#endif
{
    if (data.size() < sizeof(HilogMsg)) {
        std::cerr << "Internal error - received data less than HilogMsg size";
        return;
    }

    HilogMsg& msg = *(reinterpret_cast<HilogMsg *>(data.data()));
    // check domain id
    if (!debug && IsValidDomain(static_cast<LogType>(msg.type), msg.domain) == false) {
        std::cout << "Invalid domain id: 0x" << std::hex << msg.domain << std::dec << ", type:" << msg.type << endl;
        return;
    }
#ifdef __RECV_MSG_WITH_UCRED_
    msg.pid = cred.pid;
#endif
    bool dropped = false;
    // Domain flow control
    do {
        int ret = 0;
        if (msg.type != static_cast<uint16_t>(LOG_APP) && flowControl && !debug) {
            ret = FlowCtrlDomain(msg);
        }
        if (ret < 0) {
            // dropping message
            dropped = true;
            break;
        } else if (ret > 0) {
            // >0 means a new statistic period start, "ret" is the number of dropping lines in last period
            InsertDropInfo(msg, ret);
        }
        InsertLogToBuffer(msg);
    } while (0);

    // Log statistics
    if (countEnable) {
        StatsInfo info = {
            .level = msg.level,
            .type = msg.type,
            .len = (msg.len - sizeof(HilogMsg) - 1 - 1), // don't count '\0' of tag and content
            .dropped = dropped ? 1 : 0,
            .domain = msg.domain,
            .pid = msg.pid,
            .tv_sec = msg.tv_sec,
            .tv_nsec = msg.tv_nsec,
            .mono_sec = msg.mono_sec,
            .tag = msg.tag
        };
        m_hilogBuffer.CountLog(info);
    }
}

size_t LogCollector::InsertLogToBuffer(const HilogMsg& msg)
{
    if (msg.type >= LOG_TYPE_MAX) {
        return ERR_LOG_TYPE_INVALID;
    }
    return m_hilogBuffer.Insert(msg);
}

void LogCollector::SetLogFlowControl(bool on)
{
    flowControl = on;
}

void LogCollector::SetDebuggable(bool on)
{
    debug = on;
}
} // namespace HiviewDFX
} // namespace OHOS
