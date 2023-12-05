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

#include <gtest/gtest.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "hks_access_control_rsa_sign_verify_test_common.h"

#include "hks_mem.h"

using namespace testing::ext;
namespace Unittest::AccessControlRsaSignVerify {
struct AcRsaSignVerifyParamStruct {
    IDMParams testIDMParams;
    bool isUseIndataAfterHash;
    uint32_t hashAlgIndex;
};

const uint32_t SLEEP_TWO_SECOND = 2;

static int32_t RSAAuthTokenSign(struct HksBlob *challenge,
    const IDMParams &testIDMParams, struct HksParam *tmpParams)
{
    std::vector<uint8_t> token;
    // 添加authtoken
    int32_t ret = HksAccessControlPartTest::AuthTokenSign(challenge, testIDMParams, token);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("AuthTokenSign Failed.");
        return ret;
    }
    uint8_t *authToken = static_cast<uint8_t *>(HksMalloc(AUTH_TOKEN_LEN));
    for (uint32_t i = 0; i < AUTH_TOKEN_LEN; i++) {
        authToken[i] = token[i];
    }
    tmpParams->tag = HKS_TAG_AUTH_TOKEN;
    tmpParams->blob.data = authToken;
    tmpParams->blob.size = AUTH_TOKEN_LEN;
    return HKS_SUCCESS;
}

static int32_t AddAuthTokenParam(struct HksParamSet **paramSet, const struct HksParam *tmpParams)
{
    int32_t ret = HksAddParams(*paramSet, tmpParams, 1);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksAddParam failed!\n");
        return ret;
    }
    ret = HksBuildParamSet(paramSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksBuildParamSet failed!\n");
        return ret;
    }
    return HKS_SUCCESS;
}

static int32_t AppendToNewParamSet(const struct HksParamSet *paramSet, struct HksParamSet **outParamSet)
{
    int32_t ret;
    struct HksParamSet *newParamSet = nullptr;
    HKS_LOG_E("AppendToNewParamSet start ");
    do {
        ret = HksCheckParamSet(paramSet, paramSet->paramSetSize);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check paramSet failed");
            break;
        }

        ret = HksFreshParamSet(const_cast<struct HksParamSet *>(paramSet), false);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append fresh paramset failed");
            break;
        }

        ret = HksInitParamSet(&newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append init operation param set failed");
            break;
        }

        ret = HksAddParams(newParamSet, paramSet->params, paramSet->paramsCnt);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append params failed");
            break;
        }
        HKS_LOG_E("AppendToNewParamSet end ");

        *outParamSet = newParamSet;
        return ret;
    } while (0);

    HksFreeParamSet(&newParamSet);
    return ret;
}

static const uint32_t SLEEP_TIME = 1;

static int32_t HksAcRsaThreeStageNormalCase(struct HksBlob *keyAlias, struct HksParamSet *paramSet,
    const struct AcRsaSignVerifyParamStruct &param, struct HksBlob *inDataSign, struct HksBlob *outDataSign)
{
    struct HksBlob inData = {
        g_inData.length(),
        const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(g_inData.c_str()))
    };

    if (param.isUseIndataAfterHash) {
        inData.size = strlen(reinterpret_cast<const char *>(g_inDataArrayAfterHash[param.hashAlgIndex]));
        inData.data = const_cast<uint8_t *>(g_inDataArrayAfterHash[param.hashAlgIndex]);
    }

    // Init
    uint8_t handleS[sizeof(uint64_t)] = {0};
    struct HksBlob handle = { sizeof(uint64_t), handleS };
    uint8_t challenge[TOKEN_CHALLENGE_LEN] = {0};
    struct HksBlob challengeBlob = { TOKEN_CHALLENGE_LEN, challenge };
    int32_t ret = HksInit(keyAlias, paramSet, &handle, &challengeBlob);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";

    // sleep 1 sec for time out test
    sleep(SLEEP_TIME);
    
    // Update loop
    struct HksParam Params;
    ret = RSAAuthTokenSign(&challengeBlob, param.testIDMParams, &Params);
    EXPECT_EQ(ret, HKS_SUCCESS) << "AuthToken sign failed.";
    
    struct HksParamSet *newParamSet = nullptr;

    ret = AppendToNewParamSet(paramSet, &newParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("AppendToNewParamSet failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }

    ret = AddAuthTokenParam(&newParamSet, &Params);

    EXPECT_EQ(ret, HKS_SUCCESS) << "Add Param failed.";
    ret = HksTestUpdate(&handle, newParamSet, &inData);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Update failed, ret : %" LOG_PUBLIC "d", ret);
        HksFreeParamSet(&newParamSet);
        return ret;
    }

    // Finish
    ret = HksFinish(&handle, newParamSet, inDataSign, outDataSign);
    HksFreeParamSet(&newParamSet);
    return ret;
}

