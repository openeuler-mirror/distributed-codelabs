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

#ifndef HKS_SM4_CIPHER_PART2_TEST_H
#define HKS_SM4_CIPHER_PART2_TEST_H
namespace Unittest::Sm4Cipher {
#ifdef _USE_OPENSSL_
int HksSm4CipherPartTest001(void);

int HksSm4CipherPartTest002(void);

int HksSm4CipherPartTest003(void);

int HksSm4CipherPartTest004(void);

int HksSm4CipherPartTest005(void);

int HksSm4CipherPartTest006(void);

int HksSm4CipherPartTest007(void);

int HksSm4CipherPartTest008(void);
#endif
} // namespace Unittest::Sm4Cipher
#endif // HKS_SM4_CIPHER_PART2_TEST_H