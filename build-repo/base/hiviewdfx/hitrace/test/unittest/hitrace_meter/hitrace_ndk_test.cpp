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

#include <fstream>
#include <regex>
#include <string>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <hilog/log.h>

#include "hitrace_meter.h"
#include "hitrace_osal.h"
#include "parameters.h"

using namespace testing::ext;
using namespace std;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::HitraceOsal;

namespace OHOS {
namespace HiviewDFX {
namespace HitraceTest {
const string TRACE_MARKER_PATH = "trace_marker";
const string TRACING_ON_PATH = "tracing_on";
const string TRACING_ON = "tracing_on";
const string TRACE_PATH = "trace";
const string TRACE_MARK_WRITE = "tracing_mark_write";
const string TRACE_PATTERN = "\\s*(.*?)-(.*?)\\s+(.*?)\\[(\\d+?)\\]\\s+(.*?)\\s+((\\d+).(\\d+)?):\\s+"
    + TRACE_MARK_WRITE + ": ";
const string TRACE_START = TRACE_PATTERN + "B\\|(.*?)\\|H:";
const string TRACE_FINISH = TRACE_PATTERN + "E\\|";
const string TRACE_ASYNC_START = TRACE_PATTERN + "S\\|(.*?)\\|H:";
const string TRACE_ASYNC_FINISH = TRACE_PATTERN + "F\\|(.*?)\\|H:";
const string TRACE_COUNT = TRACE_PATTERN + "C\\|(.*?)\\|H:";
const string TRACE_PROPERTY = "debug.hitrace.tags.enableflags";
const string KEY_RO_DEBUGGABLE = "ro.debuggable";
const string KEY_APP_NUMBER = "debug.hitrace.app_number";
constexpr uint32_t TASK = 1;
constexpr uint32_t TID = 2;
constexpr uint32_t TGID = 3;
constexpr uint32_t CPU = 4;
constexpr uint32_t DNH2 = 5;
constexpr uint32_t TIMESTAMP = 6;
constexpr uint32_t PID = 9;
constexpr uint32_t TRACE_NAME = 10;
constexpr uint32_t NUM = 11;

constexpr uint32_t TRACE_FMA11 = 11;
constexpr uint32_t TRACE_FMA12 = 12;

constexpr uint64_t TRACE_INVALIDATE_TAG = 0x1000000;
constexpr uint64_t HITRACE_TAG = 0xD002D33;
const constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HITRACE_TAG, "Hitrace_TEST"};
const uint64_t TAG = HITRACE_TAG_OHOS;
static string g_traceRootPath;

bool SetProperty(const string& property, const string& value);
string GetProperty(const string& property, const string& value);
bool CleanTrace();
bool CleanFtrace();
bool SetFtrace(const string& filename, bool enabled);

class HitraceNDKTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown() {}
};

void  HitraceNDKTest::SetUpTestCase()
{
    const string debugfsDir = "/sys/kernel/debug/tracing/";
    const string tracefsDir = "/sys/kernel/tracing/";
    if (access((debugfsDir + TRACE_MARKER_PATH).c_str(), F_OK) != -1) {
        g_traceRootPath = debugfsDir;
    } else if (access((tracefsDir + TRACE_MARKER_PATH).c_str(), F_OK) != -1) {
        g_traceRootPath = tracefsDir;
    } else {
        HiLog::Error(LABEL, "Error: Finding trace folder failed");
    }
    CleanFtrace();
}

void HitraceNDKTest::TearDownTestCase()
{
    SetProperty(TRACE_PROPERTY, "0");
    SetFtrace(TRACING_ON, false);
    CleanTrace();
}

void HitraceNDKTest::SetUp()
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    string value = to_string(TAG);
    SetProperty(TRACE_PROPERTY, value);
    HiLog::Info(LABEL, "current tag is %{public}s", GetProperty(TRACE_PROPERTY, "0").c_str());
    ASSERT_TRUE(GetProperty(TRACE_PROPERTY, "-123") == value);
    UpdateTraceLabel();
}

