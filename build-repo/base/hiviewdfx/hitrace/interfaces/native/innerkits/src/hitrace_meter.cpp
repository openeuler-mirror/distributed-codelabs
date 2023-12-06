/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <atomic>
#include <cinttypes>
#include <climits>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <mutex>
#include <unistd.h>
#include <vector>
#include "securec.h"
#include "hilog/log.h"
#include "parameter.h"
#include "parameters.h"
#include "hitrace_meter.h"

using namespace std;
using namespace OHOS::HiviewDFX;

#define EXPECTANTLY(exp) (__builtin_expect(!!(exp), true))
#define UNEXPECTANTLY(exp) (__builtin_expect(!!(exp), false))
#define UNUSED_PARAM __attribute__((__unused__))

namespace {
int g_markerFd = -1;
std::once_flag g_onceFlag;

std::atomic<bool> g_isHitraceMeterDisabled(false);
std::atomic<bool> g_isHitraceMeterInit(false);
std::atomic<uint64_t> g_tagsProperty(HITRACE_TAG_NOT_READY);

const std::string KEY_TRACE_TAG = "debug.hitrace.tags.enableflags";
const std::string KEY_APP_NUMBER = "debug.hitrace.app_number";
const std::string KEY_RO_DEBUGGABLE = "ro.debuggable";
const std::string KEY_PREFIX = "debug.hitrace.app_";

constexpr int NAME_MAX_SIZE = 1000;
constexpr int VAR_NAME_MAX_SIZE = 256;

static std::vector<std::string> g_markTypes = {"B", "E", "S", "F", "C"};
enum MarkerType { MARKER_BEGIN, MARKER_END, MARKER_ASYNC_BEGIN, MARKER_ASYNC_END, MARKER_INT, MARKER_MAX };

constexpr uint64_t HITRACE_TAG = 0xD002D33;
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HITRACE_TAG, "HitraceMeter"};

static void ParameterChange(const char* key, const char* value, void* context)
{
    HiLog::Info(LABEL, "ParameterChange enableflags %{public}s", value);
    UpdateTraceLabel();
}

bool IsAppValid()
{
    // Judge if application-level tracing is enabled.
    if (OHOS::system::GetBoolParameter(KEY_RO_DEBUGGABLE, 0)) {
        std::ifstream fs;
        fs.open("/proc/self/cmdline");
        if (!fs.is_open()) {
            fprintf(stderr, "IsAppValid, open /proc/self/cmdline failed.\n");
            return false;
        }

        std::string lineStr;
        std::getline(fs, lineStr);
        int nums = OHOS::system::GetIntParameter<int>(KEY_APP_NUMBER, 0);
        for (int i = 0; i < nums; i++) {
            std::string keyStr = KEY_PREFIX + std::to_string(i);
            std::string val = OHOS::system::GetParameter(keyStr, "");
            if (val == "*" || val == lineStr) {
                fs.close();
                return true;
            }
        }
    }
    return false;
}

uint64_t GetSysParamTags()
{
    // Get the system parameters of KEY_TRACE_TAG.
    uint64_t tags = OHOS::system::GetUintParameter<uint64_t>(KEY_TRACE_TAG, 0);
    if (tags == 0) {
        // HiLog::Error(LABEL, "GetUintParameter %s error.\n", KEY_TRACE_TAG.c_str());
        return 0;
    }

    IsAppValid();
    return (tags | HITRACE_TAG_ALWAYS) & HITRACE_TAG_VALID_MASK;
}

// open file "trace_marker".
void OpenTraceMarkerFile()
{
    const std::string debugFile = "/sys/kernel/debug/tracing/trace_marker";
    const std::string traceFile = "/sys/kernel/tracing/trace_marker";
    g_markerFd = open(debugFile.c_str(), O_WRONLY | O_CLOEXEC);
    if (g_markerFd == -1) {
        HiLog::Error(LABEL, "open trace file %{public}s failed: %{public}d", debugFile.c_str(), errno);
        g_markerFd = open(traceFile.c_str(), O_WRONLY | O_CLOEXEC);
        if (g_markerFd == -1) {
            HiLog::Error(LABEL, "open trace file %{public}s failed: %{public}d", traceFile.c_str(), errno);
            g_tagsProperty = 0;
            return;
        }
    }
    g_tagsProperty = GetSysParamTags();

    if (WatchParameter(KEY_TRACE_TAG.c_str(), ParameterChange, nullptr) != 0) {
        HiLog::Error(LABEL, "WatchParameter %{public}s failed", KEY_TRACE_TAG.c_str());
        return;
    }
    g_isHitraceMeterInit = true;
}
}; // namespace

