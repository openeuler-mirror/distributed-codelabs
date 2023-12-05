/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
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

#include "cipher.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include "cipher_log.h"
#include "ctr_drbg.h"
#include "entropy.h"
#include "md.h"
#include "pk.h"
#include "rsa.h"
#include "securec.h"

#define RSA_KEY_BYTE   66
#define NUM_FOUR       4
#define NUM_THREE      3
#define MBEDTLS_RSA_PUBLIC	0 /**< Request private key operation. */
#define MBEDTLS_RSA_PRIVATE	1 /**< Request public key operation. */

static char *RsaMallocPrivateKey(const unsigned char *key, size_t *keyLen)
{
    int32_t ret;
    const char start[] = "-----BEGIN RSA PRIVATE KEY-----\n";
    const char end[] = "\n-----END RSA PRIVATE KEY-----\n";
    size_t startLen = strlen(start);
    size_t endLen = strlen(end);
    size_t keyFinalLen = *keyLen + startLen + endLen + 1;

    char *privateKey = malloc(keyFinalLen);
    if (privateKey == NULL) {
        return NULL;
    }

    (void)memset_s(privateKey, keyFinalLen, 0, keyFinalLen);
    ret = memcpy_s(privateKey, keyFinalLen, start, startLen);
    if (ret != EOK) {
        CIPHER_LOG_E("memcpy failed.");
        free(privateKey);
        return NULL;
    }

    ret = memcpy_s(privateKey + startLen, keyFinalLen - startLen, key, *keyLen);
    if (ret != EOK) {
        CIPHER_LOG_E("memcpy failed.");
        free(privateKey);
        return NULL;
    }

    ret = memcpy_s(privateKey + startLen + *keyLen, keyFinalLen - startLen - *keyLen, end, endLen);
    if (ret != EOK) {
        CIPHER_LOG_E("memcpy failed.");
        (void)memset_s(privateKey, keyFinalLen, 0, keyFinalLen);
        free(privateKey);
        return NULL;
    }

    *keyLen = keyFinalLen;
    return privateKey;
}

static char *RsaMallocPublicKey(const unsigned char *key, size_t *keyLen)
{
    int32_t ret;
    const char start[] = "-----BEGIN PUBLIC KEY-----\n";
    const char end[] = "\n-----END PUBLIC KEY-----\n";
    size_t startLen = strlen(start);
    size_t endLen = strlen(end);
    size_t keyFinalLen = *keyLen + startLen + endLen + 1;

    char *pubKey = malloc(keyFinalLen);
    if (pubKey == NULL) {
        return NULL;
    }

    (void)memset_s(pubKey, keyFinalLen, 0, keyFinalLen);
    ret = memcpy_s(pubKey, keyFinalLen, start, startLen);
    if (ret != EOK) {
        CIPHER_LOG_E("memcpy failed.");
        free(pubKey);
        return NULL;
    }

    ret = memcpy_s(pubKey + startLen, keyFinalLen - startLen, key, *keyLen);
    if (ret != EOK) {
        CIPHER_LOG_E("memcpy failed.");
        free(pubKey);
        return NULL;
    }

    ret = memcpy_s(pubKey + startLen + *keyLen, keyFinalLen - startLen - *keyLen, end, endLen);
    if (ret != EOK) {
        CIPHER_LOG_E("memcpy failed.");
        (void)memset_s(pubKey, keyFinalLen, 0, keyFinalLen);
        free(pubKey);
        return NULL;
    }

    *keyLen = keyFinalLen;
    return pubKey;
}

static void RsaInit(mbedtls_ctr_drbg_context *ctrDrbg, mbedtls_entropy_context *entropy)
{
    mbedtls_ctr_drbg_init(ctrDrbg);
    mbedtls_entropy_init(entropy);
    (void)mbedtls_ctr_drbg_seed(ctrDrbg, mbedtls_entropy_func, entropy, NULL, 0);
}