struct Param {
    string m_task;
    string m_tid;
    string m_tgid;
    string m_cpu;
    string m_dnh2;
    string m_timestamp;
    string m_pid;
    string m_traceName;
    string m_num;
};

class MyTrace {
    Param m_param;
    bool m_loaded = false;
public:
    MyTrace() : m_loaded(false)
    {
        m_param.m_task = "";
        m_param.m_tid = "";
        m_param.m_tgid = "";
        m_param.m_cpu = "";
        m_param.m_dnh2 = "";
        m_param.m_timestamp = "";
        m_param.m_pid = "";
        m_param.m_traceName = "";
        m_param.m_num = "";
    }

    ~MyTrace()
    {
    }

    // task-pid  ( tig) [cpu] ...1    timestamp: tracing_mark_write: B|pid|traceName
    // task-pid  ( tig) [cpu] ...1    timestamp: tracing_mark_write: E|pid
    void Load(const Param& param)
    {
        m_param.m_task = param.m_task;
        m_param.m_pid = param.m_pid;
        m_param.m_tid = param.m_tid;
        m_param.m_tgid = param.m_tgid;
        m_param.m_cpu = param.m_cpu;
        m_param.m_dnh2 = param.m_dnh2;
        m_param.m_timestamp = param.m_timestamp;
        m_param.m_traceName = param.m_traceName;
        m_param.m_num = param.m_num;
        m_loaded = true;
    }

    string GetTask()
    {
        return m_param.m_task;
    }

    string GetPid()
    {
        if (m_loaded) {
            return m_param.m_pid;
        }
        return "";
    }

    string GetTgid()
    {
        if (m_loaded) {
            return m_param.m_tgid;
        }
        return "";
    }

    string GetCpu()
    {
        if (m_loaded) {
            return m_param.m_cpu;
        }
        return "";
    }

    string GetDnh2()
    {
        if (m_loaded) {
            return m_param.m_dnh2;
        }
        return "";
    }

    string GetTimestamp()
    {
        if (m_loaded) {
            return m_param.m_timestamp;
        }
        return "";
    }

    string GetTraceName()
    {
        if (m_loaded) {
            return m_param.m_traceName;
        }
        return "";
    }

    string GetNum()
    {
        if (m_loaded) {
            return m_param.m_num;
        }
        return "";
    }

    string GetTid()
    {
        if (m_loaded) {
            return m_param.m_tid;
        }
        return "";
    }

    bool IsLoaded() const
    {
        return m_loaded;
    }
};

bool SetProperty(const string& property, const string& value)
{
    bool result = false;
    result = OHOS::system::SetParameter(property, value);
    if (!result) {
        HiLog::Error(LABEL, "Error: setting %s failed", property.c_str());
        return false;
    }
    return true;
}

string GetProperty(const string& property, const string& value)
{
    return OHOS::system::GetParameter(property, value);
}

bool GetTimeDuration(int64_t time1, int64_t time2, int64_t diffRange)
{
    int64_t duration = time2 - time1;
    return (duration > 0) && (duration <= diffRange ? true : false);
}

