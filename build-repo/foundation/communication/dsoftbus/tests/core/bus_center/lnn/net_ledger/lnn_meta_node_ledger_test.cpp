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
#include <securec.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "lnn_meta_node_ledger.h"
#include "lnn_network_id.h"
#include "softbus_adapter_mem.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_utils.h"

typedef struct {
    ListNode node;
    MetaNodeInfo info;
} MetaNodeStorageInfo;

namespace OHOS {
using namespace testing::ext;
constexpr char NODE_DEVICE_NAME[] = "node1_test";
constexpr char NODE_UDID[] = "123456ABCDEF";
constexpr char META_NODE_ID[] = "235689BNHFCC";
constexpr uint32_t ADDR_NUM = 6;
constexpr int32_t INFO_NUM = 0;
constexpr int32_t INVALID_INFO_NUM = -1;
using namespace testing;
class MetaNodeLedgerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MetaNodeLedgerTest::SetUpTestCase()
{
}

void MetaNodeLedgerTest::TearDownTestCase()
{
}

void MetaNodeLedgerTest::SetUp()
{
    LOG_INFO("MetaNodeLedgerTest start.");
    LnnInitMetaNodeLedger();
}

void MetaNodeLedgerTest::TearDown()
{
    LOG_INFO("MetaNodeLedgerTest finish.");
    LnnDeinitMetaNodeLedger();
}

/*
* @tc.name: LNN_ACTIVE_META_NODE_Test_001
* @tc.desc: lnn active meta node test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(MetaNodeLedgerTest, LNN_ACTIVE_META_NODE_Test_001, TestSize.Level1)
{
    MetaNodeConfigInfo info;
    (void)memset_s(&info, sizeof(MetaNodeConfigInfo), 0, sizeof(MetaNodeConfigInfo));
    info.addrNum = CONNECTION_ADDR_WLAN;
    (void)strcpy_s(info.udid, UUID_BUF_LEN, NODE_UDID);
    (void)strcpy_s(info.deviceName, DEVICE_NAME_BUF_LEN, NODE_DEVICE_NAME);
    char metaNodeId[NETWORK_ID_BUF_LEN] = {0};
    int32_t ret = LnnActiveMetaNode(&info, metaNodeId);
    EXPECT_TRUE(ret == SOFTBUS_OK);
    info.addrNum = ADDR_NUM;
    ret = LnnActiveMetaNode(&info, metaNodeId);
    EXPECT_TRUE(ret == SOFTBUS_INVALID_PARAM);
    ret = LnnActiveMetaNode(nullptr, metaNodeId);
    EXPECT_TRUE(ret == SOFTBUS_INVALID_PARAM);
}

/*
* @tc.name: LNN_GET_ALL_META_NODE_INFO_Test_001
* @tc.desc: lnn get all meta node info test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(MetaNodeLedgerTest, LNN_GET_ALL_META_NODE_INFO_Test_001, TestSize.Level1)
{
    MetaNodeInfo infos[MAX_META_NODE_NUM];
    int32_t infoNum1 = INFO_NUM;
    int32_t infoNum2 = INVALID_INFO_NUM;
    int32_t ret = LnnGetAllMetaNodeInfo(infos, &infoNum1);
    EXPECT_TRUE(ret == SOFTBUS_OK);
    ret = LnnGetAllMetaNodeInfo(infos, &infoNum2);
    EXPECT_TRUE(ret == SOFTBUS_INVALID_PARAM);
    EXPECT_TRUE(LnnGetAllMetaNodeInfo(nullptr, nullptr) == SOFTBUS_INVALID_PARAM);
}

/*
* @tc.name: LNN_DEACTIVE_META_NODE_Test_001
* @tc.desc: lnn deactive meta node test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(MetaNodeLedgerTest, LNN_DEACTIVE_META_NODE_Test_001, TestSize.Level1)
{
    int32_t ret = LnnDeactiveMetaNode(META_NODE_ID);
    EXPECT_TRUE(ret == SOFTBUS_ERR);
    EXPECT_TRUE(LnnDeactiveMetaNode(nullptr) == SOFTBUS_INVALID_PARAM);
    LnnDeinitMetaNodeLedger();
}
} // namespace OHOS
