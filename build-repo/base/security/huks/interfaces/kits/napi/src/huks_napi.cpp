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

#include "huks_napi.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "hks_type.h"
#include "huks_napi_abort.h"
#include "huks_napi_abort_session.h"
#include "huks_napi_attest_key_item.h"
#include "huks_napi_delete_key.h"
#include "huks_napi_delete_key_item.h"
#include "huks_napi_export_key.h"
#include "huks_napi_export_key_item.h"
#include "huks_napi_generate_key.h"
#include "huks_napi_generate_key_item.h"
#include "huks_napi_get_key_properties.h"
#include "huks_napi_get_key_item_properties.h"
#include "huks_napi_get_sdk_version.h"
#include "huks_napi_import_key.h"
#include "huks_napi_import_key_item.h"
#include "huks_napi_import_wrapped_key_item.h"
#include "huks_napi_init.h"
#include "huks_napi_init_session.h"
#include "huks_napi_is_key_exist.h"
#include "huks_napi_is_key_item_exist.h"
#include "huks_napi_update_finish.h"
#include "huks_napi_update_finish_session.h"

namespace HuksNapi {
inline void AddInt32Property(napi_env env, napi_value object, const char *name, int32_t value)
{
    napi_value property = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, value, &property));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, object, name, property));
}

static void AddHuksTagPart1(napi_env env, napi_value tag)
{
    /* Invalid TAG */
    AddInt32Property(env, tag, "HUKS_TAG_INVALID", HKS_TAG_INVALID);

    /* Base algrithom TAG: 1 - 200 */
    AddInt32Property(env, tag, "HUKS_TAG_ALGORITHM", HKS_TAG_ALGORITHM);
    AddInt32Property(env, tag, "HUKS_TAG_PURPOSE", HKS_TAG_PURPOSE);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_SIZE", HKS_TAG_KEY_SIZE);
    AddInt32Property(env, tag, "HUKS_TAG_DIGEST", HKS_TAG_DIGEST);
    AddInt32Property(env, tag, "HUKS_TAG_PADDING", HKS_TAG_PADDING);
    AddInt32Property(env, tag, "HUKS_TAG_BLOCK_MODE", HKS_TAG_BLOCK_MODE);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_TYPE", HKS_TAG_KEY_TYPE);
    AddInt32Property(env, tag, "HUKS_TAG_ASSOCIATED_DATA", HKS_TAG_ASSOCIATED_DATA);
    AddInt32Property(env, tag, "HUKS_TAG_NONCE", HKS_TAG_NONCE);
    AddInt32Property(env, tag, "HUKS_TAG_IV", HKS_TAG_IV);

    /* Key derivation TAG */
    AddInt32Property(env, tag, "HUKS_TAG_INFO", HKS_TAG_INFO);
    AddInt32Property(env, tag, "HUKS_TAG_SALT", HKS_TAG_SALT);
    AddInt32Property(env, tag, "HUKS_TAG_PWD", HKS_TAG_PWD);
    AddInt32Property(env, tag, "HUKS_TAG_ITERATION", HKS_TAG_ITERATION);

    AddInt32Property(env, tag, "HUKS_TAG_KEY_GENERATE_TYPE", HKS_TAG_KEY_GENERATE_TYPE);
    AddInt32Property(env, tag, "HUKS_TAG_DERIVE_MAIN_KEY", HKS_TAG_DERIVE_MAIN_KEY);
    AddInt32Property(env, tag, "HUKS_TAG_DERIVE_FACTOR", HKS_TAG_DERIVE_FACTOR);
    AddInt32Property(env, tag, "HUKS_TAG_DERIVE_ALG", HKS_TAG_DERIVE_ALG);
    AddInt32Property(env, tag, "HUKS_TAG_AGREE_ALG", HKS_TAG_AGREE_ALG);
    AddInt32Property(env, tag, "HUKS_TAG_AGREE_PUBLIC_KEY_IS_KEY_ALIAS", HKS_TAG_AGREE_PUBLIC_KEY_IS_KEY_ALIAS);
    AddInt32Property(env, tag, "HUKS_TAG_AGREE_PRIVATE_KEY_ALIAS", HKS_TAG_AGREE_PRIVATE_KEY_ALIAS);
    AddInt32Property(env, tag, "HUKS_TAG_AGREE_PUBLIC_KEY", HKS_TAG_AGREE_PUBLIC_KEY);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_ALIAS", HKS_TAG_KEY_ALIAS);
    AddInt32Property(env, tag, "HUKS_TAG_DERIVE_KEY_SIZE", HKS_TAG_DERIVE_KEY_SIZE);

    /*
     * Key authentication related TAG: 201 - 300
     *
     * Start of validity
     */
    AddInt32Property(env, tag, "HUKS_TAG_ACTIVE_DATETIME", HKS_TAG_ACTIVE_DATETIME);

    /* Date when new "messages" should not be created. */
    AddInt32Property(env, tag, "HUKS_TAG_ORIGINATION_EXPIRE_DATETIME", HKS_TAG_ORIGINATION_EXPIRE_DATETIME);

    /* Date when existing "messages" should not be used. */
    AddInt32Property(env, tag, "HUKS_TAG_USAGE_EXPIRE_DATETIME", HKS_TAG_USAGE_EXPIRE_DATETIME);

    /* Key creation time */
    AddInt32Property(env, tag, "HUKS_TAG_CREATION_DATETIME", HKS_TAG_CREATION_DATETIME);

    /* Other authentication related TAG: 301 - 500 */
    AddInt32Property(env, tag, "HUKS_TAG_ALL_USERS", HKS_TAG_ALL_USERS);
    AddInt32Property(env, tag, "HUKS_TAG_USER_ID", HKS_TAG_USER_ID);
    AddInt32Property(env, tag, "HUKS_TAG_NO_AUTH_REQUIRED", HKS_TAG_NO_AUTH_REQUIRED);
    AddInt32Property(env, tag, "HUKS_TAG_USER_AUTH_TYPE", HKS_TAG_USER_AUTH_TYPE);
    AddInt32Property(env, tag, "HUKS_TAG_AUTH_TIMEOUT", HKS_TAG_AUTH_TIMEOUT);
    AddInt32Property(env, tag, "HUKS_TAG_AUTH_TOKEN", HKS_TAG_AUTH_TOKEN);

    /* Attestation related TAG: 501 - 600 */
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_CHALLENGE", HKS_TAG_ATTESTATION_CHALLENGE);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_APPLICATION_ID", HKS_TAG_ATTESTATION_APPLICATION_ID);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_BRAND", HKS_TAG_ATTESTATION_ID_BRAND);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_DEVICE", HKS_TAG_ATTESTATION_ID_DEVICE);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_PRODUCT", HKS_TAG_ATTESTATION_ID_PRODUCT);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_SERIAL", HKS_TAG_ATTESTATION_ID_SERIAL);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_IMEI", HKS_TAG_ATTESTATION_ID_IMEI);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_MEID", HKS_TAG_ATTESTATION_ID_MEID);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_MANUFACTURER", HKS_TAG_ATTESTATION_ID_MANUFACTURER);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_MODEL", HKS_TAG_ATTESTATION_ID_MODEL);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_ALIAS", HKS_TAG_ATTESTATION_ID_ALIAS);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_SOCID", HKS_TAG_ATTESTATION_ID_SOCID);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_UDID", HKS_TAG_ATTESTATION_ID_UDID);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO", HKS_TAG_ATTESTATION_ID_SEC_LEVEL_INFO);
    AddInt32Property(env, tag, "HUKS_TAG_ATTESTATION_ID_VERSION_INFO", HKS_TAG_ATTESTATION_ID_VERSION_INFO);
}