string& Trim(string& s)
{
    if (s.empty()) {
        return s;
    }
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

int64_t GetTimeStamp(string str)
{
    if (str == "") {
        return 0;
    }
    int64_t time;
    Trim(str);
    time = atol(str.erase(str.find("."), 1).c_str());
    return time;
}

MyTrace GetTraceResult(const string& checkContent, const vector<string>& list)
{
    MyTrace trace;
    if (list.empty() || checkContent == "") {
        return trace;
    }
    regex pattern(checkContent);
    smatch match;
    Param param {""};
    for (int i = list.size() - 1; i >= 0; i--) {
        if (regex_match(list[i],  match, pattern)) {
            param.m_task = match[TASK];
            param.m_tid =  match[TID];
            param.m_tgid = match[TGID];
            param.m_cpu = match[CPU];
            param.m_dnh2 = match[DNH2];
            param.m_timestamp = match[TIMESTAMP];
            param.m_pid = match[PID];
            if (match.size() == TRACE_FMA11) {
                param.m_traceName =   match[TRACE_NAME],
                param.m_num = "";
            } else if (match.size() == TRACE_FMA12) {
                param.m_traceName =   match[TRACE_NAME],
                param.m_num = match[NUM];
            } else {
                param.m_traceName = "";
                param.m_num = "";
            }
            trace.Load(param);
            break;
        }
    }
    return trace;
}

static bool WriteStringToFile(const string& fileName, const string& str)
{
    if (g_traceRootPath.empty()) {
        HiLog::Error(LABEL, "Error: trace path not found.");
        return false;
    }
    ofstream out;
    out.open(g_traceRootPath + fileName, ios::out);
    out << str;
    out.close();
    return true;
}

bool CleanTrace()
{
    if (g_traceRootPath.empty()) {
        HiLog::Error(LABEL, "Error: trace path not found.");
        return false;
    }
    ofstream ofs;
    ofs.open(g_traceRootPath + TRACE_PATH, ofstream::out);
    if (!ofs.is_open()) {
        HiLog::Error(LABEL, "Error: opening trace path failed.");
        return false;
    }
    ofs << "";
    ofs.close();
    return true;
}

static stringstream ReadFile(const string& filename)
{
    stringstream ss;
    char resolvedPath[PATH_MAX] = { 0 };
    if (realpath(filename.c_str(), resolvedPath) == nullptr) {
        fprintf(stderr, "Error: _fullpath %s failed", filename.c_str());
        return ss;
    }
    ifstream fin(resolvedPath);
    if (!fin.is_open()) {
        fprintf(stderr, "opening file: %s failed!", filename.c_str());
        return ss;
    }
    ss << fin.rdbuf();
    fin.close();
    return ss;
}

static bool IsFileExisting(const string& filename)
{
    return access(filename.c_str(), F_OK) != -1;
}

bool SetFtrace(const string& filename, bool enabled)
{
    return WriteStringToFile(filename, enabled ? "1" : "0");
}

bool CleanFtrace()
{
    return WriteStringToFile("set_event", "");
}

string GetFinishTraceRegex(MyTrace& trace)
{
    if (!trace.IsLoaded()) {
        return "";
    } else {
        return "\\s*(.*?)-(" + trace.GetTid() + "?)\\s+(.*?)\\[(\\d+?)\\]\\s+(.*?)\\s+" + "((\\d+).(\\d+)?):\\s+" +
               TRACE_MARK_WRITE + ": E\\|(" + trace.GetPid() + ")|(.*)";
    }
}

vector<string> ReadFile2string(const string& filename)
{
    vector<string> list;
    if (IsFileExisting(filename)) {
        stringstream ss = ReadFile(filename);
        string line;
        while (getline(ss, line)) {
            list.emplace_back(move(line));
        }
    }
    return list;
}

vector<string> ReadTrace()
{
    return ReadFile2string(g_traceRootPath + TRACE_PATH);
}

bool RunCmd(const string& cmdstr)
{
    FILE *fp = popen(cmdstr.c_str(), "r");
    if (fp == nullptr) {
        return false;
    }
    pclose(fp);
    return true;
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start tracing and end tracing.
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_001, TestSize.Level0)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartTrace(TAG, "StartTraceTest001");
    FinishTrace(TAG);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_START + "(StartTraceTest001) ", list);
    ASSERT_TRUE(startTrace.IsLoaded()) << "Can't find \"B|pid|StartTraceTest001\" from trace.";
    MyTrace finishTrace = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    ASSERT_TRUE(finishTrace.IsLoaded()) << "Can't find \"E|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node has no output.
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_002, TestSize.Level0)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartTrace(TAG, "StartTraceTest002");
    FinishTrace(TAG);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_START + "(StartTraceTest002) ", list);
    ASSERT_TRUE(startTrace.IsLoaded()) << "Can't find \"B|pid|StartTraceTest002\" from trace.";
    MyTrace finishTrace = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    ASSERT_TRUE(finishTrace.IsLoaded()) << "Can't find \"E|\" from trace.";
}

