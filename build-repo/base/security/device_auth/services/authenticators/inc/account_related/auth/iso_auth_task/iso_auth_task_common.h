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

#ifndef ISO_AUTH_TASK_COMMON_H
#define ISO_AUTH_TASK_COMMON_H

#include "account_module_defines.h"
#include "account_task_main.h"
#include "account_version_util.h"
#include "iso_protocol_common.h"
#include "json_utils.h"

typedef struct {
    uint64_t versionNo;
    int32_t authForm;
    int32_t credentialType;
    int32_t localDevType;
    int32_t keyLength;
    uint8_t seed[SEED_SIZE];
    uint8_t hmacToken[HMAC_TOKEN_SIZE];
    Uint8Buff challenge;
    Uint8Buff devIdSelf;
    Uint8Buff devIdPeer;
    char *userIdSelf;
    char *userIdPeer;
    char *deviceIdSelf;
    char *deviceIdPeer;
    IsoBaseParams isoBaseParams;
} IsoAuthParams;

#ifdef __cplusplus
extern "C" {
#endif

bool IsIsoAuthTaskSupported(void);

TaskBase *CreateIsoAuthTask(const CJson *in, CJson *out, const AccountVersionInfo *verInfo);

int32_t InitIsoAuthParams(const CJson *in, IsoAuthParams *params, const AccountVersionInfo *verInfo);
void DestroyIsoAuthParams(IsoAuthParams *params);

int32_t AccountAuthGeneratePsk(IsoAuthParams *params);
int32_t ExtractAndVerifyPayload(IsoAuthParams *params, const CJson *in);
int32_t AuthIsoSendFinalToOut(IsoAuthParams *params, CJson *out);

#ifdef __cplusplus
}
#endif

#endif
