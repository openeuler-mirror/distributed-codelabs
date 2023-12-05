/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
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

#ifndef _CUT_AUTHENTICATE_
#include "hks_rkc_rw.h"

#include "hks_crypto_hal.h"
#include "hks_get_process_info.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_storage.h"
#include "hks_template.h"

#define HKS_RKC_HASH_LEN 32         /* the hash value length of root key component */
#define HKS_RKC_KSF_BUF_LEN 258     /* the length of keystore buffer */
#define USER_ID_ROOT_DEFAULT          "0"

/* the flag of keystore file, used to identify files as HKS keystore file, don't modify. */
const uint8_t g_hksRkcKsfFlag[HKS_RKC_KSF_FLAG_LEN] = { 0x5F, 0x64, 0x97, 0x8D, 0x19, 0x4F, 0x89, 0xCF };

/* the configuration of root key component */
struct HksRkcCfg g_hksRkcCfg = {
    .state = HKS_RKC_STATE_INVALID,
    .version = HKS_RKC_VER,
    .storageType = HKS_RKC_STORAGE_FILE_SYS,
    .rkCreatedTime = { 0, 0, 0, 0, 0, 0 },
    .rkExpiredTime = { 0, 0, 0, 0, 0, 0 },
    .ksfAttr = { 0, { NULL, NULL} },
    .rmkIter = HKS_RKC_RMK_ITER,
    .rmkHashAlg = HKS_RKC_RMK_HMAC_SHA256,
    .mkMask = {0},
    .mkEncryptAlg = HKS_RKC_MK_CRYPT_ALG_AES256_GCM,
    .reserve = {0}
};