/**
  * @tc.name: Hitrace
  * @tc.desc: tracing_mark_write file node normal output start trace and end trace.
  * @tc.type: FUNC
  */
HWTEST_F(HitraceNDKTest, StartTrace_003, TestSize.Level0)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartTrace(TAG, "StartTraceTest003 %s");
    FinishTrace(TAG);
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_START + "(StartTraceTest003 %s) ", list);
    ASSERT_TRUE(startTrace.IsLoaded()) << "Can't find \"B|pid|StartTraceTest003 %s\" from trace.";
    MyTrace finishTrace = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    ASSERT_TRUE(finishTrace.IsLoaded()) << "Can't find \"E|\" from trace.";
    ASSERT_TRUE(CleanTrace());
    list.clear();
    StartTrace(TAG, "StartTraceTest003 %p");
    FinishTrace(TAG);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    list = ReadTrace();
    MyTrace startTrace2 = GetTraceResult(TRACE_START + "(StartTraceTest003 %p) ", list);
    MyTrace finishTrace2 = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    ASSERT_TRUE(finishTrace2.IsLoaded()) << "Can't find \"E|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: test Input and output interval 1ms execution, time fluctuation 1ms
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_004, TestSize.Level0)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartTrace(TAG, "StartTraceTest004");
    usleep(1000);
    FinishTrace(TAG);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_START + "(StartTraceTest004) ", list);
    ASSERT_TRUE(startTrace.IsLoaded()) << "Can't find \"B|pid|StartTraceTest004\" from trace.";
    MyTrace finishTrace = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    ASSERT_TRUE(finishTrace.IsLoaded()) << "Can't find \"E|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start trace and end trace
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_005, TestSize.Level0)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartAsyncTrace(TAG, "asyncTraceTest005", 123);
    FinishAsyncTrace(TAG, "asyncTraceTest005", 123);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_ASYNC_START + "(asyncTraceTest005) (.*)", list);
    ASSERT_TRUE(startTrace.IsLoaded()) << "Can't find \"S|pid|asyncTraceTest005\" from trace.";
    MyTrace finishTrace =
        GetTraceResult(TRACE_ASYNC_FINISH + startTrace.GetTraceName() + " " + startTrace.GetNum(), list);
    ASSERT_TRUE(finishTrace.IsLoaded()) << "Can't find \"F|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start trace and end trace
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_006, TestSize.Level0)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    CountTrace(TAG, "countTraceTest006", 1);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace countTrace = GetTraceResult(TRACE_COUNT + "(countTraceTest006) (.*)", list);
    ASSERT_TRUE(countTrace.IsLoaded()) << "Can't find \"C|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start trace and end trace.
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_007, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartTrace(TRACE_INVALIDATE_TAG, "StartTraceTest007");
    FinishTrace(TRACE_INVALIDATE_TAG);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_START + "(StartTraceTest007)", list);
    EXPECT_FALSE(startTrace.IsLoaded()) << "Can't find \"B|pid|StartTraceTest007\" from trace.";
    MyTrace finishTrace = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    EXPECT_FALSE(finishTrace.IsLoaded()) << "Can't find \"E|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start trace and end trace.
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_008, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartTrace(TRACE_INVALIDATE_TAG, "StartTraceTest008 %s");
    FinishTrace(TRACE_INVALIDATE_TAG);
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_START + "(StartTraceTest008 %s)", list);
    EXPECT_FALSE(startTrace.IsLoaded()) << "Can't find \"B|pid|StartTraceTest008 %s\" from trace.";
    MyTrace finishTrace = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    EXPECT_FALSE(finishTrace.IsLoaded()) << "Can't find \"E|\" from trace.";
    ASSERT_TRUE(CleanTrace());
    list.clear();
    StartTrace(TRACE_INVALIDATE_TAG, "StartTraceTest008 %p");
    FinishTrace(TRACE_INVALIDATE_TAG);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    list = ReadTrace();
    MyTrace startTrace2 = GetTraceResult(TRACE_START + "(StartTraceTest008 %p)", list);
    MyTrace finishTrace2 = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    EXPECT_FALSE(finishTrace2.IsLoaded()) << "Can't find \"E|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start trace and end trace
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_009, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartAsyncTrace(TRACE_INVALIDATE_TAG, "asyncTraceTest009", 123);
    FinishAsyncTrace(TRACE_INVALIDATE_TAG, "asyncTraceTest009", 123);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_ASYNC_START + "(asyncTraceTest009)\\|(.*)", list);
    EXPECT_FALSE(startTrace.IsLoaded()) << "Can't find \"S|pid|asyncTraceTest009\" from trace.";
    MyTrace finishTrace = GetTraceResult(TRACE_ASYNC_FINISH + startTrace.GetTraceName() + "\\|"
        + startTrace.GetNum(), list);
    EXPECT_FALSE(finishTrace.IsLoaded()) << "Can't find \"F|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start trace and end trace
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_010, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    CountTrace(TRACE_INVALIDATE_TAG, "countTraceTest010", 1);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace countTrace = GetTraceResult(TRACE_COUNT + "(countTraceTest010)\\|(.*)", list);
    EXPECT_FALSE(countTrace.IsLoaded()) << "Can't find \"C|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node general output start and end tracing for debugging.
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_011, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartTraceDebug(true, TAG, "StartTraceTest011");
    FinishTraceDebug(true, TAG);
}

