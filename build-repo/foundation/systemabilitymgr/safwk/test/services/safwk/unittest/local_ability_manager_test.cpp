/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "gtest/gtest.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "test_log.h"

#define private public
#include "local_ability_manager.h"
#include "mock_sa_realize.h"
#undef private
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace SAFWK {
namespace {
    const std::string TEST_RESOURCE_PATH = "/data/test/resource/safwk/profile/";
    const std::u16string LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN = u"ohos.localabilitymanager.accessToken";
    constexpr int VAILD_SAID = 401;
    constexpr int SAID = 1499;
    constexpr int MUT_SAID = 9999;
    constexpr int INVALID_SAID = -1;
}

class LocalAbilityManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void LocalAbilityManagerTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void LocalAbilityManagerTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void LocalAbilityManagerTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void LocalAbilityManagerTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/**
 * @tc.name: CheckTrustSa001
 * @tc.desc:  CheckTrustSa with not all allow
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, CheckTrustSa001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles
        (TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    /**
     * @tc.steps: step2. CheckTrustSa with not all allow
     * @tc.expected: step2. load allowed sa
     */
    auto profiles = LocalAbilityManager::GetInstance().profileParser_->GetAllSaProfiles();
    auto path = TEST_RESOURCE_PATH + "test_trust_not_all_allow.xml";
    auto process = "test";
    LocalAbilityManager::GetInstance().CheckTrustSa(path, process, profiles);
    profiles = LocalAbilityManager::GetInstance().profileParser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 2);
}

/**
 * @tc.name: CheckTrustSa002
 * @tc.desc:  CheckTrustSa with all allow
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, CheckTrustSa002, TestSize.Level1)
{
    LocalAbilityManager::GetInstance().profileParser_->ClearResource();
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles
        (TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    /**
     * @tc.steps: step2. CheckTrustSa with all allow
     * @tc.expected: step2. load all sa
     */
    auto profiles = LocalAbilityManager::GetInstance().profileParser_->GetAllSaProfiles();
    auto path = TEST_RESOURCE_PATH + "test_trust_all_allow.xml";
    auto process = "test";
    LocalAbilityManager::GetInstance().CheckTrustSa(path, process, profiles);
    auto result = LocalAbilityManager::GetInstance().profileParser_->GetAllSaProfiles();
    EXPECT_EQ(result.size(), 4);
}