void AddHitraceMeterMarker(MarkerType type, uint64_t tag, const std::string& name, const std::string& value)
{
    if (UNEXPECTANTLY(g_isHitraceMeterDisabled)) {
        return;
    }
    if (UNEXPECTANTLY(!g_isHitraceMeterInit)) {
        struct timespec ts = { 0, 0 };
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1 || ts.tv_sec < 20) { // 20 : register after boot 20s
            return;
        }
        std::call_once(g_onceFlag, OpenTraceMarkerFile);
    }
    if (UNEXPECTANTLY(g_tagsProperty & tag)) {
        // record fomart: "type|pid|name value".
        std::string record = g_markTypes[type] + "|";
        record += std::to_string(getpid()) + "|";
        record += (name.size() < NAME_MAX_SIZE) ? name : name.substr(0, NAME_MAX_SIZE);
        record += " " + value;
        if (write(g_markerFd, record.c_str(), record.size()) < 0) {
            HiLog::Error(LABEL, "write trace_marker failed, %{public}d", errno);
        }
    }
}

void UpdateTraceLabel()
{
    if (!g_isHitraceMeterInit) {
        return;
    }
    g_tagsProperty = GetSysParamTags();
}

void SetTraceDisabled(bool disable)
{
    g_isHitraceMeterDisabled = disable;
}

void StartTrace(uint64_t label, const string& value, float limit UNUSED_PARAM)
{
    string traceName = "H:" + value;
    AddHitraceMeterMarker(MARKER_BEGIN, label, traceName, "");
}

void StartTraceDebug(bool isDebug, uint64_t label, const string& value, float limit UNUSED_PARAM)
{
    if (!isDebug) {
        return;
    }
    string traceName = "H:" + value;
    AddHitraceMeterMarker(MARKER_BEGIN, label, traceName, "");
}

void StartTraceArgs(uint64_t label, const char *fmt, ...)
{
    char name[VAR_NAME_MAX_SIZE] = { 0 };
    va_list args;
    va_start(args, fmt);
    int res = vsnprintf_s(name, sizeof(name), sizeof(name) - 1, fmt, args);
    va_end(args);
    if (res < 0) {
        HiLog::Error(LABEL, "vsnprintf_s failed: %{public}d", errno);
        return;
    }
    StartTrace(label, name, -1);
}

void StartTraceArgsDebug(bool isDebug, uint64_t label, const char *fmt, ...)
{
    if (!isDebug) {
        return;
    }
    char name[VAR_NAME_MAX_SIZE] = { 0 };
    va_list args;

    va_start(args, fmt);
    int res = vsnprintf_s(name, sizeof(name), sizeof(name) - 1, fmt, args);
    va_end(args);
    if (res < 0) {
        HiLog::Error(LABEL, "vsnprintf_s failed: %{public}d", errno);
        return;
    }
    StartTrace(label, name, -1);
}

void FinishTrace(uint64_t label)
{
    AddHitraceMeterMarker(MARKER_END, label, "", "");
}

void FinishTraceDebug(bool isDebug, uint64_t label)
{
    if (!isDebug) {
        return;
    }
    AddHitraceMeterMarker(MARKER_END, label, "", "");
}

void StartAsyncTrace(uint64_t label, const string& value, int32_t taskId, float limit UNUSED_PARAM)
{
    string traceName = "H:" + value;
    AddHitraceMeterMarker(MARKER_ASYNC_BEGIN, label, traceName, std::to_string(taskId));
}

void StartAsyncTraceDebug(bool isDebug, uint64_t label, const string& value, int32_t taskId, float limit UNUSED_PARAM)
{
    if (!isDebug) {
        return;
    }
    string traceName = "H:" + value;
    AddHitraceMeterMarker(MARKER_ASYNC_BEGIN, label, traceName, std::to_string(taskId));
}

