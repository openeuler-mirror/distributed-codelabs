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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/mount.h>
#include <system_error>
#include <unistd.h>

#include "device/device_info.h"
#include "device/device_manager_agent.h"
#include "mountpoint/mount_point.h"
#include "network/kernel_talker.h"
#include "network/session_pool.h"
#include "network/softbus/softbus_session.h"
#include "securec.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

static const string SRC_HEAD = "/data/service/el2/";
static const string DST_HEAD = "/mnt/hmdfs/";
static const string CACHE_HEAD = "/data/service/el2/";
static const string SAME_ACCOUNT = "account";
std::mutex cmdMutex_;

const int KEY_MAX_LEN = 32;
const int CID_MAX_LEN = 64;
struct UpdateSocketParam {
    int32_t cmd;
    int32_t newfd;
    uint8_t status;
    uint8_t protocol;
    uint16_t udpPort;
    uint8_t deviceType;
    uint8_t masterKey[KEY_MAX_LEN];
    char cid[CID_MAX_LEN];
    int32_t linkType;
    int32_t binderFd;
} __attribute__((packed));

class DistributedFileDaemonServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedFileDaemonServiceTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void DistributedFileDaemonServiceTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DistributedFileDaemonServiceTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void DistributedFileDaemonServiceTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}


/**
 * @tc.name: mount_test_001
 * @tc.desc: Verify the mount/umount function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest, mount_umount_test_001, TestSize.Level1)
{
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(
        Utils::DfsuMountArgumentDescriptors::Alpha(100, SAME_ACCOUNT));

    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        smp->Mount();
        smp->Umount();
        LOGE("testcase run OK");
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
