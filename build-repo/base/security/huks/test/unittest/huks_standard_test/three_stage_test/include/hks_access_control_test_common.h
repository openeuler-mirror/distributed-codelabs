/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HKS_ACCESS_CONTROL_TEST_COMMON_H
#define HKS_ACCESS_CONTROL_TEST_COMMON_H

#include "hks_three_stage_test_common.h"
#include "hks_core_hal_api.h"

#include <vector>
#include <string>

#define SHA256_SIGN_LEN 32
#define SHA256_KEY_LEN 32
#define AUTH_TOKEN_LEN sizeof(struct HksUserAuthToken)
#define AUTH_TOKEN_DATA_LEN (AUTH_TOKEN_LEN - SHA256_SIGN_LEN)
#define TOKEN_CHALLENGE_LEN 32
#define TOKEN_CHALLENGE_LEN_PER_POS 8
#define HKS_DEFAULT_USER_AT_KEY "huks_default_user_auth_token_key"

struct IDMParams {
    uint64_t secureUid;
    uint64_t enrolledId;
    uint64_t time;
    uint32_t authType;
};

namespace Unittest::HksAccessControlPartTest {
static const std::string g_inData = "Hks_Authtoken_Test_00000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000000000000000000000000000000000000000000000000000000000000000_string";

static const uint32_t IV_SIZE = 16;

const uint32_t KEY_PARAMSET_SIZE = 1024;

const uint32_t HMAC_COMMON_SIZE = 256;

const uint32_t DATA_COMMON_SIZE = 1024;

const uint32_t RSA_COMMON_SIZE = 1024;

const uint32_t ECDH_COMMON_SIZE = 1024;

const uint32_t DERIVE_KEY_SIZE_32 = 32;

const uint32_t DERIVE_ITERATION = 1000;

const uint32_t DERIVE_COMMON_SIZE = 2048;

const uint32_t DSA_COMMON_SIZE = 1024;

static uint8_t IV[IV_SIZE] = {0};

static uint8_t g_saltdata[16] = {0};

struct TestAccessCaseParams {
    std::vector<HksParam> genParams;
    std::vector<HksParam> initParams;
    HksErrorCode initResult = HksErrorCode::HKS_SUCCESS;
};

struct TestDsaKeyParams {
    struct HksBlob *xData;
    struct HksBlob *yData;
    struct HksBlob *pData;
    struct HksBlob *qData;
    struct HksBlob *gData;
};

struct HksTestGenAuthTokenParams {
    struct HksBlob *authChallenge;
    uint64_t secureUid;
    uint64_t enrolledId;
    uint64_t credentialId;
    uint64_t time;
    uint32_t authType;
};

int32_t AddAuthtokenUpdateFinish(struct HksBlob *handle, struct HksParamSet *initParamSet, uint32_t posNum);

int32_t CheckAccessCipherTest(const TestAccessCaseParams &testCaseParams,
    const IDMParams &testIDMParams);

int32_t CheckAccessHmacTest(const TestAccessCaseParams &testCaseParams,
    const IDMParams &testIDMParams);

int32_t CheckAccessAgreeTest(const TestAccessCaseParams &testCaseParams, struct HksParamSet *finishParamSet,
    const IDMParams &testIDMParams);

int32_t CheckAccessDeriveTest(const TestAccessCaseParams &testCaseParams, struct HksParamSet *finishParamSet,
    const IDMParams &testIDMParams);

int32_t AuthTokenImportKey();

int32_t AuthTokenSign(struct HksBlob *challenge, const IDMParams &testIDMParams, std::vector<uint8_t>& token);

int32_t AuthTokenMac(const struct HksBlob *keyAlias, const struct HksBlob *inData, HksUserAuthToken *authTokenHal);

int32_t HksBuildAuthtoken(const struct HksParamSet *initParamSet, uint8_t *authChallenge,
    const IDMParams &testIDMParams);

int32_t HksBuildAuthTokenSecure(struct HksParamSet *paramSet,
    struct HksTestGenAuthTokenParams *genAuthTokenParams, struct HksParamSet **outParamSet);

int32_t ConstructRsaKeyPair(const struct HksBlob *nDataBlob, const struct HksBlob *dDataBlob,
    const struct HksBlob *eDataBlob, uint32_t keySize, struct HksBlob *outKey);

int32_t ConstructEd25519KeyPair(uint32_t keySize, uint32_t alg, struct HksBlob *ed25519PubData,
    struct HksBlob *ed25519PrivData, struct HksBlob *outKey);

int32_t ConstructDsaKeyPair(uint32_t keySize, const struct TestDsaKeyParams *params, struct HksBlob *outKey);

int32_t GenParamSetAuthTest(struct HksParamSet **paramOutSet, const struct HksParamSet *genParamSet);
}
#endif // HKS_THREE_STAGE_TEST_COMMON_H