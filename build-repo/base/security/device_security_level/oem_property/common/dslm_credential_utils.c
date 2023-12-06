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

#include "dslm_credential_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/x509.h>

#include "securec.h"

#include "device_security_defines.h"
#include "utils_base64.h"
#include "utils_json.h"
#include "utils_log.h"
#include "utils_mem.h"

#define PK_ATTEST_LIST_LEN 3

#define PK_ATTEST_INDEX_ROOT 2
#define PK_ATTEST_INDEX_INTER 1
#define PK_ATTEST_INDEX_LAST 0

#define TYPE_ECDSA_SHA_256 0
#define TYPE_ECDSA_SHA_384 1

#define JSON_KEY_USER_PUBLIC_KEY "userPublicKey"
#define JSON_KEY_SIGNATURE "signature"
#define JSON_KEY_ALGORITHM "algorithm"

#define CRED_KEY_TYPE "type"
#define CRED_KEY_MANUFACTURE "manufacture"
#define CRED_KEY_BRAND "brand"
#define CRED_KEY_MODEL_NAME "model"
#define CRED_KEY_CRED_VERSION "version"
#define CRED_KEY_OS_VERSION "softwareVersion"
#define CRED_KEY_UDID "udid"
#define CRED_KEY_SN "sn"
#define CRED_KEY_SIGN_TIME "signTime"
#define CRED_KEY_SECURITY_LEVEL "securityLevel"

typedef struct PublicKeyAttestation {
    DataBuffer signature;
    PublicKey publicKey;
    uint32_t algorithm;
} PublicKeyAttestation;

typedef struct PayloadAttestation {
    DataBuffer signature;
    DataBuffer payload;
} PayloadAttestation;

typedef struct CredentialCb {
    char *saved;
    const char *header;
    const char *payload;
    const char *signature;
    const char *attestationInfo;
    PayloadAttestation load;
    PublicKeyAttestation root;
    PublicKeyAttestation intermediate;
    PublicKeyAttestation last;
} CredentialCb;

static bool CreateCredentialCb(const char *credentialString, CredentialCb *credCb);
static bool VerifyCredentialCb(const CredentialCb *credCb);

static void MovePublicKeysToAttestationList(CredentialCb *credCb, AttestationList *list);
static void CredentialCbToDslmCredInfo(CredentialCb *credCb, DslmCredInfo *credInfo, bool verified);

static void DestroyCredentialCb(CredentialCb *credCb);

static bool InitCredential(const char *credentialString, CredentialCb *credCb);
static bool SplitCredentialString(CredentialCb *credCb);
static bool SplitCredentialAttestationList(CredentialCb *credCb);

static void FreePayloadAttestation(PayloadAttestation *attestation);
static void FreePublicKeyAttestation(PublicKeyAttestation *attestation);

int32_t EcdsaVerify(const struct DataBuffer *srcData, const struct DataBuffer *sigData,
    const struct DataBuffer *pbkData, uint32_t algorithm);

int32_t VerifyDslmCredential(const char *credentialString, DslmCredInfo *credentialInfo, AttestationList *list)
{
    if (credentialString == NULL || credentialInfo == NULL) {
        SECURITY_LOG_ERROR("invalid prams, credentialString or credentialInfo is null");
        return ERR_PARSE_CLOUD_CRED_DATA;
    }
    CredentialCb credentialCb = {0};

    bool ret = CreateCredentialCb(credentialString, &credentialCb);
    if (!ret) {
        SECURITY_LOG_ERROR("CreateCredentialCb error");
        return ERR_PARSE_CLOUD_CRED_DATA;
    }

    ret = VerifyCredentialCb(&credentialCb);
    CredentialCbToDslmCredInfo(&credentialCb, credentialInfo, ret);
    if (!ret) {
        SECURITY_LOG_ERROR("VerifyCredentialCb error");
        DestroyCredentialCb(&credentialCb);
        return ERR_PARSE_CLOUD_CRED_DATA;
    }

    MovePublicKeysToAttestationList(&credentialCb, list);

    DestroyCredentialCb(&credentialCb);
    return SUCCESS;
}