static void AddHuksTagPart2(napi_env env, napi_value tag)
{
    /*
     * Other reserved TAG: 601 - 1000
     *
     * Extention TAG: 1001 - 9999
     */
    AddInt32Property(env, tag, "HUKS_TAG_IS_KEY_ALIAS", HKS_TAG_IS_KEY_ALIAS);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_STORAGE_FLAG", HKS_TAG_KEY_STORAGE_FLAG);
    AddInt32Property(env, tag, "HUKS_TAG_IS_ALLOWED_WRAP", HKS_TAG_IS_ALLOWED_WRAP);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_WRAP_TYPE", HKS_TAG_KEY_WRAP_TYPE);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_AUTH_ID", HKS_TAG_KEY_AUTH_ID);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_ROLE", HKS_TAG_KEY_ROLE);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_FLAG", HKS_TAG_KEY_FLAG);
    AddInt32Property(env, tag, "HUKS_TAG_IS_ASYNCHRONIZED", HKS_TAG_IS_ASYNCHRONIZED);
    AddInt32Property(env, tag, "HUKS_TAG_SECURE_KEY_ALIAS", HKS_TAG_SECURE_KEY_ALIAS);
    AddInt32Property(env, tag, "HUKS_TAG_SECURE_KEY_UUID", HKS_TAG_SECURE_KEY_UUID);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_DOMAIN", HKS_TAG_KEY_DOMAIN);

    /* Inner-use TAG: 10001 - 10999 */
    AddInt32Property(env, tag, "HUKS_TAG_PROCESS_NAME", HKS_TAG_PROCESS_NAME);
    AddInt32Property(env, tag, "HUKS_TAG_PACKAGE_NAME", HKS_TAG_PACKAGE_NAME);
    AddInt32Property(env, tag, "HUKS_TAG_ACCESS_TIME", HKS_TAG_ACCESS_TIME);
    AddInt32Property(env, tag, "HUKS_TAG_USES_TIME", HKS_TAG_USES_TIME);
    AddInt32Property(env, tag, "HUKS_TAG_CRYPTO_CTX", HKS_TAG_CRYPTO_CTX);
    AddInt32Property(env, tag, "HUKS_TAG_KEY", HKS_TAG_KEY);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_VERSION", HKS_TAG_KEY_VERSION);
    AddInt32Property(env, tag, "HUKS_TAG_PAYLOAD_LEN", HKS_TAG_PAYLOAD_LEN);
    AddInt32Property(env, tag, "HUKS_TAG_AE_TAG", HKS_TAG_AE_TAG);
    AddInt32Property(env, tag, "HUKS_TAG_IS_KEY_HANDLE", HKS_TAG_IS_KEY_HANDLE);

    /* Os version related TAG */
    AddInt32Property(env, tag, "HUKS_TAG_OS_VERSION", HKS_TAG_OS_VERSION);
    AddInt32Property(env, tag, "HUKS_TAG_OS_PATCHLEVEL", HKS_TAG_OS_PATCHLEVEL);

    /*
     * Reversed TAGs for SOTER: 11000 - 12000
     *
     * Other TAGs: 20001 - N
     * TAGs used for paramSetOut
     */
    AddInt32Property(env, tag, "HUKS_TAG_SYMMETRIC_KEY_DATA", HKS_TAG_SYMMETRIC_KEY_DATA);
    AddInt32Property(env, tag, "HUKS_TAG_ASYMMETRIC_PUBLIC_KEY_DATA", HKS_TAG_ASYMMETRIC_PUBLIC_KEY_DATA);
    AddInt32Property(env, tag, "HUKS_TAG_ASYMMETRIC_PRIVATE_KEY_DATA", HKS_TAG_ASYMMETRIC_PRIVATE_KEY_DATA);
    AddInt32Property(env, tag, "HUKS_TAG_IMPORT_KEY_TYPE", HKS_TAG_IMPORT_KEY_TYPE);
    AddInt32Property(env, tag, "HUKS_TAG_UNWRAP_ALGORITHM_SUITE", HKS_TAG_UNWRAP_ALGORITHM_SUITE);

    AddInt32Property(env, tag, "HUKS_TAG_KEY_AUTH_ACCESS_TYPE", HKS_TAG_KEY_AUTH_ACCESS_TYPE);
    AddInt32Property(env, tag, "HUKS_TAG_KEY_SECURE_SIGN_TYPE", HKS_TAG_KEY_SECURE_SIGN_TYPE);
    AddInt32Property(env, tag, "HUKS_TAG_CHALLENGE_TYPE", HKS_TAG_CHALLENGE_TYPE);
    AddInt32Property(env, tag, "HUKS_TAG_CHALLENGE_POS", HKS_TAG_CHALLENGE_POS);
}

