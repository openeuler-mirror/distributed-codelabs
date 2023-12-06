/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hks_openssl_engine.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdbool.h>
#include <stddef.h>

#include "hks_ability.h"
#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_template.h"

void HksLogOpensslError(void)
{
    char szErr[HKS_OPENSSL_ERROR_LEN] = {0};
    unsigned long errCode;

    errCode = ERR_get_error();
    ERR_error_string_n(errCode, szErr, HKS_OPENSSL_ERROR_LEN);

    HKS_LOG_E("Openssl engine fail, error code = %lu, error string = %" LOG_PUBLIC "s", errCode, szErr);
}

inline int32_t HksOpensslCheckBlob(const struct HksBlob *blob)
{
    if ((blob == NULL) || (blob->data == NULL) || (blob->size == 0)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t GenKeyCheckParam(const struct HksKeySpec *spec, const struct HksBlob *key)
{
    if ((spec == NULL) || (key == NULL)) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

static int32_t SignVerifyCheckParam(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(key), HKS_ERROR_INVALID_ARGUMENT, "Invalid param key!")

    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(message), HKS_ERROR_INVALID_ARGUMENT, "Invalid param message!")

    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(signature), HKS_ERROR_INVALID_ARGUMENT, "Invalid param signature!")

    HKS_IF_NULL_LOGE_RETURN(usageSpec, HKS_ERROR_INVALID_ARGUMENT, "Invalid param usageSpec!")

    return HKS_SUCCESS;
}

static int32_t DeriveKeyCheckParam(
    const struct HksBlob *mainKey, const struct HksKeySpec *derivationSpec, const struct HksBlob *derivedKey)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(mainKey), HKS_ERROR_INVALID_ARGUMENT,
        "Invalid mainKey params!")

    if ((derivationSpec == NULL) || (derivationSpec->algParam == NULL)) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    HKS_IF_NULL_LOGE_RETURN(derivedKey, HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    return HKS_SUCCESS;
}

static int32_t AgreeKeyCheckParam(const struct HksBlob *nativeKey, const struct HksBlob *pubKey,
    const struct HksKeySpec *spec, const struct HksBlob *sharedKey)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(nativeKey),
        HKS_ERROR_INVALID_ARGUMENT, "Invalid nativeKey params!")

    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(pubKey), HKS_ERROR_INVALID_ARGUMENT, "Invalid pubKey params!")

    HKS_IF_NULL_LOGE_RETURN(spec, HKS_ERROR_INVALID_ARGUMENT, "Invalid spec params!")

    HKS_IF_NULL_LOGE_RETURN(sharedKey, HKS_ERROR_INVALID_ARGUMENT, "Invalid sharedKey params!")

    return HKS_SUCCESS;
}

static int32_t EncryptCheckParam(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(key), HKS_ERROR_INVALID_ARGUMENT, "Invalid param key!")

    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(message), HKS_ERROR_INVALID_ARGUMENT, "Invalid param message!")

    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(cipherText),
        HKS_ERROR_INVALID_ARGUMENT, "Invalid param cipherText!")

    HKS_IF_NULL_LOGE_RETURN(usageSpec, HKS_ERROR_INVALID_ARGUMENT, "Invalid param usageSpec!")

    return HKS_SUCCESS;
}

static int32_t DecryptCheckParam(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    return EncryptCheckParam(key, usageSpec, message, cipherText);
}

const EVP_MD *GetOpensslAlg(uint32_t alg)
{
    switch (alg) {
        case HKS_DIGEST_MD5:
            return EVP_md5();
        case HKS_DIGEST_SHA1:
            return EVP_sha1();
        case HKS_DIGEST_SHA224:
            return EVP_sha224();
        case HKS_DIGEST_SHA256:
            return EVP_sha256();
        case HKS_DIGEST_SHA384:
            return EVP_sha384();
        case HKS_DIGEST_SHA512:
            return EVP_sha512();
        default:
            return NULL;
    }
}

const EVP_MD *GetOpensslAlgFromLen(uint32_t len)
{
    HKS_LOG_I("GetOpensslAlgFromLen, %" LOG_PUBLIC "d", len);
    switch (len) {
        case AFTER_HASH_LEN_16:
            return EVP_md5();
        case AFTER_HASH_LEN_20:
            return EVP_sha1();
        case AFTER_HASH_LEN_28:
            return EVP_sha224();
        case AFTER_HASH_LEN_32:
            return EVP_sha256();
        case AFTER_HASH_LEN_48:
            return EVP_sha384();
        case AFTER_HASH_LEN_64:
            return EVP_sha512();
        default:
            return NULL;
    }
}