/**
  * @tc.name: Hitrace
  * @tc.desc: tracing_mark_write file node general output start and end tracing for debugging.
  * @tc.type: FUNC
  */
HWTEST_F(HitraceNDKTest, StartTrace_012, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartTraceDebug(true, TAG, "StartTraceTest012 %s");
    FinishTraceDebug(true, TAG);
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing StartAsyncTraceDebug and FinishAsyncTraceDebug functions
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_013, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    StartAsyncTraceDebug(true, TAG, "asyncTraceTest013", 123);
    FinishAsyncTraceDebug(true, TAG, "asyncTraceTest013", 123);
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing CountTraceDebug function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_014, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    CountTraceDebug(true, TAG, "countTraceTest014", 1);
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing MiddleTrace function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_015, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    MiddleTrace(TAG, "MiddleTraceTest015", "050tseTecarTelddiM");
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing MiddleTraceDebug function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_016, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    MiddleTraceDebug(true, TAG, "MiddleTraceTest016", "061tseTecarTelddiM");
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start tracing and end tracing with args
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_017, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    int var = 1;
    StartTraceArgs(TAG, "StartTraceTest017-%d", var);
    FinishTrace(TAG);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_START + "(StartTraceTest017-1) ", list);
    ASSERT_TRUE(startTrace.IsLoaded()) << "Can't find \"B|pid|StartTraceTest017-1\" from trace.";
    MyTrace finishTrace = GetTraceResult(GetFinishTraceRegex(startTrace), list);
    ASSERT_TRUE(finishTrace.IsLoaded()) << "Can't find \"E|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: tracing_mark_write file node normal output start trace and end trace async with args
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_018, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    int var = 1;
    StartAsyncTraceArgs(TAG, 123, "asyncTraceTest018-%d", var);
    FinishAsyncTraceArgs(TAG, 123, "asyncTraceTest018-%d", var);
    ASSERT_TRUE(SetFtrace(TRACING_ON, false)) << "Setting tracing_on failed.";
    vector<string> list = ReadTrace();
    MyTrace startTrace = GetTraceResult(TRACE_ASYNC_START + "(asyncTraceTest018-1) (.*)", list);
    ASSERT_TRUE(startTrace.IsLoaded()) << "Can't find \"S|pid|asyncTraceTest018-1\" from trace.";
    MyTrace finishTrace =
        GetTraceResult(TRACE_ASYNC_FINISH + startTrace.GetTraceName() + " " + startTrace.GetNum(), list);
    ASSERT_TRUE(finishTrace.IsLoaded()) << "Can't find \"F|\" from trace.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing StartTraceArgsDebug function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_019, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    int var = 1;
    StartTraceArgsDebug(true, TAG, "StartTraceTest019-%d", var);
    FinishTrace(TAG);
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing StartAsyncTraceArgsDebug and FinishAsyncTraceArgsDebug function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_020, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    int var = 1;
    StartAsyncTraceArgsDebug(true, TAG, 123, "asyncTraceTest020-%d", var);
    FinishAsyncTraceArgsDebug(true, TAG, 123, "asyncTraceTest020-%d", var);
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing SetTraceDisabled function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_021, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    SetTraceDisabled(true);
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing GetPropertyInner function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_022, TestSize.Level1)
{
    ASSERT_TRUE(SetProperty(TRACE_PROPERTY, "0"));
    string tmp;
    ASSERT_TRUE(GetPropertyInner(TRACE_PROPERTY, tmp) == "0") << "GetPropertyInner failed.";
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing IsAppValid function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_023, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(SetFtrace(TRACING_ON, true)) << "Setting tracing_on failed.";
    SetProperty(KEY_RO_DEBUGGABLE, "true");
    SetProperty(KEY_APP_NUMBER, "1");
    StartTrace(TRACE_INVALIDATE_TAG, "StartTraceTest023");
    FinishTrace(TRACE_INVALIDATE_TAG);
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing trace cmd function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_024, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(RunCmd("hitrace -h > /data/log/test1.txt"));
    ASSERT_TRUE(RunCmd("hitrace -l > /data/log/test2.txt"));
    ASSERT_TRUE(RunCmd("hitrace --list_categories > /data/log/test3.txt"));
    ASSERT_TRUE(RunCmd("hitrace --trace_begin > /data/log/test4.txt"));
    ASSERT_TRUE(RunCmd("hitrace --trace_dump > /data/log/test5.txt"));
    ASSERT_TRUE(RunCmd("hitrace --trace_finish > /data/log/test6.txt"));
    ASSERT_TRUE(RunCmd("hitrace --hlep > /data/log/test7.txt"));
    ASSERT_TRUE(RunCmd("hitrace -a > /data/log/test8.txt"));
    ASSERT_TRUE(RunCmd("hitrace --trace_clock > /data/log/test9.txt"));
    ASSERT_TRUE(RunCmd("hitrace -t a > /data/log/test10.txt"));
    ASSERT_TRUE(RunCmd("hitrace -t -1 > /data/log/test11.txt"));
    ASSERT_TRUE(RunCmd("hitrace --time a > /data/log/test12.txt"));
    ASSERT_TRUE(RunCmd("hitrace --time -1 > /data/log/test13.txt"));
    ASSERT_TRUE(RunCmd("hitrace -b a > /data/log/test14.txt"));
    ASSERT_TRUE(RunCmd("hitrace -b -1 > /data/log/test15.txt"));
    ASSERT_TRUE(RunCmd("hitrace --buffer_size a > /data/log/test16.txt"));
    ASSERT_TRUE(RunCmd("hitrace --buffer_size -1 > /data/log/test17.txt"));
    ASSERT_TRUE(RunCmd("hitrace -z --time 1 --buffer_size 10240 --trace_clock clock ohos > /data/log/trace01"));
    ASSERT_TRUE(RunCmd("hitrace -z -t 1 -b 10240 --trace_clock clock --overwrite ohos > /data/log/trace02"));
    ASSERT_TRUE(RunCmd("hitrace -t 1 --trace_clock boot ohos > /data/log/trace03"));
    ASSERT_TRUE(RunCmd("hitrace -t 1 --trace_clock global ohos > /data/log/trace04"));
    ASSERT_TRUE(RunCmd("hitrace -t 1 --trace_clock mono ohos > /data/log/trace05"));
    ASSERT_TRUE(RunCmd("hitrace -t 1 --trace_clock uptime ohos > /data/log/trace06"));
    ASSERT_TRUE(RunCmd("hitrace -t 1 --trace_clock perf ohos > /data/log/trace07"));
}

