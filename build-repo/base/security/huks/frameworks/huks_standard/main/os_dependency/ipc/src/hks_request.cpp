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

#include "hks_request.h"

#include <securec.h>
#include "iservice_registry.h"

#include "hks_log.h"
#include "hks_param.h"
#include "hks_template.h"

using namespace OHOS;

namespace {
constexpr int SA_ID_KEYSTORE_SERVICE = 3510;
const std::u16string SA_KEYSTORE_SERVICE_DESCRIPTOR = u"ohos.security.hks.service";
}

static sptr<IRemoteObject> GetHksProxy()
{
    auto registry = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    HKS_IF_NULL_LOGE_RETURN(registry, nullptr, "GetHksProxy registry is null")

    sptr<IRemoteObject> hksProxy = registry->GetSystemAbility(SA_ID_KEYSTORE_SERVICE);
    HKS_IF_NULL_LOGE_RETURN(hksProxy, nullptr,
        "GetHksProxy GetSystemAbility %" LOG_PUBLIC "d is null", SA_ID_KEYSTORE_SERVICE)

    return hksProxy;
}

static int32_t HksReadRequestReply(MessageParcel &reply, struct HksBlob *outBlob)
{
    int32_t ret = reply.ReadInt32();
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    uint32_t outLen = reply.ReadUint32();
    if (outLen == 0) {
        if (outBlob != nullptr) {
            outBlob->size = 0;
        }
        return ret;
    }

    HKS_IF_NOT_SUCC_RETURN(CheckBlob(outBlob), HKS_ERROR_INVALID_ARGUMENT)

    const uint8_t *outData = reply.ReadBuffer(outLen);
    HKS_IF_NULL_RETURN(outData, HKS_ERROR_IPC_MSG_FAIL)

    if (outBlob->size < outLen) {
        HKS_LOG_E("outBlob size[%" LOG_PUBLIC "u] smaller than outLen[%" LOG_PUBLIC "u]", outBlob->size, outLen);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memcpy_s(outBlob->data, outBlob->size, outData, outLen);
    outBlob->size = outLen;
    return HKS_SUCCESS;
}

int32_t HksSendRequest(enum HksMessage type, const struct HksBlob *inBlob,
    struct HksBlob *outBlob, const struct HksParamSet *paramSet)
{
    enum HksSendType sendType = HKS_SEND_TYPE_SYNC;
    struct HksParam *sendTypeParam = nullptr;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_IS_ASYNCHRONIZED, &sendTypeParam);
    if (ret == HKS_SUCCESS) {
        sendType = static_cast<enum HksSendType>(sendTypeParam->uint32Param);
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (sendType == HKS_SEND_TYPE_SYNC) {
        option = MessageOption::TF_SYNC;
    } else {
        option = MessageOption::TF_ASYNC;
    }

    data.WriteInterfaceToken(SA_KEYSTORE_SERVICE_DESCRIPTOR);

    if (outBlob == nullptr) {
        data.WriteUint32(0);
    } else {
        data.WriteUint32(outBlob->size);
    }

    data.WriteUint32(inBlob->size);
    data.WriteBuffer(inBlob->data, static_cast<size_t>(inBlob->size));

    sptr<IRemoteObject> hksProxy = GetHksProxy();
    HKS_IF_NULL_LOGE_RETURN(hksProxy, HKS_ERROR_BAD_STATE, "GetHksProxy registry is null")

    int error = hksProxy->SendRequest(type, data, reply, option);
    if (error != 0) {
        return error;
    }

    return HksReadRequestReply(reply, outBlob);
}