int32_t HksCryptoHalFillRandom(struct HksBlob *randomData)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(randomData), HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    FillRandom func = (FillRandom)GetAbility(HKS_CRYPTO_ABILITY_FILL_RANDOM);
    HKS_IF_NULL_RETURN(func, HKS_ERROR_INVALID_ARGUMENT)
    return func(randomData);
}

int32_t HksCryptoHalFillPrivRandom(struct HksBlob *randomData)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(randomData), HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    FillRandom func = (FillRandom)GetAbility(HKS_CRYPTO_ABILITY_FILL_PRI_RANDOM);
    HKS_IF_NULL_RETURN(func, HKS_ERROR_INVALID_ARGUMENT)
    return func(randomData);
}

int32_t HksCryptoHalGetPubKey(const struct HksBlob *keyIn, struct HksBlob *keyOut)
{
    if (CheckBlob(keyIn) != HKS_SUCCESS || CheckBlob(keyOut) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    /* KeyMaterialRsa, KeyMaterialEcc, KeyMaterial25519's size are same */
    if (keyIn->size < sizeof(struct KeyMaterialRsa)) {
        HKS_LOG_E("Invalid params key size!");
        return HKS_ERROR_INVALID_KEY_SIZE;
    }

    struct KeyMaterialRsa *key = (struct KeyMaterialRsa *)(keyIn->data);
    PubKey func = (PubKey)GetAbility(HKS_CRYPTO_ABILITY_GET_PUBLIC_KEY(key->keyAlg));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT,
        "PubKey func is null, keyAlg:%" LOG_PUBLIC "d", key->keyAlg)
    return func(keyIn, keyOut);
}

int32_t HksCryptoHalGetMainKey(const struct HksBlob *message, struct HksBlob *mainKey)
{
    GetMainKey func = (GetMainKey)GetAbility(HKS_CRYPTO_ABILITY_GET_MAIN_KEY);
    HKS_IF_NULL_RETURN(func, HKS_ERROR_INVALID_ARGUMENT)
    return func(message, mainKey);
}

int32_t HksCryptoHalHmac(const struct HksBlob *key, uint32_t digestAlg, const struct HksBlob *msg, struct HksBlob *mac)
{
    if (CheckBlob(key) != HKS_SUCCESS || CheckBlob(msg) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    Hmac func = (Hmac)GetAbility(HKS_CRYPTO_ABILITY_HMAC);
    HKS_IF_NULL_RETURN(func, HKS_ERROR_INVALID_ARGUMENT)
    return func(key, digestAlg, msg, mac);
}

int32_t HksCryptoHalHmacInit(const struct HksBlob *key, uint32_t digestAlg, void **ctx)
{
    if (CheckBlob(key) != HKS_SUCCESS || ctx == NULL) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    HmacInit func = (HmacInit)GetAbility(HKS_CRYPTO_ABILITY_HMAC_INIT);
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "HmacInit func is null!")

    return func(ctx, key, digestAlg);
}

int32_t HksCryptoHalHmacUpdate(const struct HksBlob *chunk, void *ctx)
{
    if (CheckBlob(chunk) != HKS_SUCCESS || ctx == NULL) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    HmacUpdate func = (HmacUpdate)GetAbility(HKS_CRYPTO_ABILITY_HMAC_UPDATE);
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "HmacUpdate func is null!")

    return func(ctx, chunk);
}

int32_t HksCryptoHalHmacFinal(const struct HksBlob *msg, void **ctx, struct HksBlob *mac)
{
    if (msg == NULL || ctx == NULL || CheckBlob(mac) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    HmacFinal func = (HmacFinal)GetAbility(HKS_CRYPTO_ABILITY_HMAC_FINAL);
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "HmacFinal func is null!")

    return func(ctx, msg, mac);
}

void HksCryptoHalHmacFreeCtx(void **ctx)
{
    FreeCtx func = (FreeCtx)GetAbility(HKS_CRYPTO_ABILITY_HMAC_FREE_CTX);
    if (func == NULL) {
        HKS_LOG_E("CryptoHalHmacFreeCtx func is null");
        return;
    }

    return func(ctx);
}

int32_t HksCryptoHalHash(uint32_t alg, const struct HksBlob *msg, struct HksBlob *hash)
{
    Hash func = (Hash)GetAbility(HKS_CRYPTO_ABILITY_HASH);
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "Hash func is null!")
    return func(alg, msg, hash);
}

int32_t HksCryptoHalHashInit(uint32_t alg, void **ctx)
{
    HashInit func = (HashInit)GetAbility(HKS_CRYPTO_ABILITY_HASH_INIT);
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "HashInit func is null!")

    return func(ctx, alg);
}