/**
 * @tc.name: DoStartSAProcess001
 * @tc.desc:  DoStartSAProcess001
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, DoStartSAProcess001, TestSize.Level2)
{
    LocalAbilityManager::GetInstance().DoStartSAProcess("profile_audio.xml", SAID);
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    auto ability = sm->GetSystemAbility(SAID);
    EXPECT_TRUE(ability == nullptr);
}

/**
 * @tc.name: DoStartSAProcess002
 * @tc.desc:  DoStartSAProcess002
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, DoStartSAProcess002, TestSize.Level2)
{
    string profilePath = "/system/usr/profile_audio.xml";
    LocalAbilityManager::GetInstance().DoStartSAProcess(profilePath, SAID);
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto ability = sm->GetSystemAbility(SAID);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: DoStartSAProcess003
 * @tc.desc:  DoStartSAProcess, InitializeSaProfiles failed!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, DoStartSAProcess003, TestSize.Level2)
{
    string profilePath = "/system/usr/profile_audio.xml";
    int32_t invalidSaid = -2;
    LocalAbilityManager::GetInstance().DoStartSAProcess(profilePath, invalidSaid);
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto ability = sm->GetSystemAbility(invalidSaid);
    EXPECT_EQ(ability, nullptr);
}

/**
 * @tc.name: GetTraceTag001
 * @tc.desc:  GetTraceTag
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, GetTraceTag001, TestSize.Level1)
{
    string profilePath = "/system/usr/profile_audio.xml";
    string traceTag = LocalAbilityManager::GetInstance().GetTraceTag(profilePath);
    EXPECT_EQ(traceTag, "profile_audio");
}

/**
 * @tc.name: GetTraceTag002
 * @tc.desc:  GetTraceTag
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, GetTraceTag002, TestSize.Level1)
{
    string profilePath = "";
    string traceTag = LocalAbilityManager::GetInstance().GetTraceTag(profilePath);
    EXPECT_EQ(traceTag, "default_proc");
}

/**
 * @tc.name: GetTraceTag003
 * @tc.desc:  GetTraceTag
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, GetTraceTag003, TestSize.Level1)
{
    string profilePath = "/system/usr/test";
    string traceTag = LocalAbilityManager::GetInstance().GetTraceTag(profilePath);
    EXPECT_EQ(traceTag, "test");
}

/**
 * @tc.name: CheckAndGetProfilePath001
 * @tc.desc:  CheckAndGetProfilePath
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, CheckAndGetProfilePath001, TestSize.Level3)
{
    string profilePath = "/system/usr/profile_audio.xml";
    string realProfilePath = "";
    bool res = LocalAbilityManager::GetInstance().CheckAndGetProfilePath(profilePath, realProfilePath);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: CheckAndGetProfilePath002
 * @tc.desc:  CheckAndGetProfilePath, xmlDoc dir is not matched!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, CheckAndGetProfilePath002, TestSize.Level1)
{
    string profilePath = TEST_RESOURCE_PATH + "test_trust_all_allow.xml";
    string realProfilePath = "";
    bool res = LocalAbilityManager::GetInstance().CheckAndGetProfilePath(profilePath, realProfilePath);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: CheckSystemAbilityManagerReady001
 * @tc.desc:  CheckSystemAbilityManagerReady, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, CheckSystemAbilityManagerReady001, TestSize.Level3)
{
    string profilePath = TEST_RESOURCE_PATH + "test_trust_all_allow.xml";
    string realProfilePath = "";
    bool res = LocalAbilityManager::GetInstance().CheckSystemAbilityManagerReady();
    EXPECT_TRUE(res);
}

/**
 * @tc.name: InitSystemAbilityProfiles001
 * @tc.desc:  InitSystemAbilityProfiles, ParseSaProfiles failed!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitSystemAbilityProfiles001, TestSize.Level1)
{
    string profilePath = "";
    bool res = LocalAbilityManager::GetInstance().InitSystemAbilityProfiles(profilePath, SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: InitSystemAbilityProfiles002
 * @tc.desc:  InitSystemAbilityProfiles, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitSystemAbilityProfiles002, TestSize.Level3)
{
    string profilePath = "/system/usr/profile_audio.xml";
    bool res = LocalAbilityManager::GetInstance().InitSystemAbilityProfiles(profilePath, SAID);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: InitSystemAbilityProfiles003
 * @tc.desc:  InitSystemAbilityProfiles, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitSystemAbilityProfiles003, TestSize.Level3)
{
    string profilePath = "/system/usr/profile_audio.xml";
    int32_t defaultId = -1;
    bool res = LocalAbilityManager::GetInstance().InitSystemAbilityProfiles(profilePath, defaultId);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: AddAbility001
 * @tc.desc: AddAbility, try to add null ability!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, AddAbility001, TestSize.Level3)
{
    string path = "";
    string process = "process";
    std::list<SaProfile> saInfos;
    LocalAbilityManager::GetInstance().CheckTrustSa(path, process, saInfos);
    bool res = LocalAbilityManager::GetInstance().AddAbility(nullptr);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: AddAbility002
 * @tc.desc: AddAbility, try to add existed ability
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, AddAbility002, TestSize.Level3)
{
    MockSaRealize *sysAby = new MockSaRealize(MUT_SAID, false);
    bool ret = LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles
        (TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    LocalAbilityManager::GetInstance().abilityMap_[MUT_SAID] = sysAby;
    bool res = LocalAbilityManager::GetInstance().AddAbility(sysAby);
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    delete sysAby;
    EXPECT_FALSE(res);
}

/**
 * @tc.name: RemoveAbility001
 * @tc.desc: RemoveAbility, invalid systemAbilityId
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, RemoveAbility001, TestSize.Level1)
{
    bool res = LocalAbilityManager::GetInstance().RemoveAbility(INVALID_SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: RemoveAbility002
 * @tc.desc: RemoveAbility, invalid systemAbilityId
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, RemoveAbility002, TestSize.Level3)
{
    bool res = LocalAbilityManager::GetInstance().RemoveAbility(SAID);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: AddSystemAbilityListener001
 * @tc.desc: AddSystemAbilityListener, SA or listenerSA invalid
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, AddSystemAbilityListener001, TestSize.Level1)
{
    bool res = LocalAbilityManager::GetInstance().AddSystemAbilityListener(SAID, INVALID_SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: AddSystemAbilityListener002
 * @tc.desc: AddSystemAbilityListener, SA or listenerSA invalid
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, AddSystemAbilityListener002, TestSize.Level1)
{
    bool res = LocalAbilityManager::GetInstance().AddSystemAbilityListener(INVALID_SAID, SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: AddSystemAbilityListener003
 * @tc.desc: AddSystemAbilityListener, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, AddSystemAbilityListener003, TestSize.Level1)
{
    LocalAbilityManager::GetInstance().listenerMap_[SAID].push_back(MUT_SAID);
    bool res = LocalAbilityManager::GetInstance().AddSystemAbilityListener(SAID, SAID);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: AddSystemAbilityListener004
 * @tc.desc: AddSystemAbilityListener, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, AddSystemAbilityListener004, TestSize.Level1)
{
    LocalAbilityManager::GetInstance().listenerMap_[VAILD_SAID].push_back(VAILD_SAID);
    LocalAbilityManager::GetInstance().listenerMap_[VAILD_SAID].push_back(SAID);
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    bool res = LocalAbilityManager::GetInstance().AddSystemAbilityListener(VAILD_SAID, VAILD_SAID);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: RemoveSystemAbilityListener001
 * @tc.desc: RemoveSystemAbilityListener, SA or listenerSA invalid
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, RemoveSystemAbilityListener001, TestSize.Level1)
{
    bool res = LocalAbilityManager::GetInstance().RemoveSystemAbilityListener(INVALID_SAID, SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: RemoveSystemAbilityListener002
 * @tc.desc: RemoveSystemAbilityListener, SA or listenerSA invalid
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, RemoveSystemAbilityListener002, TestSize.Level1)
{
    bool res = LocalAbilityManager::GetInstance().RemoveSystemAbilityListener(SAID, INVALID_SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: RemoveSystemAbilityListener003
 * @tc.desc: RemoveSystemAbilityListener, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, RemoveSystemAbilityListener003, TestSize.Level1)
{
    LocalAbilityManager::GetInstance().listenerMap_.clear();
    bool res = LocalAbilityManager::GetInstance().RemoveSystemAbilityListener(SAID, SAID);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: RemoveSystemAbilityListener004
 * @tc.desc: RemoveSystemAbilityListener, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, RemoveSystemAbilityListener004, TestSize.Level3)
{
    LocalAbilityManager::GetInstance().listenerMap_[SAID].push_back(MUT_SAID);
    bool res = LocalAbilityManager::GetInstance().RemoveSystemAbilityListener(SAID, SAID);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: OnStartAbility001
 * @tc.desc: OnStartAbility, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, OnStartAbility001, TestSize.Level1)
{
    std::string deviceId = "";
    int32_t addAbility = 1;
    MockSaRealize *sysAby = new MockSaRealize(MUT_SAID, false);
    LocalAbilityManager::GetInstance().abilityMap_[MUT_SAID] = sysAby;
    LocalAbilityManager::GetInstance().NotifyAbilityListener(SAID, MUT_SAID, deviceId, addAbility);
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = sysAby;
    LocalAbilityManager::GetInstance().NotifyAbilityListener(SAID, MUT_SAID, deviceId, addAbility);
    bool res = LocalAbilityManager::GetInstance().OnStartAbility(SAID);
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    delete sysAby;
    EXPECT_TRUE(res);
}

/**
 * @tc.name: OnStartAbility002
 * @tc.desc: OnStartAbility, return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, OnStartAbility002, TestSize.Level1)
{
    std::string deviceId = "";
    int32_t removeAbility = 2;
    int32_t otherAbility = 3;
    MockSaRealize *sysAby = new MockSaRealize(MUT_SAID, false);
    LocalAbilityManager::GetInstance().abilityMap_[MUT_SAID] = sysAby;
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = sysAby;
    LocalAbilityManager::GetInstance().NotifyAbilityListener(SAID, MUT_SAID, deviceId, removeAbility);
    LocalAbilityManager::GetInstance().NotifyAbilityListener(SAID, MUT_SAID, deviceId, otherAbility);
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    bool res = LocalAbilityManager::GetInstance().OnStartAbility(SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: GetAbility001
 * @tc.desc: GetAbility, SA not register
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, GetAbility001, TestSize.Level1)
{
    std::string deviceId = "";
    int32_t removeAbility = 2;
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    SystemAbility* res = LocalAbilityManager::GetInstance().GetAbility(SAID);
    LocalAbilityManager::GetInstance().FindAndNotifyAbilityListeners(SAID, deviceId, removeAbility);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: GetAbility002
 * @tc.desc: GetAbility, SA not register
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, GetAbility002, TestSize.Level1)
{
    std::string deviceId = "";
    int32_t removeAbility = 2;
    MockSaRealize *sysAby = new MockSaRealize(SAID, false);
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = sysAby;
    SystemAbility* res = LocalAbilityManager::GetInstance().GetAbility(SAID);
    LocalAbilityManager::GetInstance().listenerMap_[SAID].push_back(MUT_SAID);
    LocalAbilityManager::GetInstance().FindAndNotifyAbilityListeners(SAID, deviceId, removeAbility);
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    delete sysAby;
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: GetRunningStatus001
 * @tc.desc: GetRunningStatus, return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, GetRunningStatus001, TestSize.Level1)
{
    std::string deviceId = "";
    int32_t removeAbility = 2;
    bool res = LocalAbilityManager::GetInstance().GetRunningStatus(SAID);
    LocalAbilityManager::GetInstance().listenerMap_[SAID].clear();
    LocalAbilityManager::GetInstance().FindAndNotifyAbilityListeners(SAID, deviceId, removeAbility);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: GetRunningStatus002
 * @tc.desc: GetRunningStatus, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, GetRunningStatus002, TestSize.Level1)
{
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = mockSa;
    bool res = LocalAbilityManager::GetInstance().GetRunningStatus(SAID);
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    delete mockSa;
    EXPECT_FALSE(res);
}

/**
 * @tc.name: StartOndemandSystemAbility001
 * @tc.desc: StartOndemandSystemAbility
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, StartOndemandSystemAbility001, TestSize.Level1)
{
    std::string profilePath = "/system/usr/profile_audio.xml";
    LocalAbilityManager::GetInstance().profileParser_->saProfiles_.clear();
    LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles(profilePath);
    LocalAbilityManager::GetInstance().StartOndemandSystemAbility(SAID);
    EXPECT_EQ(LocalAbilityManager::GetInstance().profileParser_->saProfiles_.size(), 3);
}

/**
 * @tc.name: StartOndemandSystemAbility002
 * @tc.desc: StartOndemandSystemAbility
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, StartOndemandSystemAbility002, TestSize.Level1)
{
    std::string profilePath = "/system/usr/profile_audio.xml";
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    LocalAbilityManager::GetInstance().profileParser_->saProfiles_.clear();
    LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles(profilePath);
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = mockSa;
    LocalAbilityManager::GetInstance().StartOndemandSystemAbility(SAID);
    delete mockSa;
    EXPECT_EQ(LocalAbilityManager::GetInstance().profileParser_->saProfiles_.size(), 3);
}

/**
 * @tc.name: InitializeSaProfiles001
 * @tc.desc: InitializeSaProfiles, sa profile is empty
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeSaProfiles001, TestSize.Level1)
{
    LocalAbilityManager::GetInstance().profileParser_->saProfiles_.clear();
    bool res = LocalAbilityManager::GetInstance().InitializeSaProfiles(INVALID_SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: InitializeSaProfiles002
 * @tc.desc: InitializeSaProfiles
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeSaProfiles002, TestSize.Level1)
{
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    bool res = LocalAbilityManager::GetInstance().InitializeSaProfiles(SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: InitializeRunOnCreateSaProfiles001
 * @tc.desc: InitializeSaProfiles
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeRunOnCreateSaProfiles001, TestSize.Level1)
{
    LocalAbilityManager::GetInstance().profileParser_->saProfiles_.clear();
    bool res = LocalAbilityManager::GetInstance().InitializeRunOnCreateSaProfiles();
    EXPECT_FALSE(res);
}

/**
 * @tc.name: InitializeRunOnCreateSaProfiles002
 * @tc.desc: InitializeSaProfiles
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeRunOnCreateSaProfiles002, TestSize.Level1)
{
    std::string profilePath = "/system/usr/profile_audio.xml";
    LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles(profilePath);
    bool res = LocalAbilityManager::GetInstance().InitializeRunOnCreateSaProfiles();
    EXPECT_TRUE(res);
}

/**
 * @tc.name: InitializeRunOnCreateSaProfiles003
 * @tc.desc: InitializeSaProfiles
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeRunOnCreateSaProfiles003, TestSize.Level1)
{
    std::string profilePath = "/system/usr/profile_audio.xml";
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = mockSa;
    LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles(profilePath);
    bool res = LocalAbilityManager::GetInstance().InitializeRunOnCreateSaProfiles();
    delete mockSa;
    EXPECT_TRUE(res);
}

/**
 * @tc.name: InitializeOnDemandSaProfile001
 * @tc.desc: InitializeOnDemandSaProfile
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeOnDemandSaProfile001, TestSize.Level1)
{
    std::string profilePath = "/system/usr/profile_audio.xml";
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles(profilePath);
    bool res = LocalAbilityManager::GetInstance().InitializeOnDemandSaProfile(SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: InitializeOnDemandSaProfile002
 * @tc.desc: InitializeOnDemandSaProfile
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeOnDemandSaProfile002, TestSize.Level1)
{
    LocalAbilityManager::GetInstance().profileParser_->saProfiles_.clear();
    bool res = LocalAbilityManager::GetInstance().InitializeOnDemandSaProfile(SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: InitializeSaProfilesInnerLocked001
 * @tc.desc: InitializeSaProfilesInnerLocked, SA not found!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeSaProfilesInnerLocked001, TestSize.Level1)
{
    SaProfile saProfile;
    saProfile.saId = SAID;
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    bool res = LocalAbilityManager::GetInstance().InitializeSaProfilesInnerLocked(saProfile);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: InitializeSaProfilesInnerLocked002
 * @tc.desc: InitializeSaProfilesInnerLocked, SA is null!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeSaProfilesInnerLocked002, TestSize.Level1)
{
    SaProfile saProfile;
    saProfile.saId = SAID;
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = nullptr;
    bool res = LocalAbilityManager::GetInstance().InitializeSaProfilesInnerLocked(saProfile);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: InitializeSaProfilesInnerLocked003
 * @tc.desc: InitializeSaProfilesInnerLocked, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeSaProfilesInnerLocked003, TestSize.Level1)
{
    SaProfile saProfile;
    saProfile.saId = SAID;
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = mockSa;
    bool res = LocalAbilityManager::GetInstance().InitializeSaProfilesInnerLocked(saProfile);
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    delete mockSa;
    EXPECT_TRUE(res);
}

/**
 * @tc.name: InitializeSaProfilesInnerLocked004
 * @tc.desc: InitializeSaProfilesInnerLocked, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeSaProfilesInnerLocked004, TestSize.Level1)
{
    SaProfile saProfile;
    saProfile.saId = SAID;
    const std::u16string bootStart = u"BootStartPhase";
    saProfile.bootPhase = bootStart;
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = mockSa;
    bool res = LocalAbilityManager::GetInstance().InitializeSaProfilesInnerLocked(saProfile);
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    delete mockSa;
    EXPECT_TRUE(res);
}

/**
 * @tc.name: InitializeSaProfilesInnerLocked005
 * @tc.desc: InitializeSaProfilesInnerLocked, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, InitializeSaProfilesInnerLocked005, TestSize.Level1)
{
    SaProfile saProfile;
    saProfile.saId = SAID;
    const std::u16string coreStart = u"CoreStartPhase";
    saProfile.bootPhase = coreStart;
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    LocalAbilityManager::GetInstance().abilityMap_[SAID] = mockSa;
    bool res = LocalAbilityManager::GetInstance().InitializeSaProfilesInnerLocked(saProfile);
    LocalAbilityManager::GetInstance().abilityMap_.clear();
    delete mockSa;
    EXPECT_TRUE(res);
}

/**
 * @tc.name: CheckDependencyStatus001
 * @tc.desc: CheckDependencyStatus, return size is 1!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, CheckDependencyStatus001, TestSize.Level1)
{
    std::string profilePath = "/system/usr/profile_audio.xml";
    vector<std::u16string> dependSa;
    dependSa.push_back(u"1499");
    bool ret = LocalAbilityManager::GetInstance().profileParser_->ParseSaProfiles(profilePath);
    EXPECT_TRUE(ret);
    LocalAbilityManager::GetInstance().RegisterOnDemandSystemAbility(SAID);
    vector<std::u16string> res = LocalAbilityManager::GetInstance().CheckDependencyStatus(dependSa);
    EXPECT_EQ(res.size(), 1);
}

/**
 * @tc.name: CheckDependencyStatus002
 * @tc.desc: CheckDependencyStatus, return size is 0!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, CheckDependencyStatus002, TestSize.Level1)
{
    vector<std::u16string> dependSa;
    dependSa.push_back(u"-1");
    vector<std::u16string> res = LocalAbilityManager::GetInstance().CheckDependencyStatus(dependSa);
    EXPECT_EQ(res.size(), 0);
}

/**
 * @tc.name: CheckDependencyStatus003
 * @tc.desc: CheckDependencyStatus, return size is 0!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, CheckDependencyStatus003, TestSize.Level1)
{
    vector<std::u16string> dependSa;
    dependSa.push_back(u"401");
    vector<std::u16string> res = LocalAbilityManager::GetInstance().CheckDependencyStatus(dependSa);
    EXPECT_EQ(res.size(), 0);
}

/**
 * @tc.name: NeedRegisterOnDemand001
 * @tc.desc: NeedRegisterOnDemand, return false!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, NeedRegisterOnDemand001, TestSize.Level1)
{
    SaProfile saProfile;
    saProfile.runOnCreate = true;
    LocalAbilityManager::GetInstance().StartSystemAbilityTask(nullptr);
    bool res = LocalAbilityManager::GetInstance().NeedRegisterOnDemand(saProfile, INVALID_SAID);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: NeedRegisterOnDemand002
 * @tc.desc: NeedRegisterOnDemand, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, NeedRegisterOnDemand002, TestSize.Level1)
{
    SaProfile saProfile;
    saProfile.runOnCreate = false;
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    LocalAbilityManager::GetInstance().StartSystemAbilityTask(mockSa);
    bool res = LocalAbilityManager::GetInstance().NeedRegisterOnDemand(saProfile, INVALID_SAID);
    delete mockSa;
    EXPECT_TRUE(res);
}

/**
 * @tc.name: NeedRegisterOnDemand003
 * @tc.desc: NeedRegisterOnDemand, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, NeedRegisterOnDemand003, TestSize.Level3)
{
    SaProfile saProfile;
    saProfile.saId = INVALID_SAID;
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    vector<std::u16string> dependSa;
    dependSa.push_back(u"1499");
    mockSa->SetDependSa(dependSa);
    mockSa->SetDependTimeout(200);
    LocalAbilityManager::GetInstance().StartSystemAbilityTask(mockSa);
    LocalAbilityManager::GetInstance().RegisterOnDemandSystemAbility(SAID);
    bool res = LocalAbilityManager::GetInstance().NeedRegisterOnDemand(saProfile, SAID);
    delete mockSa;
    EXPECT_TRUE(res);
}

/**
 * @tc.name: Run001
 * @tc.desc: Run, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, Run001, TestSize.Level3)
{
    std::list<SystemAbility*> systemAbilityList;
    MockSaRealize *mockSa = new MockSaRealize(SAID, false);
    vector<std::u16string> dependSa;
    dependSa.push_back(u"-1");
    mockSa->SetDependSa(dependSa);
    mockSa->SetDependTimeout(200);
    LocalAbilityManager::GetInstance().startTaskNum_ = 1;
    LocalAbilityManager::GetInstance().StartSystemAbilityTask(mockSa);
    LocalAbilityManager::GetInstance().StartPhaseTasks(systemAbilityList);
    bool res = LocalAbilityManager::GetInstance().Run(SAID);
    delete mockSa;
    EXPECT_FALSE(res);
}

/**
 * @tc.name: AddLocalAbilityManager001
 * @tc.desc: AddLocalAbilityManager, return false!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, AddLocalAbilityManager001, TestSize.Level3)
{
    std::list<SystemAbility*> systemAbilityList;
    systemAbilityList.push_back(nullptr);
    LocalAbilityManager::GetInstance().StartPhaseTasks(systemAbilityList);
    LocalAbilityManager::GetInstance().procName_ = u"";
    bool res = LocalAbilityManager::GetInstance().AddLocalAbilityManager();
    EXPECT_FALSE(res);
}

/**
 * @tc.name: AddLocalAbilityManager002
 * @tc.desc: AddLocalAbilityManager, return true!
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, AddLocalAbilityManager002, TestSize.Level3)
{
    LocalAbilityManager::GetInstance().abilityPhaseMap_.clear();
    LocalAbilityManager::GetInstance().FindAndStartPhaseTasks();
    LocalAbilityManager::GetInstance().procName_ = u"test";
    bool res = LocalAbilityManager::GetInstance().AddLocalAbilityManager();
    EXPECT_FALSE(res);
}

/**
 * @tc.name: FoundationRestart001
 * @tc.desc:  FoundationRestart001
 * @tc.type: FUNC
 * @tc.require: I5N9IY
 */
