/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "hks_attest_utils.h"

#include <stddef.h>

#include "hks_asn1.h"
#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_template.h"
#include "securec.h"

static const uint8_t g_p256SpkiHeader[] = {
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04
};

static const uint8_t g_p384SpkiHeader[] = {
    0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x05, 0x2b,
    0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04
};

static const uint8_t g_p521SpkiHeader[] = {
    0x30, 0x81, 0x9b, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x05,
    0x2b, 0x81, 0x04, 0x00, 0x23, 0x03, 0x81, 0x86, 0x00, 0x04
};

static uint8_t g_rsaEnTag[] = { 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01 };
DECLARE_OID(g_rsaEn)

static uint8_t g_x25519Tag[] = { 0x06, 0x03, 0x2B, 0x65, 0x6E };
DECLARE_OID(g_x25519)

#define ENCODED_SEC_LEVEL_SIZE 3
static uint32_t EncodeSecurityLevel(uint8_t *out, uint32_t level)
{
    HKS_ASN1_ENCODE_BYTE(out, ASN_1_TAG_TYPE_INT);
    HKS_ASN1_ENCODE_BYTE(out, 1);
    HKS_ASN1_ENCODE_BYTE(out, level);
    return ENCODED_SEC_LEVEL_SIZE;
}

int32_t HksInsertClaim(struct HksBlob *out, const struct HksBlob *oid, const struct HksAsn1Blob *value,
    uint32_t secLevel)
{
    if ((CheckBlob(out) != HKS_SUCCESS) || (CheckBlob(oid) != HKS_SUCCESS) || (CheckAsn1Blob(value) != HKS_SUCCESS)) {
        HKS_LOG_E("invalid input");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (out->size < oid->size + value->size + ENCODED_SEC_LEVEL_SIZE + ASN_1_MAX_HEADER_LEN) {
        HKS_LOG_E("out buffer too small");
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    uint8_t *buf = out->data + ASN_1_MAX_HEADER_LEN;
    buf += EncodeSecurityLevel(buf, secLevel);
    (void)memcpy_s(buf, out->size - ASN_1_MAX_HEADER_LEN - ENCODED_SEC_LEVEL_SIZE, oid->data, oid->size);
    buf += oid->size;

    uint32_t offset = buf - out->data;
    struct HksBlob tmp = { out->size - offset, buf };
    int32_t ret = HksAsn1WriteFinal(&tmp, value);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "write final value fail\n")

    uint32_t seqSize = ENCODED_SEC_LEVEL_SIZE + oid->size + tmp.size;
    struct HksAsn1Blob seq = { ASN_1_TAG_TYPE_SEQ, seqSize, out->data + ASN_1_MAX_HEADER_LEN };
    return HksAsn1InsertValue(out, NULL, &seq);
}

static int32_t GetEcPublicKey(struct HksBlob *key, const struct HksPubKeyInfo *info)
{
    uint8_t *spkiHeader = NULL;
    uint32_t spkiHeaderLen;
    switch (info->keySize) {
        case HKS_ECC_KEY_SIZE_256:
            spkiHeader = (uint8_t *)g_p256SpkiHeader;
            spkiHeaderLen = sizeof(g_p256SpkiHeader);
            break;
        case HKS_ECC_KEY_SIZE_384:
            spkiHeader = (uint8_t *)g_p384SpkiHeader;
            spkiHeaderLen = sizeof(g_p384SpkiHeader);
            break;
        case HKS_ECC_KEY_SIZE_521:
            spkiHeader = (uint8_t *)g_p521SpkiHeader;
            spkiHeaderLen = sizeof(g_p521SpkiHeader);
            break;
        default:
            HKS_LOG_E("ec curve not supported: %" LOG_PUBLIC "u\n", info->keySize);
            return HKS_ERROR_NOT_SUPPORTED;
    }

    uint32_t totalSize = spkiHeaderLen + info->nOrXSize + info->eOrYSize;
    if (key->size < totalSize) {
        HKS_LOG_E("out size is too small: %" LOG_PUBLIC "u < %" LOG_PUBLIC "u\n", key->size, totalSize);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }
    key->size = totalSize;

    uint8_t *p = key->data;
    (void)memcpy_s(p, spkiHeaderLen, spkiHeader, spkiHeaderLen);
    p += spkiHeaderLen;
    uint8_t *publicKey = (uint8_t *)(info + 1);
    (void)memcpy_s(p, totalSize - spkiHeaderLen, publicKey, info->nOrXSize);
    p += info->nOrXSize;
    (void)memcpy_s(p, totalSize - spkiHeaderLen - info->nOrXSize, publicKey + info->nOrXSize, info->eOrYSize);
    return HKS_SUCCESS;
}

static int32_t ConstructKeySeq(struct HksBlob *out, const struct HksPubKeyInfo *info)
{
    struct HksBlob buf = *out;
    if (buf.size < ASN_1_MAX_HEADER_LEN) {
        HKS_LOG_E("buf is not big enough\n");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    buf.data += ASN_1_MAX_HEADER_LEN;
    buf.size -= ASN_1_MAX_HEADER_LEN;

    uint8_t *publicKey = (uint8_t *)(info + 1);
    struct HksAsn1Blob n = { ASN_1_TAG_TYPE_INT, info->nOrXSize, publicKey };
    int32_t ret = HksAsn1InsertValue(&buf, NULL, &n);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "insert n value fail\n")

    struct HksAsn1Blob e = { ASN_1_TAG_TYPE_INT, info->eOrYSize, publicKey + info->nOrXSize };
    ret = HksAsn1InsertValue(&buf, NULL, &e);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "insert e value fail\n")

    uint32_t seqSize = buf.data - out->data - ASN_1_MAX_HEADER_LEN;
    struct HksAsn1Blob seq = { ASN_1_TAG_TYPE_SEQ, seqSize, out->data + ASN_1_MAX_HEADER_LEN };
    ret = HksAsn1WriteFinal(out, &seq);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "write final value fail\n")

    return HKS_SUCCESS;
}