/**
 * @tc.name: Hitrace
 * @tc.desc: Testing bytrace cmd function
 * @tc.type: FUNC
 */
HWTEST_F(HitraceNDKTest, StartTrace_025, TestSize.Level1)
{
    ASSERT_TRUE(CleanTrace());
    ASSERT_TRUE(RunCmd("bytrace -h > /data/log/test1.txt"));
    ASSERT_TRUE(RunCmd("bytrace -l > /data/log/test2.txt"));
    ASSERT_TRUE(RunCmd("bytrace --list_categories > /data/log/test3.txt"));
    ASSERT_TRUE(RunCmd("bytrace --trace_begin > /data/log/test4.txt"));
    ASSERT_TRUE(RunCmd("bytrace --trace_dump > /data/log/test5.txt"));
    ASSERT_TRUE(RunCmd("bytrace --trace_finish > /data/log/test6.txt"));
    ASSERT_TRUE(RunCmd("bytrace --hlep > /data/log/test7.txt"));
    ASSERT_TRUE(RunCmd("bytrace -a > /data/log/test8.txt"));
    ASSERT_TRUE(RunCmd("bytrace --trace_clock > /data/log/test9.txt"));
    ASSERT_TRUE(RunCmd("bytrace -t a > /data/log/test10.txt"));
    ASSERT_TRUE(RunCmd("bytrace -t -1 > /data/log/test11.txt"));
    ASSERT_TRUE(RunCmd("bytrace --time a > /data/log/test12.txt"));
    ASSERT_TRUE(RunCmd("bytrace --time -1 > /data/log/test13.txt"));
    ASSERT_TRUE(RunCmd("bytrace -b a > /data/log/test14.txt"));
    ASSERT_TRUE(RunCmd("bytrace -b -1 > /data/log/test15.txt"));
    ASSERT_TRUE(RunCmd("bytrace --buffer_size a > /data/log/test16.txt"));
    ASSERT_TRUE(RunCmd("bytrace --buffer_size -1 > /data/log/test17.txt"));
    ASSERT_TRUE(RunCmd("bytrace -z --time 1 --buffer_size 10240 --trace_clock clock ohos > /data/log/trace01"));
    ASSERT_TRUE(RunCmd("bytrace -z -t 1 -b 10240 --trace_clock clock --overwrite ohos > /data/log/trace02"));
    ASSERT_TRUE(RunCmd("bytrace -t 1 --trace_clock boot ohos > /data/log/trace03"));
    ASSERT_TRUE(RunCmd("bytrace -t 1 --trace_clock global ohos > /data/log/trace04"));
    ASSERT_TRUE(RunCmd("bytrace -t 1 --trace_clock mono ohos > /data/log/trace05"));
    ASSERT_TRUE(RunCmd("bytrace -t 1 --trace_clock uptime ohos > /data/log/trace06"));
    ASSERT_TRUE(RunCmd("bytrace -t 1 --trace_clock perf ohos > /data/log/trace07"));
}

} // namespace HitraceTest
} // namespace HiviewDFX
} // namespace OHOS