static int32_t HksAcRsaThreeStageAbnormalCase(struct HksBlob *keyAlias, struct HksParamSet *paramSet,
    const struct AcRsaSignVerifyParamStruct &param, struct HksBlob *inDataSign, struct HksBlob *outDataSign)
{
    struct HksBlob inData = {
        g_inData.length(),
        const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(g_inData.c_str()))
    };

    if (param.isUseIndataAfterHash) {
        inData.size = strlen(reinterpret_cast<const char *>(g_inDataArrayAfterHash[param.hashAlgIndex]));
        inData.data = const_cast<uint8_t *>(g_inDataArrayAfterHash[param.hashAlgIndex]);
    }

    // Init
    uint8_t handleS[sizeof(uint64_t)] = {0};
    struct HksBlob handle = { sizeof(uint64_t), handleS };
    uint8_t challenge[TOKEN_CHALLENGE_LEN] = {0};
    struct HksBlob challengeBlob = { TOKEN_CHALLENGE_LEN, challenge };
    int32_t ret = HksInit(keyAlias, paramSet, &handle, &challengeBlob);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";

    sleep(SLEEP_TWO_SECOND); // Waiting for time out

    // Update loop
    struct HksParam Params;
    ret = RSAAuthTokenSign(&challengeBlob, param.testIDMParams, &Params);
    EXPECT_EQ(ret, HKS_SUCCESS) << "AuthToken sign failed.";
    
    struct HksParamSet *newParamSet = nullptr;

    ret = AppendToNewParamSet(paramSet, &newParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("AppendToNewParamSet failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }

    ret = AddAuthTokenParam(&newParamSet, &Params);

    EXPECT_EQ(ret, HKS_SUCCESS) << "Add Param failed.";
    ret = HksTestUpdate(&handle, newParamSet, &inData);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Update failed, ret : %" LOG_PUBLIC "d", ret);
        HksAbort(&handle, newParamSet);
        HksFreeParamSet(&newParamSet);
        return ret;
    }

    // Finish
    ret = HksFinish(&handle, newParamSet, inDataSign, outDataSign);
    HksFreeParamSet(&newParamSet);
    return ret;
}

static int32_t AddAuthToeknParamCustomCase(const struct HksBlob &challengeBlob1,
    const struct HksBlob &challengeBlob2, struct HksParam *signParams, const IDMParams &testIDMParams)
{
    uint8_t challenge[TOKEN_CHALLENGE_LEN] = {0};
    struct HksBlob challengeBlob = { TOKEN_CHALLENGE_LEN, challenge };
    if (memcpy_s(challengeBlob.data, TOKEN_CHALLENGE_LEN, challengeBlob1.data, TOKEN_CHALLENGE_LEN_PER_POS) != EOK) {
        return HKS_FAILURE;
    }

    if (memcpy_s(challengeBlob.data + TOKEN_CHALLENGE_LEN_PER_POS, TOKEN_CHALLENGE_LEN - TOKEN_CHALLENGE_LEN_PER_POS,
        challengeBlob2.data + TOKEN_CHALLENGE_LEN_PER_POS, TOKEN_CHALLENGE_LEN_PER_POS) != EOK) {
        return HKS_FAILURE;
    }

    int32_t ret = RSAAuthTokenSign(&challengeBlob, testIDMParams, signParams);
    return ret;
}

static int32_t AddSignParamCustomCase(struct HksParam *signParams, struct HksParamSet *paramSet,
    struct HksParamSet **newParamSet, const IDMParams &testIDMParams)
{
    int32_t ret = AppendToNewParamSet(paramSet, newParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("AppendToNewParamSet failed, ret : %" LOG_PUBLIC "d", ret);
        return ret;
    }
    ret = HksAddParams(*newParamSet, signParams, 1);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksAddParam failed!\n");
        return ret;
    }
    ret = HksBuildParamSet(newParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("HksBuildParamSet failed!\n");
        return ret;
    }
    return ret;
}