static napi_value CreateHuksTag(napi_env env)
{
    napi_value tag = nullptr;
    NAPI_CALL(env, napi_create_object(env, &tag));

    AddHuksTagPart1(env, tag);
    AddHuksTagPart2(env, tag);

    return tag;
}

static napi_value CreateHuksKeySize(napi_env env)
{
    napi_value keySize = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keySize));

    AddInt32Property(env, keySize, "HUKS_RSA_KEY_SIZE_512", HKS_RSA_KEY_SIZE_512);
    AddInt32Property(env, keySize, "HUKS_RSA_KEY_SIZE_768", HKS_RSA_KEY_SIZE_768);
    AddInt32Property(env, keySize, "HUKS_RSA_KEY_SIZE_1024", HKS_RSA_KEY_SIZE_1024);
    AddInt32Property(env, keySize, "HUKS_RSA_KEY_SIZE_2048", HKS_RSA_KEY_SIZE_2048);
    AddInt32Property(env, keySize, "HUKS_RSA_KEY_SIZE_3072", HKS_RSA_KEY_SIZE_3072);
    AddInt32Property(env, keySize, "HUKS_RSA_KEY_SIZE_4096", HKS_RSA_KEY_SIZE_4096);

    AddInt32Property(env, keySize, "HUKS_ECC_KEY_SIZE_224", HKS_ECC_KEY_SIZE_224);
    AddInt32Property(env, keySize, "HUKS_ECC_KEY_SIZE_256", HKS_ECC_KEY_SIZE_256);
    AddInt32Property(env, keySize, "HUKS_ECC_KEY_SIZE_384", HKS_ECC_KEY_SIZE_384);
    AddInt32Property(env, keySize, "HUKS_ECC_KEY_SIZE_521", HKS_ECC_KEY_SIZE_521);

    AddInt32Property(env, keySize, "HUKS_AES_KEY_SIZE_128", HKS_AES_KEY_SIZE_128);
    AddInt32Property(env, keySize, "HUKS_AES_KEY_SIZE_192", HKS_AES_KEY_SIZE_192);
    AddInt32Property(env, keySize, "HUKS_AES_KEY_SIZE_256", HKS_AES_KEY_SIZE_256);
    AddInt32Property(env, keySize, "HUKS_AES_KEY_SIZE_512", HKS_AES_KEY_SIZE_512);

    AddInt32Property(env, keySize, "HUKS_CURVE25519_KEY_SIZE_256", HKS_CURVE25519_KEY_SIZE_256);

    AddInt32Property(env, keySize, "HUKS_DH_KEY_SIZE_2048", HKS_DH_KEY_SIZE_2048);
    AddInt32Property(env, keySize, "HUKS_DH_KEY_SIZE_3072", HKS_DH_KEY_SIZE_3072);
    AddInt32Property(env, keySize, "HUKS_DH_KEY_SIZE_4096", HKS_DH_KEY_SIZE_4096);

    AddInt32Property(env, keySize, "HUKS_SM2_KEY_SIZE_256", HKS_SM2_KEY_SIZE_256);
    AddInt32Property(env, keySize, "HUKS_SM4_KEY_SIZE_128", HKS_SM4_KEY_SIZE_128);

    return keySize;
}

static napi_value CreateHuksKeyAlg(napi_env env)
{
    napi_value keyAlg = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keyAlg));

    AddInt32Property(env, keyAlg, "HUKS_ALG_RSA", HKS_ALG_RSA);
    AddInt32Property(env, keyAlg, "HUKS_ALG_ECC", HKS_ALG_ECC);
    AddInt32Property(env, keyAlg, "HUKS_ALG_DSA", HKS_ALG_DSA);

    AddInt32Property(env, keyAlg, "HUKS_ALG_AES", HKS_ALG_AES);
    AddInt32Property(env, keyAlg, "HUKS_ALG_HMAC", HKS_ALG_HMAC);
    AddInt32Property(env, keyAlg, "HUKS_ALG_HKDF", HKS_ALG_HKDF);
    AddInt32Property(env, keyAlg, "HUKS_ALG_PBKDF2", HKS_ALG_PBKDF2);

    AddInt32Property(env, keyAlg, "HUKS_ALG_ECDH", HKS_ALG_ECDH);
    AddInt32Property(env, keyAlg, "HUKS_ALG_X25519", HKS_ALG_X25519);
    AddInt32Property(env, keyAlg, "HUKS_ALG_ED25519", HKS_ALG_ED25519);
    AddInt32Property(env, keyAlg, "HUKS_ALG_DH", HKS_ALG_DH);

    AddInt32Property(env, keyAlg, "HUKS_ALG_SM2", HKS_ALG_SM2);
    AddInt32Property(env, keyAlg, "HUKS_ALG_SM3", HKS_ALG_SM3);
    AddInt32Property(env, keyAlg, "HUKS_ALG_SM4", HKS_ALG_SM4);

    return keyAlg;
}