int32_t HksCryptoHalHashUpdate(const struct HksBlob *msg, void *ctx)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(msg), HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    HashUpdate func = (HashUpdate)GetAbility(HKS_CRYPTO_ABILITY_HASH_UPDATE);
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "HashUpdate func is null!")

    return func(ctx, msg);
}

int32_t HksCryptoHalHashFinal(const struct HksBlob *msg, void **ctx, struct HksBlob *hash)
{
    HashFinal func = (HashFinal)GetAbility(HKS_CRYPTO_ABILITY_HASH_FINAL);
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "HashFinal func is null!")

    return func(ctx, msg, hash);
}

void HksCryptoHalHashFreeCtx(void **ctx)
{
    FreeCtx func = (FreeCtx)GetAbility(HKS_CRYPTO_ABILITY_HASH_FREE_CTX);
    if (func == NULL) {
        HKS_LOG_E("CryptoHalHashFreeCtx func is null");
        return;
    }

    func(ctx);
}

int32_t HksCryptoHalBnExpMod(
    struct HksBlob *x, const struct HksBlob *a, const struct HksBlob *e, const struct HksBlob *n)
{
    BnExpMod func = (BnExpMod)GetAbility(HKS_CRYPTO_ABILITY_BN_EXP_MOD);
    HKS_IF_NULL_RETURN(func, HKS_ERROR_INVALID_ARGUMENT)
    return func(x, a, e, n);
}

int32_t HksCryptoHalGenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    int32_t ret = GenKeyCheckParam(spec, key);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    GenerateKey func = (GenerateKey)GetAbility(HKS_CRYPTO_ABILITY_GENERATE_KEY(spec->algType));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "GenerateKey func is null!")
    return func(spec, key);
}

int32_t HksCryptoHalAgreeKey(const struct HksBlob *nativeKey, const struct HksBlob *pubKey,
    const struct HksKeySpec *spec, struct HksBlob *sharedKey)
{
    int32_t ret = AgreeKeyCheckParam(nativeKey, pubKey, spec, sharedKey);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    AgreeKey func = (AgreeKey)GetAbility(HKS_CRYPTO_ABILITY_AGREE_KEY(spec->algType));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "AgreeKey func is null!")
    return func(nativeKey, pubKey, spec, sharedKey);
}

int32_t HksCryptoHalSign(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *signature)
{
    int32_t ret = SignVerifyCheckParam(key, usageSpec, message, signature);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    Sign func = (Sign)GetAbility(HKS_CRYPTO_ABILITY_SIGN(usageSpec->algType));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "Sign func is null!")
    return func(key, usageSpec, message, signature);
}

int32_t HksCryptoHalVerify(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature)
{
    int32_t ret = SignVerifyCheckParam(key, usageSpec, message, signature);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    Verify func = (Verify)GetAbility(HKS_CRYPTO_ABILITY_VERIFY(usageSpec->algType));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "Verify func is null!")
    return func(key, usageSpec, message, signature);
}

int32_t HksCryptoHalDeriveKey(
    const struct HksBlob *masterKey, const struct HksKeySpec *derivationSpec, struct HksBlob *derivedKey)
{
    int32_t ret = DeriveKeyCheckParam(masterKey, derivationSpec, derivedKey);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    DeriveKey func = (DeriveKey)GetAbility(HKS_CRYPTO_ABILITY_DERIVE_KEY(derivationSpec->algType));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "DeriveKey func is null!")
    return func(masterKey, derivationSpec, derivedKey);
}

int32_t HksCryptoHalEncrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    int32_t ret = EncryptCheckParam(key, usageSpec, message, cipherText);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    Encrypt func = (Encrypt)GetAbility(HKS_CRYPTO_ABILITY_ENCRYPT(usageSpec->algType));
    HKS_IF_NULL_RETURN(func, HKS_ERROR_INVALID_ARGUMENT)
    return func(key, usageSpec, message, cipherText, tagAead);
}

int32_t HksCryptoHalEncryptInit(const struct HksBlob *key, const struct HksUsageSpec *usageSpec, void **ctx)
{
    if (HksOpensslCheckBlob(key) != HKS_SUCCESS || ctx == NULL) {
        HKS_LOG_E("Invalid param key!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    HKS_IF_NULL_LOGE_RETURN(usageSpec, HKS_ERROR_INVALID_ARGUMENT, "Invalid param usageSpec!")

    EncryptInit func = (EncryptInit)GetAbility(HKS_CRYPTO_ABILITY_ENCRYPT_INIT(usageSpec->algType));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "EncryptInit func is null!")

    return func(ctx, key, usageSpec, true);
}

int32_t HksCryptoHalEncryptUpdate(const struct HksBlob *message,
    void *ctx, struct HksBlob *out, const uint32_t algtype)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(message), HKS_ERROR_INVALID_ARGUMENT, "Invalid param message!")

    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(out), HKS_ERROR_INVALID_ARGUMENT, "Invalid param outdata!")

    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_INVALID_ARGUMENT, "Invalid param ctx!")

    EncryptUpdate func = (EncryptUpdate)GetAbility(HKS_CRYPTO_ABILITY_ENCRYPT_UPDATE(algtype));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "EncryptUpdate func is null!")

    return func(ctx, message, out, true);
}

