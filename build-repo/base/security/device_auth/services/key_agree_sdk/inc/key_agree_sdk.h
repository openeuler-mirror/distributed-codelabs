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

#ifndef KEY_AGREE_SDK_H
#define KEY_AGREE_SDK_H

#include <stdint.h>

typedef enum {
    KEYAGREE_TYPE_CLIENT = 1,
    KEYAGREE_TYPE_SERVER = 2,
} KeyAgreeType;

typedef struct {
    uint64_t sessionId;
} KeyAgreeSession;

typedef enum {
    KEYAGREE_PROTOCOL_DL_SPEKE_256 = 1,
    KEYAGREE_PROTOCOL_DL_SPEKE_384 = 2,
    KEYAGREE_PROTOCOL_EC_SPEKE_P256 = 3,
    KEYAGREE_PROTOCOL_EC_SPEKE_X25519 = 4,
    KEYAGREE_PROTOCOL_ANY = 5,
} KeyAgreeProtocol;

typedef enum {
    KEYAGREE_SUCCESS = 0,
    KEYAGREE_NOT_SUPPORTED = 1,
    KEYAGREE_BAD_PARAMS = 2,
    KEYAGREE_BAD_MESSAGE = 3,
    KEYAGREE_FAIL = 4,
    KEYAGREE_UNKNOWN = 5,

    // New errorCode
    KEYAGREE_INIT_BAD_PARAMS = 10601001,
    KEYAGREE_INIT_NOT_SUPPORTED = 10601002,
    KEYAGREE_INIT_ENTITIES_FAIL = 20601001,
    KEYAGREE_INIT_CREATE_SESSION_FAIL = 20601002,
    KEYAGREE_INIT_CREATE_SESSION_MGR_FAIL = 20601003,
    KEYAGREE_INIT_INIT_SESSION_FAIL = 20601004,
    KEYAGREE_INIT_ADD_SESSION_FAIL = 20601005,

    KEYAGREE_START_BAD_PARAMS = 10602001,
    KEYAGREE_START_INVAILD_LEN = 10602002,
    KEYAGREE_START_GET_SESSION_MGR_FAIL = 20602001,
    KEYAGREE_START_GET_SESSION_FAIL = 20602002,
    KEYAGREE_START_INIT_PARAMS_SHAREDSECRET_FAIL = 20602003,
    KEYAGREE_START_MEMORY_COPY_SHAREDSECRET_FAIL = 20602004,
    KEYAGREE_START_INIT_PARAMS_DEVID_FAIL = 20602005,
    KEYAGREE_START_MEMORY_COPY_DEVID_FAIL = 20602006,
    KEYAGREE_START_ALLOC_MEMORY_FAIL = 20602007,
    KEYAGREE_START_MEMORY_COPY_EXTRAS_FAIL = 20602008,

    KEYAGREE_PROCESS_BAD_PARAMS = 10603001,
    KEYAGREE_PROCESS_INVAILD_LEN = 10603002,
    KEYAGREE_PROCESS_GET_SESSION_MGR_FAIL = 20603001,
    KEYAGREE_PROCESS_GET_SESSION_FAIL = 20603002,
    KEYAGREE_PROCESS_PEER_IS_ONERROR = 30603001,
    KEYAGREE_PROCESS_PROTOCOL_AGREE_FAIL = 20603003,
    KEYAGREE_PROCESS_PROTOCOL_SESSION_FAIL = 20603004,

    KEYAGREE_IS_FINSIH_BAD_PARAMS = 10604001,
    KEYAGREE_IS_FINSIH_GET_SESSION_MGR_FAIL = 20604001,
    KEYAGREE_IS_FINSIH_GET_SESSION_FAIL = 20604002,
    KEYAGREE_IS_FINSIH_ERROR = 20604003,
    KEYAGREE_IS_FINSIH_NOT_FINSIH = 20604004,

    KEYAGREE_GET_RESULT_BAD_PARAMS = 10605001,
    KEYAGREE_GET_RESULT_INVAILD_LEN = 10605002,
    KEYAGREE_GET_RESULT_GET_SESSION_MGR_FAIL = 20605001,
    KEYAGREE_GET_RESULT_GET_SESSION_FAIL = 20605002,
    KEYAGREE_GET_RESULT_NOT_FINSIH = 20605003,
    KEYAGREE_GET_RESULT_MEMCPY_FAILED = 20605004
} KeyAgreeResult;

typedef struct {
    uint8_t *data;
    uint32_t length; // This length is the requested memory length
} KeyAgreeBlob;

#ifdef __cplusplus
extern "C" {
#endif
KeyAgreeResult KeyAgreeInitSession(KeyAgreeSession *session, KeyAgreeProtocol protocol, KeyAgreeType type);
KeyAgreeResult KeyAgreeStartSession(KeyAgreeSession *session, const KeyAgreeBlob *sharedSecret,
    const KeyAgreeBlob *deviceId, const char *extras);
KeyAgreeResult KeyAgreeGenerateNextMessage(KeyAgreeSession *session, const KeyAgreeBlob *messageReceived,
    KeyAgreeBlob *messageToTransmit);
KeyAgreeResult KeyAgreeIsFinish(KeyAgreeSession *session);
KeyAgreeResult KeyAgreeGetResult(KeyAgreeSession *session, KeyAgreeBlob *sessionKey);
void KeyAgreeFreeSession(KeyAgreeSession *session);
#ifdef __cplusplus
}
#endif

#endif