static napi_value CreateHuksKeyPurpose(napi_env env)
{
    napi_value keyPurpose = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keyPurpose));

    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_ENCRYPT", HKS_KEY_PURPOSE_ENCRYPT);
    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_DECRYPT", HKS_KEY_PURPOSE_DECRYPT);
    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_SIGN", HKS_KEY_PURPOSE_SIGN);
    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_VERIFY", HKS_KEY_PURPOSE_VERIFY);
    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_DERIVE", HKS_KEY_PURPOSE_DERIVE);
    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_WRAP", HKS_KEY_PURPOSE_WRAP);
    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_UNWRAP", HKS_KEY_PURPOSE_UNWRAP);
    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_MAC", HKS_KEY_PURPOSE_MAC);
    AddInt32Property(env, keyPurpose, "HUKS_KEY_PURPOSE_AGREE", HKS_KEY_PURPOSE_AGREE);

    return keyPurpose;
}

static napi_value CreateHuksKeyPadding(napi_env env)
{
    napi_value keyPadding = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keyPadding));

    AddInt32Property(env, keyPadding, "HUKS_PADDING_NONE", HKS_PADDING_NONE);
    AddInt32Property(env, keyPadding, "HUKS_PADDING_OAEP", HKS_PADDING_OAEP);
    AddInt32Property(env, keyPadding, "HUKS_PADDING_PSS", HKS_PADDING_PSS);
    AddInt32Property(env, keyPadding, "HUKS_PADDING_PKCS1_V1_5", HKS_PADDING_PKCS1_V1_5);
    AddInt32Property(env, keyPadding, "HUKS_PADDING_PKCS5", HKS_PADDING_PKCS5);
    AddInt32Property(env, keyPadding, "HUKS_PADDING_PKCS7", HKS_PADDING_PKCS7);

    return keyPadding;
}

static napi_value CreateHuksCipherMode(napi_env env)
{
    napi_value keyCipherMode = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keyCipherMode));

    AddInt32Property(env, keyCipherMode, "HUKS_MODE_ECB", HKS_MODE_ECB);
    AddInt32Property(env, keyCipherMode, "HUKS_MODE_CBC", HKS_MODE_CBC);
    AddInt32Property(env, keyCipherMode, "HUKS_MODE_CTR", HKS_MODE_CTR);
    AddInt32Property(env, keyCipherMode, "HUKS_MODE_OFB", HKS_MODE_OFB);
    AddInt32Property(env, keyCipherMode, "HUKS_MODE_CCM", HKS_MODE_CCM);
    AddInt32Property(env, keyCipherMode, "HUKS_MODE_GCM", HKS_MODE_GCM);

    return keyCipherMode;
}

static napi_value CreateHuksKeyStorageType(napi_env env)
{
    napi_value keyStorageType = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keyStorageType));

    AddInt32Property(env, keyStorageType, "HUKS_STORAGE_TEMP", HKS_STORAGE_TEMP);
    AddInt32Property(env, keyStorageType, "HUKS_STORAGE_PERSISTENT", HKS_STORAGE_PERSISTENT);

    return keyStorageType;
}

static napi_value CreateHuksUnwrapSuite(napi_env env)
{
    napi_value huksUnwrapSuite = nullptr;
    NAPI_CALL(env, napi_create_object(env, &huksUnwrapSuite));

    AddInt32Property(env, huksUnwrapSuite, "HUKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING",
                     HKS_UNWRAP_SUITE_X25519_AES_256_GCM_NOPADDING);
    AddInt32Property(env, huksUnwrapSuite, "HUKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING",
                     HKS_UNWRAP_SUITE_ECDH_AES_256_GCM_NOPADDING);
    return huksUnwrapSuite;
}