void FreeAttestationList(AttestationList *list)
{
    if (list == NULL) {
        return;
    }
    FREE(list->root.data);
    FREE(list->intermediate.data);
    FREE(list->last.data);
    (void)memset_s(list, sizeof(AttestationList), 0, sizeof(AttestationList));
}

static bool CreateCredentialCb(const char *credentialString, CredentialCb *credCb)
{
    if (credentialString == NULL || credCb == NULL) {
        return false;
    }

    (void)memset_s(credCb, sizeof(CredentialCb), 0, sizeof(CredentialCb));

    bool result = false;
    do {
        if (!InitCredential(credentialString, credCb)) {
            SECURITY_LOG_ERROR("InitCredential failed");
            break;
        }
        if (!SplitCredentialString(credCb)) {
            SECURITY_LOG_ERROR("SplitCredentialString failed");
            break;
        }
        if (!SplitCredentialAttestationList(credCb)) {
            SECURITY_LOG_ERROR("SplitCredentialAttestationList failed");
            break;
        }
        result = true;
    } while (0);

    if (result != true) {
        DestroyCredentialCb(credCb);
    }

    SECURITY_LOG_INFO("success");
    return result;
}

static bool VerifyCredentialCb(const CredentialCb *credCb)
{
    const PublicKeyAttestation *root = &credCb->root;
    const PublicKeyAttestation *intermediate = &credCb->intermediate;
    const PublicKeyAttestation *last = &credCb->last;
    const PayloadAttestation *payload = &credCb->load;

    // root key, signed by self
    int32_t ret = EcdsaVerify(&root->publicKey, &root->signature, &root->publicKey, root->algorithm);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("verify root key failed, ret is %{public}d", ret);
        return false;
    }
    SECURITY_LOG_INFO("verify root success");

    // intermediate key, signed by root key
    ret = EcdsaVerify(&intermediate->publicKey, &intermediate->signature, &root->publicKey, intermediate->algorithm);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("verify intermediate key failed, ret is %{public}d", ret);
        return false;
    }
    SECURITY_LOG_INFO("verify intermediate success");

    // last key, signed by intermediate key
    ret = EcdsaVerify(&last->publicKey, &last->signature, &intermediate->publicKey, last->algorithm);
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("verify last key failed, ret is %{public}d", ret);
        return false;
    }
    SECURITY_LOG_INFO("verify last success");

    // payload, signed by last key
    ret = EcdsaVerify(&payload->payload, &payload->signature, &last->publicKey, TYPE_ECDSA_SHA_384);
    if (ret != SUCCESS) {
        ret = EcdsaVerify(&payload->payload, &payload->signature, &last->publicKey, TYPE_ECDSA_SHA_256);
    }
    if (ret != SUCCESS) {
        SECURITY_LOG_ERROR("verify payload failed, ret is %{public}d", ret);
        return false;
    }
    SECURITY_LOG_INFO("verify payload success");

    return true;
}

static bool InitCredential(const char *credentialString, CredentialCb *credCb)
{
    if (credentialString == NULL || credCb == NULL) {
        return false;
    }
    size_t strSize = strlen(credentialString) + 1;
    credCb->saved = (char *)MALLOC(strSize);
    if (credCb->saved == NULL) {
        return false;
    }
    if (strcpy_s(credCb->saved, strSize, credentialString) != EOK) {
        FREE(credCb->saved);
        credCb->saved = NULL;
        return false;
    }
    return true;
}

static void DestroyCredentialCb(CredentialCb *credCb)
{
    if (credCb == NULL) {
        return;
    }
    if (credCb->saved != NULL) {
        FREE(credCb->saved);
        credCb->saved = NULL;
    }
    FreePayloadAttestation(&credCb->load);
    FreePublicKeyAttestation(&credCb->root);
    FreePublicKeyAttestation(&credCb->intermediate);
    FreePublicKeyAttestation(&credCb->last);
    (void)memset_s(credCb, sizeof(CredentialCb), 0, sizeof(CredentialCb));
}