static int32_t RsaLoadPrivateKey(mbedtls_pk_context *pk, const unsigned char *key, size_t keyLen)
{
    int32_t ret;
    size_t finalKeyLen = keyLen;
    mbedtls_rsa_context *rsa = NULL;
    char *finalKey = RsaMallocPrivateKey(key, &finalKeyLen);
    if (finalKey == NULL) {
        CIPHER_LOG_E("malloc private key error, final Key Length:%zu.", finalKeyLen);
        return ERROR_CODE_GENERAL;
    }

    mbedtls_ctr_drbg_context ctrDrbg;
    mbedtls_entropy_context entropy;
    RsaInit(&ctrDrbg, &entropy);

    do {
        ret = mbedtls_pk_parse_key(pk, (const unsigned char *)finalKey, finalKeyLen, NULL, 0,
            mbedtls_ctr_drbg_random, &ctrDrbg);
        if (ret != 0) {
            CIPHER_LOG_E("parse private key error, ret:%d.", ret);
            break;
        }

        rsa = mbedtls_pk_rsa(*pk);
        if (rsa == NULL) {
            CIPHER_LOG_E("rsa error");
            break;
        }

        if (mbedtls_rsa_check_privkey(rsa) != 0) {
            CIPHER_LOG_E("check private key failed.");
            break;
        }

        /* set padding as OAEPWITHSHA256 */
        mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        (void)memset_s(finalKey, finalKeyLen, 0, finalKeyLen);
        free(finalKey);
        return ERROR_SUCCESS;
    } while (0);

    (void)memset_s(finalKey, finalKeyLen, 0, finalKeyLen);
    free(finalKey);
    mbedtls_ctr_drbg_free(&ctrDrbg);
    mbedtls_entropy_free(&entropy);
    return ERROR_CODE_GENERAL;
}

static int32_t RsaLoadPublicKey(mbedtls_pk_context *pk, const unsigned char *key, size_t keyLen)
{
    int32_t ret;
    size_t finalKeyLen = keyLen;
    mbedtls_rsa_context *rsa = NULL;
    char* finalKey = RsaMallocPublicKey(key, &finalKeyLen);
    if (finalKey == NULL) {
        CIPHER_LOG_E("malloc public key error, final Key Length:%zu.", finalKeyLen);
        return ERROR_CODE_GENERAL;
    }

    do {
        ret = mbedtls_pk_parse_public_key(pk, (const unsigned char *)finalKey, finalKeyLen);
        if (ret != 0) {
            CIPHER_LOG_E("parse public key error, ret:%d.", ret);
            break;
        }

        rsa = mbedtls_pk_rsa(*pk);
        if (rsa == NULL) {
            CIPHER_LOG_E("pk rsa error");
            break;
        }

        if (mbedtls_rsa_check_pubkey(rsa)) {
            CIPHER_LOG_E("check public key failed.");
            break;
        }
        /* set padding as OAEPWITHSHA256 */
        mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        (void)memset_s(finalKey, finalKeyLen, 0, finalKeyLen);
        free(finalKey);
        return ERROR_SUCCESS;
    } while (0);

    (void)memset_s(finalKey, finalKeyLen, 0, finalKeyLen);
    free(finalKey);
    return ERROR_CODE_GENERAL;
}

static void RsaDeinit(mbedtls_ctr_drbg_context *ctrDrbg, mbedtls_entropy_context *entropy)
{
    mbedtls_ctr_drbg_free(ctrDrbg);
    mbedtls_entropy_free(entropy);
}

static int32_t RsaEncryptBase64Encode(int32_t cipherTotalLen, char *cipherText, int32_t cipherTextLen)
{
    if (cipherTotalLen <= 0) {
        return ERROR_CODE_GENERAL;
    }

    char *tempBuf = malloc(cipherTotalLen);
    if (tempBuf == NULL) {
        CIPHER_LOG_E("RsaEncrypt Base64Encode malloc fail.");
        return ERROR_CODE_GENERAL;
    }

    int32_t ret = memcpy_s(tempBuf, cipherTotalLen, cipherText, cipherTotalLen);
    if (ret != EOK) {
        CIPHER_LOG_E("memcpy fail.");
        free(tempBuf);
        return ERROR_CODE_GENERAL;
    }

    (void)memset_s(cipherText, cipherTextLen, 0, cipherTextLen);
    size_t dataLen = 0;
    ret = mbedtls_base64_encode(NULL, 0, &dataLen, (const unsigned char *)tempBuf, cipherTotalLen);
    if (ret != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) {
        CIPHER_LOG_E("base64_encode fail.");
        free(tempBuf);
        return ERROR_CODE_GENERAL;
    }

    ret = mbedtls_base64_encode((unsigned char *)cipherText, cipherTextLen, &dataLen,
        (const unsigned char *)tempBuf, cipherTotalLen);
    if (ret != 0) {
        CIPHER_LOG_E("base64_encode fail.");
        free(tempBuf);
        return ERROR_CODE_GENERAL;
    }
    free(tempBuf);
    return ERROR_SUCCESS;
}