static void AddHuksErrorCodePart1(napi_env env, napi_value errorCode)
{
    AddInt32Property(env, errorCode, "HUKS_SUCCESS", HKS_SUCCESS);
    AddInt32Property(env, errorCode, "HUKS_FAILURE", HKS_FAILURE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_BAD_STATE", HKS_ERROR_BAD_STATE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_ARGUMENT", HKS_ERROR_INVALID_ARGUMENT);
    AddInt32Property(env, errorCode, "HUKS_ERROR_NOT_SUPPORTED", HKS_ERROR_NOT_SUPPORTED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_NO_PERMISSION", HKS_ERROR_NO_PERMISSION);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INSUFFICIENT_DATA", HKS_ERROR_INSUFFICIENT_DATA);
    AddInt32Property(env, errorCode, "HUKS_ERROR_BUFFER_TOO_SMALL", HKS_ERROR_BUFFER_TOO_SMALL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INSUFFICIENT_MEMORY", HKS_ERROR_INSUFFICIENT_MEMORY);
    AddInt32Property(env, errorCode, "HUKS_ERROR_COMMUNICATION_FAILURE", HKS_ERROR_COMMUNICATION_FAILURE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_STORAGE_FAILURE", HKS_ERROR_STORAGE_FAILURE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_HARDWARE_FAILURE", HKS_ERROR_HARDWARE_FAILURE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_ALREADY_EXISTS", HKS_ERROR_ALREADY_EXISTS);
    AddInt32Property(env, errorCode, "HUKS_ERROR_NOT_EXIST", HKS_ERROR_NOT_EXIST);
    AddInt32Property(env, errorCode, "HUKS_ERROR_NULL_POINTER", HKS_ERROR_NULL_POINTER);
    AddInt32Property(env, errorCode, "HUKS_ERROR_FILE_SIZE_FAIL", HKS_ERROR_FILE_SIZE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_READ_FILE_FAIL", HKS_ERROR_READ_FILE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_PUBLIC_KEY", HKS_ERROR_INVALID_PUBLIC_KEY);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_PRIVATE_KEY", HKS_ERROR_INVALID_PRIVATE_KEY);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_KEY_INFO", HKS_ERROR_INVALID_KEY_INFO);
    AddInt32Property(env, errorCode, "HUKS_ERROR_HASH_NOT_EQUAL", HKS_ERROR_HASH_NOT_EQUAL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_MALLOC_FAIL", HKS_ERROR_MALLOC_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_WRITE_FILE_FAIL", HKS_ERROR_WRITE_FILE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_REMOVE_FILE_FAIL", HKS_ERROR_REMOVE_FILE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_OPEN_FILE_FAIL", HKS_ERROR_OPEN_FILE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CLOSE_FILE_FAIL", HKS_ERROR_CLOSE_FILE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_MAKE_DIR_FAIL", HKS_ERROR_MAKE_DIR_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_KEY_FILE", HKS_ERROR_INVALID_KEY_FILE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_IPC_MSG_FAIL", HKS_ERROR_IPC_MSG_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_REQUEST_OVERFLOWS", HKS_ERROR_REQUEST_OVERFLOWS);
    AddInt32Property(env, errorCode, "HUKS_ERROR_PARAM_NOT_EXIST", HKS_ERROR_PARAM_NOT_EXIST);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CRYPTO_ENGINE_ERROR", HKS_ERROR_CRYPTO_ENGINE_ERROR);
    AddInt32Property(env, errorCode, "HUKS_ERROR_COMMUNICATION_TIMEOUT", HKS_ERROR_COMMUNICATION_TIMEOUT);
    AddInt32Property(env, errorCode, "HUKS_ERROR_IPC_INIT_FAIL", HKS_ERROR_IPC_INIT_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_IPC_DLOPEN_FAIL", HKS_ERROR_IPC_DLOPEN_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_EFUSE_READ_FAIL", HKS_ERROR_EFUSE_READ_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_NEW_ROOT_KEY_MATERIAL_EXIST", HKS_ERROR_NEW_ROOT_KEY_MATERIAL_EXIST);
    AddInt32Property(env, errorCode, "HUKS_ERROR_UPDATE_ROOT_KEY_MATERIAL_FAIL",
        HKS_ERROR_UPDATE_ROOT_KEY_MATERIAL_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_VERIFICATION_FAILED", HKS_ERROR_VERIFICATION_FAILED);
}

static void AddHuksErrorCodePart2(napi_env env, napi_value errorCode)
{
    AddInt32Property(env, errorCode, "HUKS_ERROR_GET_USERIAM_SECINFO_FAILED", HKS_ERROR_GET_USERIAM_SECINFO_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_GET_USERIAM_AUTHINFO_FAILED", HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_USER_AUTH_TYPE_NOT_SUPPORT", HKS_ERROR_USER_AUTH_TYPE_NOT_SUPPORT);
    AddInt32Property(env, errorCode, "HUKS_ERROR_KEY_AUTH_FAILED", HKS_ERROR_KEY_AUTH_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_DEVICE_NO_CREDENTIAL", HKS_ERROR_DEVICE_NO_CREDENTIAL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_API_NOT_SUPPORTED", HKS_ERROR_API_NOT_SUPPORTED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_KEY_AUTH_PERMANENTLY_INVALIDATED",
        HKS_ERROR_KEY_AUTH_PERMANENTLY_INVALIDATED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_KEY_AUTH_VERIFY_FAILED", HKS_ERROR_KEY_AUTH_VERIFY_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_KEY_AUTH_TIME_OUT", HKS_ERROR_KEY_AUTH_TIME_OUT);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_CREDENTIAL_NOT_EXIST", HKS_ERROR_CREDENTIAL_NOT_EXIST);

    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_ALG_FAIL", HKS_ERROR_CHECK_GET_ALG_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_KEY_SIZE_FAIL", HKS_ERROR_CHECK_GET_KEY_SIZE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_PADDING_FAIL", HKS_ERROR_CHECK_GET_PADDING_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_PURPOSE_FAIL", HKS_ERROR_CHECK_GET_PURPOSE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_DIGEST_FAIL", HKS_ERROR_CHECK_GET_DIGEST_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_MODE_FAIL", HKS_ERROR_CHECK_GET_MODE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_NONCE_FAIL", HKS_ERROR_CHECK_GET_NONCE_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_AAD_FAIL", HKS_ERROR_CHECK_GET_AAD_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_IV_FAIL", HKS_ERROR_CHECK_GET_IV_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_AE_TAG_FAIL", HKS_ERROR_CHECK_GET_AE_TAG_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_SALT_FAIL", HKS_ERROR_CHECK_GET_SALT_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_ITERATION_FAIL", HKS_ERROR_CHECK_GET_ITERATION_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_ALGORITHM", HKS_ERROR_INVALID_ALGORITHM);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_KEY_SIZE", HKS_ERROR_INVALID_KEY_SIZE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_PADDING", HKS_ERROR_INVALID_PADDING);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_PURPOSE", HKS_ERROR_INVALID_PURPOSE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_MODE", HKS_ERROR_INVALID_MODE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_DIGEST", HKS_ERROR_INVALID_DIGEST);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_SIGNATURE_SIZE", HKS_ERROR_INVALID_SIGNATURE_SIZE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_IV", HKS_ERROR_INVALID_IV);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_AAD", HKS_ERROR_INVALID_AAD);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_NONCE", HKS_ERROR_INVALID_NONCE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_AE_TAG", HKS_ERROR_INVALID_AE_TAG);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_SALT", HKS_ERROR_INVALID_SALT);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_ITERATION", HKS_ERROR_INVALID_ITERATION);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_OPERATION", HKS_ERROR_INVALID_OPERATION);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_WRAPPED_FORMAT", HKS_ERROR_INVALID_WRAPPED_FORMAT);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_USAGE_OF_KEY", HKS_ERROR_INVALID_USAGE_OF_KEY);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INTERNAL_ERROR", HKS_ERROR_INTERNAL_ERROR);
    AddInt32Property(env, errorCode, "HUKS_ERROR_UNKNOWN_ERROR", HKS_ERROR_UNKNOWN_ERROR);
}

