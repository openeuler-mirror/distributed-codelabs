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
#include "gtest/gtest.h"
#include "string_ex.h"
#include "test_log.h"
#include "tools.h"

#define private public
#include "parse_util.h"
using namespace std;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace SAMGR {
namespace {
    const std::string TEST_RESOURCE_PATH = "/data/test/resource/samgr/profile/";
    const std::u16string TEST_PROCESS_NAME = u"sa_test";
    const int32_t TEST_NUM = 123;
    const int32_t MOCK_SAID = 1492;
    const int32_t TEST_PROFILE_SAID = 9999;
    const int32_t TEST_PROFILE_SAID_INVAILD = 9990;
}

class ParseUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
protected:
    std::shared_ptr<ParseUtil> parser_;
};

void ParseUtilTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void ParseUtilTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void ParseUtilTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
    if (parser_ == nullptr) {
        parser_ = std::make_shared<ParseUtil>();
    }
}

void ParseUtilTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
    if (parser_ != nullptr) {
        parser_->ClearResource();
    }
}

/**
 * @tc.name: ParseSaProfile001
 * @tc.desc: Verify if can load not exist file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile001, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile001 start " << std::endl;
    /**
     * @tc.steps: step1. parse not exsit config file
     * @tc.expected: step1. return false when load not exist file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "notExist");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ParseSaProfile002
 * @tc.desc: Verify if can load invalid root file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile002, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile002 start " << std::endl;
    /**
     * @tc.steps: step1. load invalid root config file
     * @tc.expected: step1. return false when load invalid format config file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "invalid_root.xml");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ParseSaProfile003
 * @tc.desc: Verify if can load normal profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile003, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile003 start " << std::endl;
    /**
     * @tc.steps: step1. load correct system ability profile
     * @tc.expected: step1. return true when load invalid format config file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "profile.xml");
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ParseSaProfile004
 * @tc.desc: Verify if can load normal profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile004, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile004 start " << std::endl;
    /**
     * @tc.steps: step1. load correct system ability profile
     * @tc.expected: step1. return false when load noxmlfile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "no_xml_file");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ParseSaProfile005
 * @tc.desc: Verify if can load normal profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile005, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile005 start " << std::endl;
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "empty_process.xml");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ParseSaProfile006
 * @tc.desc: Verify if can load normal profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseSaProfile006, TestSize.Level2)
{
    DTEST_LOG << " ParseSaProfile006 start " << std::endl;
    /**
     * @tc.steps: step1. load correct system ability profile
     * @tc.expected: step1. return false when load invalid format config file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "empty_systemability.xml");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetSaProfiles001
 * @tc.desc:  Verify if not load sa file return empty list
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, GetSaProfiles001, TestSize.Level3)
{
    DTEST_LOG << " GetSaProfiles001 start " << std::endl;
    /**
     * @tc.steps: step1. Get empty config when not parse sa file.
     * @tc.expected: step1. return empty list when not load sa file
     */
    list<SaProfile> profiles = parser_->GetAllSaProfiles();
    EXPECT_TRUE(profiles.empty());
}

/**
 * @tc.name: GetSaProfiles002
 * @tc.desc:  Verify if can load normal sa profile
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, GetSaProfiles002, TestSize.Level3)
{
    DTEST_LOG << " GetSaProfiles002 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct profile when parse sa file.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "profile.xml");
    ASSERT_TRUE(ret);
    SaProfile saRunOnCreateTrue;
    saRunOnCreateTrue.runOnCreate = true;
    SaProfile saRunOnCreateFalse;
    parser_->saProfiles_.emplace_back(saRunOnCreateTrue);
    parser_->saProfiles_.emplace_back(saRunOnCreateFalse);
    parser_->OpenSo();
    list<SaProfile> profiles = parser_->GetAllSaProfiles();
    if (!profiles.empty()) {
        SaProfile& profile = *(profiles.begin());
        EXPECT_EQ(profile.process, TEST_PROCESS_NAME);
        EXPECT_EQ(profile.saId, TEST_PROFILE_SAID);
    }
}

/**
 * @tc.name: ParseTrustConfig001
 * @tc.desc:  Verify if can load file with one sa
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig001, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig001 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct map when parse config file.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "test_trust_one_sa.xml", values);
    ASSERT_TRUE(ret);
    /**
     * @tc.steps: step2. Check map values
     * @tc.expected: step2. return expect values
     */
    for (const auto& [process, saIds] : values) {
        EXPECT_EQ(Str16ToStr8(process), "test");
        EXPECT_EQ(saIds.size(), 1);
        EXPECT_EQ(saIds.count(1401), 1);
    }
}

