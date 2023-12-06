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
#include <gtest/gtest.h>

#include "securec.h"
#include "softbus_errcode.h"
#include "softbus_adapter_mem.h"
#include "softbus_log.h"
#include "softbus_common.h"
#include "softbus_hidumper_interface.h"
#include "softbus_hidumper.h"
#include "softbus_app_info.h"
#include "softbus_hidumper_disc.h"
#include "softbus_hidumper_conn.h"
#include "softbus_hidumper_buscenter.h"
#include "softbus_hidumper_trans.h"
#include "softbus_hidumper_nstack.h"

using namespace std;
using namespace testing::ext;

#define HIDUMPER_VAR_CASE_NUM 3
#define HIDUMPER_ARGV_NUM 10
#define TRANS_DUMP_PROCESS_TEST_NUM 4
#define DISC_DUMP_PROCESS_VALID_TEST_NUM 7
#define DISC_DUMP_PROCESS_INVALID_TEST_NUM 7
#define CONN_DUMP_PROCESS_VALID_TEST_NUM 6
#define CONN_DUMP_PROCESS_INVALID_TEST_NUM 5
#define BUSCENTER_DUMP_PROCESS_VALID_TEST_NUM 6
#define BUSCENTER_DUMP_PROCESS_INVALID_TEST_NUM 5
#define NSTACK_DUMP_PROCESS_VALID_TEST_NUM 4
#define TEST_UID 101
#define TEST_PID 202
#define ERR_FD (-1)
#define TEST_FD_ZERO 0
#define TEST_FD_ONE 1
#define ERR_ARGC (-1)
#define TEST_ARGC_ONE 1
#define TEST_ARGC_TWO 2
#define TEST_ARGC_THREE 3