void StartAsyncTraceArgs(uint64_t label, int32_t taskId, const char *fmt, ...)
{
    char name[VAR_NAME_MAX_SIZE] = { 0 };
    va_list args;

    va_start(args, fmt);
    int res = vsnprintf_s(name, sizeof(name), sizeof(name) - 1, fmt, args);
    va_end(args);
    if (res < 0) {
        HiLog::Error(LABEL, "vsnprintf_s failed: %{public}d", errno);
        return;
    }
    StartAsyncTrace(label, name, taskId, -1);
}

void StartAsyncTraceArgsDebug(bool isDebug, uint64_t label, int32_t taskId, const char *fmt, ...)
{
    if (!isDebug) {
        return;
    }
    char name[VAR_NAME_MAX_SIZE] = { 0 };
    va_list args;

    va_start(args, fmt);
    int res = vsnprintf_s(name, sizeof(name), sizeof(name) - 1, fmt, args);
    va_end(args);
    if (res < 0) {
        HiLog::Error(LABEL, "vsnprintf_s failed: %{public}d", errno);
        return;
    }
    StartAsyncTrace(label, name, taskId, -1);
}

void FinishAsyncTrace(uint64_t label, const string& value, int32_t taskId)
{
    string traceName = "H:" + value;
    AddHitraceMeterMarker(MARKER_ASYNC_END, label, traceName, std::to_string(taskId));
}

void FinishAsyncTraceDebug(bool isDebug, uint64_t label, const string& value, int32_t taskId)
{
    if (!isDebug) {
        return;
    }
    string traceName = "H:" + value;
    AddHitraceMeterMarker(MARKER_ASYNC_END, label, traceName, std::to_string(taskId));
}

void FinishAsyncTraceArgs(uint64_t label, int32_t taskId, const char *fmt, ...)
{
    char name[VAR_NAME_MAX_SIZE] = { 0 };
    va_list args;

    va_start(args, fmt);
    int res = vsnprintf_s(name, sizeof(name), sizeof(name) - 1, fmt, args);
    va_end(args);
    if (res < 0) {
        HiLog::Error(LABEL, "vsnprintf_s failed: %{public}d", errno);
        return;
    }
    FinishAsyncTrace(label, name, taskId);
}

void FinishAsyncTraceArgsDebug(bool isDebug, uint64_t label, int32_t taskId, const char *fmt, ...)
{
    if (!isDebug) {
        return;
    }
    char name[VAR_NAME_MAX_SIZE] = { 0 };
    va_list args;

    va_start(args, fmt);
    int res = vsnprintf_s(name, sizeof(name), sizeof(name) - 1, fmt, args);
    va_end(args);
    if (res < 0) {
        HiLog::Error(LABEL, "vsnprintf_s failed: %{public}d", errno);
        return;
    }
    FinishAsyncTrace(label, name, taskId);
}

void MiddleTrace(uint64_t label, const string& beforeValue UNUSED_PARAM, const std::string& afterValue)
{
    string traceName = "H:" + afterValue;
    AddHitraceMeterMarker(MARKER_END, label, "", "");
    AddHitraceMeterMarker(MARKER_BEGIN, label, traceName, "");
}

void MiddleTraceDebug(bool isDebug, uint64_t label, const string& beforeValue UNUSED_PARAM, 
    const std::string& afterValue)
{
    if (!isDebug) {
        return;
    }
    string traceName = "H:" + afterValue;
    AddHitraceMeterMarker(MARKER_END, label, "", "");
    AddHitraceMeterMarker(MARKER_BEGIN, label, traceName, "");
}

void CountTrace(uint64_t label, const string& name, int64_t count)
{
    string traceName = "H:" + name;
    AddHitraceMeterMarker(MARKER_INT, label, traceName, std::to_string(count));
}

void CountTraceDebug(bool isDebug, uint64_t label, const string& name, int64_t count)
{
    if (!isDebug) {
        return;
    }
    string traceName = "H:" + name;
    AddHitraceMeterMarker(MARKER_INT, label, traceName, std::to_string(count));
}

HitraceMeterFmtScoped::HitraceMeterFmtScoped(uint64_t label, const char *fmt, ...) : mTag(label)
{
    char name[VAR_NAME_MAX_SIZE] = { 0 };
    va_list args;

    va_start(args, fmt);
    int res = vsnprintf_s(name, sizeof(name), sizeof(name) - 1, fmt, args);
    va_end(args);
    if (res < 0) {
        HiLog::Error(LABEL, "vsnprintf_s failed: %{public}d", errno);
        return;
    }
    StartTrace(label, name, -1);
}