static void AddHuksErrorCodePart3(napi_env env, napi_value errorCode)
{
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_AUTH_TYP_FAILED", HKS_ERROR_CHECK_GET_AUTH_TYP_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_CHALLENGE_TYPE_FAILED",
        HKS_ERROR_CHECK_GET_CHALLENGE_TYPE_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_ACCESS_TYPE_FAILED",
        HKS_ERROR_CHECK_GET_ACCESS_TYPE_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_CHECK_GET_AUTH_TOKEN_FAILED", HKS_ERROR_CHECK_GET_AUTH_TOKEN_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_TIME_OUT", HKS_ERROR_INVALID_TIME_OUT);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_AUTH_TYPE", HKS_ERROR_INVALID_AUTH_TYPE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_CHALLENGE_TYPE", HKS_ERROR_INVALID_CHALLENGE_TYPE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_ACCESS_TYPE", HKS_ERROR_INVALID_ACCESS_TYPE);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_AUTH_TOKEN", HKS_ERROR_INVALID_AUTH_TOKEN);
    AddInt32Property(env, errorCode, "HUKS_ERROR_INVALID_SECURE_SIGN_TYPE", HKS_ERROR_INVALID_SECURE_SIGN_TYPE);
}

static napi_value CreateHuksErrorCode(napi_env env)
{
    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_object(env, &errorCode));

    AddHuksErrorCodePart1(env, errorCode);
    AddHuksErrorCodePart2(env, errorCode);
    AddHuksErrorCodePart3(env, errorCode);

    return errorCode;
}

static void AddHuksErrCodePart(napi_env env, napi_value errorCode)
{
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_PERMISSION_FAIL", HUKS_ERR_CODE_PERMISSION_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_ILLEGAL_ARGUMENT", HUKS_ERR_CODE_ILLEGAL_ARGUMENT);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_NOT_SUPPORTED_API", HUKS_ERR_CODE_NOT_SUPPORTED_API);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_FEATURE_NOT_SUPPORTED", HUKS_ERR_CODE_FEATURE_NOT_SUPPORTED);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT",
        HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT",
        HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_FILE_OPERATION_FAIL", HUKS_ERR_CODE_FILE_OPERATION_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_COMMUNICATION_FAIL", HUKS_ERR_CODE_COMMUNICATION_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_CRYPTO_FAIL", HUKS_ERR_CODE_CRYPTO_FAIL);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_KEY_AUTH_PERMANENTLY_INVALIDATED",
        HUKS_ERR_CODE_KEY_AUTH_PERMANENTLY_INVALIDATED);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_KEY_AUTH_VERIFY_FAILED", HUKS_ERR_CODE_KEY_AUTH_VERIFY_FAILED);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_KEY_AUTH_TIME_OUT", HUKS_ERR_CODE_KEY_AUTH_TIME_OUT);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_SESSION_LIMIT", HUKS_ERR_CODE_SESSION_LIMIT);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_ITEM_NOT_EXIST", HUKS_ERR_CODE_ITEM_NOT_EXIST);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_EXTERNAL_ERROR", HUKS_ERR_CODE_EXTERNAL_ERROR);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_CREDENTIAL_NOT_EXIST", HUKS_ERR_CODE_CREDENTIAL_NOT_EXIST);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_INSUFFICIENT_MEMORY", HUKS_ERR_CODE_INSUFFICIENT_MEMORY);
    AddInt32Property(env, errorCode, "HUKS_ERR_CODE_CALL_SERVICE_FAILED", HUKS_ERR_CODE_CALL_SERVICE_FAILED);
}

static napi_value CreateHuksErrCode(napi_env env)
{
    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_object(env, &errorCode));

    AddHuksErrCodePart(env, errorCode);

    return errorCode;
}

static napi_value CreateHuksKeyDigest(napi_env env)
{
    napi_value keyDisgest = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keyDisgest));

    AddInt32Property(env, keyDisgest, "HUKS_DIGEST_NONE", HKS_DIGEST_NONE);
    AddInt32Property(env, keyDisgest, "HUKS_DIGEST_MD5", HKS_DIGEST_MD5);
    AddInt32Property(env, keyDisgest, "HUKS_DIGEST_SHA1", HKS_DIGEST_SHA1);
    AddInt32Property(env, keyDisgest, "HUKS_DIGEST_SHA224", HKS_DIGEST_SHA224);
    AddInt32Property(env, keyDisgest, "HUKS_DIGEST_SHA256", HKS_DIGEST_SHA256);
    AddInt32Property(env, keyDisgest, "HUKS_DIGEST_SHA384", HKS_DIGEST_SHA384);
    AddInt32Property(env, keyDisgest, "HUKS_DIGEST_SHA512", HKS_DIGEST_SHA512);
    AddInt32Property(env, keyDisgest, "HUKS_DIGEST_SM3", HKS_DIGEST_SM3);

    return keyDisgest;
}

