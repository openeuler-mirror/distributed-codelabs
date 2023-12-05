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

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#include <stdint.h>

#include "hks_ability.h"
#include "hks_openssl_aes.h"
#include "hks_openssl_bn.h"
#include "hks_openssl_common.h"
#include "hks_openssl_curve25519.h"
#include "hks_openssl_dh.h"
#include "hks_openssl_dsa.h"
#include "hks_openssl_ecc.h"
#include "hks_openssl_hash.h"
#include "hks_openssl_hmac.h"
#include "hks_openssl_kdf.h"
#include "hks_openssl_rsa.h"
#include "hks_openssl_sm2.h"
#include "hks_openssl_sm3.h"
#include "hks_openssl_sm4.h"
#include "hks_type.h"

static void RegisterAbilityGenerateKey(void)
{
#if defined(HKS_SUPPORT_AES_C) && defined(HKS_SUPPORT_AES_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_AES), HksOpensslAesGenerateKey);
#endif
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_RSA), HksOpensslRsaGenerateKey);
#endif
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECC_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_ECC), HksOpensslEccGenerateKey);
#endif
#if defined(HKS_SUPPORT_X25519_C) && defined(HKS_SUPPORT_X25519_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_X25519), HksOpensslCurve25519GenerateKey);
#endif
#if defined(HKS_SUPPORT_ED25519_C) && defined(HKS_SUPPORT_ED25519_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_ED25519), HksOpensslCurve25519GenerateKey);
#endif
#if defined(HKS_SUPPORT_HMAC_C) && defined(HKS_SUPPORT_HMAC_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_HMAC), HksOpensslHmacGenerateKey);
#endif
#if defined(HKS_SUPPORT_DSA_C) && defined(HKS_SUPPORT_DSA_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_DSA), HksOpensslDsaGenerateKey);
#endif
#if defined(HKS_SUPPORT_DH_C) && defined(HKS_SUPPORT_DH_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_DH), HksOpensslDhGenerateKey);
#endif
#if defined(HKS_SUPPORT_SM2_C) && defined(HKS_SUPPORT_SM2_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_SM2), HksOpensslSm2GenerateKey);
#endif
#if defined(HKS_SUPPORT_SM3_C) && defined(HKS_SUPPORT_SM3_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_SM3), HksOpensslSm3GenerateKey);
#endif
#if defined(HKS_SUPPORT_SM4_C) && defined(HKS_SUPPORT_SM4_GENERATE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(HKS_ALG_SM4), HksOpensslSm4GenerateKey);
#endif
}

static void RegisterAbilityGetPublicKey(void)
{
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_GET_PUBLIC_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GET_PUBLIC_KEY(HKS_ALG_RSA), HksOpensslGetRsaPubKey);
#endif
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECC_GET_PUBLIC_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GET_PUBLIC_KEY(HKS_ALG_ECC), HksOpensslGetEccPubKey);
#endif
#if defined(HKS_SUPPORT_ED25519_C) && defined(HKS_SUPPORT_ED2519_GET_PUBLIC_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GET_PUBLIC_KEY(HKS_ALG_ED25519), HksOpensslGetEd25519PubKey);
#endif
#if defined(HKS_SUPPORT_X25519_C) && defined(HKS_SUPPORT_X25519_GET_PUBLIC_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GET_PUBLIC_KEY(HKS_ALG_X25519), HksOpensslGetEd25519PubKey);
#endif
#if defined(HKS_SUPPORT_DH_C) && defined(HKS_SUPPORT_DH_GET_PUBLIC_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GET_PUBLIC_KEY(HKS_ALG_DH), HksOpensslGetDhPubKey);
#endif
#if defined(HKS_SUPPORT_DSA_C) && defined(HKS_SUPPORT_DSA_GET_PUBLIC_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GET_PUBLIC_KEY(HKS_ALG_DSA), HksOpensslGetDsaPubKey);
#endif
#if defined(HKS_SUPPORT_SM2_C) && defined(HKS_SUPPORT_SM2_GET_PUBLIC_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GET_PUBLIC_KEY(HKS_ALG_SM2), HksOpensslGetSm2PubKey);
#endif
}