/**
 * @tc.name: ParseTrustConfig002
 * @tc.desc:  Verify if can load file with muti sa
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig002, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig002 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct map when parse config file.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "test_trust_muti_sa.xml", values);
    ASSERT_TRUE(ret);
    /**
     * @tc.steps: step2. Check map values
     * @tc.expected: step2. return expect values
     */
    for (const auto& [process, saIds] : values) {
        EXPECT_EQ(Str16ToStr8(process), "test");
        EXPECT_EQ(saIds.size(), 5);
        EXPECT_EQ(saIds.count(1401), 1);
    }
}

/**
 * @tc.name: ParseTrustConfig003
 * @tc.desc:  Verify if can load not invalid root file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig003, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig003 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct map when parse config file.
     * @tc.expected: step1. return false when load invalid root file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "invalid_root_trust.xml", values);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: ParseTrustConfig004
 * @tc.desc:  Verify if can load not exist file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig004, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig004 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct profile when parse sa file.
     * @tc.expected: step1. return false when not exist file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "notExist", values);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: ParseTrustConfig005
 * @tc.desc:  Verify if can load invalid element config
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig005, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig005 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct profile when parse invalid element config.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "invalid_element_trust.xml", values);
    ASSERT_TRUE(ret);
    for (const auto& [process, saIds] : values) {
        EXPECT_EQ(Str16ToStr8(process), "test");
        EXPECT_EQ(saIds.size(), 3);
        EXPECT_EQ(saIds.count(1401), 1);
    }
}

/**
 * @tc.name: ParseTrustConfig006
 * @tc.desc:  Verify if can load invalid muti root file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, ParseTrustConfig006, TestSize.Level1)
{
    DTEST_LOG << " ParseTrustConfig006 start " << std::endl;
    /**
     * @tc.steps: step1. Get correct profile when parse sa file.
     * @tc.expected: step1. return correct profile object when load correct config file
     */
    std::map<std::u16string, std::set<int32_t>> values;
    bool ret = parser_->ParseTrustConfig(TEST_RESOURCE_PATH + "invalid_muti_root_trust.xml", values);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: RemoveSaProfile001
 * @tc.desc:  Verify if can remove not-existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile001, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile001 start " << std::endl;
    /**
     * @tc.steps: step1. parse not exsit config file
     * @tc.expected: step1. return false when load not exist file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "notExist");
    EXPECT_FALSE(ret);
    /**
     * @tc.steps: step2. remove not-existed id
     * @tc.expected: step2. not crash
     */
    parser_->RemoveSaProfile(111);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 0);
}

/**
 * @tc.name: RemoveSaProfile002
 * @tc.desc:  Verify if can can remove not-existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile002, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile002 start " << std::endl;
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
    /**
     * @tc.steps: step2. remove not-existed id
     * @tc.expected: step2. not crash
     */
    parser_->RemoveSaProfile(111);
    profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
}

/**
 * @tc.name: RemoveSaProfile003
 * @tc.desc:  Verify if can remove one existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile003, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile003 start " << std::endl;
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
    /**
     * @tc.steps: step2. remove one existed id
     * @tc.expected: step2. remove successfully
     */
    parser_->RemoveSaProfile(9999);
    profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 3);
}