static int32_t RsaEncryptMultipleBlock(mbedtls_rsa_context *rsa, const char *plainText,
    char *cipherText, int32_t cipherTextLen)
{
    mbedtls_ctr_drbg_context ctrDrbg;
    mbedtls_entropy_context entropy;
    int32_t rsaLen = mbedtls_rsa_get_len(rsa);
    int32_t rsaContentLen = rsaLen - RSA_KEY_BYTE;
    if ((rsaContentLen <= 0) || (rsaLen <= 0)) {
        CIPHER_LOG_E("rsa content len:%d, rsaLen:%d.", rsaContentLen, rsaLen);
        return ERROR_CODE_GENERAL;
    }
    int32_t count = strlen((const char *)(uintptr_t)plainText) / rsaContentLen;
    int32_t remain = strlen((const char *)(uintptr_t)plainText) % rsaContentLen;
    int32_t cipherTotalLen = 0;
    unsigned char *buf = (unsigned char *)malloc(rsaLen);
    if (buf == NULL) {
        return ERROR_CODE_GENERAL;
    }
    int32_t ret = ERROR_CODE_GENERAL;
    do {
        RsaInit(&ctrDrbg, &entropy);
        bool isBreak = false;
        for (int32_t i = 0; i < count; i++) {
            (void)memset_s(buf, rsaLen, 0, rsaLen);
            if (mbedtls_rsa_pkcs1_encrypt(rsa, mbedtls_ctr_drbg_random, &ctrDrbg,
                rsaContentLen, (const unsigned char *)(plainText + i * rsaContentLen), buf)) {
                isBreak = true;
                break;
            }
            if (memcpy_s(cipherText + i * rsaLen, cipherTextLen - i * rsaLen, buf, rsaLen)) {
                isBreak = true;
                break;
            }
            cipherTotalLen += rsaLen;
        }
        if (isBreak) {
            break;
        }
        if (remain > 0) {
            (void)memset_s(buf, rsaLen, 0, rsaLen);
            if (mbedtls_rsa_pkcs1_encrypt(rsa, mbedtls_ctr_drbg_random, &ctrDrbg,
                remain, (const unsigned char *)(plainText + count * rsaContentLen), buf)) {
                break;
            }
            if (memcpy_s(cipherText + count * rsaLen, cipherTextLen - count * rsaLen, buf, rsaLen)) {
                break;
            }
            cipherTotalLen += rsaLen;
        }
        if (RsaEncryptBase64Encode(cipherTotalLen, cipherText, cipherTextLen)) {
            break;
        }
        ret = ERROR_SUCCESS;
    } while (0);

    free(buf);
    RsaDeinit(&ctrDrbg, &entropy);
    return ret;
}

