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

#include "hks_sm4_cipher_part_test.h"

#include <gtest/gtest.h>

#include "hks_sm4_cipher_test_common.h"

using namespace testing::ext;
namespace Unittest::Sm4Cipher {
class HksSm4CipherPartTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void HksSm4CipherPartTest::SetUpTestCase(void)
{
}

void HksSm4CipherPartTest::TearDownTestCase(void)
{
}

void HksSm4CipherPartTest::SetUp()
{
    EXPECT_EQ(HksInitialize(), 0);
}

void HksSm4CipherPartTest::TearDown()
{
}

#ifdef _USE_OPENSSL_
static struct HksParam g_genParams001[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4,
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }
};

static struct HksParam g_genParams002[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4,
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }
};

static struct HksParam g_genParams003[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4,
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CTR
    }
};

static struct HksParam g_genParams004[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4,
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_ECB
    }
};

static struct HksParam g_genParams005[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4,
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_ECB
    }
};

static struct HksParam g_genParams006[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4,
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }
};

static uint8_t g_hksSm4TestIv[HKS_SM4_IV_SIZE] = {0};

static struct HksParam g_encryptParams001[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_encryptParams002[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_encryptParams003[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CTR
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_encryptParams004[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_ECB
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_encryptParams005[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_ECB
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_encryptParams006[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_ECB
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_decryptParams001[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_decryptParams002[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CBC
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_decryptParams003[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_CTR
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_decryptParams004[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_NONE
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_ECB
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_decryptParams005[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_ECB
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static struct HksParam g_decryptParams006[] = {
    {
        .tag = HKS_TAG_ALGORITHM,
        .uint32Param = HKS_ALG_SM4
    }, {
        .tag = HKS_TAG_PURPOSE,
        .uint32Param = HKS_KEY_PURPOSE_DECRYPT
    }, {
        .tag = HKS_TAG_KEY_SIZE,
        .uint32Param = HKS_SM4_KEY_SIZE_128
    }, {
        .tag = HKS_TAG_PADDING,
        .uint32Param = HKS_PADDING_PKCS7
    }, {
        .tag = HKS_TAG_BLOCK_MODE,
        .uint32Param = HKS_MODE_ECB
    }, {
        .tag = HKS_TAG_IV,
        .blob = {
            .size = HKS_SM4_IV_SIZE,
            .data = (uint8_t *)g_hksSm4TestIv
        }
    }
};

static const struct FailureCaseParam g_genFailParams[] = {
    {   0,
        HKS_ERROR_INVALID_KEY_SIZE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_PKCS7
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_CBC
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },

    {   1,
        HKS_ERROR_INVALID_PADDING,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM4_KEY_SIZE_128
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_OAEP
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_CBC
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },

    {   2,
        HKS_ERROR_INVALID_MODE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM4_KEY_SIZE_128
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_PKCS7
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_OFB
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },
};
static const struct FailureCaseParam g_encryptFailParams[] = {
    {   0,
        HKS_ERROR_INVALID_KEY_SIZE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_PKCS7
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_CBC
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },

    {   1,
        HKS_ERROR_INVALID_PADDING,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM4_KEY_SIZE_128
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_OAEP
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_CBC
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },

    {   2,
        HKS_ERROR_INVALID_MODE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM4_KEY_SIZE_128
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_PKCS7
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_OFB
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },
};
static const struct FailureCaseParam g_decryptFailParams[] = {
    {   0,
        HKS_ERROR_INVALID_KEY_SIZE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM2_KEY_SIZE_256
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_DECRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_PKCS7
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_CBC
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },

    {   1,
        HKS_ERROR_INVALID_PADDING,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM4_KEY_SIZE_128
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_DECRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_OAEP
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_CBC
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },

    {   2,
        HKS_ERROR_INVALID_MODE,
        {
            {
                .tag = HKS_TAG_ALGORITHM,
                .uint32Param = HKS_ALG_SM4
            }, {
                .tag = HKS_TAG_KEY_SIZE,
                .uint32Param = HKS_SM4_KEY_SIZE_128
            }, {
                .tag = HKS_TAG_PURPOSE,
                .uint32Param = HKS_KEY_PURPOSE_DECRYPT
            }, {
                .tag = HKS_TAG_PADDING,
                .uint32Param = HKS_PADDING_PKCS7
            }, {
                .tag = HKS_TAG_BLOCK_MODE,
                .uint32Param = HKS_MODE_OFB
            }, {
                .tag = HKS_TAG_IV,
                .blob = {
                    .size = HKS_SM4_IV_SIZE,
                    .data = (uint8_t *)g_hksSm4TestIv
                }
            },
        },
    },
};

HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest001, TestSize.Level0)
{
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest001";
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genParams001, sizeof(g_genParams001) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptParams001, sizeof(g_encryptParams001) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptParams001, sizeof(g_decryptParams001) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}

HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest002, TestSize.Level0)
{
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest002";
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genParams002, sizeof(g_genParams002) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptParams002, sizeof(g_encryptParams002) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptParams002, sizeof(g_encryptParams002) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}

HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest003, TestSize.Level0)
{
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest003";
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genParams003, sizeof(g_genParams003) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptParams003, sizeof(g_encryptParams003) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptParams003, sizeof(g_decryptParams003) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}

HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest004, TestSize.Level0)
{
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest004";
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genParams004, sizeof(g_genParams004) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptParams004, sizeof(g_encryptParams004) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptParams004, sizeof(g_decryptParams004) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}

HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest005, TestSize.Level0)
{
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest005";
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genParams005, sizeof(g_genParams005) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptParams005, sizeof(g_encryptParams005) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptParams005, sizeof(g_decryptParams005) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}

HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest006, TestSize.Level0)
{
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest006";
    uint32_t index = 0;
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genFailParams[index].params,
        sizeof(g_genFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptFailParams[index].params,
        sizeof(g_encryptFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptFailParams[index].params,
        sizeof(g_decryptFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_KEY_SIZE) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}

HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest007, TestSize.Level0)
{
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest007";
    uint32_t index = 1;
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genFailParams[index].params,
        sizeof(g_genFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptFailParams[index].params,
        sizeof(g_encryptFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptFailParams[index].params,
        sizeof(g_decryptFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_PADDING) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}

HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest008, TestSize.Level0)
{
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest008";
    uint32_t index = 2;
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genFailParams[index].params,
        sizeof(g_genFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptFailParams[index].params,
        sizeof(g_encryptFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptFailParams[index].params,
        sizeof(g_decryptFailParams[index].params) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_ERROR_INVALID_MODE) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}

/**
 * @tc.name: HksSm4CipherPartTest.HksSm4CipherPartTest009
 * @tc.desc: normal parameter test case. And When generating the key, only the necessary parameters are passed in.
 * @tc.type: FUNC
 * @tc.require:issueI611S5
 */
HWTEST_F(HksSm4CipherPartTest, HksSm4CipherPartTest009, TestSize.Level0)
{
    HKS_LOG_E("Enter HksSm4CipherPartTest009");
    char tmpKeyAlias[] = "HksSm4CipherKeyAliasTest009";
    struct HksBlob keyAlias = { strlen(tmpKeyAlias), (uint8_t *)tmpKeyAlias };

    struct HksParamSet *genParamSet = nullptr;
    int32_t ret = InitParamSet(&genParamSet, g_genParams006, sizeof(g_genParams006) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(gen) failed.";

    struct HksParamSet *encryptParamSet = nullptr;
    ret = InitParamSet(&encryptParamSet, g_encryptParams006, sizeof(g_encryptParams006) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(encrypt) failed.";

    struct HksParamSet *decryptParamSet = nullptr;
    ret = InitParamSet(&decryptParamSet, g_decryptParams006, sizeof(g_decryptParams006) / sizeof(HksParam));
    EXPECT_EQ(ret, HKS_SUCCESS) << "InitParamSet(decrypt) failed.";

    ret = HksSm4CipherTestCaseOther(&keyAlias, genParamSet, encryptParamSet, decryptParamSet);
    EXPECT_EQ(ret, HKS_SUCCESS) << "this case failed.";

    HksFreeParamSet(&genParamSet);
    HksFreeParamSet(&encryptParamSet);
    HksFreeParamSet(&decryptParamSet);
}
#endif
}
