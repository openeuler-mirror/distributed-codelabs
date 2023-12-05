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

#ifndef KEY_AGREE_SESSION_H
#define KEY_AGREE_SESSION_H

#include "device_auth_defines.h"
#include "json_utils.h"
#include "key_agree_sdk.h"
#include "pake_defs.h"

#define FIELD_SDK_ERROR_CODE "_ec"
#define FIELD_SDK_STEP "_st"
#define FIELD_SDK_PAYLOAD "_pd"
#define FIELD_SDK_SEND_TO_PEER "_d"
#define FIELD_SDK_VERSION "v"
#define FIELD_SDK_CURRENT_VERSION "cv"
#define FIELD_PEER_AUTH_ID "peerAuthId"
#define FIELD_KCF_DATA "kcfData"
#define FIELD_EPK "epk"
#define FIELD_SALT "salt"

#define CHECK_PTR_RETURN_ERROR_CODE(ptr, paramTag) \
    do { \
        if ((ptr) == NULL) { \
            LOGE("%s is null ptr", (paramTag)); \
            return HC_ERR_NULL_PTR; \
        } \
    } while (0)

typedef struct {
    uint64_t first;
    uint64_t second;
    uint64_t third;
} VersionStruct;

typedef enum {
    INITIAL,
    VERSION_CONFIRM,
    VERSION_DECIDED,
} VersionAgreementStatus;

typedef struct VersionInfoT {
    VersionAgreementStatus versionStatus;
    VersionStruct curVersion;
} VersionInfo;

typedef enum {
    SPEKE_MOD_NONE = 0x00000000,
    DL_SPEKE_MOD_256 = 0x00000001,
    DL_SPEKE_MOD_384 = 0x00000002,
    EC_SPEKE_P256 = 0x00000004,
    EC_SPEKE_X25519 = 0x00000008,
} ProtocolPrimeMod;

typedef enum {
    STEP_INIT = -1,
    STEP_ONE = 1,
    STEP_TWO = 2,
    STEP_THREE = 3,
    STEP_FOUR = 4,
} ProtocolStep;

typedef struct SpekeSessionT {
    PakeBaseParams baseParam;
    uint64_t sessionId;
    ProtocolStep step;
    char *extras;
    bool isFinish;
    KeyAgreeProtocol protocol;
    KeyAgreeType keyAgreeType;
    VersionInfo versionInfo;
    Uint8Buff deviceId;
    Uint8Buff sharedSecret;
    int32_t (*processProtocolAgree)(struct SpekeSessionT *spekeSession, const KeyAgreeBlob *in, KeyAgreeBlob *out);
    int32_t (*processSession)(struct SpekeSessionT *spekeSession, const KeyAgreeBlob *in, KeyAgreeBlob *out);
    int32_t (*initSpekeSession)(struct SpekeSessionT *spekeSession, KeyAgreeProtocol protocol);
    int32_t (*checkAndInitProtocol)(struct SpekeSessionT *spekeSession, KeyAgreeProtocol protocol);
} SpekeSession;

SpekeSession *CreateSpekeSession(void);
void DestroySpekeSession(SpekeSession *spekeSession);

#endif
