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

#include "accesstoken_kit.h"
#include "communication_provider.h"
#include "gtest/gtest.h"
#include "kvstore_meta_manager.h"
#include "metadata/meta_data_manager.h"
#include "metadata/store_meta_data.h"
#include "session_manager/route_head_handler_impl.h"
#include "session_manager/upgrade_manager.h"
#include "user_delegate.h"

namespace {
using namespace testing::ext;
using namespace OHOS::DistributedKv;
using namespace OHOS::DistributedData;
using namespace OHOS;
using namespace OHOS::Security::AccessToken;
constexpr const char *PEER_DEVICE_ID = "PEER_DEVICE_ID";
constexpr int PEER_USER_ID = 101;
class SessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        KvStoreMetaManager::GetInstance().InitMetaParameter();
        KvStoreMetaManager::GetInstance().InitMetaListener();

        // init peer device
        UserMetaData userMetaData;
        userMetaData.deviceId = PEER_DEVICE_ID;

        UserStatus status;
        status.isActive = true;
        status.id = PEER_USER_ID;
        userMetaData.users = { status };

        auto peerUserMetaKey = UserMetaRow::GetKeyFor(userMetaData.deviceId);
        MetaDataManager::GetInstance().SaveMeta({ peerUserMetaKey.begin(), peerUserMetaKey.end() }, userMetaData);

        CapMetaData capMetaData;
        capMetaData.version = CapMetaData::CURRENT_VERSION;

        auto peerCapMetaKey = CapMetaRow::GetKeyFor(userMetaData.deviceId);
        MetaDataManager::GetInstance().SaveMeta({ peerCapMetaKey.begin(), peerCapMetaKey.end() }, capMetaData);

        StoreMetaData metaData;
        metaData.bundleName = "ohos.test.demo";
        metaData.storeId = "test_store";
        metaData.user = "100";
        metaData.deviceId = OHOS::AppDistributedKv::CommunicationProvider::GetInstance().GetLocalDevice().uuid;
        metaData.tokenId = AccessTokenKit::GetHapTokenID(100, "ohos.test.demo", 0);
        metaData.uid = 2000000;
        metaData.storeType = 1;
        MetaDataManager::GetInstance().SaveMeta(metaData.GetKey(), metaData);
    }
    static void TearDownTestCase()
    {
        auto peerUserMetaKey = UserMetaRow::GetKeyFor(PEER_DEVICE_ID);
        MetaDataManager::GetInstance().DelMeta(std::string(peerUserMetaKey.begin(), peerUserMetaKey.end()));
        auto peerCapMetaKey = CapMetaRow::GetKeyFor(PEER_DEVICE_ID);
        MetaDataManager::GetInstance().DelMeta(std::string(peerCapMetaKey.begin(), peerCapMetaKey.end()));
        StoreMetaData metaData;
        metaData.bundleName = "ohos.test.demo";
        metaData.storeId = "test_store";
        metaData.user = "100";
        metaData.deviceId = OHOS::AppDistributedKv::CommunicationProvider::GetInstance().GetLocalDevice().uuid;
        metaData.tokenId = AccessTokenKit::GetHapTokenID(100, "ohos.test.demo", 0);
        metaData.uid = 2000000;
        metaData.storeType = 1;
        MetaDataManager::GetInstance().DelMeta(metaData.GetKey());
    }
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

/**
* @tc.name: PackAndUnPack01
* @tc.desc: test get db dir
* @tc.type: FUNC
* @tc.require:
* @tc.author: illybyy
*/
HWTEST_F(SessionManagerTest, PackAndUnPack01, TestSize.Level2)
{
    const DistributedDB::ExtendInfo info = {
        .appId = "ohos.test.demo", .storeId = "test_store", .userId = "100", .dstTarget = PEER_DEVICE_ID
    };
    auto sendHandler = RouteHeadHandlerImpl::Create(info);
    ASSERT_NE(sendHandler, nullptr);
    uint32_t routeHeadSize = 0;
    sendHandler->GetHeadDataSize(routeHeadSize);
    ASSERT_GT(routeHeadSize, 0);
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(routeHeadSize);
    sendHandler->FillHeadData(data.get(), routeHeadSize, routeHeadSize);

    std::vector<std::string> users;
    auto recvHandler = RouteHeadHandlerImpl::Create({});
    ASSERT_NE(recvHandler, nullptr);
    uint32_t parseSize = 0;
    recvHandler->ParseHeadData(data.get(), routeHeadSize, parseSize, users);
    EXPECT_EQ(routeHeadSize, parseSize);
    ASSERT_EQ(users.size(), 1);
    EXPECT_EQ(users[0], "101");
}
} // namespace