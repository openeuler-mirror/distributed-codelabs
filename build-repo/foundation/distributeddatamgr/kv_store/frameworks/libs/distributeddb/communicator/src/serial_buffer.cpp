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

#include "serial_buffer.h"
#include <new>
#include "securec.h"
#include "db_errno.h"
#include "communicator_type_define.h"
#include "log_print.h"

namespace DistributedDB {
SerialBuffer::~SerialBuffer()
{
    if (!isExternalStackMemory_ && oringinalBytes_ != nullptr) {
        delete[] oringinalBytes_;
    }
    oringinalBytes_ = nullptr;
    bytes_ = nullptr;
    externalBytes_ = nullptr;
}

void SerialBuffer::SetExtendHeadLength(uint32_t extendHeaderLen)
{
    extendHeadLen_ = extendHeaderLen;
}

uint32_t SerialBuffer::GetExtendHeadLength() const
{
    return extendHeadLen_;
}

// In case buffer be directly send out, so padding is needed
int SerialBuffer::AllocBufferByPayloadLength(uint32_t inPayloadLen, uint32_t inHeaderLen)
{
    if (oringinalBytes_ != nullptr || bytes_ != nullptr || externalBytes_ != nullptr ||
        BYTE_8_ALIGN(static_cast<uint64_t>(payloadLen_) + static_cast<uint64_t>(headerLen_)) > INT32_MAX) {
        return -E_NOT_PERMIT;
    }

    payloadLen_ = inPayloadLen;
    headerLen_ = inHeaderLen;
    totalLen_ = BYTE_8_ALIGN(payloadLen_ + headerLen_);
    paddingLen_ = totalLen_ - payloadLen_ - headerLen_;
    if (totalLen_ == 0 || totalLen_ > MAX_TOTAL_LEN) {
        return -E_INVALID_ARGS;
    }
    oringinalBytes_ = new (std::nothrow) uint8_t[totalLen_ + extendHeadLen_]();
    if (oringinalBytes_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    bytes_ = oringinalBytes_ + extendHeadLen_;
    return E_OK;
}

// In case assemble fragment to frame, so no padding is needed, using frameLen as inTotalLen
int SerialBuffer::AllocBufferByTotalLength(uint32_t inTotalLen, uint32_t inHeaderLen)
{
    if (bytes_ != nullptr || externalBytes_ != nullptr) {
        return -E_NOT_PERMIT;
    }
    if (inTotalLen == 0 || inTotalLen > MAX_TOTAL_LEN || inTotalLen < inHeaderLen) {
        return -E_INVALID_ARGS;
    }

    totalLen_ = inTotalLen;
    headerLen_ = inHeaderLen;
    payloadLen_ = totalLen_ - headerLen_;
    paddingLen_ = 0;
    bytes_ = new (std::nothrow) uint8_t[inTotalLen]();
    if (bytes_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    oringinalBytes_ = bytes_;
    return E_OK;
}

// In case directly received, inTotalLen not include the padding, using frameLen as inTotalLen
int SerialBuffer::SetExternalBuff(const uint8_t *buff, uint32_t inTotalLen, uint32_t inHeaderLen)
{
    if (bytes_ != nullptr || externalBytes_ != nullptr) {
        return -E_NOT_PERMIT;
    }
    if (buff == nullptr || inTotalLen == 0 || inTotalLen > MAX_TOTAL_LEN || inTotalLen < inHeaderLen) {
        return -E_INVALID_ARGS;
    }

    totalLen_ = inTotalLen;
    headerLen_ = inHeaderLen;
    payloadLen_ = totalLen_ - headerLen_;
    paddingLen_ = 0;
    isExternalStackMemory_ = true;
    externalBytes_ = buff;
    return E_OK;
}

SerialBuffer *SerialBuffer::Clone(int &outErrorNo)
{
    SerialBuffer *twinBuffer = new (std::nothrow) SerialBuffer();
    if (twinBuffer == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    if (bytes_ == nullptr) {
        twinBuffer->bytes_ = nullptr;
    } else {
        twinBuffer->bytes_ = new (std::nothrow) uint8_t[totalLen_];
        if (twinBuffer->bytes_ == nullptr) {
            outErrorNo = -E_OUT_OF_MEMORY;
            delete twinBuffer;
            twinBuffer = nullptr;
            return nullptr;
        }
        errno_t errCode = memcpy_s(twinBuffer->bytes_, totalLen_, bytes_, totalLen_);
        if (errCode != EOK) {
            outErrorNo = -E_SECUREC_ERROR;
            delete twinBuffer;
            twinBuffer = nullptr;
            return nullptr;
        }
    }
    twinBuffer->oringinalBytes_ = twinBuffer->bytes_;
    twinBuffer->externalBytes_ = externalBytes_;
    twinBuffer->totalLen_ = totalLen_;
    twinBuffer->headerLen_ = headerLen_;
    twinBuffer->payloadLen_ = payloadLen_;
    twinBuffer->paddingLen_ = paddingLen_;
    twinBuffer->isExternalStackMemory_ = isExternalStackMemory_;
    twinBuffer->extendHeadLen_ = extendHeadLen_;
    outErrorNo = E_OK;
    return twinBuffer;
}

int SerialBuffer::ConvertForCrossThread()
{
    if (externalBytes_ == nullptr) {
        // No associated external stack memory. Do nothing and return E_OK.
        return E_OK;
    }
    // Logic guarantee all the member value: isExternalStackMemory_ is true; bytes_ is nullptr; totalLen_ is correct.
    bytes_ = new (std::nothrow) uint8_t[totalLen_];
    if (bytes_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    errno_t errCode = memcpy_s(bytes_, totalLen_, externalBytes_, totalLen_);
    if (errCode != EOK) {
        delete[] bytes_;
        bytes_ = nullptr;
        return -E_SECUREC_ERROR;
    }
    // Reset external related info
    externalBytes_ = nullptr;
    isExternalStackMemory_ = false;
    oringinalBytes_ = bytes_;
    extendHeadLen_ = 0;
    return E_OK;
}

uint32_t SerialBuffer::GetSize() const
{
    if (bytes_ == nullptr && externalBytes_ == nullptr) {
        return 0;
    }
    return totalLen_;
}

uint8_t *SerialBuffer::GetOringinalAddr() const
{
    return oringinalBytes_;
}

std::pair<uint8_t *, uint32_t> SerialBuffer::GetWritableBytesForEntireBuffer()
{
    if (bytes_ == nullptr) {
        return std::make_pair(nullptr, 0);
    } else {
        return std::make_pair(bytes_, totalLen_);
    }
}

std::pair<uint8_t *, uint32_t> SerialBuffer::GetWritableBytesForEntireFrame()
{
    if (bytes_ == nullptr) {
        return std::make_pair(nullptr, 0);
    } else {
        return std::make_pair(bytes_, totalLen_ - paddingLen_);
    }
}

std::pair<uint8_t *, uint32_t> SerialBuffer::GetWritableBytesForHeader()
{
    if (bytes_ == nullptr) {
        return std::make_pair(nullptr, 0);
    } else {
        return std::make_pair(bytes_, headerLen_);
    }
}

std::pair<uint8_t *, uint32_t> SerialBuffer::GetWritableBytesForPayload()
{
    if (bytes_ == nullptr) {
        return std::make_pair(nullptr, 0);
    } else {
        return std::make_pair(bytes_ + headerLen_, payloadLen_);
    }
}

// For receive case, using Const Function
std::pair<const uint8_t *, uint32_t> SerialBuffer::GetReadOnlyBytesForEntireBuffer() const
{
    if (isExternalStackMemory_) {
        return std::make_pair(externalBytes_, totalLen_);
    } else if (bytes_ != nullptr) {
        return std::make_pair(bytes_, totalLen_);
    } else {
        return std::make_pair(nullptr, 0);
    }
}

std::pair<const uint8_t *, uint32_t> SerialBuffer::GetReadOnlyBytesForEntireFrame() const
{
    if (isExternalStackMemory_) {
        return std::make_pair(externalBytes_, totalLen_ - paddingLen_);
    } else if (bytes_ != nullptr) {
        return std::make_pair(bytes_, totalLen_ - paddingLen_);
    } else {
        return std::make_pair(nullptr, 0);
    }
}

std::pair<const uint8_t *, uint32_t> SerialBuffer::GetReadOnlyBytesForHeader() const
{
    if (isExternalStackMemory_) {
        return std::make_pair(externalBytes_, headerLen_);
    } else if (bytes_ != nullptr) {
        return std::make_pair(bytes_, headerLen_);
    } else {
        return std::make_pair(nullptr, 0);
    }
}

std::pair<const uint8_t *, uint32_t> SerialBuffer::GetReadOnlyBytesForPayload() const
{
    if (isExternalStackMemory_) {
        return std::make_pair(externalBytes_ + headerLen_, payloadLen_);
    } else if (bytes_ != nullptr) {
        return std::make_pair(bytes_ + headerLen_, payloadLen_);
    } else {
        return std::make_pair(nullptr, 0);
    }
}
} // namespace DistributedDB