static int32_t RkcExtractKsfFileFlag(const struct HksBlob *ksfFromFile, uint32_t *ksfBufOffset)
{
    uint8_t fileFlag[HKS_RKC_KSF_FLAG_LEN] = {0};

    /* Extract file flag. */
    if (memcpy_s(fileFlag, HKS_RKC_KSF_FLAG_LEN, ksfFromFile->data + *ksfBufOffset, HKS_RKC_KSF_FLAG_LEN) != EOK) {
        HKS_LOG_E("Memcpy file flag failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }

    /* Check file flag. */
    if (HksMemCmp(fileFlag, g_hksRkcKsfFlag, HKS_RKC_KSF_FLAG_LEN) != 0) {
        HKS_LOG_E("Ksf file flag is invalid!");
        return HKS_ERROR_READ_FILE_FAIL;
    }

    *ksfBufOffset += HKS_RKC_KSF_FLAG_LEN;
    return HKS_SUCCESS;
}

static int32_t RkcExtractTime(const struct HksBlob *ksfFromFile, uint32_t *ksfBufOffset, struct HksTime *time)
{
    if (memcpy_s(&(time->hksYear), sizeof(uint16_t), ksfFromFile->data + *ksfBufOffset, sizeof(uint16_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint16_t);

    if (memcpy_s(&(time->hksMon), sizeof(uint8_t), ksfFromFile->data + *ksfBufOffset, sizeof(uint8_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    if (memcpy_s(&(time->hksDay), sizeof(uint8_t), ksfFromFile->data + *ksfBufOffset, sizeof(uint8_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    if (memcpy_s(&(time->hksHour), sizeof(uint8_t), ksfFromFile->data + *ksfBufOffset, sizeof(uint8_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    if (memcpy_s(&(time->hksMin), sizeof(uint8_t), ksfFromFile->data + *ksfBufOffset, sizeof(uint8_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    if (memcpy_s(&(time->hksSec), sizeof(uint8_t), ksfFromFile->data + *ksfBufOffset, sizeof(uint8_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    return HKS_SUCCESS;
}

static int32_t RkcExtractKsfRk(const struct HksBlob *ksfFromFile,
    uint32_t *ksfBufOffset, struct HksRkcKsfData *ksfData)
{
    /* Extract version */
    if (memcpy_s(&(ksfData->version), sizeof(uint16_t), ksfFromFile->data + *ksfBufOffset, sizeof(uint16_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint16_t);

    /* Extract rkCreatedTime */
    int32_t ret = RkcExtractTime(ksfFromFile, ksfBufOffset, &(ksfData->rkCreatedTime));
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* Extract rkExpiredTime */
    ret = RkcExtractTime(ksfFromFile, ksfBufOffset, &(ksfData->rkExpiredTime));
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* Extract the first material */
    if (memcpy_s(&(ksfData->rkMaterial1), HKS_RKC_MATERIAL_LEN,
        ksfFromFile->data + *ksfBufOffset, HKS_RKC_MATERIAL_LEN) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_MATERIAL_LEN;

    /* Extract the second material */
    if (memcpy_s(&(ksfData->rkMaterial2), HKS_RKC_MATERIAL_LEN,
        ksfFromFile->data + *ksfBufOffset, HKS_RKC_MATERIAL_LEN) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_MATERIAL_LEN;

    /* Extract iterator number */
    if (memcpy_s(&(ksfData->rmkIter), sizeof(uint32_t), ksfFromFile->data + *ksfBufOffset, sizeof(uint32_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint32_t);

    /* Extract salt */
    if (memcpy_s(&(ksfData->rmkSalt), HKS_RKC_SALT_LEN, ksfFromFile->data + *ksfBufOffset, HKS_RKC_SALT_LEN) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_SALT_LEN;

    /* Extract hash algorithm */
    if (memcpy_s(&(ksfData->rmkHashAlg), sizeof(uint32_t),
        ksfFromFile->data + *ksfBufOffset, sizeof(uint32_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint32_t);

    /* Extract reserve field */
    if (memcpy_s(&(ksfData->rkRsv), HKS_RKC_KSF_DATA_RSV_LEN,
        ksfFromFile->data + *ksfBufOffset, HKS_RKC_KSF_DATA_RSV_LEN) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_KSF_DATA_RSV_LEN;

    return HKS_SUCCESS;
}

static int32_t RkcExtractKsfMk(const struct HksBlob *ksfFromFile,
    uint32_t *ksfBufOffset, struct HksRkcKsfData *ksfData)
{
    /* Extract mkCreatedTime */
    int32_t ret = RkcExtractTime(ksfFromFile, ksfBufOffset, &(ksfData->mkCreatedTime));
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Extract mkCreatedTime failed!")

    /* Extract mkExpiredTime */
    ret = RkcExtractTime(ksfFromFile, ksfBufOffset, &(ksfData->mkExpiredTime));
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Extract mkExpiredTime failed!")

    /* Fill encrption algorithm */
    if (memcpy_s(&(ksfData->mkEncryptAlg), sizeof(uint32_t),
        ksfFromFile->data + *ksfBufOffset, sizeof(uint32_t)) != EOK) {
        HKS_LOG_E("Memcpy mkEncryptAlg failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint32_t);

    /* Fill IV */
    if (memcpy_s(&(ksfData->mkIv), HKS_RKC_MK_IV_LEN, ksfFromFile->data + *ksfBufOffset, HKS_RKC_MK_IV_LEN) != EOK) {
        HKS_LOG_E("Memcpy mkIv failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_MK_IV_LEN;

    /* Fill ciphertext */
    if (memcpy_s(&(ksfData->mkCiphertext), HKS_RKC_MK_CIPHER_TEXT_LEN,
        ksfFromFile->data + *ksfBufOffset, HKS_RKC_MK_CIPHER_TEXT_LEN) != EOK) {
        HKS_LOG_E("Memcpy mkCiphertext failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_MK_CIPHER_TEXT_LEN;

    /* Fill reserve field */
    if (memcpy_s(&(ksfData->mkRsv), HKS_RKC_KSF_DATA_RSV_LEN,
        ksfFromFile->data + *ksfBufOffset, HKS_RKC_KSF_DATA_RSV_LEN) != EOK) {
        HKS_LOG_E("Memcpy mkRsv failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_KSF_DATA_RSV_LEN;

    return HKS_SUCCESS;
}

static int32_t RkcExtractKsfHash(const struct HksBlob *ksfFromFile, uint32_t *ksfBufOffset)
{
    /* calculate sha256, skip file flag, begin with version, end with reserve field. */
    uint8_t hashResult[HKS_RKC_HASH_LEN] = {0};
    struct HksBlob hashResultBlob = { HKS_RKC_HASH_LEN, hashResult };
    /* the upper layer ensures no overflow */
    const struct HksBlob hashSrc = { *ksfBufOffset - HKS_RKC_KSF_FLAG_LEN, ksfFromFile->data + HKS_RKC_KSF_FLAG_LEN };
    int32_t ret = HksCryptoHalHash(HKS_DIGEST_SHA256, &hashSrc, &hashResultBlob);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Hks hash failed! ret = 0x%" LOG_PUBLIC "X", ret)

    /* Extract hash from ksf buffer */
    uint8_t ksfHash[HKS_RKC_HASH_LEN] = {0};
    if (memcpy_s(&ksfHash, HKS_RKC_HASH_LEN, ksfFromFile->data + *ksfBufOffset, HKS_RKC_HASH_LEN) != EOK) {
        HKS_LOG_E("Memcpy ksfHash failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_HASH_LEN;

    /* Check hash result. */
    if (HksMemCmp(hashResult, ksfHash, HKS_RKC_HASH_LEN) != 0) {
        HKS_LOG_E("Ksf hash result is Invalid!");
        return HKS_ERROR_INVALID_KEY_FILE;
    }

    return HKS_SUCCESS;
}

static int32_t RkcExtractKsfBuf(const struct HksBlob *ksfFromFile, struct HksRkcKsfData *ksfData)
{
    uint32_t ksfBufOffset = 0;

    /* Extract file flag. */
    int32_t ret = RkcExtractKsfFileFlag(ksfFromFile, &ksfBufOffset);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* Extract root key data */
    ret = RkcExtractKsfRk(ksfFromFile, &ksfBufOffset, ksfData);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Rkc extract ksf rk failed! ret = 0x%" LOG_PUBLIC "X", ret)

    /* Extract main key data */
    ret = RkcExtractKsfMk(ksfFromFile, &ksfBufOffset, ksfData);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* Extract hash */
    return RkcExtractKsfHash(ksfFromFile, &ksfBufOffset);
}

static int32_t GetProcessInfo(char **processName, char **userId)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksGetProcessName(processName), HKS_ERROR_INTERNAL_ERROR, "get process name failed")

    HKS_IF_NOT_SUCC_LOGE_RETURN(HksGetUserId(userId), HKS_ERROR_INTERNAL_ERROR, "get user id failed")

    return HKS_SUCCESS;
}

int32_t HksRkcReadKsf(const char *ksfName, struct HksRkcKsfData *ksfData)
{
    struct HksBlob tmpKsf;
    tmpKsf.data = (uint8_t *)HksMalloc(HKS_RKC_KSF_BUF_LEN);
    HKS_IF_NULL_RETURN(tmpKsf.data, HKS_ERROR_MALLOC_FAIL)

    tmpKsf.size = HKS_RKC_KSF_BUF_LEN;
    (void)memset_s(tmpKsf.data, tmpKsf.size, 0, tmpKsf.size);

    int32_t ret;
    do {
        char *processName = NULL;
        char *userId = NULL;
        if (GetProcessInfo(&processName, &userId) != HKS_SUCCESS) {
            HKS_LOG_E("get process info failed");
            ret = HKS_ERROR_INTERNAL_ERROR;
            break;
        }

        struct HksProcessInfo processInfo = {
            { strlen(userId), (uint8_t *)userId },
            { strlen(processName), (uint8_t *)processName },
            0,
            0
        };
        const struct HksBlob fileNameBlob = { strlen(ksfName), (uint8_t *)ksfName };

        ret = HksStoreGetKeyBlob(&processInfo, &fileNameBlob, HKS_STORAGE_TYPE_ROOT_KEY, &tmpKsf);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "Get ksf file failed! ret = 0x%" LOG_PUBLIC "X", ret)

        ret = RkcExtractKsfBuf(&tmpKsf, ksfData);
    } while (0);

    /* the data of root key should be cleared after use */
    (void)memset_s(tmpKsf.data, tmpKsf.size, 0, tmpKsf.size);
    HKS_FREE_BLOB(tmpKsf);
    return ret;
}

static int32_t RkcFillKsfTime(const struct HksTime *time, struct HksBlob *ksfBuf, uint32_t *ksfBufOffset)
{
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint16_t), &(time->hksYear), sizeof(uint16_t)) != EOK) {
        HKS_LOG_E("Memcpy hksYear failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint16_t);

    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint8_t), &(time->hksMon), sizeof(uint8_t)) != EOK) {
        HKS_LOG_E("Memcpy hksMon failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint8_t), &(time->hksDay), sizeof(uint8_t)) != EOK) {
        HKS_LOG_E("Memcpy hksDay failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint8_t), &(time->hksHour), sizeof(uint8_t)) != EOK) {
        HKS_LOG_E("Memcpy hksHour failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint8_t), &(time->hksMin), sizeof(uint8_t)) != EOK) {
        HKS_LOG_E("Memcpy hksMin failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint8_t), &(time->hksSec), sizeof(uint8_t)) != EOK) {
        HKS_LOG_E("Memcpy hksSec failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint8_t);

    return HKS_SUCCESS;
}

static int32_t RkcFillKsfRk(const struct HksRkcKsfData *ksfData, struct HksBlob *ksfBuf, uint32_t *ksfBufOffset)
{
    /* Fill version */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint16_t), &(ksfData->version), sizeof(uint16_t)) != EOK) {
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint16_t);

    /* Fill rkCreatedTime */
    int32_t ret = RkcFillKsfTime(&(ksfData->rkCreatedTime), ksfBuf, ksfBufOffset);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* Fill rkExpiredTime */
    ret = RkcFillKsfTime(&(ksfData->rkExpiredTime), ksfBuf, ksfBufOffset);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    /* Fill the first material */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, HKS_RKC_MATERIAL_LEN,
        ksfData->rkMaterial1, HKS_RKC_MATERIAL_LEN) != EOK) {
        HKS_LOG_E("Memcpy first material to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_MATERIAL_LEN;

    /* Fill the second material */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, HKS_RKC_MATERIAL_LEN,
        ksfData->rkMaterial2, HKS_RKC_MATERIAL_LEN) != EOK) {
        HKS_LOG_E("Memcpy second material to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_MATERIAL_LEN;

    /* Fill iterator number */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint32_t), &(ksfData->rmkIter), sizeof(uint32_t)) != EOK) {
        HKS_LOG_E("Memcpy iterator number to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint32_t);

    /* Fill salt */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, HKS_RKC_SALT_LEN, ksfData->rmkSalt, HKS_RKC_SALT_LEN) != EOK) {
        HKS_LOG_E("Memcpy salt to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_SALT_LEN;

    /* Fill hash algorithm */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint32_t), &(ksfData->rmkHashAlg), sizeof(uint32_t)) != EOK) {
        HKS_LOG_E("Memcpy hash algorithm to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint32_t);

    /* Fill reserve field */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, HKS_RKC_KSF_DATA_RSV_LEN,
        ksfData->rkRsv, HKS_RKC_KSF_DATA_RSV_LEN) != EOK) {
        HKS_LOG_E("Memcpy reserve field to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_KSF_DATA_RSV_LEN;

    return HKS_SUCCESS;
}

static int32_t RkcFillKsfMk(const struct HksRkcKsfData *ksfData, struct HksBlob *ksfBuf, uint32_t *ksfBufOffset)
{
    /* Fill mkCreatedTime */
    int32_t ret = RkcFillKsfTime(&(ksfData->mkCreatedTime), ksfBuf, ksfBufOffset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Fill mk created time to ksf buf failed!")

    /* Fill mkExpiredTime */
    ret = RkcFillKsfTime(&(ksfData->mkExpiredTime), ksfBuf, ksfBufOffset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Fill mk expired time to ksf buf failed!")

    /* Fill encrption algorithm */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, sizeof(uint32_t), &(ksfData->mkEncryptAlg), sizeof(uint32_t)) != EOK) {
        HKS_LOG_E("Memcpy encrption algorithm to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += sizeof(uint32_t);

    /* Fill IV */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, HKS_RKC_MK_IV_LEN, ksfData->mkIv, HKS_RKC_MK_IV_LEN) != EOK) {
        HKS_LOG_E("Memcpy iv to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_MK_IV_LEN;

    /* Fill ciphertext */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, HKS_RKC_MK_CIPHER_TEXT_LEN,
        ksfData->mkCiphertext, HKS_RKC_MK_CIPHER_TEXT_LEN) != EOK) {
        HKS_LOG_E("Memcpy ciphertext to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_MK_CIPHER_TEXT_LEN;

    /* Fill reserve field */
    if (memcpy_s(ksfBuf->data + *ksfBufOffset, HKS_RKC_KSF_DATA_RSV_LEN,
        ksfData->mkRsv, HKS_RKC_KSF_DATA_RSV_LEN) != EOK) {
        HKS_LOG_E("Memcpy reserve field to ksf buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    *ksfBufOffset += HKS_RKC_KSF_DATA_RSV_LEN;

    return HKS_SUCCESS;
}

static int32_t RkcFillKsfHash(struct HksBlob *ksfBuf, uint32_t *ksfBufOffset)
{
    if ((ksfBuf->size < HKS_RKC_KSF_FLAG_LEN) || (*ksfBufOffset <= HKS_RKC_KSF_FLAG_LEN) ||
        (ksfBuf->size < *ksfBufOffset)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    /* calculate sha256, skip file flag, begin with version, end with reserve field. */
    const struct HksBlob hashSrc = { *ksfBufOffset - HKS_RKC_KSF_FLAG_LEN, ksfBuf->data + HKS_RKC_KSF_FLAG_LEN };
    struct HksBlob hash = { HKS_RKC_HASH_LEN, ksfBuf->data + *ksfBufOffset };
    int32_t ret = HksCryptoHalHash(HKS_DIGEST_SHA256, &hashSrc, &hash);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Hash failed! ret = 0x%" LOG_PUBLIC "X", ret)

    *ksfBufOffset += HKS_RKC_HASH_LEN;
    return HKS_SUCCESS;
}

static int32_t RkcFillKsfBuf(const struct HksRkcKsfData *ksfData, struct HksBlob *ksfBuf)
{
    uint32_t ksfBufOffset = 0;

    /* Fill file flag */
    if (memcpy_s(ksfBuf->data, HKS_RKC_KSF_FLAG_LEN, g_hksRkcKsfFlag, HKS_RKC_KSF_FLAG_LEN) != EOK) {
        HKS_LOG_E("Memcpy file flag to ksd buf failed!");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    ksfBufOffset += HKS_RKC_KSF_FLAG_LEN;

    /* Fill root key */
    int32_t ret = RkcFillKsfRk(ksfData, ksfBuf, &ksfBufOffset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Fill root key info to ksf buf failed! ret = 0x%" LOG_PUBLIC "X", ret)

    /* Fill main key */
    ret = RkcFillKsfMk(ksfData, ksfBuf, &ksfBufOffset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Fill main key info to ksf buf failed! ret = 0x%" LOG_PUBLIC "X", ret)

    /* calculate and fill SHA256 result, skip file flag, begin with version, end with reserve field. */
    ret = RkcFillKsfHash(ksfBuf, &ksfBufOffset);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "Fill hash to ksf buf failed! ret = 0x%" LOG_PUBLIC "X", ret)

    return HKS_SUCCESS;
}

int32_t HksRkcWriteKsf(const char *ksfName, const struct HksRkcKsfData *ksfData)
{
    struct HksBlob ksfBuf;
    ksfBuf.data = (uint8_t *)HksMalloc(HKS_RKC_KSF_BUF_LEN);
    HKS_IF_NULL_LOGE_RETURN(ksfBuf.data, HKS_ERROR_MALLOC_FAIL, "Malloc ksf buffer failed!")

    ksfBuf.size = HKS_RKC_KSF_BUF_LEN;
    (void)memset_s(ksfBuf.data, ksfBuf.size, 0, ksfBuf.size);

    int32_t ret;
    do {
        /* Fill data into buffer */
        ret = RkcFillKsfBuf(ksfData, &ksfBuf);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "Fill ksf buf failed! ret = 0x%" LOG_PUBLIC "X", ret)

        char *processName = NULL;
        char *userId = NULL;
        if (GetProcessInfo(&processName, &userId) != HKS_SUCCESS) {
            HKS_LOG_E("get process info failed");
            ret = HKS_ERROR_INTERNAL_ERROR;
            break;
        }

        struct HksProcessInfo processInfo = {
            { strlen(userId), (uint8_t *)userId },
            { strlen(processName), (uint8_t *)processName },
            0,
            0
        };
        /* write buffer data into keystore file */
        const struct HksBlob fileNameBlob = { strlen(ksfName), (uint8_t *)ksfName };
        ret = HksStoreKeyBlob(&processInfo, &fileNameBlob, HKS_STORAGE_TYPE_ROOT_KEY, &ksfBuf);
        HKS_IF_NOT_SUCC_LOGE(ret, "Store ksf failed! ret = 0x%" LOG_PUBLIC "X", ret)
    } while (0);

    (void)memset_s(ksfBuf.data, ksfBuf.size, 0, ksfBuf.size);
    HKS_FREE_BLOB(ksfBuf);
    return ret;
}

bool RkcKsfExist(void)
{
    char *processName = NULL;
    char *userId = NULL;
    HKS_IF_NOT_SUCC_LOGE_RETURN(GetProcessInfo(&processName, &userId),
        HKS_ERROR_INTERNAL_ERROR, "get process info failed")

    struct HksProcessInfo processInfo = {
        { strlen(userId), (uint8_t *)userId },
        { strlen(processName), (uint8_t *)processName },
        0,
        0
    };

    for (uint32_t i = 0; i < g_hksRkcCfg.ksfAttr.num; ++i) {
        const struct HksBlob fileNameBlob = {
            strlen(g_hksRkcCfg.ksfAttr.name[i]),
            (uint8_t *)(g_hksRkcCfg.ksfAttr.name[i])
        };

        int32_t ret = HksStoreIsKeyBlobExist(&processInfo, &fileNameBlob, HKS_STORAGE_TYPE_ROOT_KEY);
        if (ret == HKS_SUCCESS) {
            /* return true if one exists */
            return true;
        }
    }

    return false;
}
#endif /* _CUT_AUTHENTICATE_ */
