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

#include "dslm_oem_property_test.h"

#include <gtest/gtest.h>

#include "file_ex.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"

#include "dslm_credential.h"
#include "dslm_credential_utils.h"
#include "dslm_crypto.h"
#include "external_interface_adapter.h"
#include "hks_adapter.h"
#include "hks_type.h"
#include "utils_mem.h"
#include "utils_tlv.h"

using namespace std;
using namespace std::chrono;
using namespace testing;
using namespace testing::ext;

// for testing
extern "C" {
extern int32_t EcdsaVerify(const struct DataBuffer *srcData, const struct DataBuffer *sigData,
    const struct DataBuffer *pbkData, uint32_t algorithm);
extern int32_t FillHksParamSet(struct HksParamSet **paramSet, struct HksParam *param, int32_t paramNums);
extern int32_t HksGenerateKeyAdapter(const struct HksBlob *keyAlias);
extern int32_t BufferToHksCertChain(const uint8_t *data, uint32_t dataLen, struct HksCertChain *hksCertChain);
extern int32_t HksCertChainToBuffer(const struct HksCertChain *hksCertChain, uint8_t **data, uint32_t *dataLen);
extern void DestroyHksCertChain(struct HksCertChain *certChain);
extern int32_t ConstructHksCertChain(struct HksCertChain **certChain,
    const struct HksCertChainInitParams *certChainParam);
}

