/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stream_adaptor.h"

#include <map>
#include <mutex>
#include <string>
#include <sys/types.h>

#include "client_trans_udp_stream_interface.h"
#include "securec.h"
#include "softbus_adapter_crypto.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "stream_adaptor_listener.h"
#include "stream_common.h"

using namespace OHOS;

StreamAdaptor::StreamAdaptor(const std::string &pkgName) : pkgName_(pkgName) {}

ssize_t StreamAdaptor::GetEncryptOverhead()
{
    return OVERHEAD_LEN;
}

int StreamAdaptor::GetStreamType()
{
    return streamType_;
}

const std::pair<uint8_t*, uint32_t> StreamAdaptor::GetSessionKey()
{
    return sessionKey_;
}

int64_t StreamAdaptor::GetChannelId()
{
    return channelId_;
}

const IStreamListener *StreamAdaptor::GetListenerCallback()
{
    return callback_;
}

std::shared_ptr<Communication::SoftBus::IStreamManager> StreamAdaptor::GetStreamManager()
{
    return streamManager_;
}

bool StreamAdaptor::GetAliveState()
{
    return aliveState_;
}

void StreamAdaptor::SetAliveState(bool state)
{
    aliveState_.exchange(state);
}

void StreamAdaptor::InitAdaptor(int32_t channelId, const VtpStreamOpenParam *param, bool isServerSide,
    const IStreamListener *callback)
{
    auto adaptor = shared_from_this();
    auto adaptorListener = std::make_shared<StreamAdaptorListener>(adaptor);
    streamManager_ =  Communication::SoftBus::IStreamManager::GetInstance(nullptr, adaptorListener);
    streamManager_->PrepareEnvironment(param->pkgName);
    serverSide_ = isServerSide;
    if (sessionKey_.first == nullptr) {
        sessionKey_.first = new uint8_t[param->keyLen];
    }
    if (memcpy_s(sessionKey_.first, param->keyLen, param->sessionKey, param->keyLen) != EOK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "memcpy key error.");
        return;
    }

    sessionKey_.second = param->keyLen;
    callback_ = callback;
    streamType_ = param->type;
    channelId_ = channelId;
}

void StreamAdaptor::ReleaseAdaptor()
{
    streamManager_->DestroyStreamDataChannel();
    streamManager_->DestroyEnvironment(pkgName_);
    channelId_ = -1;
    if (sessionKey_.first != nullptr) {
        (void)memset_s(sessionKey_.first, sessionKey_.second, 0, sessionKey_.second);
        delete [] sessionKey_.first;
    }
    sessionKey_.first = nullptr;
}

ssize_t StreamAdaptor::Encrypt(const void *in, ssize_t inLen, void *out, ssize_t outLen,
    std::pair<uint8_t*, uint32_t> sessionKey)
{
    AesGcmCipherKey cipherKey = {0};

    if (inLen - OVERHEAD_LEN > outLen) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Encrypt invalid para.");
        return SOFTBUS_ERR;
    }

    cipherKey.keyLen = SESSION_KEY_LENGTH;
    if (memcpy_s(cipherKey.key, SESSION_KEY_LENGTH, sessionKey.first, sessionKey.second) != EOK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "memcpy key error.");
        return SOFTBUS_ERR;
    }

    int ret = SoftBusEncryptData(&cipherKey, (unsigned char *)in, inLen, (unsigned char *)out, (unsigned int *)&outLen);
    (void)memset_s(&cipherKey, sizeof(AesGcmCipherKey), 0, sizeof(AesGcmCipherKey));
    if (ret != SOFTBUS_OK || outLen != inLen + OVERHEAD_LEN) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Encrypt Data fail. %d", ret);
        return SOFTBUS_ENCRYPT_ERR;
    }

    return outLen;
}

ssize_t StreamAdaptor::Decrypt(const void *in, ssize_t inLen, void *out, ssize_t outLen,
    std::pair<uint8_t*, uint32_t> sessionKey)
{
    AesGcmCipherKey cipherKey = {0};

    if (inLen - OVERHEAD_LEN > outLen) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Decrypt invalid para.");
        return SOFTBUS_ERR;
    }

    cipherKey.keyLen = SESSION_KEY_LENGTH; // 256 bit encryption
    if (memcpy_s(cipherKey.key, SESSION_KEY_LENGTH, sessionKey.first, sessionKey.second) != EOK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "memcpy key error.");
        return SOFTBUS_ERR;
    }
    int ret = SoftBusDecryptData(&cipherKey, (unsigned char *)in, inLen, (unsigned char *)out, (unsigned int *)&outLen);
    (void)memset_s(&cipherKey, sizeof(AesGcmCipherKey), 0, sizeof(AesGcmCipherKey));
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Decrypt Data fail. %d ", ret);
        return SOFTBUS_DECRYPT_ERR;
    }

    return outLen;
}
