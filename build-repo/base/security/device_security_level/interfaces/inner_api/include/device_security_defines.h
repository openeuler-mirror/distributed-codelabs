/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DEVICE_SECURITY_DEFINES_H
#define DEVICE_SECURITY_DEFINES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEVICE_ID_MAX_LEN 64

typedef struct DeviceIdentify {
    uint32_t length;
    uint8_t identity[DEVICE_ID_MAX_LEN];
} DeviceIdentify;

typedef struct RequestOption {
    uint64_t challenge;
    uint32_t timeout;
    uint32_t extra;
} RequestOption;

#define DEFAULT_OPTION NULL

enum {
    SUCCESS = 0,
    ERR_INVALID_PARA = 1,
    ERR_INVALID_LEN_PARA = 2,
    ERR_NO_MEMORY = 3,
    ERR_MEMORY_ERR = 4,
    ERR_NO_CHALLENGE = 5,
    ERR_NO_CRED = 6,
    ERR_SA_BUSY = 7,
    ERR_TIMEOUT = 8,
    ERR_NOEXIST_REQUEST = 9,
    ERR_INVALID_VERSION = 10,
    ERR_OEM_ERR = 11,
    ERR_HUKS_ERR = 12,
    ERR_CHALLENGE_ERR = 13,
    ERR_NOT_ONLINE = 14,
    ERR_INIT_SELF_ERR = 15,
    ERR_JSON_ERR = 16,
    ERR_IPC_ERR = 17,
    ERR_IPC_REGISTER_ERR = 18,
    ERR_IPC_REMOTE_OBJ_ERR = 19,
    ERR_IPC_PROXY_ERR = 20,
    ERR_IPC_RET_PARCEL_ERR = 21,
    ERR_PROXY_REMOTE_ERR = 22,
    ERR_MSG_NEIGHBOR_FULL = 23,
    ERR_MSG_FULL = 24,
    ERR_MSG_ADD_NEIGHBOR = 25,
    ERR_MSG_NOT_INIT = 26,
    ERR_MSG_CREATE_WORKQUEUE = 27,
    ERR_NEED_COMPATIBLE = 28,
    ERR_REG_CALLBACK = 29,
    ERR_PERMISSION_DENIAL = 30,
    ERR_REQUEST_CODE_ERR = 31,
    ERR_VERIFY_MODE_CRED_ERR = 32,
    ERR_VERIFY_SIGNED_MODE_CRED_ERR = 33,
    ERR_VERIFY_MODE_HUKS_ERR = 34,
    ERR_PROFILE_CONNECT_ERR = 35,
    ERR_MSG_OPEN_SESSION = 36,
    ERR_QUERY_WAITING = 37,
    ERR_NOEXIST_DEVICE = 38,
    ERR_NOEXIST_COMMON_PK_INFO = 39,
    ERR_ECC_VERIFY_ERR = 40,
    ERR_GET_CLOUD_CRED_INFO = 41,
    ERR_CALL_EXTERNAL_FUNC = 42,
    ERR_PARSE_NONCE = 43,
    ERR_ROOT_PUBKEY_NOT_RIGHT = 44,
    ERR_PARSE_CLOUD_CRED_DATA = 45,
    ERR_PARSE_PUBKEY_CHAIN = 46,
    ERR_CHECK_CRED_INFO = 47,
    ERR_ATTEST_NOT_READY = 48,
    ERR_CLOUD_CRED_NOT_EXIST = 49,
    ERR_DEFAULT = 0xFFFF
};

#ifdef __cplusplus
}
#endif

#endif // DEVICE_SECURITY_DEFINES_H
