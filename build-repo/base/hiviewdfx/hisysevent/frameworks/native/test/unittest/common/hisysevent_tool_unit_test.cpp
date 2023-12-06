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

#include "hisysevent_tool_unit_test.h"

#include <thread>
#include <chrono>

#include "hilog/log.h"

#include "hisysevent_delegate.h"
#include "hisysevent_record.h"
#include "hisysevent_tool_listener.h"
#include "hisysevent_tool_query.h"
#include "hisysevent_tool.h"
#include "json_flatten_parser.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int ARGV_START_INDEX = 1;
constexpr int SLEEP_DURATION = 1000;
inline uint64_t GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

void RunCmds(std::shared_ptr<HiSysEventTool> tool, int argc, char** argv)
{
    auto ret = tool->ParseCmdLine(argc, argv);
    ASSERT_TRUE(ret);
    ret = tool->DoAction();
    ASSERT_TRUE(ret);
    thread notifyThread([tool] () {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION));
        tool->NotifyClient();
    });
    notifyThread.detach();
    tool->WaitClient();
    ASSERT_TRUE(true);
}
}

void HiSysEventToolUnitTest::SetUpTestCase() {}

void HiSysEventToolUnitTest::TearDownTestCase() {}

void HiSysEventToolUnitTest::SetUp() {}

void HiSysEventToolUnitTest::TearDown() {}