static napi_value CreateHuksKeyGenerateType(napi_env env)
{
    napi_value keyGenerateType = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keyGenerateType));

    AddInt32Property(env, keyGenerateType, "HUKS_KEY_GENERATE_TYPE_DEFAULT", HKS_KEY_GENERATE_TYPE_DEFAULT);
    AddInt32Property(env, keyGenerateType, "HUKS_KEY_GENERATE_TYPE_DERIVE", HKS_KEY_GENERATE_TYPE_DERIVE);
    AddInt32Property(env, keyGenerateType, "HUKS_KEY_GENERATE_TYPE_AGREE", HKS_KEY_GENERATE_TYPE_AGREE);

    return keyGenerateType;
}

static napi_value CreateHuksKeyFlag(napi_env env)
{
    napi_value keyFlag = nullptr;
    NAPI_CALL(env, napi_create_object(env, &keyFlag));

    AddInt32Property(env, keyFlag, "HUKS_KEY_FLAG_IMPORT_KEY", HKS_KEY_FLAG_IMPORT_KEY);
    AddInt32Property(env, keyFlag, "HUKS_KEY_FLAG_GENERATE_KEY", HKS_KEY_FLAG_GENERATE_KEY);
    AddInt32Property(env, keyFlag, "HUKS_KEY_FLAG_AGREE_KEY", HKS_KEY_FLAG_AGREE_KEY);
    AddInt32Property(env, keyFlag, "HUKS_KEY_FLAG_DERIVE_KEY", HKS_KEY_FLAG_DERIVE_KEY);

    return keyFlag;
}

static napi_value CreateHuksTagType(napi_env env)
{
    napi_value tagType = nullptr;
    NAPI_CALL(env, napi_create_object(env, &tagType));

    AddInt32Property(env, tagType, "HUKS_TAG_TYPE_INVALID", HKS_TAG_TYPE_INVALID);
    AddInt32Property(env, tagType, "HUKS_TAG_TYPE_INT", HKS_TAG_TYPE_INT);
    AddInt32Property(env, tagType, "HUKS_TAG_TYPE_UINT", HKS_TAG_TYPE_UINT);
    AddInt32Property(env, tagType, "HUKS_TAG_TYPE_ULONG", HKS_TAG_TYPE_ULONG);
    AddInt32Property(env, tagType, "HUKS_TAG_TYPE_BOOL", HKS_TAG_TYPE_BOOL);
    AddInt32Property(env, tagType, "HUKS_TAG_TYPE_BYTES", HKS_TAG_TYPE_BYTES);

    return tagType;
}

static napi_value CreateHuksImportKeyType(napi_env env)
{
    napi_value ImportKeyType = nullptr;
    NAPI_CALL(env, napi_create_object(env, &ImportKeyType));

    AddInt32Property(env, ImportKeyType, "HUKS_KEY_TYPE_PUBLIC_KEY", HKS_KEY_TYPE_PUBLIC_KEY);
    AddInt32Property(env, ImportKeyType, "HUKS_KEY_TYPE_PRIVATE_KEY", HKS_KEY_TYPE_PRIVATE_KEY);
    AddInt32Property(env, ImportKeyType, "HUKS_KEY_TYPE_KEY_PAIR", HKS_KEY_TYPE_KEY_PAIR);

    return ImportKeyType;
}

static napi_value CreateHuksSendType(napi_env env)
{
    napi_value sendType = nullptr;
    NAPI_CALL(env, napi_create_object(env, &sendType));

    AddInt32Property(env, sendType, "HUKS_SEND_TYPE_ASYNC", HKS_SEND_TYPE_ASYNC);
    AddInt32Property(env, sendType, "HUKS_SEND_TYPE_SYNC", HKS_SEND_TYPE_SYNC);

    return sendType;
}

static napi_value CreateHuksUserAuthType(napi_env env)
{
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_object(env, &value));

    AddInt32Property(env, value, "HUKS_USER_AUTH_TYPE_FINGERPRINT", HKS_USER_AUTH_TYPE_FINGERPRINT);
    AddInt32Property(env, value, "HUKS_USER_AUTH_TYPE_FACE", HKS_USER_AUTH_TYPE_FACE);
    AddInt32Property(env, value, "HUKS_USER_AUTH_TYPE_PIN", HKS_USER_AUTH_TYPE_PIN);

    return value;
}

static napi_value CreateHuksAuthAccessType(napi_env env)
{
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_object(env, &value));

    AddInt32Property(env, value, "HUKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD", HKS_AUTH_ACCESS_INVALID_CLEAR_PASSWORD);
    AddInt32Property(env, value, "HUKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL", HKS_AUTH_ACCESS_INVALID_NEW_BIO_ENROLL);

    return value;
}

static napi_value CreateHuksChallengeType(napi_env env)
{
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_object(env, &value));

    AddInt32Property(env, value, "HUKS_CHALLENGE_TYPE_NORMAL", HKS_CHALLENGE_TYPE_NORMAL);
    AddInt32Property(env, value, "HUKS_CHALLENGE_TYPE_CUSTOM", HKS_CHALLENGE_TYPE_CUSTOM);
    AddInt32Property(env, value, "HUKS_CHALLENGE_TYPE_NONE", HKS_CHALLENGE_TYPE_NONE);

    return value;
}