/**
 * @tc.name: RemoveSaProfile004
 * @tc.desc:  Verify if can remove one existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile004, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile004 start " << std::endl;
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
    /**
     * @tc.steps: step2. remove one existed id
     * @tc.expected: step2. remove successfully
     */
    parser_->RemoveSaProfile(9997);
    profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 2);
}

/**
 * @tc.name: RemoveSaProfile005
 * @tc.desc:  Verify if can remove more existed id
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, RemoveSaProfile005, TestSize.Level3)
{
    DTEST_LOG << " RemoveSaProfile004 start " << std::endl;
    /**
     * @tc.steps: step1. parse multi-sa profile
     * @tc.expected: step1. return true when load multi-sa profile
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
    auto profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 4);
    /**
     * @tc.steps: step2. remove more existed id
     * @tc.expected: step2. remove successfully
     */
    parser_->RemoveSaProfile(9997);
    parser_->RemoveSaProfile(9998);
    parser_->RemoveSaProfile(9998);
    profiles = parser_->GetAllSaProfiles();
    EXPECT_EQ(profiles.size(), 1);
}

/**
 * @tc.name: CheckPathExist001
 * @tc.desc:  Verify if can check not exist file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, CheckPathExist001, TestSize.Level2)
{
    DTEST_LOG << " CheckPathExist001 start " << std::endl;
    /**
     * @tc.steps: step1. check not exsit config file
     * @tc.expected: step1. return false when check not exist file
     */
    bool ret = parser_->CheckPathExist(TEST_RESOURCE_PATH + "not_exist.xml");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CheckPathExist002
 * @tc.desc:  Verify if can check exist file
 * @tc.type: FUNC
 */
HWTEST_F(ParseUtilTest, CheckPathExist002, TestSize.Level2)
{
    DTEST_LOG << " CheckPathExist002 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit config file
     * @tc.expected: step1. return true when load not exist file
     */
    bool ret = parser_->CheckPathExist(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetProfile001
 * @tc.desc: Verify if can get not-exist profile
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, GetProfile001, TestSize.Level2)
{
    DTEST_LOG << " GetProfile001 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit config file
     * @tc.expected: step1. return true when load not exist file
     */
    SaProfile saProfile;
    bool ret = parser_->GetProfile(TEST_PROFILE_SAID, saProfile);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetProfile002
 * @tc.desc: Verify if can get exist profile
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, GetProfile002, TestSize.Level2)
{
    DTEST_LOG << " GetProfile002 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit config file
     * @tc.expected: step1. return true when load not exist file
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    SaProfile saProfile;
    ret = parser_->GetProfile(TEST_PROFILE_SAID, saProfile);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(saProfile.saId, TEST_PROFILE_SAID);
    EXPECT_EQ(saProfile.runOnCreate, true);
}

/**
 * @tc.name: LoadSaLib001
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib001, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib001 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return true when load exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(TEST_PROFILE_SAID);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: LoadSaLib002
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib002, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib002 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return false when load not exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(TEST_PROFILE_SAID_INVAILD);
    parser_->CloseSo(MOCK_SAID);
    EXPECT_NE(ret, true);
}

/**
 * @tc.name: LoadSaLib003
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib003, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib003 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return false when load not exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "mock.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(MOCK_SAID);
    parser_->CloseSo(MOCK_SAID);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: LoadSaLib004
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib004, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib004 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return false when load not exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "mock.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(MOCK_SAID);
    EXPECT_EQ(ret, true);
    parser_->LoadSaLib(MOCK_SAID);
}

/**
 * @tc.name: LoadSaLib005
 * @tc.desc: Verify if can load salib
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, LoadSaLib005, TestSize.Level2)
{
    DTEST_LOG << " LoadSaLib005 start " << std::endl;
    /**
     * @tc.steps: step1. check exsit salib
     * @tc.expected: step1. return false when load not exist salib
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "empty_libpath.xml");
    EXPECT_EQ(ret, true);
    ret = parser_->LoadSaLib(MOCK_SAID);
    EXPECT_EQ(ret, true);
}
/**
 * @tc.name: GetProcessName001
 * @tc.desc: Verify if can get procesname
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, GetProcessName001, TestSize.Level3)
{
    DTEST_LOG << " GetProcessName001 " << std::endl;
    /**
     * @tc.steps: step1. get SaProfiles
     * @tc.expected: step1. return true when SaProfiles
     */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    std::u16string Name = parser_->GetProcessName();
    EXPECT_EQ(Str16ToStr8(Name), "test");
}