namespace OHOS {
namespace Security {
namespace DslmUnitTest {
void DslmOemPropertyTest::SetUpTestCase()
{
}
void DslmOemPropertyTest::TearDownTestCase()
{
}
void DslmOemPropertyTest::SetUp()
{
}
void DslmOemPropertyTest::TearDown()
{
}

/**
 * @tc.name: VerifyDslmCredential_case4
 * @tc.desc: function VerifyDslmCredential with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case1, TestSize.Level0)
{
    int32_t ret;
    const char *cred = "test";
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    ret = VerifyDslmCredential(cred, nullptr, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);

    ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case2
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case2, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    {
        const char *cred = "eyJ0eXAiOiAiRFNMIn0=";

        int32_t ret = VerifyDslmCredential(cred, &info, &list);
        EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
    }

    {
        const char *cred = "eyJ0eXAiOiAiRFNMIn0=."
                           "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjo"
                           "gInJrMzU2OCIsICJzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnbl"
                           "RpbWUiOiAiMjAyMjExMjYxNzMzNDMiLCAidmVyc2lvbiI6ICIxLjAuMSJ9";

        int32_t ret = VerifyDslmCredential(cred, &info, &list);
        EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
    }

    {
        const char *cred = "eyJ0eXAiOiAiRFNMIn0=."
                           "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjo"
                           "gInJrMzU2OCIsICJzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnbl"
                           "RpbWUiOiAiMjAyMjExMjYxNzMzNDMiLCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGUCMEPpiP8hOZlve/"
                           "H81B7AvL4Fuwe8YYAdKckLEOcEQKKTiNRM6irjXSwboMppAFNMSgIxAILC1S6KMp6Zp2ACppXF3j3fV0PBdLZOSO1Lm"
                           "9sqtdiJ5FidaAaMYlwdLMy3vfBeSg==";

        int32_t ret = VerifyDslmCredential(cred, &info, &list);
        EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
    }
}

/**
 * @tc.name: VerifyDslmCredential_case3
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case3, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    {
        const char *cred = "eyJ0eXAiOiAiRFNMIn0=.";

        int32_t ret = VerifyDslmCredential(cred, &info, &list);
        EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
    }

    {
        const char *cred = "eyJ0eXAiOiAiRFNMIn0=."
                           "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjo"
                           "gInJrMzU2OCIsICJzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnbl"
                           "RpbWUiOiAiMjAyMjExMjYxNzMzNDMiLCAidmVyc2lvbiI6ICIxLjAuMSJ9.";

        int32_t ret = VerifyDslmCredential(cred, &info, &list);
        EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
    }

    {
        const char *cred = "eyJ0eXAiOiAiRFNMIn0=."
                           "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjo"
                           "gInJrMzU2OCIsICJzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnbl"
                           "RpbWUiOiAiMjAyMjExMjYxNzMzNDMiLCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGUCMEPpiP8hOZlve/"
                           "H81B7AvL4Fuwe8YYAdKckLEOcEQKKTiNRM6irjXSwboMppAFNMSgIxAILC1S6KMp6Zp2ACppXF3j3fV0PBdLZOSO1Lm"
                           "9sqtdiJ5FidaAaMYlwdLMy3vfBeSg==.";

        int32_t ret = VerifyDslmCredential(cred, &info, &list);
        EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
    }
}

/**
 * @tc.name: VerifyDslmCredential_case4
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case4, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes SplitCredentialAttestationList()
    // failed when CreateJson()
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjYxNzMzNDMi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGUCMEPpiP8hOZlve/"
        "H81B7AvL4Fuwe8YYAdKckLEOcEQKKTiNRM6irjXSwboMppAFNMSgIxAILC1S6KMp6Zp2ACppXF3j3fV0PBdLZOSO1Lm9sqtdiJ5FidaAaM"
        "YlwdLMy3vfBeSg==."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HVUNNUUNIUVdzYXNYc1NpL3dJUThmWW5PRlhsaWhTem"
        "5ETG1RSjBEOGp4U3RVM2Z2bk4xZkgzUVJJUnRzM1lIK293bE9zQ01EY2pJU0pOK2J6M2g0VUU2UTl1NW92K0RHcFRHL2Vqd0xTU2FyMHJz"
        "Z09ZSVovODdRb0p2QllaM2hFamlDcWQ1dz09In0sIHsidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQV"
        "FFTEEySUFCRkRMR2M4YlhQT2RBYVpLN25OQUZrYkRoVHBwcTNaQW92T3FKZDJKMy9vdW14eG84QnQ4ZGhiQjBtR3FHQjE4V0hpTkUwNFRC"
        "S1RvYU9lQ3NtZEZ0dUtXcEtwZEtIRDdGL3YvaXhxbHd6MnMzSk9scFQ3dUQzbjNieHFaVHJzMnFnPT0iLCAic2lnbmF0dXJlIjxxIk1HUU"
        "NNSGthczBkZDgwUVpiQVB6eElhMXhBYmd1WlxxNjU0T29rL2VGR2M0ekxLczlqYjVKK24xxHJDcytoa0JrR0N0b3dJd1pYcGlYUjRiS1h3"
        "RUlTZmdpSDI4dkZaZVQxcFJCcnFkSHd2d3ErOXxxxWQzMkhkeC90YWhHZ1kySHVZZFNHZDUifSwgeyJ1c2VyUHVibGljS2V5IjogIk1Ib3"
        "dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUUVMQTJJQUJEVTVaYkhESGl2TGgzRFN4UDEwbGluL2FIMXJabG1XMnBMZ3JwZ3BiL0lnWkkr"
        "MzJyWC9QdFhURGZWYmVyRG93VkhURTJ0MFZMNzlnQ2wrbUVCL1dBeDVEZW1lamlMNTJ6S0l6M2RTNWJxVHdYVExvRHZTSml3Z3dxYmZPME"
        "ZtK3c9PSIsICJzaWxxYXR1cmUiOiAiTUdRQ01HWlI0MUdsd1RnL0xUMGtFT3lTZnRHTDBlV04xx2dXdFo0NTZ2VkdqMm56WnhsamFlN2pv"
        "eWw4cWZHNjZSTUdTQUl3S2M3V2VpQ1c1UlFGSjROWitSRUErNVNpMHhRVFpOdzlhb1FTUG5LVTA0L2ZIWUhkVERNWitncUY3U3RJMDZTbS"
        "J9XQ==";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case5
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case5, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes SplitCredentialAttestationList()
    // failed when GetJsonFieldJsonArraySize()
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxNzQzMzUi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGQCMA0DfA8wWdMn08lA8h76mpNadU0EJIoJGmaub6ccWZynHaPcSsMtCz6CrNTYMcP9+"
        "QIwGc5T3K0csJUYxnFgvgfMuq2rHPzACTls0b7e+s8pZtsK97MFzwIxX6oskIYxkVra."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HVUNNSC9LWDVDVmljZ3ZQYmUvYzFWUTl1eFV5N2x6ZX"
        "lMQmtQYkc4UnNLcURKSERRK1NZeFpoZWd5SzZRZ3JXbTVhcUFJeEFJcHhpNVd1eG5QeXNvZ3MreE1YWmU5ZXVJSHFxUzlmdWtrK1ROQzhv"
        "dXFjU29Wd3RkbUgzSUpNb091b3JQdFpiZz09In0sIHsidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQV"
        "FFTEEySUFCRkRMR2M4YlhQT2RBYVpLN25OQUZrYkRoVHBwcTNaQW92T3FKZDJKMy9vdW14eG84QnQ4ZGhiQjBtR3FHQjE4V0hpTkUwNFRC"
        "S1RvYU9lQ3NtZEZ0dUtXcEtwZEtIRDdGL3YvaXhxbHd6MnMzSk9scFQ3dUQzbjNieHFaVHJzMnFnPT0iLCAic2lnbmF0dXJlIjogIk1HUU"
        "NNSFU3b3M3UEgzd0ttcDdmN0krUFNlRzY3K2cvWkRwZE1XM1ZwMWVhUEgzVEpCendSTlJKNE83SlNneE4zZjVCbVFJd0ZUOHBBTVVLNTY0"
        "NXRTNDZnQ3d5UzdxM2RKWElMbGJhVkxCeWVZY0RMN3B2WUZERWhPSlV0aEhPdGoxRnZxQ0wifV0=";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case6
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case6, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes SplitCredentialAttestationList()
    // failed when ParsePublicKeyAttestation() for root key
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxODA0Mzki"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9."
        "MGQCMAGiIvvXg9REZbTBVRy3VCcM8yP11x3R0mfLYQbZr71sAjS2jMJdXzlyJaCK8SQ73wIwW8DDJsYlHKHGPmRZkvS2x+Va4rglp+"
        "GXWsbP2wYeMeGh/"
        "degW3Azc7BjbNiUU6M4."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HUUNNRHRmODVrTldIMkM3UEljbHZNWWVyOG1MdnhFVD"
        "FPZHUvc0R3RUlEUGFCSGkrZ2JhbzJQZjlqUXdScHkwZGJmcGdJd1hHdVNMUG0xbVNLQXRnNFg5blFYNjdiRTBJNjB4RHRWSEFuRzBGNVFJ"
        "SSt3UFBEYlE2L2wxOHpHQ21LcjlJck0ifSwgeyJ1c2VyUHVibGljS2V5IjogIk1Ib3dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUUVMQT"
        "JJQUJGRExHYzhiWFBPZEFhWks3bk5BRmtiRGhUcHBxM1pBb3ZPcUpkMkozL291bXh4bzhCdDhkaGJCMG1HcUdCMThXSGlORTA0VEJLVG9h"
        "T2VDc21kRnR1S1dwS3BkS0hEN0Yvdi9peHFsd3oyczNKT2xwVDd1RDNuM2J4cVpUcnMycWc9PSIsICJzaWduYXR1cmUiOiAiTUdRQ01IQ3"
        "NncFVtcFZCamlraTduOHlBb1ZBajROUnI1R1dLcXJhUitramc4MllKWnh4cFV1VlI5QWJjZEFZaUFQcGdEUUl3ZWhxenB2MXNwemd0elho"
        "cnY0cGdONVcvdVdRZFRHclk3bndOdldaTE04M0JReG1aSW1oQ0thSFZrejlJeWlJcSJ9LCB7InNlY3JldGtleSI6ICJNSG93RkFZSEtvWk"
        "l6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCRFU1WmJIREhpdkxoM0RTeFAxMGxpbi9hSDFyWmxtVzJwTGdycGdwYi9JZ1pJKzMyclgvUHRY"
        "VERmVmJlckRvd1ZIVEUydDBWTDc5Z0NsK21FQi9XQXg1RGVtZWppTDUyektJejNkUzVicVR3WFRMb0R2U0ppd2d3cWJmTzBGbSt3PT0iLC"
        "Aic2lnbmF0dXJlIjogIk1HUUNNQ2xvdDJqTWVRN3RlTzJVRXVkSjE0VDVIVTArWFd5cVYzcE4zaElSOXZkaVdXVGhVSmlkRTQvRzZjaVU3"
        "Tjh6N2dJd09weDd6MHdhbDl4SlRERURIZjBsZlU5Z2FCVXRreVkrM2Zmd2FlaVpmVVJteWpUQ1RveitSdVpibjRxUzhtclkifV0=";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case7
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case7, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes SplitCredentialAttestationList()
    // failed when ParsePublicKeyAttestation() for intermediate key
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxODA3Mzci"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGQCMCIp5PS8K+rFeHY0uQU9e/"
        "6Xa3mv+GEzZl0POKBspU+2Qh9PAKmFr8yFgzvWVV8M0QIwLk2GUeWOa46BqF4klWphRZmXZ/"
        "lnDOfoDIILQdCDsV1UpObyol6i1fOYp3AF1DpS."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HUUNNR29iWVhGM2VHYjZpTm9DQU4ySmI4dkliajY4VV"
        "A4eWlleGxaa2R4VlZVNDEvL2I3cDBVa01LbmtnbFFNNnFhOHdJd2F3RDVoNXI2N25yRW5PRzJ4NUNBVVpwZkJTNmJZdEUrR0dPODdwVHRl"
        "WngvZG5uRXk1UlE5VElYRENzS0F4aWYifSwgeyJ1c2VyUHVibGljS2V5IjogIk1Ib3dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUUVMQT"
        "JJQUJGRExHYzhiWFBPZEFhWks3bk5BRmtiRGhUcHBxM1pBb3ZPcUpkMkozL291bXh4bzhCdDhkaGJCMG1HcUdCMThXSGlORTA0VEJLVG9h"
        "T2VDc21kRnR1S1dwS3BkS0hEN0Yvdi9peHFsd3oyczNKT2xwVDd1RDNuM2J4cVpUcnMycWc9PSIsICJuby1zaWciOiAiTUdRQ01DTXBhc0"
        "ZlUFdvd01aOTRXc1lwR0ZXQ3l4SDJTamU0OWozTTVwVWJkdHdEeUlpNDZLSUJEQ1c4Rnl1dXBsYTBSQUl3WkFSaXZlaUN6MWd5bk5yOXhM"
        "S0hHVTgvb05WMFh1NEYrY3lHcmIveFA0bkNmaVBtN0N5Q29ycFljVjREN250MiJ9LCB7InVzZXJQdWJsaWNLZXkiOiAiTUhvd0ZBWUhLb1"
        "pJemowQ0FRWUpLeVFEQXdJSUFRRUxBMklBQkRVNVpiSERIaXZMaDNEU3hQMTBsaW4vYUgxclpsbVcycExncnBncGIvSWdaSSszMnJYL1B0"
        "WFREZlZiZXJEb3dWSFRFMnQwVkw3OWdDbCttRUIvV0F4NURlbWVqaUw1MnpLSXozZFM1YnFUd1hUTG9EdlNKaXdnd3FiZk8wRm0rdz09Ii"
        "wgInNpZ25hdHVyZSI6ICJNR1VDTVFDR1hoRlA4SnVoWnRDekZQZ1JpL1NZNXNNNEJqYmdhRFRvSkFaNFcxdXFzV05sSmFUZjJXREg2T2l2"
        "bEs2QkhYZ0NNQXhLc3pNRzFsYStSYzVDNnlubEthS3pVMnZnRVJYbXFDcmFwQWs3L0pzYitRVGxHbGZJT3plQXhJbUIybmFuZHc9PSJ9XQ"
        "==";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case8
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case8, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes SplitCredentialAttestationList()
    // failed when ParsePublicKeyAttestation() for last key
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxODExNTUi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGQCMF8Z7Ql+"
        "YrX4TEcFRVGwfKMRR2og3SYe9AaBUgR0szDXzpA1RKl3mEkzR4lAQ6Iw8AIwI498oHh9d80CveF41ntRSoxLT3VtpG+BVnTgWGe6wUm4/"
        "egsSNTHEgeUDjU+SrGp."
        "W3sibm8tcHVibGljIjogIk1Ib3dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUUVMQTJJQUJCKzNMclZRTXdxaXBzZWc5QUFPS3AwMkV4M0"
        "o5MmVTN2srSTlwUk9adW84VkVCZW9vMXpGKzYxaG1VTlMya2M3Rzc1MFU4TE5PalRqaFRUanY1bUJCN0F2eGdQMzBzd3dIMnV0VWhzOEZH"
        "MDBBTGw5Rm5YVmxKY2loajlIYnRaM2c9PSIsICJzaWduYXR1cmUiOiAiTUdRQ01BT0F6ajluMFVkSUtrWWZrWjN2SkJMOWQ5WGpSMUxQMj"
        "lwNXBwVUJ4bjRrVGRBTzN1OGh6bVBySWFQdkEwRTR6d0l3VVpscm03dmlhUUdoL3IxY2tjRW1BNEpYTVM1UGwvcUdMdTJUSnd3RUtjMnBO"
        "MzVHbXB5bExUV2xIWHpVZC90MyJ9LCB7InVzZXJQdWJsaWNLZXkiOiAiTUhvd0ZBWUhLb1pJemowQ0FRWUpLeVFEQXdJSUFRRUxBMklBQk"
        "ZETEdjOGJYUE9kQWFaSzduTkFGa2JEaFRwcHEzWkFvdk9xSmQySjMvb3VteHhvOEJ0OGRoYkIwbUdxR0IxOFdIaU5FMDRUQktUb2FPZUNz"
        "bWRGdHVLV3BLcGRLSEQ3Ri92L2l4cWx3ejJzM0pPbHBUN3VEM24zYnhxWlRyczJxZz09IiwgInNpZ25hdHVyZSI6ICJNR1FDTUVKWnlUbj"
        "dRMG9GUUdmMzl2TUJSa3pjeFcxSE5RZWplbjYxNG12UjA2eHNIejU4Q2FKUFB5UldKbVEzbWRma1hBSXdZODBodmxCWWNFczBDdXc1c1h3"
        "Vi95QkJwWHFzTU1JdlplM1hkbTFpZVhlY1RjTFBXMHdnU0VUZVJWeDdRaE4vIn0sIHsidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWk"
        "l6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCRFU1WmJIREhpdkxoM0RTeFAxMGxpbi9hSDFyWmxtVzJwTGdycGdwYi9JZ1pJKzMyclgvUHRY"
        "VERmVmJlckRvd1ZIVEUydDBWTDc5Z0NsK21FQi9XQXg1RGVtZWppTDUyektJejNkUzVicVR3WFRMb0R2U0ppd2d3cWJmTzBGbSt3PT0iLC"
        "Aic2lnbmF0dXJlIjogIk1HUUNNQ3ZCQ3VqTUNTMXBTS3ZMRjJ1U3VRQUlrb3UrU2Fhb1ZXajFsWFVWU3A0bjZqeWRyL2NRQU81VFZGODZ3"
        "ODl1OXdJd0FXZ2ZlRWxDRWNmTXgwNzkra2pEQjlTRHdZdFRqMWVYWWljRHREOVV6b0tkcm1uNlhPeStsZ21vVHpWd2xtbUIifV0=";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case9
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case9, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // add algorithm to PublicKeyAttestation, just for coverage
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxODIwMTMi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGQCMFICCTcS8DeMzex9czJn049f/"
        "rJGaRRQKNoxA0d7Cszdq2w1m9XyCmPhG4kARQ6T0gIwZTkJspsaEQ3F/"
        "5+NwAaE+MemIdCE79kDCaFAYWEM2GwDFmlMGA7XI2PxQ3b7EsBL."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HUUNNR3dVejlydDVEcUhHRmJBTnVTMTZNZEQ5THJtWG"
        "5EMnBGOTI5U2U0Ujk3Ui91TUp1eGVMejUvSW9uelVBQmNYaWdJd05oc3dMRjhVS2x4U1B5TUo0WncyMlJTYTdZei93dlZQS1pXL2pjSGha"
        "V2RLV1YxL3pWbDZ0ODRlYzI0NkpMZ3MiLCAiYWxnb3JpdGhtIjogIlNIQTI1NndpdGhFQ0RTQSJ9LCB7InVzZXJQdWJsaWNLZXkiOiAiTU"
        "hvd0ZBWUhLb1pJemowQ0FRWUpLeVFEQXdJSUFRRUxBMklBQkZETEdjOGJYUE9kQWFaSzduTkFGa2JEaFRwcHEzWkFvdk9xSmQySjMvb3Vt"
        "eHhvOEJ0OGRoYkIwbUdxR0IxOFdIaU5FMDRUQktUb2FPZUNzbWRGdHVLV3BLcGRLSEQ3Ri92L2l4cWx3ejJzM0pPbHBUN3VEM24zYnhxWl"
        "RyczJxZz09IiwgInNpZ25hdHVyZSI6ICJNR1FDTUE5bWc0akowQzhIR1dTM0dkNWlVZXFCaGhPWFRoVUNXRW1PM010TU9qcHUrN3EzeGRm"
        "VnZySXNRMlF5Z1NOMTZ3SXdmWnlHdi9YczZtRlFEakhFc2E3bzg2NW1LUzl2RmdjYzB4Tzc2OXpGaklmYlU3aUMwS3NDdSsrREdhZnJIWj"
        "dPIiwgImFsZ29yaXRobSI6ICJTSEEzODR3aXRoRUNEU0EifSwgeyJ1c2VyUHVibGljS2V5IjogIk1Ib3dGQVlIS29aSXpqMENBUVlKS3lR"
        "REF3SUlBUUVMQTJJQUJEVTVaYkhESGl2TGgzRFN4UDEwbGluL2FIMXJabG1XMnBMZ3JwZ3BiL0lnWkkrMzJyWC9QdFhURGZWYmVyRG93Vk"
        "hURTJ0MFZMNzlnQ2wrbUVCL1dBeDVEZW1lamlMNTJ6S0l6M2RTNWJxVHdYVExvRHZTSml3Z3dxYmZPMEZtK3c9PSIsICJzaWduYXR1cmUi"
        "OiAiTUdRQ01GeUJXUlREVVRUN08vVmYwaHJTYnEvdmZKNGNVckFSVEtYVU9RTXRUZnVUTFNRbEVMTVkxa05sNVIxelB1STI3UUl3VlY2Yk"
        "dRNFlmYTJuaUJNWlc4WlVVbENDdmM3QUY1ZGdRYVR4NTZneHRNaDFKelNQeUhicnFKd05ZUnBCck9yMyIsICJhbGdvcml0aG0iOiAiU0hB"
        "Mzk0d2l0aEVDRFNBIn1d";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case10
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case10, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes VerifyCredentialCb()
    // failed when checking root key
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxOTI3MDgi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGQCMBvYjmGGfMs3SLnyHgqLe5jq78Er8SqzpvEmj/"
        "h90X6rapua6LFD2C7BCZwuNmTqdgIwWXQWQ82XtSp9xWuCOgl0ix2TTn03zQKN/"
        "zqzpj436bDEgq+JrRRx3SSvKCNkwvGg."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HUUNNRWhLUGt6bnVjc09HdFBERTRxWXhrWjVYeEJuY1"
        "c2VDNsZk9PWVcweFlBVmh0dVFzcmZBb1ljL2pBaHVIaGdYbWdJd0RadFJ1UjJIbFFjekdzbHlOVjFlbU5sREVHQ1pnTldnYlZ4UGpwWWJJ"
        "QmRYZkUwNnk0Qm1zQTFXUUN6TGFERUgifSwgeyJ1c2VyUHVibGljS2V5IjogIk1Ib3dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUUVMQT"
        "JJQUJGRExHYzhiWFBPZEFhWks3bk5BRmtiRGhUcHBxM1pBb3ZPcUpkMkozL291bXh4bzhCdDhkaGJCMG1HcUdCMThXSGlORTA0VEJLVG9h"
        "T2VDc21kRnR1S1dwS3BkS0hEN0Yvdi9peHFsd3oyczNKT2xwVDd1RDNuM2J4cVpUcnMycWc9PSIsICJzaWduYXR1cmUiOiAiTUdRQ01DWF"
        "gwUEFJbU5vejlSV2s1YnFjMWxGaUUwODZGaGFrN1dRVlBRNG1VVnNvNzhXRFVoOUNYbERIdVcyZUoydFM5d0l3YmF1NDg4bEZaYS8vbGVD"
        "RW5PaC9RWXNtZnVKZWlTRU5hYkRjSGJVcnQ3OWFVK09rME9ZTCsrcTVhNEJ0VzFBTiJ9LCB7InVzZXJQdWJsaWNLZXkiOiAiTUhvd0ZBWU"
        "hLb1pJemowQ0FRWUpLeVFEQXdJSUFRRUxBMklBQkZETEdjOGJYUE9kQWFaSzduTkFGa2JEaFRwcHEzWkFvdk9xSmQySjMvb3VteHhvOEJ0"
        "OGRoYkIwbUdxR0IxOFdIaU5FMDRUQktUb2FPZUNzbWRGdHVLV3BLcGRLSEQ3Ri92L2l4cWx3ejJzM0pPbHBUN3VEM24zYnhxWlRyczJxZz"
        "09IiwgInNpZ25hdHVyZSI6ICJNR1FDTUFMam1MTnVJRDNLRDNEODJiRXhKMVExVWZWOHVvbkd1enVoZUIvZFBXNnRDeFZ6dExQdXo4dG54"
        "K0d4UGdIZVNRSXdVNjhXVW1qazdsZzhWRDlQSU5pTm5NUWJ1RHR3MjlDZndvV0Nra1h0VzhKTjN4b3hXaWpOeXY3MGkrRlNqQUF4In1d";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case11
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case11, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes VerifyCredentialCb()
    // failed when checking intermediate key
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxOTI4MTEi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9."
        "MGQCMB6eGkGKFcsVJnohs3UFMYSnEAQ7gr3iz0ejIq9vTQYSyuHiPxRGjDInO3b4jxi0SwIwNI0liljb0PgRLZuLRL6RaXF3c5BXRS/"
        "QtkW8O/"
        "kcGXVR19N4gbyU9tUs9Pnj36tA."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HVUNNSDJuaTdTZmtDZUIyNm1RMncyK0lFWk56OXd2V2"
        "R3dTVjQksxZVp2Sjh1K0s3K3ZrNkV2Z0lURnlLcWlqZ2t6R3dJeEFJUHlyNnBIQkprU2hPM3dWUXNmdTgxNGVtNGVXbldUNENDa3h5ZExC"
        "WDdoSGs3MEl4STlET1h2SEdmMU1IdFQxdz09In0sIHsidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQV"
        "FFTEEySUFCRkRMR2M4YlhQT2RBYVpLN25OQUZrYkRoVHBwcTNaQW92T3FKZDJKMy9vdW14eG84QnQ4ZGhiQjBtR3FHQjE4V0hpTkUwNFRC"
        "S1RvYU9lQ3NtZEZ0dUtXcEtwZEtIRDdGL3YvaXhxbHd6MnMzSk9scFQ3dUQzbjNieHFaVHJzMnFnPT0iLCAic2lnbmF0dXJlIjogIk1HUU"
        "NNRXFtZytYY3plM1gzOCt0UW5Fa2p2TkNYNlJ6V2lpTE1uNzVaaTU5WCtyWGUveVlTcHNHcS8ybjFRUzVHd3hkOXdJd2NlOHJJY3dCUDd0"
        "dXVBYXY5TTBiK1hKTDZhYUNkLzdyS1F5ekdFdmlIQnlEeHE1ck4wMkd1bXlaZ2o0UmZNdDIifSwgeyJ1c2VyUHVibGljS2V5IjogIk1Ib3"
        "dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUUVMQTJJQUJEVTVaYkhESGl2TGgzRFN4UDEwbGluL2FIMXJabG1XMnBMZ3JwZ3BiL0lnWkkr"
        "MzJyWC9QdFhURGZWYmVyRG93VkhURTJ0MFZMNzlnQ2wrbUVCL1dBeDVEZW1lamlMNTJ6S0l6M2RTNWJxVHdYVExvRHZTSml3Z3dxYmZPME"
        "ZtK3c9PSIsICJzaWduYXR1cmUiOiAiTUdRQ01FcW1nK1hjemUzWDM4K3RRbkVranZOQ1g2UnpXaWlMTW43NVppNTlYK3JYZS95WVNwc0dx"
        "LzJuMVFTNUd3eGQ5d0l3Y2U4ckljd0JQN3R1dUFhdjlNMGIrWEpMNmFhQ2QvN3JLUXl6R0V2aUhCeUR4cTVyTjAyR3VteVpnajRSZk10Mi"
        "J9XQ==";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case12
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case12, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes VerifyCredentialCb()
    // failed when checking last key
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxOTI5MTMi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGQCMHRBxkv50DwejU9fnK1ZxCsLRWgbZS1+uHyJ1uTLZ9w1+"
        "zOnG67IUSppeTUGrBHwdwIwLIdK1tK6JYCUyaXcbcvNhi8bQAjHLHF99TowvQPP/IoqFhMK1p+Yn/"
        "lgUqzFtkro."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HUUNNQ1FoN2dJMTN6ZHhLaTBBQU9WWEFVQnQ5UjVOWm"
        "JyWklnampUc214bmF2ZW1JZzM0YzBwbjlSU2JTckNRZXV0NEFJd0k5VVlDZW14U05kRytHWE9hLzdxOSthVDk2aklNVVFVZjNQcUpMb2pw"
        "TlRENGhFckxGVG93TkM0NEFIQUExeTAifSwgeyJ1c2VyUHVibGljS2V5IjogIk1Ib3dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUUVMQT"
        "JJQUJGRExHYzhiWFBPZEFhWks3bk5BRmtiRGhUcHBxM1pBb3ZPcUpkMkozL291bXh4bzhCdDhkaGJCMG1HcUdCMThXSGlORTA0VEJLVG9h"
        "T2VDc21kRnR1S1dwS3BkS0hEN0Yvdi9peHFsd3oyczNKT2xwVDd1RDNuM2J4cVpUcnMycWc9PSIsICJzaWduYXR1cmUiOiAiTUdRQ01HUG"
        "9GVERFYit4QTBLcm9RODQvN3czYXMrVU4yTFhJRDJaeHZBMWR4UVJ1b3FDTHY4cTZhc3p2VkEvRmxOY2dOd0l3TEZySDhuMXNlQWxMbkhN"
        "b0xEWHQ1bS9JVHVYb3JJdWZTU3Q2enYzVGVnWHhMa3hwakJZVzFJempFM3JOZEF6aSJ9LCB7InVzZXJQdWJsaWNLZXkiOiAiTUhvd0ZBWU"
        "hLb1pJemowQ0FRWUpLeVFEQXdJSUFRRUxBMklBQkRVNVpiSERIaXZMaDNEU3hQMTBsaW4vYUgxclpsbVcycExncnBncGIvSWdaSSszMnJY"
        "L1B0WFREZlZiZXJEb3dWSFRFMnQwVkw3OWdDbCttRUIvV0F4NURlbWVqaUw1MnpLSXozZFM1YnFUd1hUTG9EdlNKaXdnd3FiZk8wRm0rdz"
        "09IiwgInNpZ25hdHVyZSI6ICJNR1FDTUNRaDdnSTEzemR4S2kwQUFPVlhBVUJ0OVI1TlpiclpJZ2pqVHNteG5hdmVtSWczNGMwcG45UlNi"
        "U3JDUWV1dDRBSXdJOVVZQ2VteFNOZEcrR1hPYS83cTkrYVQ5NmpJTVVRVWYzUHFKTG9qcE5URDRoRXJMRlRvd05DNDRBSEFBMXkwIn1d";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case13
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case13, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed attestation makes VerifyCredentialCb()
    // failed when checking payload signature
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIiwgIm1hbnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxOTMxMDgi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9."
        "MGQCMGf97swhV6iTcrsLrPOwzwFr0QsX82yHBZo9BK68zSQEJv4LCMIchJnoCnSrW2EfaQIwP98PE/9OP08thSNBS89s5FT/"
        "z+p+Kvf0hQ3SfZdN7UrvpaTKq9ZGZhP/"
        "JHJYXfEj."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HVUNNUUNHeFNtRWE5TXRSZjNLRHdrNlhDU2N1Smcwc1"
        "FJNUIvMGJkbnJOUEFQZkJxNVFOT3pVelc1Y0VQRVdzOU5lK1BVQ01FU0JmS3FkNlRmUThZZThuTlFjK1pXNDZlYk96N3VUSDRoUVZQMFQ1"
        "SjRHOWQ3ZVNEMXg0UzlKbHNTanljUk1Wdz09In0sIHsidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQV"
        "FFTEEySUFCRkRMR2M4YlhQT2RBYVpLN25OQUZrYkRoVHBwcTNaQW92T3FKZDJKMy9vdW14eG84QnQ4ZGhiQjBtR3FHQjE4V0hpTkUwNFRC"
        "S1RvYU9lQ3NtZEZ0dUtXcEtwZEtIRDdGL3YvaXhxbHd6MnMzSk9scFQ3dUQzbjNieHFaVHJzMnFnPT0iLCAic2lnbmF0dXJlIjogIk1HVU"
        "NNUUNBMmZDWUVUS2dKcGJFSkU3Qmc2QlVYMzNSWnZZTU5GdFdwOUpsbnV4a0FOS0RHa29xUXJMelJveGt1cFFSb2ZrQ01FeFYzTmFXVHVP"
        "U0M4OTBmRjFSVVVYQVdWRVBBRk5FWGlZQzJ4TXVuVWRKZTlUVWNXZHZEakc4TjdkSFRiM01GZz09In0sIHsidXNlclB1YmxpY0tleSI6IC"
        "JNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCRFU1WmJIREhpdkxoM0RTeFAxMGxpbi9hSDFyWmxtVzJwTGdycGdwYi9J"
        "Z1pJKzMyclgvUHRYVERmVmJlckRvd1ZIVEUydDBWTDc5Z0NsK21FQi9XQXg1RGVtZWppTDUyektJejNkUzVicVR3WFRMb0R2U0ppd2d3cW"
        "JmTzBGbSt3PT0iLCAic2lnbmF0dXJlIjogIk1HUUNNSGpXMWpiVTRPdnFld1VWQyt6a1dVdm5NNkJQc2MrZlA2M3BpTTM1UktRYWI0T20w"
        "cjNvNE1LQjFpZjJmNEd2NXdJd0lEaEdZTTdHcXpVRnloUW1Oc0kvdFdpTnFLcHNpazRZMmZsRDZSNlJWR0QxbjdrMDNoeEhEazh2em1RR3"
        "RnOTAifV0=";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case14
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case14, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // malformed payload makes CredentialCbToDslmCredInfo()
    // failed when CreateJson()
    const char *cred =
        "eyJ0eXAiOiAiRFNMIn0=."
        "eyJ0eXBlIjogImRlYnVnIxxxIm1hbnVmYWN0dXJlIxxxIk9IT1MiLCAiYnJhbmQiOiAixxxxNTY4IiwgIm1vZGVsIjogInJrMzU2OCIsIC"
        "Jzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlMZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjcxOTI3MDgi"
        "LCAidmVyc2lvbiI6ICIxLjAuMSJ9.MGQCMBvYjmGGfMs3SLnyHgqLe5jq78Er8SqzpvEmj/"
        "h90X6rapua6LFD2C7BCZwuNmTqdgIwWXQWQ82XtSp9xWuCOgl0ix2TTn03zQKN/"
        "zqzpj436bDEgq+JrRRx3SSvKCNkwvGg."
        "W3sidXNlclB1YmxpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2VnOUFBT0twMD"
        "JFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMmtjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVo"
        "czhGRzAwQUxsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HUUNNRWhLUGt6bnVjc09HdFBERTRxWXhrWjVYeEJuY1"
        "c2VDNsZk9PWVcweFlBVmh0dVFzcmZBb1ljL2pBaHVIaGdYbWdJd0RadFJ1UjJIbFFjekdzbHlOVjFlbU5sREVHQ1pnTldnYlZ4UGpwWWJJ"
        "QmRYZkUwNnk0Qm1zQTFXUUN6TGFERUgifSwgeyJ1c2VyUHVibGljS2V5IjogIk1Ib3dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUUVMQT"
        "JJQUJGRExHYzhiWFBPZEFhWks3bk5BRmtiRGhUcHBxM1pBb3ZPcUpkMkozL291bXh4bzhCdDhkaGJCMG1HcUdCMThXSGlORTA0VEJLVG9h"
        "T2VDc21kRnR1S1dwS3BkS0hEN0Yvdi9peHFsd3oyczNKT2xwVDd1RDNuM2J4cVpUcnMycWc9PSIsICJzaWduYXR1cmUiOiAiTUdRQ01DWF"
        "gwUEFJbU5vejlSV2s1YnFjMWxGaUUwODZGaGFrN1dRVlBRNG1VVnNvNzhXRFVoOUNYbERIdVcyZUoydFM5d0l3YmF1NDg4bEZaYS8vbGVD"
        "RW5PaC9RWXNtZnVKZWlTRU5hYkRjSGJVcnQ3OWFVK09rME9ZTCsrcTVhNEJ0VzFBTiJ9LCB7InVzZXJQdWJsaWNLZXkiOiAiTUhvd0ZBWU"
        "hLb1pJemowQ0FRWUpLeVFEQXdJSUFRRUxBMklBQkZETEdjOGJYUE9kQWFaSzduTkFGa2JEaFRwcHEzWkFvdk9xSmQySjMvb3VteHhvOEJ0"
        "OGRoYkIwbUdxR0IxOFdIaU5FMDRUQktUb2FPZUNzbWRGdHVLV3BLcGRLSEQ3Ri92L2l4cWx3ejJzM0pPbHBUN3VEM24zYnhxWlRyczJxZz"
        "09IiwgInNpZ25hdHVyZSI6ICJNR1FDTUFMam1MTnVJRDNLRDNEODJiRXhKMVExVWZWOHVvbkd1enVoZUIvZFBXNnRDeFZ6dExQdXo4dG54"
        "K0d4UGdIZVNRSXdVNjhXVW1qazdsZzhWRDlQSU5pTm5NUWJ1RHR3MjlDZndvV0Nra1h0VzhKTjN4b3hXaWpOeXY3MGkrRlNqQUF4In1d";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(ERR_PARSE_CLOUD_CRED_DATA, ret);
}

/**
 * @tc.name: VerifyDslmCredential_case15
 * @tc.desc: function VerifyDslmCredential with malformed input
 *           credentials composed in <base64-header>.<base64-payload>.<base64-signature>.<base64-attestation>
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, VerifyDslmCredential_case15, TestSize.Level0)
{
    DslmCredInfo info;
    AttestationList list;
    memset_s(&info, sizeof(DslmCredInfo), 0, sizeof(DslmCredInfo));
    memset_s(&list, sizeof(AttestationList), 0, sizeof(AttestationList));

    // success case, for covering MovePublicKeysToAttestationList()
    const char *cred = "eyJ0eXAiOiAiRFNMIn0=.eyJ0eXBlIjogImRlYnVnIiwgIm1h"
                       "bnVmYWN0dXJlIjogIk9IT1MiLCAiYnJhbmQiOiAicmszNTY4IiwgIm1vZGVsIjog"
                       "InJrMzU2OCIsICJzb2Z0d2FyZVZlcnNpb24iOiAiMy4wLjAiLCAic2VjdXJpdHlM"
                       "ZXZlbCI6ICJTTDMiLCAic2lnblRpbWUiOiAiMjAyMjExMjYxNzMzNDMiLCAidmVy"
                       "c2lvbiI6ICIxLjAuMSJ9.MGUCMEPpiP8hOZlve/H81B7AvL4Fuwe8YYAdKckLEOc"
                       "EQKKTiNRM6irjXSwboMppAFNMSgIxAILC1S6KMp6Zp2ACppXF3j3fV0PBdLZOSO1"
                       "Lm9sqtdiJ5FidaAaMYlwdLMy3vfBeSg==.W3sidXNlclB1YmxpY0tleSI6ICJNSG"
                       "93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCQiszTHJWUU13cWlwc2"
                       "VnOUFBT0twMDJFeDNKOTJlUzdrK0k5cFJPWnVvOFZFQmVvbzF6Ris2MWhtVU5TMm"
                       "tjN0c3NTBVOExOT2pUamhUVGp2NW1CQjdBdnhnUDMwc3d3SDJ1dFVoczhGRzAwQU"
                       "xsOUZuWFZsSmNpaGo5SGJ0WjNnPT0iLCAic2lnbmF0dXJlIjogIk1HVUNNUUNIUV"
                       "dzYXNYc1NpL3dJUThmWW5PRlhsaWhTem5ETG1RSjBEOGp4U3RVM2Z2bk4xZkgzUV"
                       "JJUnRzM1lIK293bE9zQ01EY2pJU0pOK2J6M2g0VUU2UTl1NW92K0RHcFRHL2Vqd0"
                       "xTU2FyMHJzZ09ZSVovODdRb0p2QllaM2hFamlDcWQ1dz09In0sIHsidXNlclB1Ym"
                       "xpY0tleSI6ICJNSG93RkFZSEtvWkl6ajBDQVFZSkt5UURBd0lJQVFFTEEySUFCRk"
                       "RMR2M4YlhQT2RBYVpLN25OQUZrYkRoVHBwcTNaQW92T3FKZDJKMy9vdW14eG84Qn"
                       "Q4ZGhiQjBtR3FHQjE4V0hpTkUwNFRCS1RvYU9lQ3NtZEZ0dUtXcEtwZEtIRDdGL3"
                       "YvaXhxbHd6MnMzSk9scFQ3dUQzbjNieHFaVHJzMnFnPT0iLCAic2lnbmF0dXJlIj"
                       "ogIk1HUUNNSGthczBkZDgwUVpiQVB6eElhMXhBYmd1WlhwNjU0T29rL2VGR2M0ek"
                       "tLczlqYjVKK24waHJDcytoa0JrR0N0b3dJd1pYcGlYUjRiS1h3RUlTZmdpSDI4dk"
                       "ZaZVQxcFJCcnFkSHd2d3ErOXcrdWQzMkhkeC90YWhHZ1kySHVZZFNHZDUifSwgey"
                       "J1c2VyUHVibGljS2V5IjogIk1Ib3dGQVlIS29aSXpqMENBUVlKS3lRREF3SUlBUU"
                       "VMQTJJQUJEVTVaYkhESGl2TGgzRFN4UDEwbGluL2FIMXJabG1XMnBMZ3JwZ3BiL0"
                       "lnWkkrMzJyWC9QdFhURGZWYmVyRG93VkhURTJ0MFZMNzlnQ2wrbUVCL1dBeDVEZW"
                       "1lamlMNTJ6S0l6M2RTNWJxVHdYVExvRHZTSml3Z3dxYmZPMEZtK3c9PSIsICJzaW"
                       "duYXR1cmUiOiAiTUdRQ01HWlI0MUdsd1RnL0xUMGtFT3lTZnRHTDBlV04zb2dXdF"
                       "o0NTZ2VkdqMm56WnhsamFlN2pveWw4cWZHNjZSTUdTQUl3S2M3V2VpQ1c1UlFGSj"
                       "ROWitSRUErNVNpMHhRVFpOdzlhb1FTUG5LVTA0L2ZIWUhkVERNWitncUY3U3RJMD"
                       "ZTbSJ9XQ==";

    int32_t ret = VerifyDslmCredential(cred, &info, &list);
    EXPECT_EQ(0, ret);
}

HWTEST_F(DslmOemPropertyTest, EcdsaVerify_case1, TestSize.Level0)
{
    const char *data = "test";
    uint32_t length = strlen(data) + 1;
    const DataBuffer srcData = {.length = length, .data = (uint8_t *)data};
    const DataBuffer sigData = {.length = length, .data = (uint8_t *)data};
    DataBuffer *pbkData = nullptr;
    uint32_t algorithm = TYPE_ECDSA_SHA_256;

    int32_t ret = EcdsaVerify(&srcData, &sigData, pbkData, algorithm);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

HWTEST_F(DslmOemPropertyTest, EcdsaVerify_case2, TestSize.Level0)
{
    const char *data = "test";
    uint32_t length = strlen(data) + 1;
    uint32_t algorithm = TYPE_ECDSA_SHA_256;

    {
        // malformed srcData
        const DataBuffer srcData = {.length = length, .data = nullptr};
        const DataBuffer sigData = {.length = length, .data = (uint8_t *)data};
        const DataBuffer pbkData = {.length = length, .data = (uint8_t *)data};

        int32_t ret = EcdsaVerify(&srcData, &sigData, &pbkData, algorithm);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        const DataBuffer srcData = {.length = length, .data = (uint8_t *)data};
        // malformed sigData
        const DataBuffer sigData = {.length = length, .data = nullptr};
        const DataBuffer pbkData = {.length = length, .data = (uint8_t *)data};

        int32_t ret = EcdsaVerify(&srcData, &sigData, &pbkData, algorithm);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        const DataBuffer srcData = {.length = length, .data = (uint8_t *)data};
        const DataBuffer sigData = {.length = length, .data = (uint8_t *)data};
        // malformed pkgData
        const DataBuffer pbkData = {.length = length, .data = nullptr};

        int32_t ret = EcdsaVerify(&srcData, &sigData, &pbkData, algorithm);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }
}

HWTEST_F(DslmOemPropertyTest, EcdsaVerify_case3, TestSize.Level0)
{
    const char *data = "test";
    uint32_t length = strlen(data) + 1;
    uint32_t algorithm = TYPE_ECDSA_SHA_256;

    {
        // malformed srcData
        const DataBuffer srcData = {.length = 0, .data = (uint8_t *)data};
        const DataBuffer sigData = {.length = length, .data = (uint8_t *)data};
        const DataBuffer pbkData = {.length = length, .data = (uint8_t *)data};

        int32_t ret = EcdsaVerify(&srcData, &sigData, &pbkData, algorithm);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        const DataBuffer srcData = {.length = length, .data = (uint8_t *)data};
        // malformed sigData
        const DataBuffer sigData = {.length = 0, .data = (uint8_t *)data};
        const DataBuffer pbkData = {.length = length, .data = (uint8_t *)data};

        int32_t ret = EcdsaVerify(&srcData, &sigData, &pbkData, algorithm);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        const DataBuffer srcData = {.length = length, .data = (uint8_t *)data};
        const DataBuffer sigData = {.length = length, .data = (uint8_t *)data};
        // malformed pkgData
        const DataBuffer pbkData = {.length = 0, .data = (uint8_t *)data};

        int32_t ret = EcdsaVerify(&srcData, &sigData, &pbkData, algorithm);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }
}

HWTEST_F(DslmOemPropertyTest, EcdsaVerify_case4, TestSize.Level0)
{
    const char *data = "test";
    uint32_t length = strlen(data) + 1;
    const DataBuffer srcData = {.length = length, .data = (uint8_t *)data};
    const DataBuffer sigData = {.length = length, .data = (uint8_t *)data};
    const DataBuffer pbkData = {.length = length, .data = (uint8_t *)data};

    {
        uint32_t algorithm = TYPE_ECDSA_SHA_256 + 2;

        int32_t ret = EcdsaVerify(&srcData, &sigData, &pbkData, algorithm);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        uint32_t algorithm = TYPE_ECDSA_SHA_384;

        int32_t ret = EcdsaVerify(&srcData, &sigData, &pbkData, algorithm);
        EXPECT_EQ(ERR_ECC_VERIFY_ERR, ret);
    }
}

HWTEST_F(DslmOemPropertyTest, FreeAttestationList_case1, TestSize.Level0)
{
    AttestationList list;
    uint32_t len = 5;
    uint8_t *dataRoot = (uint8_t *)MALLOC(len);
    uint8_t *dataIntermediate = (uint8_t *)MALLOC(len);
    uint8_t *dataLast = (uint8_t *)MALLOC(len);
    ASSERT_NE(nullptr, dataRoot);
    ASSERT_NE(nullptr, dataIntermediate);
    ASSERT_NE(nullptr, dataLast);

    PublicKey root = {.length = len, .data = dataRoot};
    PublicKey intermediate = {.length = len, .data = dataIntermediate};
    PublicKey last = {.length = len, .data = dataLast};
    list.root = root;
    list.intermediate = intermediate;
    list.last = last;

    {
        FreeAttestationList(nullptr);
    }

    {
        FreeAttestationList(&list);
        EXPECT_EQ(0U, list.root.length);
        EXPECT_EQ(0U, list.intermediate.length);
        EXPECT_EQ(0U, list.last.length);
    }
}

HWTEST_F(DslmOemPropertyTest, ValidateCertChainAdapter_case1, TestSize.Level0)
{
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
    struct DslmInfoInCertChain result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));

    int32_t ret = ValidateCertChainAdapter(data, dataLen, &result);
    EXPECT_EQ(ERR_CALL_EXTERNAL_FUNC, ret);
}

/**
 * @tc.name: FillHksParamSet_case1
 * @tc.desc: huks adapter with null input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, FillHksParamSet_case1, TestSize.Level0)
{
    int32_t ret = FillHksParamSet(nullptr, nullptr, 0);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

/**
 * @tc.name: HksGenerateKeyAdapter_case1
 * @tc.desc: huks adapter with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, HksGenerateKeyAdapter_case1, TestSize.Level0)
{
    int32_t ret = HksGenerateKeyAdapter(nullptr);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

// just for coverage
/**
 * @tc.name: DestroyDslmInfoInCertChain_case1
 * @tc.desc: function DestroyDslmInfoInCertChain with null/non-null input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, DestroyDslmInfoInCertChain_case1, TestSize.Level0)
{
    struct DslmInfoInCertChain *info = (struct DslmInfoInCertChain *)MALLOC(sizeof(struct DslmInfoInCertChain));
    ASSERT_NE(nullptr, info);
    memset_s(info, sizeof(struct DslmInfoInCertChain), 0, sizeof(struct DslmInfoInCertChain));

    DestroyDslmInfoInCertChain(nullptr);
    DestroyDslmInfoInCertChain(info);
    FREE(info);
}

// just for coverage
/**
 * @tc.name: InitDslmInfoInCertChain_case1
 * @tc.desc: function InitDslmInfoInCertChain with null input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, InitDslmInfoInCertChain_case1, TestSize.Level0)
{
    InitDslmInfoInCertChain(nullptr);
}

/**
 * @tc.name: BufferToHksCertChain_case1
 * @tc.desc: function BufferToHksCertChain with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, BufferToHksCertChain_case1, TestSize.Level0)
{
    {
        int32_t ret = BufferToHksCertChain(nullptr, 1, nullptr);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        const uint8_t data[] = {'0'};
        uint32_t len = 1;
        struct HksCertChain chain;
        memset_s(&chain, sizeof(struct HksCertChain), 0, sizeof(struct HksCertChain));

        int32_t ret = BufferToHksCertChain(data, len, &chain);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        uint8_t buff[8];
        uint32_t len = 8;
        memset_s(buff, sizeof(buff), 'c', sizeof(buff));
        TlvCommon *ptr = (TlvCommon *)buff;
        ptr->tag = 0x99;
        ptr->len = 4;
        struct HksCertChain chain;
        memset_s(&chain, sizeof(struct HksCertChain), 0, sizeof(struct HksCertChain));

        int32_t ret = BufferToHksCertChain(buff, len, &chain);
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(0U, chain.certsCount);
    }

    {
        uint8_t buff[8];
        uint32_t len = 8;
        memset_s(buff, sizeof(buff), 'c', sizeof(buff));
        TlvCommon *ptr = (TlvCommon *)buff;
        ptr->tag = 0x110;
        ptr->len = 4;
        struct HksCertChain chain;
        memset_s(&chain, sizeof(struct HksCertChain), 0, sizeof(struct HksCertChain));

        int32_t ret = BufferToHksCertChain(buff, len, &chain);
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(0U, chain.certsCount);
    }
}

/**
 * @tc.name: HksCertChainToBuffer_case1
 * @tc.desc: function HksCertChainToBuffer with malformed input
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, HksCertChainToBuffer_case1, TestSize.Level0)
{
    uint32_t len = 5;
    uint8_t *data = nullptr;

    int32_t ret = HksCertChainToBuffer(nullptr, &data, &len);
    EXPECT_EQ(ERR_INVALID_PARA, ret);
}

/**
 * @tc.name: DestroyHksCertChain_case1
 * @tc.desc: function DestroyHksCertChain with malformed inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, DestroyHksCertChain_case1, TestSize.Level0)
{
    struct HksCertChain *chain = (struct HksCertChain *)MALLOC(sizeof(struct HksCertChain));
    ASSERT_NE(nullptr, chain);
    struct HksBlob *blob = (struct HksBlob *)MALLOC(sizeof(struct HksBlob));
    ASSERT_NE(nullptr, blob);

    {
        DestroyHksCertChain(nullptr);
    }

    { // cert != NULL && cert.certs == NULL
        chain->certs = nullptr;
        DestroyHksCertChain(chain);
    }

    { // cert != NULL && cert.certs != NULL && cert.certsCount <= 0
        chain->certs = blob;
        chain->certsCount = 0;

        DestroyHksCertChain(chain);
    }

    { // cert != NULL && cert.certs != NULL && cert.certsCount == 1 && cert.certs[0].data == NULL
        blob->size = 5;
        blob->data = nullptr;
        chain->certs = blob;
        chain->certsCount = 1;

        DestroyHksCertChain(chain);
    }
}

/**
 * @tc.name: ConstructHksCertChain_case1
 * @tc.desc: function ConstructHksCertChain with malformed inputs
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DslmOemPropertyTest, ConstructHksCertChain_case1, TestSize.Level0)
{
    struct HksCertChain *chain = nullptr;

    {
        int32_t ret = ConstructHksCertChain(&chain, nullptr);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        struct HksCertChainInitParams param;
        param.certChainExist = false;
        param.certCountValid = true;
        param.certDataExist = true;

        int32_t ret = ConstructHksCertChain(&chain, &param);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        struct HksCertChainInitParams param;
        param.certChainExist = true;
        param.certCountValid = false;
        param.certDataExist = true;

        int32_t ret = ConstructHksCertChain(&chain, &param);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }

    {
        struct HksCertChainInitParams param;
        param.certChainExist = true;
        param.certCountValid = true;
        param.certDataExist = false;

        int32_t ret = ConstructHksCertChain(&chain, &param);
        EXPECT_EQ(ERR_INVALID_PARA, ret);
    }
}
} // namespace DslmUnitTest
} // namespace Security
} // namespace OHOS