/**
 * @tc.name: HiSysEventToolUnitTest001
 * @tc.desc: Test APIs of class HiSysEventToolListener
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest001, testing::ext::TestSize.Level3)
{
    constexpr char origin[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"UINT64_T\":18446744073709551610,\"DOUBLE_T\":3.3,\"INT64_T\":9223372036854775800,\
        \"PARAM_B\":[\"123\", \"456\", \"789\"],\"PARAM_C\":[]}";
    auto sysEventRecord = std::make_shared<HiSysEventRecord>(origin);
    HiSysEventToolListener listener1(true);
    listener1.OnEvent(nullptr);
    ASSERT_TRUE(true);
    listener1.OnEvent(sysEventRecord);
    ASSERT_TRUE(true);
    HiSysEventToolListener listener2(false);
    listener2.OnEvent(nullptr);
    ASSERT_TRUE(true);
    listener2.OnEvent(sysEventRecord);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: HiSysEventToolUnitTest002
 * @tc.desc: Test APIs of class JsonFlattenParser
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest002, testing::ext::TestSize.Level3)
{
    constexpr char origin[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"UINT64_T\":18446744073709551610,\"DOUBLE_T\":3.3,\"INT64_T\":9223372036854775800,\
        \"PARAM_B\":[\"123\", \"456\", \"789\"],\"PARAM_C\":[\"A\",\"B\",\"C\"]}";
    JsonFlattenParser parser(origin);
    auto ret = parser.Print([] (auto& kv) {
        return kv.first.append(kv.second);
    });
    ASSERT_TRUE(ret.find("UINT64_T18446744073709551610") != string::npos);
    parser.Parse(origin);
    ret = parser.Print([] (auto& kv) {
        return kv.first.append("|").append(kv.second);
    });
    ASSERT_TRUE(ret.find("UINT64_T|18446744073709551610") != string::npos);
}

/**
 * @tc.name: HiSysEventToolUnitTest003
 * @tc.desc: Test APIs of class HiSysEventJsonDecorator
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest003, testing::ext::TestSize.Level3)
{
    constexpr char origin0[] = "{\"domain_\":\"USERIAM_PIN\",}";
    HiSysEventRecord record0(origin0);
    HiSysEventJsonDecorator decorator;
    auto ret = decorator.DecorateEventJsonStr(record0);
    ASSERT_EQ(std::string(origin0), ret);
    constexpr char origin1[] = "{\"domain_\":\"USERIAM_PIN\",\"name_\":\"USERIAM_TEMPLATE_CHANGE\",\"type_\":3,"
        "\"time_\":1502965663170,\"tz_\":\"+0800\",\"pid_\":1710,\"tid_\":1747,"
        "\"uid_\":20010037,\"UID\":1,\"PID\":1,\"PROCESS_NAME\":\"hiview js test suite\","
        "\"PACKAGE_NAME\":\"com.test.testHiSysEvent\",\"level_\":\"CRITICAL\","
        "\"id_\":\"14645518577780955344\",\"info_\":\"\",\"seq_\":357}";
    HiSysEventRecord record1(origin1);
    ret = decorator.DecorateEventJsonStr(record1);
    ASSERT_TRUE(ret.find("\033[31m") != string::npos);
    constexpr char origin2[] = "{\"domain_\":\"USERIAM_PIN\",\"name_\":\"USERIAM_TEMPLATE_CHANGE\",\"type_\":3,"
        "\"time_\":1502965663170,\"tz_\":\"+0800\",\"pid_\":1710,\"tid_\":1747,"
        "\"uid_\":20010037,\"level_\":\"CRITICAL\","
        "\"id_\":\"14645518577780955344\",\"info_\":\"\"}";
    HiSysEventRecord record2(origin2);
    ret = decorator.DecorateEventJsonStr(record2);
    ASSERT_TRUE(ret.find("\033[31m") == string::npos);
    constexpr char origin3[] = "{\"domain_\":\"USERIAM_PIN\",\"name_\":\"USERIAM_TEMPLATE_CHANGE\",\"type_\":3,"
        "\"time_\":1502965663170,\"tz_\":\"+0800\",\"pid_\":1710,\"tid_\":1747,"
        "\"uid_\":20010037,\"CHANGE_TYPE\":\"hiview js test suite\",\"level_\":\"CRITICAL\","
        "\"id_\":\"14645518577780955344\",\"info_\":\"\",\"seq_\":357}";
    HiSysEventRecord record3(origin3);
    ret = decorator.DecorateEventJsonStr(record3);
    ASSERT_TRUE(ret.find("\033[31m") != string::npos);
}

/**
 * @tc.name: HiSysEventToolUnitTest004
 * @tc.desc: Test APIs of class HiSysEventToolQuery
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest004, testing::ext::TestSize.Level3)
{
    constexpr char origin[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":3.4,\"UINT64_T\":18446744073709551610,\"DOUBLE_T\":3.3,\"INT64_T\":9223372036854775800,\
        \"PARAM_B\":[\"123\", \"456\", \"789\"],\"PARAM_C\":[]}";
    HiSysEventRecord sysEventRecord(origin);
    auto records1 = std::make_shared<std::vector<HiSysEventRecord>>();
    records1->emplace_back(sysEventRecord);
    HiSysEventToolQuery query1(true);
    query1.OnQuery(nullptr);
    ASSERT_TRUE(true);
    query1.OnQuery(records1);
    ASSERT_TRUE(true);
    HiSysEventToolQuery query2(false, false);
    query2.OnQuery(nullptr);
    ASSERT_TRUE(true);
    query2.OnQuery(records1);
    ASSERT_TRUE(true);
    int32_t reason = 0;
    int32_t count = 10;
    query2.OnComplete(reason, count);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: HiSysEventToolUnitTest005
 * @tc.desc: Test show help, wait/notify information
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest005, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    tool->DoCmdHelp();
    ASSERT_TRUE(true);
    constexpr int argc = 2;
    const char* argv[] = {
        "hisysevent",
        "-h",
    };
    optind = ARGV_START_INDEX;
    auto ret = tool->ParseCmdLine(argc, const_cast<char**>(argv));
    ASSERT_TRUE(!ret);
}

/**
 * @tc.name: HiSysEventToolUnitTest006
 * @tc.desc: Test subscribe hisysevents by tag
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest006, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 7;
    const char* argv[] = {
        "hisysevent",
        "-r",
        "-c",
        "WHOLE_WORD",
        "-t",
        "TAG1",
        "-v",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}

/**
 * @tc.name: HiSysEventToolUnitTest007
 * @tc.desc: Test subscribe hisysevents by domain and event name
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest007, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 9;
    const char* argv[] = {
        "hisysevent",
        "-r",
        "-c",
        "PREFIX",
        "-o",
        "USERIAM_PIN",
        "-n",
        "USERIAM_TEMPLATE_CHANGE",
        "-v",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}

/**
 * @tc.name: HiSysEventToolUnitTest008
 * @tc.desc: Test subscribe sysevents in debug mode
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest008, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 3;
    const char* argv[] = {
        "hisysevent",
        "-r",
        "-d",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}

/**
 * @tc.name: HiSysEventToolUnitTest009
 * @tc.desc: Test query hisysevents by event type
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest009, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 6;
    const char* argv[] = {
        "hisysevent",
        "-l",
        "-m",
        "1",
        "-g",
        "SECURITY",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}

/**
 * @tc.name: HiSysEventToolUnitTest010
 * @tc.desc: Test query hisysevents by default timestamp
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest010, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 9;
    const char* argv[] = {
        "hisysevent",
        "-l",
        "-m",
        "1",
        "-s",
        "-1",
        "-e",
        "-1",
        "-o",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}

/**
 * @tc.name: HiSysEventToolUnitTest011
 * @tc.desc: Test query hisysevents by accurate timestamp
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest011, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 9;
    auto endTimeStamp = GetMilliseconds();
    int64_t offset = 1000;
    auto beginTimeStamp = endTimeStamp - offset;
    const char* argv[] = {
        "hisysevent",
        "-l",
        "-m",
        "1000",
        "-s",
        std::to_string(beginTimeStamp).c_str(),
        "-e",
        std::to_string(endTimeStamp).c_str(),
        "-o",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}

/**
 * @tc.name: HiSysEventToolUnitTest012
 * @tc.desc: Test query hisysevents by domain and event name
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest012, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 9;
    const char* argv[] = {
        "hisysevent",
        "-l",
        "-m",
        "2000",
        "-o",
        "USERIAM_PIN",
        "-n",
        "USERIAM_TEMPLATE_CHANGE",
        "-v",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}

/**
 * @tc.name: HiSysEventToolUnitTest013
 * @tc.desc: Test APIs of class HiSysEventJsonDecorator
 * @tc.type: FUNC
 * @tc.require: issueI66JWR
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest013, testing::ext::TestSize.Level3)
{
    HiSysEventJsonDecorator decorator;
    constexpr char origin0[] = "";
    HiSysEventRecord record0(origin0);
    auto ret = decorator.DecorateEventJsonStr(record0);
    ASSERT_TRUE(ret.empty());
    constexpr char origin1[] = "{\"domain_\":\"UNKNOWN_DOMAIN\"}";
    HiSysEventRecord record1(origin1);
    ret = decorator.DecorateEventJsonStr(record1);
    ASSERT_EQ(ret, std::string(origin1));
    constexpr char origin2[] = "{\"domain_\":\"USERIAM_PIN\",\"name_\":\"UNKNOWN_EVENTNAME\",\"type_\":3}";
    HiSysEventRecord record2(origin2);
    ret = decorator.DecorateEventJsonStr(record2);
    ASSERT_EQ(ret, std::string(origin2));
}

/**
 * @tc.name: HiSysEventToolUnitTest014
 * @tc.desc: Test query hisysevents by valid formatted timestamp
 * @tc.type: FUNC
 * @tc.require: issueI68VXJ
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest014, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 6;
    const char* argv[] = {
        "hisysevent",
        "-l",
        "-S",
        "2023-01-05 10:28:33",
        "-E",
        "2023-01-05 10:29:33",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}

/**
 * @tc.name: HiSysEventToolUnitTest015
 * @tc.desc: Test query hisysevents by invalid formatted timestamp
 * @tc.type: FUNC
 * @tc.require: issueI68VXJ
 */
HWTEST_F(HiSysEventToolUnitTest, HiSysEventToolUnitTest015, testing::ext::TestSize.Level3)
{
    auto tool = std::make_shared<HiSysEventTool>(false);
    constexpr int argc = 6;
    const char* argv[] = {
        "hisysevent",
        "-l",
        "-S",
        "2023-00-00 10:70:33",
        "-E",
        "2023-00-00 10:70:70",
    };
    optind = ARGV_START_INDEX;
    RunCmds(tool, argc, const_cast<char**>(argv));
}
} // HiviewDFX
} // OHOS