int32_t HksCryptoHalEncryptFinal(const struct HksBlob *message, void **ctx, struct HksBlob *cipherText,
    struct HksBlob *tagAead, const uint32_t algtype)
{
    HKS_IF_NULL_LOGE_RETURN(message, HKS_ERROR_INVALID_ARGUMENT, "Invalid param message!")

    if (ctx == NULL || *ctx == NULL) {
        HKS_LOG_E("Invalid param ctx!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    EncryptFinal func = (EncryptFinal)GetAbility(HKS_CRYPTO_ABILITY_ENCRYPT_FINAL(algtype));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "EncryptFinal func is null!")

    return func(ctx, message, cipherText, tagAead, true);
}

void HksCryptoHalEncryptFreeCtx(void **ctx, const uint32_t algtype)
{
    FreeCtx func = (FreeCtx)GetAbility(HKS_CRYPTO_ABILITY_ENCRYPT_FREE_CTX(algtype));
    if (func == NULL) {
        HKS_LOG_E("CryptoHalEncryptFreeCtx func is null");
        return;
    }

    return func(ctx);
}

int32_t HksCryptoHalDecrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    int32_t ret = DecryptCheckParam(key, usageSpec, message, cipherText);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_INVALID_ARGUMENT, "Invalid params!")

    Decrypt func = (Decrypt)GetAbility(HKS_CRYPTO_ABILITY_DECRYPT(usageSpec->algType));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "Unsupport alg now!")
    return func(key, usageSpec, message, cipherText);
}

int32_t HksCryptoHalDecryptInit(const struct HksBlob *key, const struct HksUsageSpec *usageSpec, void **ctx)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(HksOpensslCheckBlob(key), HKS_ERROR_INVALID_ARGUMENT, "Invalid param key!")

    HKS_IF_NULL_LOGE_RETURN(usageSpec, HKS_ERROR_INVALID_ARGUMENT, "Invalid param usageSpec!")

    DecryptInit func = (DecryptInit)GetAbility(HKS_CRYPTO_ABILITY_DECRYPT_INIT(usageSpec->algType));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "DecryptInit func is null!")

    return func(ctx, key, usageSpec, false);
}

int32_t HksCryptoHalDecryptUpdate(const struct HksBlob *message,
    void *ctx, struct HksBlob *out, const uint32_t algtype)
{
    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(message), HKS_ERROR_INVALID_ARGUMENT, "Invalid param message!")

    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(out), HKS_ERROR_INVALID_ARGUMENT, "Invalid param outdata!")

    HKS_IF_NULL_LOGE_RETURN(ctx, HKS_ERROR_INVALID_ARGUMENT, "Invalid param ctx !")

    DecryptUpdate func = (DecryptUpdate)GetAbility(HKS_CRYPTO_ABILITY_DECRYPT_UPDATE(algtype));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "Unsupport alg now!")
    return func(ctx, message, out, false);
}

int32_t HksCryptoHalDecryptFinal(const struct HksBlob *message, void **ctx, struct HksBlob *cipherText,
    struct HksBlob *tagAead, const uint32_t algtype)
{
    HKS_IF_NULL_LOGE_RETURN(message, HKS_ERROR_INVALID_ARGUMENT, "Invalid param message!")

    if (ctx == NULL || *ctx == NULL) {
        HKS_LOG_E("Invalid param ctx!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    DecryptFinal func = (DecryptFinal)GetAbility(HKS_CRYPTO_ABILITY_DECRYPT_FINAL(algtype));
    HKS_IF_NULL_LOGE_RETURN(func, HKS_ERROR_INVALID_ARGUMENT, "DecryptFinal func is null!")

    return func(ctx, message, cipherText, tagAead, false);
}

void HksCryptoHalDecryptFreeCtx(void **ctx, const uint32_t algtype)
{
    FreeCtx func = (FreeCtx)GetAbility(HKS_CRYPTO_ABILITY_DECRYPT_FREE_CTX(algtype));
    if (func == NULL) {
        HKS_LOG_E("CryptoHalDecryptFreeCtx func is null");
        return;
    }

    return func(ctx);
}
