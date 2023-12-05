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

#include "native_huks_api.h"

#include "hks_api.h"
#include "hks_errcode_adapter.h"
#include "native_huks_api_adapter.h"

static struct OH_Huks_Result ConvertApiResult(int32_t ret)
{
    struct HksResult result = HksConvertErrCode(ret);
    return *((struct OH_Huks_Result *)(&result));
}

struct OH_Huks_Result OH_Huks_GetSdkVersion(struct OH_Huks_Blob *sdkVersion)
{
    int32_t result = HksGetSdkVersion((struct HksBlob *) sdkVersion);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_GenerateKeyItem(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_ParamSet *paramSetIn, struct OH_Huks_ParamSet *paramSetOut)
{
    int32_t result = HksGenerateKey((const struct HksBlob *) keyAlias,
        (const struct HksParamSet *) paramSetIn, (struct HksParamSet *) paramSetOut);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_ImportKeyItem(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_ParamSet *paramSet, const struct OH_Huks_Blob *key)
{
    int32_t result = HksImportKey((const struct HksBlob *) keyAlias,
        (const struct HksParamSet *) paramSet, (const struct HksBlob *) key);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_ImportWrappedKeyItem(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_Blob *wrappingKeyAlias, const struct OH_Huks_ParamSet *paramSet,
    const struct OH_Huks_Blob *wrappedKeyData)
{
    int32_t result = HksImportWrappedKey((const struct HksBlob *) keyAlias,
        (const struct HksBlob *) wrappingKeyAlias, (const struct HksParamSet *) paramSet,
        (const struct HksBlob *) wrappedKeyData);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_ExportPublicKeyItem(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_ParamSet *paramSet, struct OH_Huks_Blob *key)
{
    int32_t result = HksExportPublicKey((const struct HksBlob *) keyAlias,
        (const struct HksParamSet *) paramSet, (struct HksBlob *) key);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_DeleteKeyItem(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_ParamSet *paramSet)
{
    int32_t result = HksDeleteKey((const struct HksBlob *) keyAlias, (const struct HksParamSet *) paramSet);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_GetKeyItemParamSet(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_ParamSet *paramSetIn, struct OH_Huks_ParamSet *paramSetOut)
{
    int32_t result = HksGetKeyParamSet((const struct HksBlob *) keyAlias,
        (const struct HksParamSet *) paramSetIn, (struct HksParamSet *) paramSetOut);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_IsKeyItemExist(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_ParamSet *paramSet)
{
    int32_t result = HksKeyExist((const struct HksBlob *) keyAlias, (const struct HksParamSet *) paramSet);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_AttestKeyItem(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_ParamSet *paramSet, struct OH_Huks_CertChain *certChain)
{
    int32_t result = HuksAttestAdapter(keyAlias, paramSet, certChain);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_InitSession(const struct OH_Huks_Blob *keyAlias,
    const struct OH_Huks_ParamSet *paramSet, struct OH_Huks_Blob *handle, struct OH_Huks_Blob *token)
{
    int32_t result = HksInit((const struct HksBlob *) keyAlias,
        (const struct HksParamSet *) paramSet, (struct HksBlob *) handle, (struct HksBlob *) token);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_UpdateSession(const struct OH_Huks_Blob *handle,
    const struct OH_Huks_ParamSet *paramSet, const struct OH_Huks_Blob *inData, struct OH_Huks_Blob *outData)
{
    int32_t result = HksUpdate((const struct HksBlob *) handle,
        (const struct HksParamSet *) paramSet, (const struct HksBlob *) inData, (struct HksBlob *) outData);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_FinishSession(const struct OH_Huks_Blob *handle,
    const struct OH_Huks_ParamSet *paramSet, const struct OH_Huks_Blob *inData, struct OH_Huks_Blob *outData)
{
    int32_t result = HksFinish((const struct HksBlob *) handle,
        (const struct HksParamSet *) paramSet, (const struct HksBlob *) inData, (struct HksBlob *) outData);
    return ConvertApiResult(result);
}

struct OH_Huks_Result OH_Huks_AbortSession(const struct OH_Huks_Blob *handle,
    const struct OH_Huks_ParamSet *paramSet)
{
    int32_t result = HksAbort((const struct HksBlob *) handle, (const struct HksParamSet *) paramSet);
    return ConvertApiResult(result);
}
