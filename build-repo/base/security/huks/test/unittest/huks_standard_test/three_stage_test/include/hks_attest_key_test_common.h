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

#ifndef HKS_ATTEST_KEY_TEST_COMMON_H
#define HKS_ATTEST_KEY_TEST_COMMON_H

#include <securec.h>

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type.h"

enum ParamType {
    IDS_PARAM,
    NON_IDS_PARAM,
};

struct HksTestCertChain {
    bool certChainExist;
    bool certCountValid;
    bool certDataExist;
    uint32_t certDataSize;
};

static uint32_t g_size = 4096;
static uint32_t CERT_COUNT = 4;

namespace Unittest::AttestKey {
#define SEC_INFO_DATA "hi_security_level_info"
#define CHALLENGE_DATA "hi_challenge_data"
#define VERSION_DATA "hi_os_version_data"
#define ALIAS "testKey"
#define UDID_DATA "hi_udid_data"
#define SN_DATA "hi_sn_data"
#define DEVICE_ID "test_device_id"
#define APP_ID "test_app_id"
#define ATTEST_BRAND "brand"

void FreeCertChain(struct HksCertChain **certChain, const uint32_t pos);

int32_t TestGenerateKey(const struct HksBlob *keyAlias);

int32_t ConstructDataToCertChain(struct HksCertChain **certChain,
    const struct HksTestCertChain *certChainParam);

int32_t GenerateParamSet(struct HksParamSet **paramSet, const struct HksParam tmpParams[], uint32_t paramCount);

int32_t ValidateCertChainTest(const struct HksCertChain *certChain, const struct HksParam tmpParam[],
    ParamType type);
}

#endif