static bool SplitCredentialString(CredentialCb *credCb)
{
    if (credCb == NULL || credCb->saved == NULL) {
        return false;
    }
    char *context = NULL;
    credCb->header = strtok_s(credCb->saved, ".", &context);
    if (context == NULL || credCb->header == NULL) {
        return false;
    }

    credCb->payload = strtok_s(NULL, ".", &context);
    if (context == NULL || credCb->payload == NULL) {
        return false;
    }
    credCb->signature = strtok_s(NULL, ".", &context);
    if (context == NULL || credCb->signature == NULL) {
        return false;
    }
    credCb->attestationInfo = strtok_s(NULL, ".", &context);
    if (context == NULL || credCb->attestationInfo == NULL) {
        return false;
    }
    return true;
}

static uint32_t GetAlgorithmType(const char *algorithm)
{
    if (algorithm == NULL) {
        return TYPE_ECDSA_SHA_384;
    }

    if (strncmp(algorithm, "SHA256withECDSA", strlen("SHA256withECDSA")) == 0) {
        return TYPE_ECDSA_SHA_256;
    }

    if (strncmp(algorithm, "SHA384withECDSA", strlen("SHA384withECDSA")) == 0) {
        return TYPE_ECDSA_SHA_384;
    }

    return TYPE_ECDSA_SHA_384;
}

static void FreePublicKeyAttestation(PublicKeyAttestation *attestation)
{
    if (attestation == NULL) {
        return;
    }
    if (attestation->publicKey.data != NULL) {
        FREE(attestation->publicKey.data);
    }
    if (attestation->signature.data != NULL) {
        FREE(attestation->signature.data);
    }
    (void)memset_s(attestation, sizeof(PublicKeyAttestation), 0, sizeof(PublicKeyAttestation));
}

static void FreePayloadAttestation(PayloadAttestation *attestation)
{
    if (attestation == NULL) {
        return;
    }
    if (attestation->payload.data != NULL) {
        FREE(attestation->payload.data);
    }
    if (attestation->signature.data != NULL) {
        FREE(attestation->signature.data);
    }
    (void)memset_s(attestation, sizeof(PayloadAttestation), 0, sizeof(PayloadAttestation));
}

static bool ParsePayloadAttestation(CredentialCb *credCb, PayloadAttestation *attestation)
{
    if (credCb == NULL || attestation == NULL) {
        return false;
    }
    bool result = false;
    do {
        DataBuffer *signature = &attestation->signature;
        signature->length = (uint32_t)Base64UrlDecodeApp((uint8_t *)credCb->signature, &signature->data);
        if (signature->length == 0 || signature->data == NULL) {
            break;
        }
        DataBuffer *payload = &attestation->payload;
        payload->length = strlen(credCb->header) + strlen(credCb->payload) + 1;
        payload->data = MALLOC(payload->length + 1);
        if (attestation->payload.data == NULL) {
            break;
        }
        int ret = sprintf_s((char *)payload->data, payload->length + 1, "%s.%s", credCb->header, credCb->payload);
        if (ret <= EOK) {
            break;
        }
        result = true;
    } while (0);

    if (result == false) {
        FreePayloadAttestation(attestation);
    }
    return result;
}

static bool ParsePublicKeyAttestation(JsonHandle json, uint32_t attestationIndex, PublicKeyAttestation *attestation)
{
    if (json == NULL || attestation == NULL || attestationIndex > PK_ATTEST_INDEX_ROOT) {
        return false;
    }
    (void)memset_s(attestation, sizeof(PublicKeyAttestation), 0, sizeof(PublicKeyAttestation));

    JsonHandle item = GetJsonFieldJsonArray(json, attestationIndex);
    if (item == NULL) {
        return false;
    }

    bool result = false;
    do {
        const char *sigData = GetJsonFieldString(item, JSON_KEY_SIGNATURE);
        if (sigData == NULL) {
            break;
        }
        attestation->signature.length = (uint32_t)Base64UrlDecodeApp((uint8_t *)sigData, &attestation->signature.data);
        if (attestation->signature.length == 0 || attestation->signature.data == NULL) {
            break;
        }

        const char *pbkData = GetJsonFieldString(item, JSON_KEY_USER_PUBLIC_KEY);
        if (pbkData == NULL) {
            break;
        }
        attestation->publicKey.length = (uint32_t)Base64UrlDecodeApp((uint8_t *)pbkData, &attestation->publicKey.data);
        if (attestation->publicKey.length == 0 || attestation->publicKey.data == NULL) {
            break;
        }

        attestation->algorithm = GetAlgorithmType(GetJsonFieldString(item, JSON_KEY_ALGORITHM));

        result = true;
    } while (0);

    if (result == false) {
        FreePublicKeyAttestation(attestation);
    }
    return result;
}

