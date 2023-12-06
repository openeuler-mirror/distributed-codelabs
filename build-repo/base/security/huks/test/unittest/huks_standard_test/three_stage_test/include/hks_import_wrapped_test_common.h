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

#ifndef HKS_IMPORT_WRAPPED_KEY_TEST_COMMON_H
#define HKS_IMPORT_WRAPPED_KEY_TEST_COMMON_H

#include "hks_type.h"

namespace Unittest::ImportWrappedKey {
    struct HksImportWrappedKeyTestParams {
        struct HksBlob *wrappingKeyAlias;
        struct HksBlob *agreeKeyAlgName;
        struct HksParamSet *genWrappingKeyParamSet;
        uint32_t publicKeySize;

        struct HksBlob *callerKeyAlias;
        struct HksParamSet *genCallerKeyParamSet;

        struct HksBlob *callerKekAlias;
        struct HksBlob *callerKek;
        struct HksParamSet *importCallerKekParamSet;

        struct HksBlob *callerAgreeKeyAlias;
        struct HksParamSet *agreeParamSet;

        struct HksParamSet *importWrappedKeyParamSet;
        struct HksBlob *importedKeyAlias;
        struct HksBlob *importedPlainKey;
        uint32_t keyMaterialLen;
    };

    static const uint32_t IV_SIZE = 16;
    static uint8_t IV[IV_SIZE] = "bababababababab";

    static const uint32_t AAD_SIZE = 16;
    static uint8_t AAD[AAD_SIZE] = "abababababababa";

    static const uint32_t NONCE_SIZE = 12;
    static uint8_t NONCE[NONCE_SIZE] = "hahahahahah";

    static const uint32_t AEAD_TAG_SIZE = 16;

    static const uint32_t X25519_256_SIZE = 256;

    void HksImportWrappedKeyTestCommonCase(const struct HksImportWrappedKeyTestParams *params);

    void HksClearKeysForWrappedKeyTest(const struct HksImportWrappedKeyTestParams *params);
}
#endif // HKS_IMPORT_WRAPPED_KEY_TEST_COMMON_H