/**
 * @tc.name: GetProcessName002
 * @tc.desc: Verify if can get procesname
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, GetProcessName002, TestSize.Level3)
{
    DTEST_LOG << " GetProcessName002 " << std::endl;
    /**
    * @tc.steps: step1. get SaProfiles
    * @tc.expected: step1. return true when SaProfiles
    */
    bool ret = parser_->ParseSaProfiles(TEST_RESOURCE_PATH + "multi_sa_profile.xml");
    EXPECT_EQ(ret, true);
    std::u16string Name = parser_->GetProcessName();
    EXPECT_NE(Str16ToStr8(Name), "test_1");
}

/**
 * @tc.name: ParseSAProp001
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp001, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp001 " << std::endl;
    string nodeName = "depend";
    string nodeContent = "TEST";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(saProfile.dependSa.size(), 1);
}

/**
 * @tc.name: ParseSAProp002
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp002, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp002 " << std::endl;
    string nodeName = "depend-time-out";
    string nodeContent = "123";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(saProfile.dependTimeout, TEST_NUM);
}

/**
 * @tc.name: ParseSAProp003
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp003, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp003 " << std::endl;
    string nodeName = "capability";
    string nodeContent = "TEST";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(nodeContent, Str16ToStr8(saProfile.capability));
}

/**
 * @tc.name: ParseSAProp004
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp004, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp004 " << std::endl;
    string nodeName = "permission";
    string nodeContent = "TEST";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(nodeContent, Str16ToStr8(saProfile.permission));
}

/**
 * @tc.name: ParseSAProp005
 * @tc.desc: Verify if can ParseSAProp
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSAProp005, TestSize.Level3)
{
    DTEST_LOG << " ParseSAProp005 " << std::endl;
    string nodeName = "bootphase";
    string nodeContent = "TEST";
    SaProfile saProfile;
    parser_->ParseSAProp(nodeName, nodeContent, saProfile);
    EXPECT_EQ(nodeContent, Str16ToStr8(saProfile.bootPhase));
}

/**
 * @tc.name: ParseSystemAbility001
 * @tc.desc: Verify if can ParseSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSystemAbility001, TestSize.Level3)
{
    DTEST_LOG << " ParseSystemAbility001 " << std::endl;
    xmlNode rootNode;
    rootNode.xmlChildrenNode = nullptr;
    u16string process = u"tempprocess";
    bool res = parser_->ParseSystemAbility(rootNode, process);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ParseSystemAbility002
 * @tc.desc: Verify if can ParseSystemAbility
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSystemAbility002, TestSize.Level3)
{
    DTEST_LOG << " ParseSystemAbility002 " << std::endl;
    xmlNode rootNode;
    xmlNode tempNode;
    xmlNode* childrenNode = &tempNode;
    const char* nodeName = "test";
    childrenNode->name = reinterpret_cast<const xmlChar*>(nodeName);
    childrenNode->type = XML_PI_NODE;
    childrenNode->content = nullptr;
    childrenNode->next = nullptr;
    rootNode.xmlChildrenNode = childrenNode;
    u16string process = u"tempprocess";
    bool res = parser_->ParseSystemAbility(rootNode, process);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: ParseProcess001
 * @tc.desc: Verify if can ParseProcess
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseProcess001, TestSize.Level3)
{
    DTEST_LOG << " ParseProcess001 " << std::endl;
    xmlNode tempNode;
    xmlNode* rootNode = &tempNode;
    u16string process = u"tempprocess";
    bool res = parser_->ParseProcess(rootNode, process);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ParseProcess002
 * @tc.desc: Verify if can ParseProcess
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseProcess002, TestSize.Level3)
{
    DTEST_LOG << " ParseProcess002 " << std::endl;
    xmlNode tempNode;
    xmlNode* rootNode = &tempNode;
    const char* nodeName = "test";
    rootNode->name = reinterpret_cast<const xmlChar*>(nodeName);
    rootNode->type = XML_PI_NODE;
    rootNode->content = nullptr;
    u16string process = u"tempprocess";
    bool res = parser_->ParseProcess(rootNode, process);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ParseTrustConfigInner001
 * @tc.desc: Verify if can ParseTrustConfigInner
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseTrustConfigInner001, TestSize.Level2)
{
    DTEST_LOG << " ParseTrustConfigInner001 " << std::endl;
    xmlNode tempNode;
    xmlNode* rootNode = &tempNode;
    rootNode->xmlChildrenNode = nullptr;
    std::map<std::u16string, std::set<int32_t>> values;
    bool res = parser_->ParseTrustConfigInner(rootNode, values);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ParseTrustConfigInner002
 * @tc.desc: Verify if can ParseTrustConfigInner
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseTrustConfigInner002, TestSize.Level2)
{
    DTEST_LOG << " ParseTrustConfigInner002 " << std::endl;
    xmlNode rootNode;
    xmlNode* rootNodeptr = &rootNode;
    xmlNode tempNode;
    xmlNode* childrenNode = &tempNode;
    const char* nodeName = "name";
    childrenNode->name = reinterpret_cast<const xmlChar*>(nodeName);
    childrenNode->type = XML_PI_NODE;
    childrenNode->content = nullptr;
    childrenNode->next = nullptr;
    rootNodeptr->xmlChildrenNode = childrenNode;
    std::map<std::u16string, std::set<int32_t>> values;
    bool res = parser_->ParseTrustConfigInner(rootNodeptr, values);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ParseSaid001
 * @tc.desc: Verify if can ParseSaid
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSaid001, TestSize.Level1)
{
    DTEST_LOG << " ParseSaid001 " << std::endl;
    xmlNode rootNode;
    xmlNode* rootNodeptr = &rootNode;
    rootNodeptr->name = nullptr;
    int32_t tempSaId = -1;
    bool res = parser_->ParseSaId(rootNodeptr, tempSaId);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: ParseSaid002
 * @tc.desc: Verify if can ParseSaid
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, ParseSaid002, TestSize.Level1)
{
    DTEST_LOG << " ParseSaid002 " << std::endl;
    xmlNode rootNode;
    xmlNode* rootNodeptr = &rootNode;
    const char* nodeName = "noNull";
    rootNodeptr->name = reinterpret_cast<const xmlChar*>(nodeName);
    rootNodeptr->type = XML_PI_NODE;
    rootNodeptr->content = nullptr;
    int32_t tempSaId = -1;
    bool res = parser_->ParseSaId(rootNodeptr, tempSaId);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: DeleteAllMark001
 * @tc.desc: Verify if can DeleteAllMark
 * @tc.type: FUNC
 * @tc.require: I5KMF7
 */
HWTEST_F(ParseUtilTest, DeleteAllMark001, TestSize.Level3)
{
    DTEST_LOG << " DeleteAllMark001 " << std::endl;
    u16string temp = u"stests";
    u16string mask = u"s";
    u16string res = DeleteAllMark(temp, mask);
    EXPECT_EQ(res, u"tet");
}
} // namespace SAMGR
} // namespace OHOS