static const struct HksBlob *GetRsaSignOid(uint32_t digest)
{
    (void)digest;
    return &g_rsaEnOid;
}

static int32_t GetRsaPublicKey(struct HksBlob *key, const struct HksPubKeyInfo *info,
    const struct HksUsageSpec *usageSpec)
{
    struct HksBlob tmp = *key;
    tmp.data += ASN_1_MAX_HEADER_LEN;
    tmp.size -= ASN_1_MAX_HEADER_LEN;
    if (tmp.size < ASN_1_MAX_HEADER_LEN) {
        HKS_LOG_E("buf is not big enough\n");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }

    const struct HksBlob *signOid = GetRsaSignOid(usageSpec->digest);
    HKS_IF_NULL_LOGE_RETURN(signOid, HKS_ERROR_INVALID_ALGORITHM, "invalid digest\n")

    struct HksAsn1Blob signOidBlob = { ASN_1_TAG_TYPE_SEQ, signOid->size, signOid->data };
    int32_t ret = HksAsn1InsertValue(&tmp, NULL, &signOidBlob);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "insert signOid value fail\n")

    struct HksBlob spki = tmp;
    ret = ConstructKeySeq(&spki, info);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "construct key seq fail\n")

    struct HksAsn1Blob spkiBlob = { ASN_1_TAG_TYPE_BIT_STR, spki.size, spki.data };
    ret = HksAsn1InsertValue(&tmp, NULL, &spkiBlob);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "insert spkiBlob value fail\n")

    uint8_t *seqData = key->data + ASN_1_MAX_HEADER_LEN;
    uint32_t seqSize = tmp.data - seqData;
    struct HksAsn1Blob seqDataBlob = { ASN_1_TAG_TYPE_SEQ, seqSize, seqData };
    return HksAsn1WriteFinal(key, &seqDataBlob);
}

static int32_t GetX25519PublicKey(struct HksBlob *key, const struct HksPubKeyInfo *info)
{
    struct HksBlob tmp = *key;
    tmp.data += ASN_1_MAX_HEADER_LEN;
    tmp.size -= ASN_1_MAX_HEADER_LEN;
    if (tmp.size < ASN_1_MAX_HEADER_LEN) {
        HKS_LOG_E("buf is not big enough\n");
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }

    struct HksAsn1Blob x25519Oid = { ASN_1_TAG_TYPE_SEQ, g_x25519Oid.size, g_x25519Oid.data };
    int32_t ret = HksAsn1InsertValue(&tmp, NULL, &x25519Oid);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "insert oid value fail\n")

    uint8_t *publicKey = (uint8_t *)(info + 1);
    struct HksAsn1Blob spkiBlob = { ASN_1_TAG_TYPE_BIT_STR, info->nOrXSize, publicKey };
    ret = HksAsn1InsertValue(&tmp, NULL, &spkiBlob);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "insert pub value fail\n")

    uint8_t *seqData = key->data + ASN_1_MAX_HEADER_LEN;
    uint32_t seqSize = tmp.data - seqData;
    struct HksAsn1Blob seqDataBlob = { ASN_1_TAG_TYPE_SEQ, seqSize, seqData };
    return HksAsn1WriteFinal(key, &seqDataBlob);
}

int32_t HksGetPublicKey(struct HksBlob *key, const struct HksPubKeyInfo *info, const struct HksUsageSpec *usageSpec)
{
    if ((CheckBlob(key) != HKS_SUCCESS) || (info == NULL) || (usageSpec == NULL)) {
        HKS_LOG_E("invalid get public key params\n");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (info->keyAlg == HKS_ALG_RSA) {
        return GetRsaPublicKey(key, info, usageSpec);
    } else if (info->keyAlg == HKS_ALG_ECC) {
        return GetEcPublicKey(key, info);
    } else if (info->keyAlg == HKS_ALG_X25519) {
        return GetX25519PublicKey(key, info);
    } else {
        HKS_LOG_E("alg %" LOG_PUBLIC "u not supported to export public key\n", info->keyAlg);
        return HKS_ERROR_NOT_SUPPORTED;
    }
}