uint8_t outDataS1[RSA_COMMON_SIZE] = {0};
struct HksBlob outData1 = { RSA_COMMON_SIZE, outDataS1 };
uint8_t outDataS2[RSA_COMMON_SIZE] = {0};
struct HksBlob outData2 = { RSA_COMMON_SIZE, outDataS2 };

static int32_t HksAcRsaThreeStageSignCustomCase(struct HksBlob *keyAlias1, struct HksParamSet *paramSet1,
    struct HksBlob *keyAlias2, struct HksParamSet *paramSet2, const struct AcRsaSignVerifyParamStruct &param)
{
    struct HksBlob inData = {
        g_inData.length(),
        const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(g_inData.c_str()))
    };

    if (param.isUseIndataAfterHash) {
        inData.size = strlen(reinterpret_cast<const char *>(g_inDataArrayAfterHash[param.hashAlgIndex]));
        inData.data = const_cast<uint8_t *>(g_inDataArrayAfterHash[param.hashAlgIndex]);
    }

    uint8_t handleS1[sizeof(uint64_t)] = {0};
    struct HksBlob handle1 = { sizeof(uint64_t), handleS1 };
    uint8_t challenge1[TOKEN_CHALLENGE_LEN] = {0};
    struct HksBlob challengeBlob1 = { TOKEN_CHALLENGE_LEN, challenge1 };
    int32_t ret = HksInit(keyAlias1, paramSet1, &handle1, &challengeBlob1);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";

    uint8_t handleS2[sizeof(uint64_t)] = {0};
    struct HksBlob handle2 = { sizeof(uint64_t), handleS2 };
    uint8_t challenge2[TOKEN_CHALLENGE_LEN] = {0};
    struct HksBlob challengeBlob2 = { TOKEN_CHALLENGE_LEN, challenge2 };
    ret = HksInit(keyAlias2, paramSet2, &handle2, &challengeBlob2);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";

    // Update loop
    struct HksParamSet *newParamSet1 = nullptr;
    struct HksParamSet *newParamSet2 = nullptr;
    struct HksParam signParams;
    ret = AddAuthToeknParamCustomCase(challengeBlob1, challengeBlob2, &signParams, param.testIDMParams);
    EXPECT_EQ(ret, HKS_SUCCESS) << "AddAuthToeknParam failed.";

    ret = AddSignParamCustomCase(&signParams, paramSet1, &newParamSet1, param.testIDMParams);
    EXPECT_EQ(ret, HKS_SUCCESS) << "AddPosParam failed.";
    ret = HksTestUpdate(&handle1, newParamSet1, &inData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Update failed.";
 
    ret = AddSignParamCustomCase(&signParams, paramSet2, &newParamSet2, param.testIDMParams);
    EXPECT_EQ(ret, HKS_SUCCESS) << "AddPosParam failed.";
    ret = HksTestUpdate(&handle2, newParamSet2, &inData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Update failed.";

    // Finish
    uint8_t tmpIn1[] = "tempIn";
    struct HksBlob finishInData1 = { 0, tmpIn1 };
    ret = HksFinish(&handle1, paramSet1, &finishInData1, &outData1);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Finish failed.";

    uint8_t tmpIn2[] = "tempIn";
    struct HksBlob finishInData2 = { 0, tmpIn2 };
    ret = HksFinish(&handle2, paramSet2, &finishInData2, &outData2);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Finish failed.";
    HksFreeParamSet(&newParamSet1);
    HksFreeParamSet(&newParamSet2);
    return ret;
}

static int32_t HksAcRsaThreeStageVerifyCustomCase(struct HksBlob *keyAlias1, struct HksParamSet *paramSet1,
    struct HksBlob *keyAlias2, struct HksParamSet *paramSet2, const struct AcRsaSignVerifyParamStruct &param)
{
    struct HksBlob inData = {
        g_inData.length(),
        const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(g_inData.c_str()))
    };

    if (param.isUseIndataAfterHash) {
        inData.size = strlen(reinterpret_cast<const char *>(g_inDataArrayAfterHash[param.hashAlgIndex]));
        inData.data = const_cast<uint8_t *>(g_inDataArrayAfterHash[param.hashAlgIndex]);
    }

    uint8_t handleS1[sizeof(uint64_t)] = {0};
    struct HksBlob handle1 = { sizeof(uint64_t), handleS1 };
    uint8_t challenge1[TOKEN_CHALLENGE_LEN] = {0};
    struct HksBlob challengeBlob1 = { TOKEN_CHALLENGE_LEN, challenge1 };
    int32_t ret = HksInit(keyAlias1, paramSet1, &handle1, &challengeBlob1);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";

    uint8_t handleS2[sizeof(uint64_t)] = {0};
    struct HksBlob handle2 = { sizeof(uint64_t), handleS2 };
    uint8_t challenge2[TOKEN_CHALLENGE_LEN] = {0};
    struct HksBlob challengeBlob2 = { TOKEN_CHALLENGE_LEN, challenge2 };
    ret = HksInit(keyAlias2, paramSet2, &handle2, &challengeBlob2);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Init failed.";

    // Update loop
    struct HksParamSet *newParamSet1 = nullptr;
    struct HksParamSet *newParamSet2 = nullptr;
    struct HksParam signParams;
    ret = AddAuthToeknParamCustomCase(challengeBlob1, challengeBlob2, &signParams, param.testIDMParams);
    EXPECT_EQ(ret, HKS_SUCCESS) << "AddAuthToeknParam failed.";

    ret = AddSignParamCustomCase(&signParams, paramSet1, &newParamSet1, param.testIDMParams);
    EXPECT_EQ(ret, HKS_SUCCESS) << "AddPosParam failed.";
    ret = HksTestUpdate(&handle1, newParamSet1, &inData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Update failed.";
 
    ret = AddSignParamCustomCase(&signParams, paramSet2, &newParamSet2, param.testIDMParams);
    EXPECT_EQ(ret, HKS_SUCCESS) << "AddPosParam failed.";
    ret = HksTestUpdate(&handle2, newParamSet2, &inData);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Update failed.";

    // Finish
    uint8_t tmpIn1[] = "tempIn";
    struct HksBlob finishOutData1 = { 0, tmpIn1 };
    ret = HksFinish(&handle1, paramSet1, &outData1, &finishOutData1);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Finish failed.";

    uint8_t tmpIn2[] = "tempIn";
    struct HksBlob finishOutData2 = { 0, tmpIn2 };
    ret = HksFinish(&handle2, paramSet2, &outData2, &finishOutData2);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Finish failed.";
    HksFreeParamSet(&newParamSet1);
    HksFreeParamSet(&newParamSet2);
    return ret;
}

static int32_t AcRsaSignVerifyTestNormalCase(const TestAccessCaseRSAParams &testCaseParams,
    const struct AcRsaSignVerifyParamStruct &param)
{
    /* 1. Generate Key */
    // Generate Key
    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, testCaseParams.genParams.data(), testCaseParams.genParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitgenParamSet failed.";
    uint8_t alias[] = "AcRSASignVerify";
    struct HksBlob keyAlias = { sizeof(alias), alias };
    ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "RSA generate key failed.";
    
    uint8_t tmpIn[] = "tempIn";
    struct HksBlob finishInData = { 0, tmpIn };
    uint8_t outDataS[RSA_COMMON_SIZE] = {0};
    struct HksBlob outDataSign = { RSA_COMMON_SIZE, outDataS };
    struct HksParamSet *signParamSet = nullptr;
    ret = InitParamSet(&signParamSet, testCaseParams.signParams.data(), testCaseParams.signParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitsignParamSet failed.";
    ret = HksAcRsaThreeStageNormalCase(&keyAlias, signParamSet, param, &finishInData, &outDataSign);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Three stage failed.";

    /* 3. Export Public Key */
    uint8_t pubKey[HKS_RSA_KEY_SIZE_1024] = {0};
    struct HksBlob publicKey = { HKS_RSA_KEY_SIZE_1024, pubKey };
    ret = HksExportPublicKey(&keyAlias, genParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ExportPublicKey failed.";

    /* 4. Import Key */
    struct HksParamSet *verifyParamSet = nullptr;
    ret = InitParamSet(&verifyParamSet, testCaseParams.verifyParams.data(), testCaseParams.verifyParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitverifyParamSet failed.";

    char newKey[] = "RSA_Sign_Verify_Import_KeyAlias";
    struct HksBlob newKeyAlias = { .size = strlen(newKey), .data = reinterpret_cast<uint8_t *>(newKey) };
    ret = HksImportKey(&newKeyAlias, verifyParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ImportKey failed";
    
    uint8_t temp[] = "out";
    struct HksBlob verifyOut = { sizeof(temp), temp };
    ret = HksAcRsaThreeStageNormalCase(&newKeyAlias, verifyParamSet, param, &outDataSign, &verifyOut);
    EXPECT_EQ(ret, HKS_SUCCESS) << "Three stage failed.";

    /* 6. Delete New Key */
    int32_t deleteRet = HksDeleteKey(&newKeyAlias, verifyParamSet);
    EXPECT_EQ(deleteRet, HKS_SUCCESS) << "Delete ImportKey failed.";
    
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);

    return ret;
}

int32_t HksAcRsaSignVerifyTestNormalCase(const TestAccessCaseRSAParams &testCaseParams,
    const IDMParams &testIDMParams)
{
    struct HksParamSet *signParamSet = nullptr;
    int32_t ret = InitParamSet(&signParamSet, testCaseParams.signParams.data(), testCaseParams.signParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitsignParamSet failed.";

    struct AcRsaSignVerifyParamStruct param = { testIDMParams, false, 0 };
    struct HksParam *digestAlg = nullptr;
    ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if (digestAlg->uint32Param == HKS_DIGEST_NONE) {
        param.isUseIndataAfterHash = true;
    }

    if (param.isUseIndataAfterHash) {
        for (uint32_t i = 0; i < (sizeof(g_inDataArrayAfterHash) / sizeof(uint8_t *)); i++) {
            param.hashAlgIndex = i;
            ret = AcRsaSignVerifyTestNormalCase(testCaseParams, param);
            if (ret != HKS_SUCCESS) {
                return ret;
            }
        }
    } else {
        return AcRsaSignVerifyTestNormalCase(testCaseParams, param);
    }

    return HKS_SUCCESS;
}

static int32_t AcRsaSignVerifyTestAbnormalCase(const TestAccessCaseRSAParams &testCaseParams,
    const struct AcRsaSignVerifyParamStruct &param)
{
    /* 1. Generate Key */
    // Generate Key
    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, testCaseParams.genParams.data(), testCaseParams.genParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitgenParamSet failed.";
    uint8_t alias[] = "AcRSASignVerify";
    struct HksBlob keyAlias = { sizeof(alias), alias };
    ret = HksGenerateKey(&keyAlias, genParamSet, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "RSA generate key failed.";
    
    uint8_t tmpIn[] = "tempIn";
    struct HksBlob finishInData = { 0, tmpIn };
    uint8_t outDataS[RSA_COMMON_SIZE] = {0};
    struct HksBlob outDataSign = { RSA_COMMON_SIZE, outDataS };
    struct HksParamSet *signParamSet = nullptr;
    ret = InitParamSet(&signParamSet, testCaseParams.signParams.data(), testCaseParams.signParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitsignParamSet failed.";
    ret = HksAcRsaThreeStageAbnormalCase(&keyAlias, signParamSet, param, &finishInData, &outDataSign);
    EXPECT_NE(ret, HKS_SUCCESS) << "Three stage failed.";

    /* 3. Export Public Key */
    uint8_t pubKey[HKS_RSA_KEY_SIZE_1024] = {0};
    struct HksBlob publicKey = { HKS_RSA_KEY_SIZE_1024, pubKey };
    ret = HksExportPublicKey(&keyAlias, genParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ExportPublicKey failed.";

    /* 4. Import Key */
    struct HksParamSet *verifyParamSet = nullptr;
    ret = InitParamSet(&verifyParamSet, testCaseParams.verifyParams.data(), testCaseParams.verifyParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitverifyParamSet failed.";

    char newKey[] = "RSA_Sign_Verify_Import_KeyAlias";
    struct HksBlob newKeyAlias = { .size = strlen(newKey), .data = reinterpret_cast<uint8_t *>(newKey) };
    ret = HksImportKey(&newKeyAlias, verifyParamSet, &publicKey);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ImportKey failed";
    
    uint8_t temp[] = "out";
    struct HksBlob verifyOut = { sizeof(temp), temp };
    ret = HksAcRsaThreeStageAbnormalCase(&newKeyAlias, verifyParamSet, param, &outDataSign, &verifyOut);
    EXPECT_NE(ret, HKS_SUCCESS) << "Three stage failed.";

    /* 6. Delete New Key */
    int32_t deleteRet = HksDeleteKey(&newKeyAlias, verifyParamSet);
    EXPECT_EQ(deleteRet, HKS_SUCCESS) << "Delete ImportKey failed.";
    
    ret = HksDeleteKey(&keyAlias, genParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&signParamSet);
    HksFreeParamSet(&verifyParamSet);

    return ret;
}

int32_t HksAcRsaSignVerifyTestAbnormalCase(const TestAccessCaseRSAParams &testCaseParams,
    const IDMParams &testIDMParams)
{
    struct HksParamSet *signParamSet = nullptr;
    int32_t ret = InitParamSet(&signParamSet, testCaseParams.signParams.data(), testCaseParams.signParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitsignParamSet failed.";

    struct AcRsaSignVerifyParamStruct param = { testIDMParams, false, 0 };
    struct HksParam *digestAlg = nullptr;
    ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if (digestAlg->uint32Param == HKS_DIGEST_NONE) {
        param.isUseIndataAfterHash = true;
    }

    if (param.isUseIndataAfterHash) {
        for (uint32_t i = 0; i < (sizeof(g_inDataArrayAfterHash) / sizeof(uint8_t *)); i++) {
            param.hashAlgIndex = i;
            ret = AcRsaSignVerifyTestAbnormalCase(testCaseParams, param);
            if (ret != HKS_SUCCESS) {
                return ret;
            }
        }
    } else {
        return AcRsaSignVerifyTestAbnormalCase(testCaseParams, param);
    }

    return HKS_SUCCESS;
}

static int32_t HksAcRsaVerifyTestCustomCase(const TestAccessCaseRSAParams &testCaseParams1,
    const TestAccessCaseRSAParams &testCaseParams2, struct HksBlob *publicKey1, struct HksBlob *publicKey2,
    const struct AcRsaSignVerifyParamStruct &param)
{
    /* 4. Import Key */
    struct HksParamSet *verifyParamSet1 = nullptr;
    int32_t ret = InitParamSet(&verifyParamSet1, testCaseParams1.verifyParams.data(),
        testCaseParams1.verifyParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitverifyParamSet failed.";
    char newKey1[] = "RSA_Sign_Verify_Import_KeyAlia1";
    struct HksBlob newKeyAlias1 = { .size = strlen(newKey1), .data = reinterpret_cast<uint8_t *>(newKey1) };
    ret = HksImportKey(&newKeyAlias1, verifyParamSet1, publicKey1);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ImportKey failed";

    struct HksParamSet *verifyParamSet2 = nullptr;
    ret = InitParamSet(&verifyParamSet2, testCaseParams2.verifyParams.data(), testCaseParams2.verifyParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitverifyParamSet failed.";
    char newKey2[] = "RSA_Sign_Verify_Import_KeyAlia2";
    struct HksBlob newKeyAlias2 = { .size = strlen(newKey2), .data = reinterpret_cast<uint8_t *>(newKey2) };
    ret = HksImportKey(&newKeyAlias2, verifyParamSet2, publicKey2);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ImportKey failed";

    ret = HksAcRsaThreeStageVerifyCustomCase(&newKeyAlias1, verifyParamSet1, &newKeyAlias2, verifyParamSet2, param);
    EXPECT_EQ(ret, HKS_SUCCESS) << "verify custom case failed.";

    /* 6. Delete New Key */
    int32_t deleteRet = HksDeleteKey(&newKeyAlias1, verifyParamSet1);
    EXPECT_EQ(deleteRet, HKS_SUCCESS) << "Delete ImportKey failed.";
    deleteRet = HksDeleteKey(&newKeyAlias2, verifyParamSet2);
    EXPECT_EQ(deleteRet, HKS_SUCCESS) << "Delete ImportKey failed.";

    HksFreeParamSet(&verifyParamSet1);
    HksFreeParamSet(&verifyParamSet2);

    return ret;
}

static int32_t AcRsaSignTestCustomCase(const TestAccessCaseRSAParams &testCaseParams1,
    const TestAccessCaseRSAParams &testCaseParams2, const struct AcRsaSignVerifyParamStruct &param)
{
    /* 1. Generate Key */
    // Generate Key
    struct HksParamSet *genParamSet1 = nullptr;
    int32_t ret = InitParamSet(&genParamSet1, testCaseParams1.genParams.data(), testCaseParams1.genParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitgenParamSet failed.";
    uint8_t alias1[] = "Ac_RSA_Sign_Verify1";
    struct HksBlob keyAlias1 = { sizeof(alias1), alias1 };
    ret = HksGenerateKey(&keyAlias1, genParamSet1, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "RSA generate key failed.";

    struct HksParamSet *genParamSet2 = nullptr;
    ret = InitParamSet(&genParamSet2, testCaseParams2.genParams.data(), testCaseParams2.genParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitgenParamSet failed.";
    uint8_t alias2[] = "Ac_RSA_Sign_Verify2";
    struct HksBlob keyAlias2 = { sizeof(alias2), alias2 };
    ret = HksGenerateKey(&keyAlias2, genParamSet2, nullptr);
    EXPECT_EQ(ret, HKS_SUCCESS) << "RSA generate key failed.";

    struct HksParamSet *signParamSet1 = nullptr;
    ret = InitParamSet(&signParamSet1, testCaseParams1.signParams.data(), testCaseParams1.signParams.size());

    EXPECT_EQ(ret, HKS_SUCCESS) << "InitsignParamSet failed.";
    
    struct HksParamSet *signParamSet2 = nullptr;
    ret = InitParamSet(&signParamSet2, testCaseParams2.signParams.data(), testCaseParams2.signParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitsignParamSet failed.";

    ret = HksAcRsaThreeStageSignCustomCase(&keyAlias1, signParamSet1, &keyAlias2, signParamSet2, param);
    EXPECT_EQ(ret, HKS_SUCCESS) << "RSA Sign failed.";
    
    /* 3. Export Public Key */
    uint8_t pubKey1[HKS_RSA_KEY_SIZE_1024] = {0};
    struct HksBlob publicKey1 = { HKS_RSA_KEY_SIZE_1024, pubKey1 };
    ret = HksExportPublicKey(&keyAlias1, genParamSet1, &publicKey1);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ExportPublicKey failed.";

    uint8_t pubKey2[HKS_RSA_KEY_SIZE_1024] = {0};
    struct HksBlob publicKey2 = { HKS_RSA_KEY_SIZE_1024, pubKey2 };
    ret = HksExportPublicKey(&keyAlias2, genParamSet2, &publicKey2);
    EXPECT_EQ(ret, HKS_SUCCESS) << "ExportPublicKey failed.";

    ret = HksAcRsaVerifyTestCustomCase(testCaseParams1, testCaseParams2, &publicKey1, &publicKey2, param);
    EXPECT_EQ(ret, HKS_SUCCESS) << "RSA Verify failed.";

    ret = HksDeleteKey(&keyAlias1, genParamSet1);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";
    ret = HksDeleteKey(&keyAlias2, genParamSet2);
    EXPECT_EQ(ret, HKS_SUCCESS) << "DeleteKey failed.";

    HksFreeParamSet(&genParamSet1);
    HksFreeParamSet(&signParamSet1);
    HksFreeParamSet(&genParamSet2);
    HksFreeParamSet(&signParamSet2);

    return ret;
}

int32_t HksAcRsaSignTestCustomCase(const TestAccessCaseRSAParams &testCaseParams1,
    const TestAccessCaseRSAParams &testCaseParams2, const IDMParams &testIDMParams)
{
    struct HksParamSet *signParamSet = nullptr;
    int32_t ret = InitParamSet(&signParamSet, testCaseParams1.signParams.data(), testCaseParams1.signParams.size());
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitsignParamSet failed.";

    struct AcRsaSignVerifyParamStruct param = { testIDMParams, false, 0 };
    struct HksParam *digestAlg = nullptr;
    ret = HksGetParam(signParamSet, HKS_TAG_DIGEST, &digestAlg);
    EXPECT_EQ(ret, HKS_SUCCESS) << "GetParam failed.";
    if (digestAlg->uint32Param == HKS_DIGEST_NONE) {
        param.isUseIndataAfterHash = true;
    }

    if (param.isUseIndataAfterHash) {
        for (uint32_t i = 0; i < (sizeof(g_inDataArrayAfterHash) / sizeof(uint8_t *)); i++) {
            param.hashAlgIndex = i;
            ret = AcRsaSignTestCustomCase(testCaseParams1, testCaseParams2, param);
            if (ret != HKS_SUCCESS) {
                return ret;
            }
        }
    } else {
        return AcRsaSignTestCustomCase(testCaseParams1, testCaseParams2, param);
    }

    return HKS_SUCCESS;
}
}