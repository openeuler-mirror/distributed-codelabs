/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "account_version_util.h"
#include "common_defs.h"
#include "hc_log.h"
#include "hc_types.h"
#include "iso_auth_task_common.h"
#include "pake_defs.h"
#include "pake_protocol_dl_common.h"
#include "pake_protocol_ec_common.h"
#include "pake_v2_auth_task_common.h"

#define IS_SUPPORT_CURVE_256 true
#define IS_SUPPORT_CURVE_25519 false

DECLARE_HC_VECTOR(AccountVersionInfoVec, void *)
IMPLEMENT_HC_VECTOR(AccountVersionInfoVec, void *, 1)

static AccountVersionInfoVec g_authVersionInfoVec;

static uint64_t g_authVersionNo = 0;

static bool IsAuthPakeV2EcP256Supported(void)
{
    return IsPakeV2AuthTaskSupported() && (GetPakeEcAlg() == PAKE_ALG_EC) && IS_SUPPORT_CURVE_256;
}

static bool IsAuthIsoSupported(void)
{
    return IsIsoAuthTaskSupported();
}

static AccountVersionInfo g_authVersionInfoAll[] = {
    { AUTH_PAKE_V2_EC_P256, PAKE_V2, PAKE_ALG_EC, CURVE_256, false, IsAuthPakeV2EcP256Supported, CreatePakeV2AuthTask},
    { AUTH_ISO, ISO, PAKE_ALG_NONE, CURVE_NONE, false, IsAuthIsoSupported, CreateIsoAuthTask }
};

void InitVersionInfos(void)
{
    g_authVersionInfoVec = CREATE_HC_VECTOR(AccountVersionInfoVec);
    uint32_t size = sizeof(g_authVersionInfoAll) / sizeof(AccountVersionInfo);
    for (uint32_t i = 0; i < size; i++) {
        if (!g_authVersionInfoAll[i].isTaskSupported()) {
            continue;
        }
        (void)g_authVersionInfoVec.pushBackT(&g_authVersionInfoVec, (void *)(&g_authVersionInfoAll[i]));
        g_authVersionNo |= g_authVersionInfoAll[i].versionNo;
    }
}

void DestroyVersionInfos(void)
{
    DESTROY_HC_VECTOR(AccountVersionInfoVec, &g_authVersionInfoVec);
}

static const AccountVersionInfo *NegotiateForAuth(int32_t credentialType)
{
    uint64_t versionNo;
    if (credentialType == SYMMETRIC_CRED) {
        versionNo = AUTH_ISO;
    } else if (credentialType == ASYMMETRIC_CRED) {
        versionNo = AUTH_PAKE_V2_EC_P256;
    } else {
        LOGE("Invalid credential type for auth: %d.", credentialType);
        return NULL;
    }
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_authVersionInfoVec, index, ptr) {
        if (ptr == NULL || *ptr == NULL) {
            continue;
        }
        AccountVersionInfo *temp = (AccountVersionInfo *)(*ptr);
        if ((temp->versionNo & versionNo) == versionNo) {
            return temp;
        }
    }
    LOGE("Version is not matched, failed to negotiate for account auth.");
    return NULL;
}

const AccountVersionInfo *GetNegotiatedVersionInfo(int32_t operationCode, int32_t credentialType)
{
    // Now, only support auth negotiate.
    if (operationCode != AUTHENTICATE) {
        LOGE("operationCode is not auth, not supported.");
        return NULL;
    }
    return NegotiateForAuth(credentialType);
}

uint64_t GetSupportedVersionNo(int32_t operationCode)
{
    (void)operationCode; // Now, only support auth negotiate.
    return g_authVersionNo;
}