static void RegisterAbilitySign(void)
{
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_SIGN(HKS_ALG_RSA), HksOpensslRsaSign);
#endif
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECDSA_C) && defined(HKS_SUPPORT_ECDSA_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_SIGN(HKS_ALG_ECC), HksOpensslEcdsaSign);
#endif
#if defined(HKS_SUPPORT_ED25519_C) && defined(HKS_SUPPORT_ED25519_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_SIGN(HKS_ALG_ED25519), HksOpensslEd25519Sign);
#endif
#if defined(HKS_SUPPORT_DSA_C) && defined(HKS_SUPPORT_DSA_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_SIGN(HKS_ALG_DSA), HksOpensslDsaSign);
#endif
#if defined(HKS_SUPPORT_SM2_C) && defined(HKS_SUPPORT_SM2_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_SIGN(HKS_ALG_SM2), HksOpensslSm2Sign);
#endif
}

static void RegisterAbilityVerify(void)
{
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_VERIFY(HKS_ALG_RSA), HksOpensslRsaVerify);
#endif
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECDSA_C) && defined(HKS_SUPPORT_ECDSA_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_VERIFY(HKS_ALG_ECC), HksOpensslEcdsaVerify);
#endif
#if defined(HKS_SUPPORT_ED25519_C) && defined(HKS_SUPPORT_ED25519_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_VERIFY(HKS_ALG_ED25519), HksOpensslEd25519Verify);
#endif
#if defined(HKS_SUPPORT_DSA_C) && defined(HKS_SUPPORT_DSA_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_VERIFY(HKS_ALG_DSA), HksOpensslDsaVerify);
#endif
#if defined(HKS_SUPPORT_SM2_C) && defined(HKS_SUPPORT_SM2_SIGN_VERIFY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_VERIFY(HKS_ALG_SM2), HksOpensslSm2Verify);
#endif
}

static void RegisterAbilityEncrypt(void)
{
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_CRYPT)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT(HKS_ALG_RSA), HksOpensslRsaEncrypt);
#endif
#ifdef HKS_SUPPORT_AES_C
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT(HKS_ALG_AES), HksOpensslAesEncrypt);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT_INIT(HKS_ALG_AES),
        HksOpensslAesEncryptInit);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT_UPDATE(HKS_ALG_AES),
        HksOpensslAesEncryptUpdate);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT_FINAL(HKS_ALG_AES),
        HksOpensslAesEncryptFinal);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT_FREE_CTX(HKS_ALG_AES),
        HksOpensslAesHalFreeCtx);
#endif
#ifdef HKS_SUPPORT_SM4_C
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT_INIT(HKS_ALG_SM4),
        HksOpensslSm4EncryptInit);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT_UPDATE(HKS_ALG_SM4),
        HksOpensslSm4EncryptUpdate);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT_FINAL(HKS_ALG_SM4),
        HksOpensslSm4EncryptFinal);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_ENCRYPT_FREE_CTX(HKS_ALG_SM4),
        HksOpensslSm4HalFreeCtx);
#endif
}

static void RegisterAbilityDecrypt(void)
{
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_CRYPT)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT(HKS_ALG_RSA), HksOpensslRsaDecrypt);
#endif
#ifdef HKS_SUPPORT_AES_C
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT(HKS_ALG_AES), HksOpensslAesDecrypt);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT_INIT(HKS_ALG_AES),
        HksOpensslAesDecryptInit);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT_UPDATE(HKS_ALG_AES),
        HksOpensslAesDecryptUpdate);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT_FINAL(HKS_ALG_AES),
        HksOpensslAesDecryptFinal);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT_FREE_CTX(HKS_ALG_AES),
        HksOpensslAesHalFreeCtx);