static int32_t RsaEncrypt(RsaKeyData *key, const RsaData *plain, RsaData *cipher)
{
    if ((key->trans != NULL) && (strcmp(key->trans, "RSA/None/OAEPWithSHA256AndMGF1Padding"))) {
        return ERROR_CODE_GENERAL;
    }

    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    if (RsaLoadPublicKey(&pk, (const unsigned char *)key->key, key->keyLen) != 0) {
        mbedtls_pk_free(&pk);
        return ERROR_CODE_GENERAL;
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
    if (rsa == NULL) {
        mbedtls_pk_free(&pk);
        return ERROR_CODE_GENERAL;
    }

    int32_t rsaLen = mbedtls_rsa_get_len(rsa);
    int32_t rsaContentLen = rsaLen - RSA_KEY_BYTE;
    if (rsaContentLen <= 0) {
        mbedtls_pk_free(&pk);
        return ERROR_CODE_GENERAL;
    }

    int32_t count = plain->length / rsaContentLen;
    int32_t remain = plain->length % rsaContentLen;
    if (cipher->data == NULL) {
        cipher->length = rsaLen * count + (remain ? rsaLen : 0);
        cipher->length = (cipher->length / NUM_THREE + 1) * NUM_FOUR + 1;
        mbedtls_pk_free(&pk);
        return ERROR_SUCCESS;
    }

    if (RsaEncryptMultipleBlock(rsa, plain->data, cipher->data, cipher->length) != 0) {
        CIPHER_LOG_E("Rsa encrypt block error.");
        mbedtls_pk_free(&pk);
        return ERROR_CODE_GENERAL;
    }

    mbedtls_pk_free(&pk);
    return ERROR_SUCCESS;
}

static int32_t CheckParamAndMallocBuf(size_t rsaLen, const RsaData *cipher, unsigned char **buf, unsigned char **tembuf)
{
    if ((rsaLen == 0) || (cipher->length == 0)) {
        return ERROR_CODE_GENERAL;
    }
    *buf = (unsigned char*)malloc(rsaLen);
    if (*buf == NULL) {
        return ERROR_CODE_GENERAL;
    }
    *tembuf = (unsigned char*)malloc(cipher->length);
    if (*tembuf == NULL) {
        free(*buf);
        *buf = NULL;
        return ERROR_CODE_GENERAL;
    }
    return ERROR_SUCCESS;
}

static int32_t RsaPkcs1Decrypt(mbedtls_rsa_context *rsa, size_t rsaLen, RsaData *cipher, RsaData *plain)
{
    size_t plainLen = 0;
    int32_t totalPlainLen = 0;

    unsigned char *buf = NULL;
    unsigned char *tembuf = NULL;

    int32_t ret = CheckParamAndMallocBuf(rsaLen, cipher, &buf, &tembuf);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    (void)memset_s(tembuf, cipher->length, 0, cipher->length);
    mbedtls_ctr_drbg_context ctrDrbg;
    mbedtls_entropy_context entropy;
    RsaInit(&ctrDrbg, &entropy);
    size_t dataLen;

    do {
        if (mbedtls_base64_decode(tembuf, cipher->length, &dataLen, (const unsigned char *)cipher->data,
            cipher->length)) {
            break;
        }
        int32_t count = dataLen / rsaLen;
        bool isBreak = false;
        for (int32_t i = 0; i < count; i++) {
            (void)memset_s(buf, rsaLen, 0, rsaLen);
            if (mbedtls_rsa_pkcs1_decrypt(rsa, mbedtls_ctr_drbg_random, &ctrDrbg,
                &plainLen, tembuf + i * rsaLen, buf, rsaLen)) {
                isBreak = true;
                break;
            }
            if (memcpy_s(plain->data + totalPlainLen, plain->length - totalPlainLen, buf, plainLen)) {
                isBreak = true;
                break;
            }
            totalPlainLen += plainLen;
        }
        if (isBreak) {
            break;
        }
        plain->length = totalPlainLen;
        RsaDeinit(&ctrDrbg, &entropy);
        free(tembuf);
        free(buf);
        return ERROR_SUCCESS;
    } while (0);

    RsaDeinit(&ctrDrbg, &entropy);
    free(tembuf);
    free(buf);
    return ERROR_CODE_GENERAL;
}

static int32_t RsaDecrypt(RsaKeyData *key, RsaData *cipher, RsaData *plain)
{
    if ((key->trans != NULL) && (strcmp(key->trans, "RSA/None/OAEPWithSHA256AndMGF1Padding"))) {
        return ERROR_CODE_GENERAL;
    }

    if (plain->data == NULL) {
        plain->length = cipher->length;
        return ERROR_SUCCESS;
    }

    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    if (RsaLoadPrivateKey(&pk, (const unsigned char *)key->key, key->keyLen) != 0) {
        mbedtls_pk_free(&pk);
        return ERROR_CODE_GENERAL;
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
    size_t rsaLen = mbedtls_rsa_get_len(rsa);
    int32_t ret = RsaPkcs1Decrypt(rsa, rsaLen, cipher, plain);
    if (ret != ERROR_SUCCESS) {
        CIPHER_LOG_E("Rsa pkcs1 decrypt failed.");
        mbedtls_pk_free(&pk);
        return ERROR_CODE_GENERAL;
    }

    mbedtls_pk_free(&pk);
    return ERROR_SUCCESS;
}

int32_t RsaCrypt(RsaKeyData *key, RsaData *inData, RsaData *outData)
{
    if (key == NULL || inData == NULL || outData == NULL) {
        return ERROR_CODE_GENERAL;
    }

    if ((key->action == NULL) || (key->key == NULL) || (inData->data == NULL)) {
        return ERROR_CODE_GENERAL;
    }

    if (!strcmp(key->action, "encrypt")) {
        return RsaEncrypt(key, inData, outData);
    } else if (!strcmp(key->action, "decrypt")) {
        return RsaDecrypt(key, inData, outData);
    } else {
        return ERROR_CODE_GENERAL;
    }
}