static const char *g_testModuleName = "testModuleName";
static const char *g_testDumpVar = "test1";
static const char *g_testHelpInfo = "help";
namespace OHOS {
class HidumperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HidumperTest::SetUpTestCase(void) {}

void HidumperTest::TearDownTestCase(void) {}

void HidumperTest::SetUp(void) {}

void HidumperTest::TearDown(void) {}

typedef struct {
    char varName[SOFTBUS_MODULE_NAME_LEN];
    SoftBusVarDumpCb dumpCallback;
} HiDumperVarNode;

typedef struct {
    int32_t fd;
    int32_t argc;
    const char* argv[HIDUMPER_ARGV_NUM];
} HiDumperArgvNode;

/**
 * @tc.name: SoftBusHiDumperInitTest001
 * @tc.desc: Verify SoftBusHiDumperInit function, no parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusHiDumperInitTest001, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusHiDumperInitTest001, Start");
    int32_t ret = SoftBusHiDumperInit();
    EXPECT_TRUE(ret == 0);
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusHiDumperInitTest001, end");
}


// -------------------------------------------------disc----------------------------------------- //
static int32_t DiscVarTest1Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "DiscVarTest1Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

static int32_t DiscVarTest2Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "DiscVarTest2Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

static int32_t DiscVarTest3Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "DiscVarTest3Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

/**
 * @tc.name: SoftBusRegDiscVarDump001
 * @tc.desc: Verify SoftBusRegDiscVarDump function, use discVarArray param, return SOFTBUS_OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegDiscVarDump001, TestSize.Level1)
{
    HiDumperVarNode testDiscVarArray[HIDUMPER_VAR_CASE_NUM] = {
    {"disc_var_test_1", &DiscVarTest1Dumper},
    {"disc_var_test_2", &DiscVarTest2Dumper},
    {"disc_var_test_3", &DiscVarTest3Dumper},
};
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump001, Start");
    int32_t ret;
    for (int i = 0; i < HIDUMPER_VAR_CASE_NUM; i++) {
        ret = SoftBusRegDiscVarDump(testDiscVarArray[i].varName, testDiscVarArray[i].dumpCallback);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump001, end");
}

/**
 * @tc.name: SoftBusRegDiscVarDump002
 * @tc.desc: Verify SoftBusRegDiscVarDump function, first param dumpVar is NULL, return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegDiscVarDump002, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump002, Start");
    int32_t ret = SoftBusRegDiscVarDump(NULL, &DiscVarTest1Dumper);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump002, end");
}

/**
 * @tc.name: SoftBusRegDiscVarDump003
 * @tc.desc: Verify SoftBusRegDiscVarDump function, second param cb is NULL, return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegDiscVarDump003, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump003, Start");
    int32_t ret = SoftBusRegDiscVarDump(const_cast<char *>(g_testDumpVar), NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump003, end");
}

/**
 * @tc.name: SoftBusRegDiscVarDump004
 * @tc.desc: Verify SoftBusRegDiscVarDump function, first param dumpVar is NULL, second cb is null return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegDiscVarDump004, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump004, Start");
    int32_t ret = SoftBusRegDiscVarDump(NULL, NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump004, end");
}

// -------------------------------------------------conn-----------------------------------------//
static int32_t ConnVarTest1Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "ConnVarTest1Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

static int32_t ConnVarTest2Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "ConnVarTest2Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

static int32_t ConnVarTest3Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "ConnVarTest3Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

/**
 * @tc.name: SoftBusRegConnVarDump001
 * @tc.desc: Verify SoftBusRegConnVarDump function, use connVarArray param, return SOFTBUS_OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegConnVarDump001, TestSize.Level1)
{
    HiDumperVarNode testConnVarArray[HIDUMPER_VAR_CASE_NUM] = {
        {"conn_var_test_1", &ConnVarTest1Dumper},
        {"conn_var_test_2", &ConnVarTest2Dumper},
        {"conn_var_test_3", &ConnVarTest3Dumper},
    };
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegConnVarDump001, Start");
    int32_t ret;
    for (int i = 0; i < HIDUMPER_VAR_CASE_NUM; i++) {
        ret = SoftBusRegConnVarDump(testConnVarArray[i].varName, testConnVarArray[i].dumpCallback);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegConnVarDump001, end");
}

/**
 * @tc.name: SoftBusRegConnVarDump002
 * @tc.desc: Verify SoftBusRegConnVarDump function, first param dumpVar is NULL, return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegConnVarDump002, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegConnVarDump002, Start");
    int32_t ret = SoftBusRegConnVarDump(NULL, &ConnVarTest1Dumper);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegConnVarDump002, end");
}

/**
 * @tc.name: SoftBusRegConnVarDump003
 * @tc.desc: Verify SoftBusRegConnVarDump function, second param cb is NULL, return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegConnVarDump003, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegConnVarDump003, Start");
    int32_t ret = SoftBusRegConnVarDump(const_cast<char *>(g_testDumpVar), NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegConnVarDump003, end");
}

/**
 * @tc.name: SoftBusRegConnVarDump004
 * @tc.desc: Verify SoftBusRegConnVarDump004 function, first param dumpVar is NULL,
 *     second cb is null return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegConnVarDump004, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegConnVarDump004, Start");
    int32_t ret = SoftBusRegConnVarDump(NULL, NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegConnVarDump004, end");
}

// -------------------------------------------------buscenter-----------------------------------------//
static int32_t LnnVarTest1Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "LnnVarTest1Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

static int32_t LnnVarTest2Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "LnnVarTest2Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

static int32_t LnnVarTest3Dumper(int fd)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "LnnVarTest3Dumper called, fd = %d", fd);
    return SOFTBUS_OK;
}

/**
 * @tc.name: SoftBusRegBusCenterVarDump001
 * @tc.desc: Verify SoftBusRegBusCenterVarDump function, use lnnVarArray param, return SOFTBUS_OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegBusCenterVarDump001, TestSize.Level1)
{
    HiDumperVarNode testLnnVarArray[HIDUMPER_VAR_CASE_NUM] = {
        {"lnn_var_test_1", &LnnVarTest1Dumper},
        {"lnn_var_test_2", &LnnVarTest2Dumper},
        {"lnn_var_test_3", &LnnVarTest3Dumper},
    };
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump001, Start");
    int32_t ret;
    for (int i = 0; i < HIDUMPER_VAR_CASE_NUM; i++) {
        ret = SoftBusRegBusCenterVarDump(testLnnVarArray[i].varName, testLnnVarArray[i].dumpCallback);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegDiscVarDump001, end");
}

/**
 * @tc.name: SoftBusRegBusCenterVarDump002
 * @tc.desc: Verify SoftBusRegBusCenterVarDump function, first param dumpVar is NULL, return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegBusCenterVarDump002, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegBusCenterVarDump002, Start");
    int32_t ret = SoftBusRegBusCenterVarDump(NULL, &DiscVarTest1Dumper);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegBusCenterVarDump002, end");
}

/**
 * @tc.name: SoftBusRegBusCenterVarDump003
 * @tc.desc: Verify SoftBusRegBusCenterVarDump function, second param cb is NULL, return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegBusCenterVarDump003, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegBusCenterVarDump003, Start");
    int32_t ret = SoftBusRegBusCenterVarDump(const_cast<char *>(g_testDumpVar), NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegBusCenterVarDump003, end");
}

/**
 * @tc.name: SoftBusRegBusCenterVarDump004
 * @tc.desc: Verify SoftBusRegBusCenterVarDump function, first param dumpVar is NULL,
 *     second cb is null return SOFTBUS_ERR.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegBusCenterVarDump004, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegBusCenterVarDump004, Start");
    int32_t ret = SoftBusRegBusCenterVarDump(NULL, NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegBusCenterVarDump004, end");
}

// --------------------------------------------trans------------------------------------------------- //
HiDumperArgvNode g_validTransCmdArray[TRANS_DUMP_PROCESS_TEST_NUM] = {
    {TEST_FD_ZERO, TEST_ARGC_ONE, {"-h"}},
    {TEST_FD_ZERO, TEST_ARGC_ONE, {"-l"}},
    {TEST_FD_ZERO, TEST_ARGC_TWO, {"-l", "registed_sessionlist"}},
    {TEST_FD_ZERO, TEST_ARGC_TWO, {"-l", "concurrent_sessionlist"}},
};

/**
 * @tc.name: SoftBusDumpProcess001
 * @tc.desc: Verify SoftBusDumpProcess function, valid param, use disc hidumper cmd, return SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusDumpProcess001, TestSize.Level1)
{
    HiDumperArgvNode testValidDiscCmdArray[DISC_DUMP_PROCESS_VALID_TEST_NUM] = {
        {TEST_FD_ZERO, TEST_ARGC_ONE, {"-h"}},
        {TEST_FD_ZERO, TEST_ARGC_ONE, {"disc"}},
        {TEST_FD_ZERO, TEST_ARGC_TWO, {"disc", "-h"}},
        {TEST_FD_ZERO, TEST_ARGC_TWO, {"disc", "-l"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"disc", "-l", "disc_var_test_1"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"disc", "-l", "disc_var_test_2"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"disc", "-l", "disc_var_test_3"}},
    };
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess001, Start");
    int32_t ret;
    for (int i = 0; i < DISC_DUMP_PROCESS_VALID_TEST_NUM; i++) {
        ret = SoftBusDumpProcess(testValidDiscCmdArray[i].fd, testValidDiscCmdArray[i].argc,
            testValidDiscCmdArray[i].argv);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess001, end");
}

/**
 * @tc.name: SoftBusDumpProcess002
 * @tc.desc: Verify SoftBusDumpProcess function, invalid param, use disc hidumper,return SOFTBUS_ERR
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusDumpProcess002, TestSize.Level1)
{
    HiDumperArgvNode testInvalidDiscCmdArray[DISC_DUMP_PROCESS_VALID_TEST_NUM] = {
        {ERR_FD, TEST_ARGC_ONE, {"-h"}},
        {TEST_FD_ZERO, ERR_ARGC, {"disc"}},
        {ERR_FD, ERR_ARGC, {"disc", "-h"}},
        {ERR_FD, TEST_ARGC_TWO, {"disc", "-l"}},
        {ERR_FD, TEST_ARGC_THREE, {"disc", "-l", "disc_var_test_1"}},
        {ERR_FD, TEST_ARGC_ONE, {"disc", "-l", "disc_var_test_2"}},
        {TEST_FD_ZERO, ERR_ARGC, {"disc", "-l", "disc_var_test_3"}},
    };
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess002, Start");
    int32_t ret;
    for (int i = 0; i < DISC_DUMP_PROCESS_VALID_TEST_NUM; i++) {
        ret = SoftBusDumpProcess(testInvalidDiscCmdArray[i].fd, testInvalidDiscCmdArray[i].argc,
            testInvalidDiscCmdArray[i].argv);
        EXPECT_EQ(SOFTBUS_ERR, ret);
    }
    ret = SoftBusDumpProcess(0, 1, NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess002, end");
}

/**
 * @tc.name: SoftBusDumpProcess003
 * @tc.desc: Verify SoftBusDumpProcess function, valid param, use buscenter hidumper cmd, return SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusDumpProcess003, TestSize.Level1)
{
    HiDumperArgvNode testValidConnCmdArray[CONN_DUMP_PROCESS_VALID_TEST_NUM] = {
        {TEST_FD_ZERO, TEST_ARGC_ONE, {"conn"}},
        {TEST_FD_ZERO, TEST_ARGC_TWO, {"conn", "-h"}},
        {TEST_FD_ZERO, TEST_ARGC_TWO, {"conn", "-l"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"conn", "-l", "conn_var_test_1"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"conn", "-l", "conn_var_test_2"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"conn", "-l", "conn_var_test_3"}},
    };
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess003, Start");
    int32_t ret;
    for (int i = 0; i < CONN_DUMP_PROCESS_VALID_TEST_NUM; i++) {
        ret = SoftBusDumpProcess(testValidConnCmdArray[i].fd, testValidConnCmdArray[i].argc,
            testValidConnCmdArray[i].argv);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess003, end");
}

/**
 * @tc.name: SoftBusDumpProcess004
 * @tc.desc: Verify SoftBusDumpProcess function, invalid param, use conn hidumper,return SOFTBUS_ERR
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusDumpProcess004, TestSize.Level1)
{
    HiDumperArgvNode testInValidConnCmdArray[CONN_DUMP_PROCESS_INVALID_TEST_NUM] = {
        {ERR_FD, TEST_ARGC_ONE, {"-h"}},
        {TEST_FD_ONE, ERR_ARGC, {"conn"}},
        {ERR_FD, ERR_ARGC, {"conn", "-h"}},
        {ERR_FD, TEST_ARGC_TWO, {"conn", "-l"}},
        {ERR_FD, TEST_ARGC_THREE, {"conn", "-l", "conn_var_test_1"}},
    };
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess004, Start");
    int32_t ret;
    for (int i = 0; i < CONN_DUMP_PROCESS_INVALID_TEST_NUM; i++) {
        ret = SoftBusDumpProcess(testInValidConnCmdArray[i].fd, testInValidConnCmdArray[i].argc,
            testInValidConnCmdArray[i].argv);
        EXPECT_EQ(SOFTBUS_ERR, ret);
    }
    ret = SoftBusDumpProcess(0, 1, NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess004, end");
}

/**
 * @tc.name: SoftBusDumpProcess005
 * @tc.desc: Verify SoftBusDumpProcess function, valid param, use buscenter hidumper cmd, return SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusDumpProcess005, TestSize.Level1)
{
    HiDumperArgvNode testValiBusCenterCmdArray[BUSCENTER_DUMP_PROCESS_VALID_TEST_NUM] = {
        {TEST_FD_ZERO, TEST_ARGC_ONE, {"buscenter"}},
        {TEST_FD_ZERO, TEST_ARGC_TWO, {"buscenter", "-h"}},
        {TEST_FD_ZERO, TEST_ARGC_TWO, {"buscenter", "-l"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"buscenter", "-l", "buscenter_var_test_1"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"buscenter", "-l", "buscenter_var_test_2"}},
        {TEST_FD_ZERO, TEST_ARGC_THREE, {"buscenter", "-l", "buscenter_var_test_3"}},
    };
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess005, Start");
    int32_t ret;
    for (int i = 0; i < BUSCENTER_DUMP_PROCESS_VALID_TEST_NUM; i++) {
        ret = SoftBusDumpProcess(testValiBusCenterCmdArray[i].fd, testValiBusCenterCmdArray[i].argc,
            testValiBusCenterCmdArray[i].argv);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess005, end");
}

/**
 * @tc.name: SoftBusDumpProcess006
 * @tc.desc: Verify SoftBusDumpProcess function, invalid param, use buscenter hidumper,return SOFTBUS_ERR
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusDumpProcess006, TestSize.Level1)
{
    HiDumperArgvNode testInValidBusCenterCmdArray[BUSCENTER_DUMP_PROCESS_INVALID_TEST_NUM] = {
        {ERR_FD, TEST_ARGC_ONE, {"-h"}},
        {TEST_FD_ZERO, ERR_ARGC, {"buscenter"}},
        {ERR_FD, ERR_ARGC, {"buscenter", "-h"}},
        {ERR_FD, TEST_ARGC_TWO, {"buscenter", "-l"}},
        {ERR_FD, TEST_ARGC_THREE, {"buscenter", "-l", "buscenter_var_test_1"}},
    };
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess006, Start");
    int32_t ret;
    for (int i = 0; i < BUSCENTER_DUMP_PROCESS_INVALID_TEST_NUM; i++) {
        ret = SoftBusDumpProcess(testInValidBusCenterCmdArray[i].fd, testInValidBusCenterCmdArray[i].argc,
            testInValidBusCenterCmdArray[i].argv);
        EXPECT_EQ(SOFTBUS_ERR, ret);
    }
    ret = SoftBusDumpProcess(0, 1, NULL);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess006, end");
}

/**
 * @tc.name: SoftBusDumpProcess007
 * @tc.desc: Verify SoftBusDumpProcess function, valid param, use nstack hidumper cmd, return SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusDumpProcess007, TestSize.Level1)
{
    HiDumperArgvNode testValidNstackCmdArray[NSTACK_DUMP_PROCESS_VALID_TEST_NUM] = {
        {TEST_FD_ZERO, TEST_ARGC_ONE, {"dstream"}},
        {TEST_FD_ZERO, TEST_ARGC_ONE, {"dmsg", }},
        {TEST_FD_ZERO, TEST_ARGC_ONE, {"dfile"}},
        {TEST_FD_ZERO, TEST_ARGC_ONE, {"dmsg"}},
    };
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess007, Start");
    int32_t ret;
    for (int i = 0; i < NSTACK_DUMP_PROCESS_VALID_TEST_NUM; i++) {
        ret = SoftBusDumpProcess(testValidNstackCmdArray[i].fd, testValidNstackCmdArray[i].argc,
            testValidNstackCmdArray[i].argv);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusDumpProcess007, end");
}

int32_t DumpHandlerTest1(int fd, int32_t argc, const char **argv)
{
    SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "DumpHandlerTest1:fd = %d, argc = %d", fd, argc);
    return SOFTBUS_OK;
}
/**
 * @tc.name: SoftBusRegHiDumperHandler001
 * @tc.desc: Verify SoftBusRegHiDumperHandler function, invalid param, return SOFTBUS_ERR.
 *     test the void function using invalid param
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusRegHiDumperHandler001, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegHiDumperHandler001, Start");
    int32_t ret = SoftBusRegHiDumperHandler(const_cast<char *>(g_testModuleName),
        const_cast<char *>(g_testHelpInfo), DumpHandlerTest1);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = SoftBusRegHiDumperHandler(nullptr, const_cast<char *>(g_testHelpInfo), DumpHandlerTest1);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    ret = SoftBusRegHiDumperHandler(const_cast<char *>(g_testModuleName), const_cast<char *>(g_testHelpInfo), nullptr);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusDumpShowHelp(ERR_FD);
    SoftBusDumpErrInfo(ERR_FD, g_testDumpVar);
    SoftBusDumpSubModuleHelp(ERR_FD, const_cast<char *>(g_testModuleName), nullptr);
    SoftBusAddDumpVarToList(nullptr, nullptr, nullptr);
    SoftBusReleaseDumpVar(nullptr);
    const char *testHandler = "testHandler";
    const char **tmpTestHandle = &testHandler;

    ret = SoftBusDumpDispatch(ERR_FD, TEST_ARGC_ONE, tmpTestHandle);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    ret = SoftBusDumpDispatch(TEST_FD_ZERO, ERR_ARGC, tmpTestHandle);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    ret = SoftBusDumpDispatch(TEST_FD_ZERO, TEST_ARGC_ONE, nullptr);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusRegHiDumperHandler001, end");
}

/**
 * @tc.name: SoftBusHiDumperDeinitTest001
 * @tc.desc: Verify SoftBusHiDumperDeinit function, no parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(HidumperTest, SoftBusHiDumperDeinitTest001, TestSize.Level1)
{
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusHiDumperDeinitTest001, Start");
    SoftBusHiDumperDeinit();
    SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_INFO, "HidumperTest, SoftBusHiDumperDeinitTest001, end");
}
} // namespace OHOS