HWTEST_F(LocalAbilityManagerTest, FoundationRestart001, TestSize.Level3)
{
    std::ifstream foundationCfg;
    foundationCfg.open("/etc/init/foundation.cfg", std::ios::in);
    ASSERT_TRUE(foundationCfg.is_open());
    std::string cfg = "";
    char ch;
    while (foundationCfg.get(ch)) {
        cfg.push_back(ch);
    }
    foundationCfg.close();
    EXPECT_TRUE(cfg.find("critical") == std::string::npos);
}

/**
 * @tc.name: OnRemoteRequest001
 * @tc.desc: OnRemoteRequest001
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, OnRemoteRequest001, TestSize.Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::string deviceId = "";
    LocalAbilityManager::SystemAbilityListener *sysListener = new LocalAbilityManager::SystemAbilityListener();
    sysListener->OnAddSystemAbility(SAID, deviceId);
    int32_t result = LocalAbilityManager::GetInstance().OnRemoteRequest(0, data, reply, option);
    delete sysListener;
    EXPECT_NE(result, ERR_NONE);
}

/**
 * @tc.name: OnRemoteRequest002
 * @tc.desc: OnRemoteRequest002
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, OnRemoteRequest002, TestSize.Level2)
{
    MessageParcel data;
    data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    std::string deviceId = "";
    LocalAbilityManager::SystemAbilityListener *sysListener = new LocalAbilityManager::SystemAbilityListener();
    sysListener->OnAddSystemAbility(INVALID_SAID, deviceId);
    int32_t result = LocalAbilityManager::GetInstance().OnRemoteRequest(0, data, reply, option);
    delete sysListener;
    EXPECT_NE(result, ERR_NONE);
}

/**
 * @tc.name: OnRemoteRequest003
 * @tc.desc: OnRemoteRequest003
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, OnRemoteRequest003, TestSize.Level2)
{
    MessageParcel data;
    data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    std::string deviceId = "";
    LocalAbilityManager::SystemAbilityListener *sysListener = new LocalAbilityManager::SystemAbilityListener();
    sysListener->OnRemoveSystemAbility(INVALID_SAID, deviceId);
    int32_t result = LocalAbilityManager::GetInstance().OnRemoteRequest(1, data, reply, option);
    delete sysListener;
    EXPECT_NE(result, ERR_NONE);
}
/**
 * @tc.name: OnRemoteRequest004
 * @tc.desc: OnRemoteRequest004
 * @tc.type: FUNC
 */
HWTEST_F(LocalAbilityManagerTest, OnRemoteRequest004, TestSize.Level2)
{
    MessageParcel data;
    data.WriteInterfaceToken(LOCAL_ABILITY_MANAGER_INTERFACE_TOKEN);
    data.WriteInt32(1);
    MessageParcel reply;
    MessageOption option;
    std::string deviceId = "";
    LocalAbilityManager::SystemAbilityListener *sysListener = new LocalAbilityManager::SystemAbilityListener();
    sysListener->OnRemoveSystemAbility(SAID, deviceId);
    int32_t result = LocalAbilityManager::GetInstance().OnRemoteRequest(1, data, reply, option);
    delete sysListener;
    EXPECT_EQ(result, ERR_NONE);
}
} // namespace SAFWK
} // namespace OHOS