static bool SplitCredentialAttestationList(CredentialCb *credCb)
{
    if (credCb == NULL) {
        return false;
    }
    uint8_t *buffer = NULL;
    JsonHandle json = NULL;
    bool result = false;
    do {
        if (!ParsePayloadAttestation(credCb, &credCb->load)) {
            SECURITY_LOG_ERROR("ParsePayloadAttestation failed");
            break;
        }
        Base64DecodeApp((uint8_t *)credCb->attestationInfo, &buffer);
        if (buffer == NULL) {
            SECURITY_LOG_ERROR("Base64DecodeApp failed");
            break;
        }
        json = CreateJson((char *)buffer);
        if (json == NULL) {
            break;
        }
        if (GetJsonFieldJsonArraySize(json) != PK_ATTEST_LIST_LEN) {
            SECURITY_LOG_ERROR("GetJsonFieldJsonArraySize failed");
            break;
        }
        if (!ParsePublicKeyAttestation(json, PK_ATTEST_INDEX_ROOT, &credCb->root)) {
            SECURITY_LOG_ERROR("ParsePublicKeyAttestation root failed");
            break;
        }
        if (!ParsePublicKeyAttestation(json, PK_ATTEST_INDEX_INTER, &credCb->intermediate)) {
            SECURITY_LOG_ERROR("ParsePublicKeyAttestation intermediate failed");
            break;
        }
        if (!ParsePublicKeyAttestation(json, PK_ATTEST_INDEX_LAST, &credCb->last)) {
            SECURITY_LOG_ERROR("ParsePublicKeyAttestation last failed");
            break;
        }

        result = true;
    } while (0);

    if (buffer) {
        FREE(buffer);
    }

    if (json) {
        DestroyJson(json);
    }

    return result;
}

static void MovePublicKeysToAttestationList(CredentialCb *credCb, AttestationList *list)
{
    if (credCb == NULL || list == NULL) {
        return;
    }

    list->root.data = credCb->root.publicKey.data;
    list->root.length = credCb->root.publicKey.length;
    list->intermediate.data = credCb->intermediate.publicKey.data;
    list->intermediate.length = credCb->intermediate.publicKey.length;
    list->last.data = credCb->last.publicKey.data;
    list->last.length = credCb->last.publicKey.length;

    credCb->root.publicKey.data = NULL;
    credCb->intermediate.publicKey.data = NULL;
    credCb->last.publicKey.data = NULL;
}

static int32_t GetDataFromJson(JsonHandle json, const char *paramKey, char *dest, uint32_t destLen)
{
    const char *data = GetJsonFieldString(json, paramKey);
    if (data == NULL) {
        return ERR_INVALID_PARA;
    }
    if (strncpy_s(dest, destLen, data, destLen - 1) != EOK) {
        return ERR_MEMORY_ERR;
    }
    return SUCCESS;
}