#endif
#ifdef HKS_SUPPORT_SM4_C
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT_INIT(HKS_ALG_SM4),
        HksOpensslSm4DecryptInit);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT_UPDATE(HKS_ALG_SM4),
        HksOpensslSm4DecryptUpdate);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT_FINAL(HKS_ALG_SM4),
        HksOpensslSm4DecryptFinal);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DECRYPT_FREE_CTX(HKS_ALG_SM4),
        HksOpensslSm4HalFreeCtx);
#endif
}

static void RegisterAbilityAgree(void)
{
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECDH_C) && defined(HKS_SUPPORT_ECDH_AGREE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_AGREE_KEY(HKS_ALG_ECDH), HksOpensslEcdhAgreeKey);
#endif
#if defined(HKS_SUPPORT_X25519_C) && defined(HKS_SUPPORT_X25519_AGREE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_AGREE_KEY(HKS_ALG_X25519), HksOpensslX25519AgreeKey);
#endif
#ifdef HKS_SUPPORT_ED25519_TO_X25519
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_AGREE_KEY(HKS_ALG_ED25519), HksOpensslEd25519AgreeKey);
#endif
#if defined(HKS_SUPPORT_DH_C) && defined(HKS_SUPPORT_DH_AGREE_KEY)
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_AGREE_KEY(HKS_ALG_DH), HksOpensslDhAgreeKey);
#endif
}

static void RegisterAbilityDerive(void)
{
#ifdef HKS_SUPPORT_KDF_HKDF
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DERIVE_KEY(HKS_ALG_HKDF), HksOpensslHkdf);
#endif
#ifdef HKS_SUPPORT_KDF_PBKDF2
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_DERIVE_KEY(HKS_ALG_PBKDF2), HksOpensslPbkdf2);
#endif
}

static void RegisterAbilityHash(void)
{
#ifdef HKS_SUPPORT_HASH_C
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HASH, HksOpensslHash);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HASH_INIT, HksOpensslHashInit);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HASH_UPDATE, HksOpensslHashUpdate);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HASH_FINAL, HksOpensslHashFinal);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HASH_FREE_CTX, HksOpensslHashFreeCtx);
#endif
}

static void RegisterAbilityHmac(void)
{
#ifdef HKS_SUPPORT_HMAC_C
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HMAC, HksOpensslHmac);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HMAC_INIT, HksOpensslHmacInit);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HMAC_UPDATE, HksOpensslHmacUpdate);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HMAC_FINAL, HksOpensslHmacFinal);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_HMAC_FREE_CTX, HksOpensslHmacHalFreeCtx);
#endif
}

static void RegisterAbilityGetMainKey(void)
{
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_GET_MAIN_KEY, HksOpensslGetMainKey);
}

static void RegisterAbilityFillRandom(void)
{
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_FILL_RANDOM, HksOpensslFillRandom);
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_FILL_PRI_RANDOM, HksOpensslFillPrivRandom);
}

static void RegisterAbilityBnExpMod(void)
{
#ifdef HKS_SUPPORT_BN_C
    (void)RegisterAbility(HKS_CRYPTO_ABILITY_BN_EXP_MOD, HksOpensslBnExpMod);
#endif
}

int32_t HksCryptoAbilityInit(void)
{
    RegisterAbilityGetMainKey();
    RegisterAbilityGenerateKey();
    RegisterAbilityGetPublicKey();
    RegisterAbilitySign();
    RegisterAbilityVerify();
    RegisterAbilityEncrypt();
    RegisterAbilityDecrypt();
    RegisterAbilityAgree();
    RegisterAbilityDerive();
    RegisterAbilityHmac();
    RegisterAbilityHash();
    RegisterAbilityFillRandom();
    RegisterAbilityBnExpMod();
    return HKS_SUCCESS;
}