static napi_value CreateHuksChallengePosition(napi_env env)
{
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_object(env, &value));

    AddInt32Property(env, value, "HUKS_CHALLENGE_POS_0", HKS_CHALLENGE_POS_0);
    AddInt32Property(env, value, "HUKS_CHALLENGE_POS_1", HKS_CHALLENGE_POS_1);
    AddInt32Property(env, value, "HUKS_CHALLENGE_POS_2", HKS_CHALLENGE_POS_2);
    AddInt32Property(env, value, "HUKS_CHALLENGE_POS_3", HKS_CHALLENGE_POS_3);

    return value;
}

static napi_value CreateHuksSecureSignType(napi_env env)
{
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_object(env, &value));

    AddInt32Property(env, value, "HUKS_SECURE_SIGN_WITH_AUTHINFO", HKS_SECURE_SIGN_WITH_AUTHINFO);

    return value;
}
}  // namespace HuksNapi

using namespace HuksNapi;
using namespace HuksNapiItem;

extern "C" {
static napi_value HuksNapiRegister(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("HuksExceptionErrCode", CreateHuksErrCode(env)),
        DECLARE_NAPI_PROPERTY("HuksErrorCode", CreateHuksErrorCode(env)),
        DECLARE_NAPI_PROPERTY("HuksKeyPurpose", CreateHuksKeyPurpose(env)),
        DECLARE_NAPI_PROPERTY("HuksKeyDigest", CreateHuksKeyDigest(env)),
        DECLARE_NAPI_PROPERTY("HuksKeyPadding", CreateHuksKeyPadding(env)),
        DECLARE_NAPI_PROPERTY("HuksCipherMode", CreateHuksCipherMode(env)),
        DECLARE_NAPI_PROPERTY("HuksKeySize", CreateHuksKeySize(env)),
        DECLARE_NAPI_PROPERTY("HuksKeyAlg", CreateHuksKeyAlg(env)),
        DECLARE_NAPI_PROPERTY("HuksKeyGenerateType", CreateHuksKeyGenerateType(env)),
        DECLARE_NAPI_PROPERTY("HuksKeyFlag", CreateHuksKeyFlag(env)),
        DECLARE_NAPI_PROPERTY("HuksKeyStorageType", CreateHuksKeyStorageType(env)),
        DECLARE_NAPI_PROPERTY("HuksTagType", CreateHuksTagType(env)),
        DECLARE_NAPI_PROPERTY("HuksTag", CreateHuksTag(env)),
        DECLARE_NAPI_PROPERTY("HuksImportKeyType", CreateHuksImportKeyType(env)),
        DECLARE_NAPI_PROPERTY("HuksUnwrapSuite", CreateHuksUnwrapSuite(env)),
        DECLARE_NAPI_PROPERTY("HuksSendType", CreateHuksSendType(env)),
        DECLARE_NAPI_PROPERTY("HuksUserAuthType", CreateHuksUserAuthType(env)),
        DECLARE_NAPI_PROPERTY("HuksAuthAccessType", CreateHuksAuthAccessType(env)),
        DECLARE_NAPI_PROPERTY("HuksChallengeType", CreateHuksChallengeType(env)),
        DECLARE_NAPI_PROPERTY("HuksChallengePosition", CreateHuksChallengePosition(env)),
        DECLARE_NAPI_PROPERTY("HuksSecureSignType", CreateHuksSecureSignType(env)),

        DECLARE_NAPI_FUNCTION("generateKey", HuksNapiGenerateKey),
        DECLARE_NAPI_FUNCTION("deleteKey", HuksNapiDeleteKey),
        DECLARE_NAPI_FUNCTION("getSdkVersion", HuksNapiGetSdkVersion),
        DECLARE_NAPI_FUNCTION("importKey", HuksNapiImportKey),
        DECLARE_NAPI_FUNCTION("exportKey", HuksNapiExportKey),
        DECLARE_NAPI_FUNCTION("getKeyProperties", HuksNapiGetKeyProperties),
        DECLARE_NAPI_FUNCTION("isKeyExist", HuksNapiIsKeyExist),
        DECLARE_NAPI_FUNCTION("init", HuksNapiInit),
        DECLARE_NAPI_FUNCTION("update", HuksNapiUpdate),
        DECLARE_NAPI_FUNCTION("finish", HuksNapiFinish),
        DECLARE_NAPI_FUNCTION("abort", HuksNapiAbort),

        DECLARE_NAPI_FUNCTION("generateKeyItem", HuksNapiItemGenerateKey),
        DECLARE_NAPI_FUNCTION("deleteKeyItem", HuksNapiDeleteKeyItem),
        DECLARE_NAPI_FUNCTION("importKeyItem", HuksNapiImportKeyItem),
        DECLARE_NAPI_FUNCTION("importWrappedKeyItem", HuksNapiImportWrappedKeyItem),
        DECLARE_NAPI_FUNCTION("exportKeyItem", HuksNapiExportKeyItem),
        DECLARE_NAPI_FUNCTION("getKeyItemProperties", HuksNapiGetKeyItemProperties),
        DECLARE_NAPI_FUNCTION("isKeyItemExist", HuksNapiIsKeyItemExist),
        DECLARE_NAPI_FUNCTION("attestKeyItem", HuksNapiAttestKeyItem),
        DECLARE_NAPI_FUNCTION("initSession", HuksNapiInitSession),
        DECLARE_NAPI_FUNCTION("updateSession", HuksNapiUpdateSession),
        DECLARE_NAPI_FUNCTION("finishSession", HuksNapiFinishSession),
        DECLARE_NAPI_FUNCTION("abortSession", HuksNapiAbortSession),

    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = HuksNapiRegister,
    .nm_modname = "security.huks",
    .nm_priv = reinterpret_cast<void *>(0),
    .reserved = { 0 },
};

__attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
}