static void CredentialCbToDslmCredInfo(CredentialCb *credCb, DslmCredInfo *credInfo, bool verified)
{
    if (credCb == NULL || credInfo == NULL) {
        return;
    }

    CredType credType = credInfo->credType;
    (void)memset_s(credInfo, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    credInfo->credType = credType;

    uint8_t *buffer = NULL;

    (void)Base64DecodeApp((uint8_t *)credCb->payload, &buffer);
    if (buffer == NULL) {
        return;
    }

    JsonHandle json = CreateJson((char *)buffer);
    if (json == NULL) {
        return;
    }

    (void)GetDataFromJson(json, CRED_KEY_TYPE, credInfo->releaseType, CRED_INFO_TYPE_LEN);
    (void)GetDataFromJson(json, CRED_KEY_MANUFACTURE, credInfo->manufacture, CRED_INFO_MANU_LEN);
    (void)GetDataFromJson(json, CRED_KEY_BRAND, credInfo->brand, CRED_INFO_BRAND_LEN);
    (void)GetDataFromJson(json, CRED_KEY_MODEL_NAME, credInfo->model, CRED_INFO_MODEL_LEN);
    (void)GetDataFromJson(json, CRED_KEY_CRED_VERSION, credInfo->version, CRED_INFO_VERSION_LEN);
    (void)GetDataFromJson(json, CRED_KEY_OS_VERSION, credInfo->softwareVersion, CRED_INFO_SOFTVERSION_LEN);
    (void)GetDataFromJson(json, CRED_KEY_UDID, credInfo->udid, CRED_INFO_UDID_LEN);
    (void)GetDataFromJson(json, CRED_KEY_SIGN_TIME, credInfo->signTime, CRED_INFO_SIGNTIME_LEN);
    (void)GetDataFromJson(json, CRED_KEY_SECURITY_LEVEL, credInfo->securityLevel, CRED_INFO_LEVEL_LEN);

    if (verified) {
        if (sscanf_s(credInfo->securityLevel, "SL%u", &credInfo->credLevel) <= 0) {
            SECURITY_LOG_ERROR("formatting securityLevel string failed");
        }
    }

    FREE(buffer);
    DestroyJson(json);
}

int32_t EcdsaVerify(const struct DataBuffer *srcData, const struct DataBuffer *sigData,
    const struct DataBuffer *pbkData, uint32_t algorithm)
{
    if (srcData == NULL || sigData == NULL || pbkData == NULL) {
        return ERR_INVALID_PARA;
    }
    if (srcData->data == NULL || sigData->data == NULL || pbkData->data == NULL || srcData->length == 0 ||
        sigData->length == 0 || pbkData->length == 0) {
        return ERR_INVALID_PARA;
    }
    if ((algorithm != TYPE_ECDSA_SHA_256) && (algorithm != TYPE_ECDSA_SHA_384)) {
        return ERR_INVALID_PARA;
    }

    const unsigned char *publicKey = (const unsigned char *)pbkData->data;
    const EVP_MD *type = (algorithm == TYPE_ECDSA_SHA_256) ? EVP_sha256() : EVP_sha384();
    EVP_PKEY *pkey = d2i_PUBKEY(NULL, &publicKey, pbkData->length);
    if (pkey == NULL) {
        SECURITY_LOG_ERROR("d2i_PUBKEY failed, length = %{public}d", pbkData->length);
        return ERR_ECC_VERIFY_ERR;
    }
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        SECURITY_LOG_ERROR("EVP_MD_CTX_new failed");
        EVP_PKEY_free(pkey);
        return ERR_ECC_VERIFY_ERR;
    }

    int32_t result = ERR_ECC_VERIFY_ERR;
    do {
        if (EVP_DigestVerifyInit(ctx, NULL, type, NULL, pkey) <= 0) {
            SECURITY_LOG_ERROR("EVP_DigestVerifyInit failed");
            break;
        }

        if (EVP_DigestUpdate(ctx, srcData->data, srcData->length) <= 0) {
            SECURITY_LOG_ERROR("EVP_DigestUpdate failed");
            break;
        }

        if (EVP_DigestVerifyFinal(ctx, sigData->data, sigData->length) <= 0) {
            SECURITY_LOG_ERROR("EVP_DigestVerifyFinal failed");
            break;
        }

        result = SUCCESS;
    } while (0);

    EVP_PKEY_free(pkey);
    EVP_MD_CTX_free(ctx);
    return result;
}
