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

#include "protocol_proto.h"
#include <new>
#include <iterator>
#include "hash.h"
#include "securec.h"
#include "version.h"
#include "db_common.h"
#include "log_print.h"
#include "macro_utils.h"
#include "endian_convert.h"
#include "header_converter.h"

namespace DistributedDB {
namespace {
const uint16_t MAGIC_CODE = 0xAAAA;
// const uint16_t MAGIC_CODE = 0x1234;
const uint16_t PROTOCOL_VERSION = 0;
// Compatibility Final Method. 3 Correspond To Version 1.1.4(104)
const uint16_t DB_GLOBAL_VERSION = SOFTWARE_VERSION_CURRENT - SOFTWARE_VERSION_EARLIEST;
const uint8_t PACKET_TYPE_FRAGMENTED = BITX(0); // Use bit 0
const uint8_t PACKET_TYPE_NOT_FRAGMENTED = 0;
const uint8_t MAX_PADDING_LEN = 7;
const uint32_t LENGTH_BEFORE_SUM_RANGE = sizeof(uint64_t) + sizeof(uint64_t);
const uint32_t MAX_FRAME_LEN = 32 * 1024 * 1024; // Max 32 MB, 1024 is scale
const uint16_t MIN_FRAGMENT_COUNT = 2; // At least a frame will be splited into 2 parts
// LabelExchange(Ack) Frame Field Length
const uint32_t LABEL_VER_LEN = sizeof(uint64_t);
const uint32_t DISTINCT_VALUE_LEN = sizeof(uint64_t);
const uint32_t SEQUENCE_ID_LEN = sizeof(uint64_t);
// Note: COMM_LABEL_LENGTH is defined in communicator_type_define.h
const uint32_t COMM_LABEL_COUNT_LEN = sizeof(uint64_t);
// Local func to set and get frame Type from packet Type field
void SetFrameType(uint8_t &inPacketType, FrameType inFrameType)
{
    inPacketType &= 0x0F; // Use 0x0F to clear high for bits
    inPacketType |= (static_cast<uint8_t>(inFrameType) << 4); // frame type is on high 4 bits
}
FrameType GetFrameType(uint8_t inPacketType)
{
    uint8_t frameType = ((inPacketType & 0xF0) >> 4); // Use 0xF0 to get high 4 bits
    if (frameType >= static_cast<uint8_t>(FrameType::INVALID_MAX_FRAME_TYPE)) {
        return FrameType::INVALID_MAX_FRAME_TYPE;
    }
    return static_cast<FrameType>(frameType);
}
}

std::map<uint32_t, TransformFunc> ProtocolProto::msgIdMapFunc_;
std::shared_mutex ProtocolProto::msgIdMutex_;

uint32_t ProtocolProto::GetAppLayerFrameHeaderLength()
{
    uint32_t length = sizeof(CommPhyHeader) + sizeof(CommDivergeHeader);
    return length;
}

uint32_t ProtocolProto::GetLengthBeforeSerializedData()
{
    uint32_t length = sizeof(CommPhyHeader) + sizeof(CommDivergeHeader) + sizeof(MessageHeader);
    return length;
}

uint32_t ProtocolProto::GetCommLayerFrameHeaderLength()
{
    uint32_t length = sizeof(CommPhyHeader);
    return length;
}

SerialBuffer *ProtocolProto::ToSerialBuffer(const Message *inMsg, int &outErrorNo,
    std::shared_ptr<ExtendHeaderHandle> &extendHandle, bool onlyMsgHeader)
{
    LOGI("[HP_DEBUG] ProtocolProto::ToSerialBuffer");
    if (inMsg == nullptr) {
        outErrorNo = -E_INVALID_ARGS;
        return nullptr;
    }

    uint32_t serializeLen = 0;
    if (!onlyMsgHeader) {
        int errCode = CalculateDataSerializeLength(inMsg, serializeLen);
        if (errCode != E_OK) {
            outErrorNo = errCode;
            return nullptr;
        }
    }
    uint32_t headSize = 0;
    int errCode = GetExtendHeadDataSize(extendHandle, headSize);
    if (errCode != E_OK) {
        outErrorNo = errCode;
        return nullptr;
    }

    SerialBuffer *buffer = new (std::nothrow) SerialBuffer();
    if (buffer == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    if (headSize > 0) {
        buffer->SetExtendHeadLength(headSize);
    }
    // serializeLen maybe not 8-bytes aligned, let SerialBuffer deal with the padding.
    uint32_t payLoadLength = serializeLen + sizeof(MessageHeader);
    errCode = buffer->AllocBufferByPayloadLength(payLoadLength, GetAppLayerFrameHeaderLength());
    if (errCode != E_OK) {
        LOGE("[Proto][ToSerial] Alloc Fail, errCode=%d.", errCode);
        goto ERROR_HANDLE;
    }
    LOGI("[HP_DEBUG] ProtocolProto::ToSerialBuffer before FillExtendHeadDataIfNeed");
    errCode = FillExtendHeadDataIfNeed(extendHandle, buffer, headSize);
    if (errCode != E_OK) {
        goto ERROR_HANDLE;
    }

    // Serialize the MessageHeader and data if need
    errCode = SerializeMessage(buffer, inMsg);
    if (errCode != E_OK) {
        LOGE("[Proto][ToSerial] Serialize Fail, errCode=%d.", errCode);
        goto ERROR_HANDLE;
    }
    outErrorNo = E_OK;
    return buffer;
ERROR_HANDLE:
    outErrorNo = errCode;
    delete buffer;
    buffer = nullptr;
    return nullptr;
}

Message *ProtocolProto::ToMessage(const SerialBuffer *inBuff, int &outErrorNo, bool onlyMsgHeader)
{
    if (inBuff == nullptr) {
        outErrorNo = -E_INVALID_ARGS;
        return nullptr;
    }
    Message *outMsg = new (std::nothrow) Message();
    if (outMsg == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    int errCode = DeSerializeMessage(inBuff, outMsg, onlyMsgHeader);
    if (errCode != E_OK && errCode != -E_NOT_REGISTER) {
        LOGE("[Proto][ToMessage] DeSerialize Fail, errCode=%d.", errCode);
        outErrorNo = errCode;
        delete outMsg;
        outMsg = nullptr;
        return nullptr;
    }
    // If messageId not register in this software version, we return errCode and the Message without an object.
    outErrorNo = errCode;
    return outMsg;
}

SerialBuffer *ProtocolProto::BuildEmptyFrameForVersionNegotiate(int &outErrorNo)
{
    SerialBuffer *buffer = new (std::nothrow) SerialBuffer();
    if (buffer == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
        return nullptr;
    }

    // Empty frame has no payload, only header
    int errCode = buffer->AllocBufferByPayloadLength(0, GetCommLayerFrameHeaderLength());
    if (errCode != E_OK) {
        LOGE("[Proto][BuildEmpty] Alloc Fail, errCode=%d.", errCode);
        outErrorNo = errCode;
        delete buffer;
        buffer = nullptr;
        return nullptr;
    }
    outErrorNo = E_OK;
    return buffer;
}

SerialBuffer *ProtocolProto::BuildFeedbackMessageFrame(const Message *inMsg, const LabelType &inLabel,
    int &outErrorNo)
{
    std::shared_ptr<ExtendHeaderHandle> extendHandle = nullptr;
    SerialBuffer *buffer = ToSerialBuffer(inMsg, outErrorNo, extendHandle, true);
    if (buffer == nullptr) {
        // outErrorNo had already been set in ToSerialBuffer
        return nullptr;
    }
    int errCode = ProtocolProto::SetDivergeHeader(buffer, inLabel);
    if (errCode != E_OK) {
        LOGE("[Proto][BuildFeedback] Set DivergeHeader fail, label=%s, errCode=%d.", VEC_TO_STR(inLabel), errCode);
        outErrorNo = errCode;
        delete buffer;
        buffer = nullptr;
        return nullptr;
    }
    outErrorNo = E_OK;
    return buffer;
}

SerialBuffer *ProtocolProto::BuildLabelExchange(uint64_t inDistinctValue, uint64_t inSequenceId,
    const std::set<LabelType> &inLabels, int &outErrorNo)
{
    // Size of inLabels won't be too large.
    // The upper layer code(inside this communicator module) guarantee that size of each Label equals COMM_LABEL_LENGTH
    uint64_t payloadLen = LABEL_VER_LEN + DISTINCT_VALUE_LEN + SEQUENCE_ID_LEN + COMM_LABEL_COUNT_LEN +
        inLabels.size() * COMM_LABEL_LENGTH;
    if (payloadLen > INT32_MAX) {
        outErrorNo = -E_INVALID_ARGS;
        return nullptr;
    }
    SerialBuffer *buffer = new (std::nothrow) SerialBuffer();
    if (buffer == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    int errCode = buffer->AllocBufferByPayloadLength(static_cast<uint32_t>(payloadLen),
        GetCommLayerFrameHeaderLength());
    if (errCode != E_OK) {
        LOGE("[Proto][BuildLabel] Alloc Fail, errCode=%d.", errCode);
        outErrorNo = errCode;
        delete buffer;
        buffer = nullptr;
        return nullptr;
    }

    auto payloadByteLen = buffer->GetWritableBytesForPayload();
    auto fieldPtr = reinterpret_cast<uint64_t *>(payloadByteLen.first);
    *fieldPtr++ = HostToNet(static_cast<uint64_t>(PROTOCOL_VERSION));
    *fieldPtr++ = HostToNet(inDistinctValue);
    *fieldPtr++ = HostToNet(inSequenceId);
    *fieldPtr++ = HostToNet(static_cast<uint64_t>(inLabels.size()));
    // Note: don't worry, memory length had been carefully calculated above
    auto bytePtr = reinterpret_cast<uint8_t *>(fieldPtr);
    for (const auto &eachLabel : inLabels) {
        for (const auto &eachByte : eachLabel) {
            *bytePtr++ = eachByte;
        }
    }
    outErrorNo = E_OK;
    return buffer;
}

SerialBuffer *ProtocolProto::BuildLabelExchangeAck(uint64_t inDistinctValue, uint64_t inSequenceId, int &outErrorNo)
{
    uint32_t payloadLen = LABEL_VER_LEN + DISTINCT_VALUE_LEN + SEQUENCE_ID_LEN;
    SerialBuffer *buffer = new (std::nothrow) SerialBuffer();
    if (buffer == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    int errCode = buffer->AllocBufferByPayloadLength(payloadLen, GetCommLayerFrameHeaderLength());
    if (errCode != E_OK) {
        LOGE("[Proto][BuildLabelAck] Alloc Fail, errCode=%d.", errCode);
        outErrorNo = errCode;
        delete buffer;
        buffer = nullptr;
        return nullptr;
    }

    auto payloadByteLen = buffer->GetWritableBytesForPayload();
    auto fieldPtr = reinterpret_cast<uint64_t *>(payloadByteLen.first);
    *fieldPtr++ = HostToNet(static_cast<uint64_t>(PROTOCOL_VERSION));
    *fieldPtr++ = HostToNet(inDistinctValue);
    *fieldPtr++ = HostToNet(inSequenceId);
    outErrorNo = E_OK;
    return buffer;
}

int ProtocolProto::SplitFrameIntoPacketsIfNeed(const SerialBuffer *inBuff, uint32_t inMtuSize,
    std::vector<std::pair<std::vector<uint8_t>, uint32_t>> &outPieces)
{
    auto bufferBytesLen = inBuff->GetReadOnlyBytesForEntireBuffer();
    if ((bufferBytesLen.second + inBuff->GetExtendHeadLength()) <= inMtuSize) {
        return E_OK;
    }
    uint32_t modifyMtuSize = inMtuSize - inBuff->GetExtendHeadLength();
    // Do Fragmentaion! This function aims at calculate how many fragments to be split into.
    auto frameBytesLen = inBuff->GetReadOnlyBytesForEntireFrame(); // Padding not in the range of fragmentation.
    uint32_t lengthToSplit = frameBytesLen.second - sizeof(CommPhyHeader); // The former is always larger than latter.
    // The inMtuSize pass from CommunicatorAggregator is large enough to be subtract by the latter two.
    uint32_t maxFragmentLen = modifyMtuSize - sizeof(CommPhyHeader) - sizeof(CommPhyOptHeader);
    // It can be proved that lengthToSplit is always larger than maxFragmentLen, so quotient won't be zero.
    // The maxFragmentLen won't be zero and in fact large enough to make sure no precision loss during division
    uint16_t quotient = lengthToSplit / maxFragmentLen;
    uint32_t remainder = lengthToSplit % maxFragmentLen;
    // Finally we get the fragCount for this frame
    uint16_t fragCount = ((remainder == 0) ? quotient : (quotient + 1));
    // Get CommPhyHeader of this frame to be modified for each packets (Header in network endian)
    auto oriPhyHeader = reinterpret_cast<const CommPhyHeader *>(frameBytesLen.first);
    FrameFragmentInfo fragInfo = {inBuff->GetOringinalAddr(), inBuff->GetExtendHeadLength(), lengthToSplit, fragCount};
    return FrameFragmentation(frameBytesLen.first + sizeof(CommPhyHeader), fragInfo, *oriPhyHeader, outPieces);
}

int ProtocolProto::AnalyzeSplitStructure(const ParseResult &inResult, uint32_t &outFragLen, uint32_t &outLastFragLen)
{
    uint32_t frameLen = inResult.GetFrameLen();
    uint16_t fragCount = inResult.GetFragCount();
    uint16_t fragNo = inResult.GetFragNo();

    // Firstly: Check frameLen
    if (frameLen <= sizeof(CommPhyHeader) || frameLen > MAX_FRAME_LEN) {
        LOGE("[Proto][ParsePhyOpt] FrameLen=%u illegal.", frameLen);
        return -E_PARSE_FAIL;
    }

    // Secondly: Check fragCount and fragNo
    uint32_t lengthBeSplit = frameLen - sizeof(CommPhyHeader);
    if (fragCount == 0 || fragCount < MIN_FRAGMENT_COUNT || fragCount > lengthBeSplit || fragNo >= fragCount) {
        LOGE("[Proto][ParsePhyOpt] FragCount=%u or fragNo=%u illegal.", fragCount, fragNo);
        return -E_PARSE_FAIL;
    }

    // Finally: Check length relation deeply
    uint32_t quotient = lengthBeSplit / fragCount;
    uint16_t remainder = lengthBeSplit % fragCount;
    outFragLen = quotient;
    outLastFragLen = quotient + remainder;
    uint32_t thisFragLen = ((fragNo != fragCount - 1) ? outFragLen : outLastFragLen); // subtract by 1 for index
    if (sizeof(CommPhyHeader) + sizeof(CommPhyOptHeader) + thisFragLen +
        inResult.GetPaddingLen() != inResult.GetPacketLen()) {
        LOGE("[Proto][ParsePhyOpt] Length Error: FrameLen=%u, FragCount=%u, fragNo=%u, PaddingLen=%u, PacketLen=%u",
            frameLen, fragCount, fragNo, inResult.GetPaddingLen(), inResult.GetPacketLen());
        return -E_PARSE_FAIL;
    }

    return E_OK;
}

int ProtocolProto::CombinePacketIntoFrame(SerialBuffer *inFrame, const uint8_t *pktBytes, uint32_t pktLength,
    uint32_t fragOffset, uint32_t fragLength)
{
    // inFrame is the destination, pktBytes and pktLength are the source, fragOffset and fragLength give the boundary
    // Firstly: Check the length relation of source, even this check is not supposed to fail
    if (sizeof(CommPhyHeader) + sizeof(CommPhyOptHeader) + fragLength > pktLength) {
        return -E_LENGTH_ERROR;
    }
    // Secondly: Check the length relation of destination, even this check is not supposed to fail
    auto frameByteLen = inFrame->GetWritableBytesForEntireFrame();
    if (sizeof(CommPhyHeader) + fragOffset + fragLength > frameByteLen.second) {
        return -E_LENGTH_ERROR;
    }
    // Finally: Do Combination!
    const uint8_t *srcByteHead = pktBytes + sizeof(CommPhyHeader) + sizeof(CommPhyOptHeader);
    uint8_t *dstByteHead = frameByteLen.first + sizeof(CommPhyHeader) + fragOffset;
    uint32_t dstLeftLen = frameByteLen.second - sizeof(CommPhyHeader) - fragOffset;
    errno_t errCode = memcpy_s(dstByteHead, dstLeftLen, srcByteHead, fragLength);
    if (errCode != EOK) {
        return -E_SECUREC_ERROR;
    }
    return E_OK;
}

int ProtocolProto::RegTransformFunction(uint32_t msgId, const TransformFunc &inFunc)
{
    std::unique_lock<std::shared_mutex> autoLock(msgIdMutex_);
    if (msgIdMapFunc_.count(msgId) != 0) {
        return -E_ALREADY_REGISTER;
    }
    if (!inFunc.computeFunc || !inFunc.serializeFunc || !inFunc.deserializeFunc) {
        return -E_INVALID_ARGS;
    }
    msgIdMapFunc_[msgId] = inFunc;
    return E_OK;
}

void ProtocolProto::UnRegTransformFunction(uint32_t msgId)
{
    std::unique_lock<std::shared_mutex> autoLock(msgIdMutex_);
    if (msgIdMapFunc_.count(msgId) != 0) {
        msgIdMapFunc_.erase(msgId);
    }
}

int ProtocolProto::SetDivergeHeader(SerialBuffer *inBuff, const LabelType &inCommLabel)
{
    if (inBuff == nullptr) {
        return -E_INVALID_ARGS;
    }
    auto headerByteLen = inBuff->GetWritableBytesForHeader();
    if (headerByteLen.second != GetAppLayerFrameHeaderLength()) {
        return -E_INVALID_ARGS;
    }
    auto payloadByteLen = inBuff->GetReadOnlyBytesForPayload();

    CommDivergeHeader divergeHeader;
    divergeHeader.version = PROTOCOL_VERSION;
    divergeHeader.reserved = 0;
    divergeHeader.payLoadLen = payloadByteLen.second;
    // The upper layer code(inside this communicator module) guarantee that size of inCommLabel equal COMM_LABEL_LENGTH
    for (unsigned int i = 0; i < COMM_LABEL_LENGTH; i++) {
        divergeHeader.commLabel[i] = inCommLabel[i];
    }
    HeaderConverter::ConvertHostToNet(divergeHeader, divergeHeader);

    errno_t errCode = memcpy_s(headerByteLen.first + sizeof(CommPhyHeader),
        headerByteLen.second - sizeof(CommPhyHeader), &divergeHeader, sizeof(CommDivergeHeader));
    if (errCode != EOK) {
        return -E_SECUREC_ERROR;
    }
    return E_OK;
}

namespace {
void FillPhyHeaderLenInfo(CommPhyHeader &header, uint32_t packetLen, uint64_t sum, uint8_t type, uint8_t paddingLen)
{
    header.packetLen = packetLen;
    header.checkSum = sum;
    header.packetType |= type;
    header.paddingLen = paddingLen;
}
}

int ProtocolProto::SetPhyHeader(SerialBuffer *inBuff, const PhyHeaderInfo &inInfo)
{
    if (inBuff == nullptr) {
        return -E_INVALID_ARGS;
    }
    auto headerByteLen = inBuff->GetWritableBytesForHeader();
    if (headerByteLen.second < sizeof(CommPhyHeader)) {
        return -E_INVALID_ARGS;
    }
    auto bufferByteLen = inBuff->GetReadOnlyBytesForEntireBuffer();
    auto frameByteLen = inBuff->GetReadOnlyBytesForEntireFrame();

    uint32_t packetLen = bufferByteLen.second;
    uint8_t paddingLen = static_cast<uint8_t>(bufferByteLen.second - frameByteLen.second);
    uint8_t packetType = PACKET_TYPE_NOT_FRAGMENTED;
    if (inInfo.frameType != FrameType::INVALID_MAX_FRAME_TYPE) {
        SetFrameType(packetType, inInfo.frameType);
    } else {
        return -E_INVALID_ARGS;
    }

    CommPhyHeader phyHeader;
    phyHeader.magic = MAGIC_CODE;
    phyHeader.version = PROTOCOL_VERSION;
    phyHeader.sourceId = inInfo.sourceId;
    phyHeader.frameId = inInfo.frameId;
    phyHeader.packetType = 0;
    phyHeader.dbIntVer = DB_GLOBAL_VERSION;
    FillPhyHeaderLenInfo(phyHeader, packetLen, 0, packetType, paddingLen); // Sum is calculated afterwards
    HeaderConverter::ConvertHostToNet(phyHeader, phyHeader);

    errno_t retCode = memcpy_s(headerByteLen.first, headerByteLen.second, &phyHeader, sizeof(CommPhyHeader));
    if (retCode != EOK) {
        return -E_SECUREC_ERROR;
    }

    uint64_t sumResult = 0;
    int errCode = CalculateXorSum(bufferByteLen.first + LENGTH_BEFORE_SUM_RANGE,
        bufferByteLen.second - LENGTH_BEFORE_SUM_RANGE, sumResult);
    if (errCode != E_OK) {
        return -E_SUM_CALCULATE_FAIL;
    }

    auto ptrPhyHeader = reinterpret_cast<CommPhyHeader *>(headerByteLen.first);
    ptrPhyHeader->checkSum = HostToNet(sumResult);

    return E_OK;
}

int ProtocolProto::CheckAndParsePacket(const std::string &srcTarget, const uint8_t *bytes, uint32_t length,
    ParseResult &outResult)
{
    if (bytes == nullptr || length > MAX_TOTAL_LEN) {
        return -E_INVALID_ARGS;
    }
    int errCode = ParseCommPhyHeader(srcTarget, bytes, length, outResult);
    if (errCode != E_OK) {
        LOGE("[Proto][ParsePacket] Parse PhyHeader Fail, errCode=%d.", errCode);
        return errCode;
    }

    if (outResult.GetFrameTypeInfo() == FrameType::EMPTY) {
        return E_OK; // Do nothing more for empty frame
    }

    if (outResult.IsFragment()) {
        errCode = ParseCommPhyOptHeader(bytes, length, outResult);
        if (errCode != E_OK) {
            LOGE("[Proto][ParsePacket] Parse CommPhyOptHeader Fail, errCode=%d.", errCode);
            return errCode;
        }
    } else if (outResult.GetFrameTypeInfo() != FrameType::APPLICATION_MESSAGE) {
        errCode = ParseCommLayerPayload(bytes, length, outResult);
        if (errCode != E_OK) {
            LOGE("[Proto][ParsePacket] Parse CommLayerPayload Fail, errCode=%d.", errCode);
            return errCode;
        }
    } else {
        errCode = ParseCommDivergeHeader(bytes, length, outResult);
        if (errCode != E_OK) {
            LOGE("[Proto][ParsePacket] Parse DivergeHeader Fail, errCode=%d.", errCode);
            return errCode;
        }
    }
    return E_OK;
}

int ProtocolProto::CheckAndParseFrame(const SerialBuffer *inBuff, ParseResult &outResult)
{
    if (inBuff == nullptr || outResult.IsFragment()) {
        return -E_INTERNAL_ERROR;
    }
    auto frameBytesLen = inBuff->GetReadOnlyBytesForEntireFrame();
    if (outResult.GetFrameTypeInfo() != FrameType::APPLICATION_MESSAGE) {
        int errCode = ParseCommLayerPayload(frameBytesLen.first, frameBytesLen.second, outResult);
        if (errCode != E_OK) {
            LOGE("[Proto][ParseFrame] Parse CommLayerPayload Fail, errCode=%d.", errCode);
            return errCode;
        }
    } else {
        int errCode = ParseCommDivergeHeader(frameBytesLen.first, frameBytesLen.second, outResult);
        if (errCode != E_OK) {
            LOGE("[Proto][ParseFrame] Parse DivergeHeader Fail, errCode=%d.", errCode);
            return errCode;
        }
    }
    return E_OK;
}

void ProtocolProto::DisplayPacketInformation(const uint8_t *bytes, uint32_t length)
{
    static std::map<FrameType, std::string> frameTypeStr{
        {FrameType::EMPTY, "EmptyFrame"},
        {FrameType::APPLICATION_MESSAGE, "AppLayerFrame"},
        {FrameType::COMMUNICATION_LABEL_EXCHANGE, "CommLayerFrame_LabelExchange"},
        {FrameType::COMMUNICATION_LABEL_EXCHANGE_ACK, "CommLayerFrame_LabelExchangeAck"}};

    if (length < sizeof(CommPhyHeader)) {
        return;
    }
    auto phyHeader = reinterpret_cast<const CommPhyHeader *>(bytes);
    uint32_t frameId = NetToHost(phyHeader->frameId);
    uint8_t pktType = NetToHost(phyHeader->packetType);
    bool isFragment = ((pktType & PACKET_TYPE_FRAGMENTED) != 0);
    FrameType frameType = GetFrameType(pktType);
    if (frameType == FrameType::INVALID_MAX_FRAME_TYPE) {
        LOGW("[Proto][Display] This is unrecognized frame, pktType=%" PRIu8 ".", pktType);
        return;
    }
    if (isFragment) {
        if (length < sizeof(CommPhyHeader) + sizeof(CommPhyOptHeader)) {
            return;
        }
        auto phyOpt = reinterpret_cast<const CommPhyOptHeader *>(bytes + sizeof(CommPhyHeader));
        LOGI("[Proto][Display] This is %s, frameId=%u, frameLen=%u, fragCount=%u, fragNo=%u.",
            frameTypeStr[frameType].c_str(), frameId, NetToHost(phyOpt->frameLen),
            NetToHost(phyOpt->fragCount), NetToHost(phyOpt->fragNo));
    } else {
        LOGI("[Proto][Display] This is %s, frameId=%u.", frameTypeStr[frameType].c_str(), frameId);
    }
}

int ProtocolProto::CalculateXorSum(const uint8_t *bytes, uint32_t length, uint64_t &outSum)
{
    if (length % sizeof(uint64_t) != 0) {
        LOGE("[Proto][CalcuXorSum] Length=%d not multiple of eight.", length);
        return -E_LENGTH_ERROR;
    }
    int count = length / sizeof(uint64_t);
    auto array = reinterpret_cast<const uint64_t *>(bytes);
    outSum = 0;
    for (int i = 0; i < count; i++) {
        outSum ^= array[i];
    }
    return E_OK;
}

int ProtocolProto::CalculateDataSerializeLength(const Message *inMsg, uint32_t &outLength)
{
    uint32_t messageId = inMsg->GetMessageId();
    TransformFunc function;
    if (GetTransformFunc(messageId, function) != E_OK) {
        LOGE("[Proto][CalcuDataSerialLen] Not registered for messageId=%" PRIu32 ".", messageId);
        return -E_NOT_REGISTER;
    }

    uint32_t serializeLen = function.computeFunc(inMsg);
    uint32_t alignedLen = BYTE_8_ALIGN(serializeLen);
    // Currently not allowed the upper module to send a message without data. Regard serializeLen zero as abnormal.
    if (serializeLen == 0 || alignedLen > MAX_FRAME_LEN - GetLengthBeforeSerializedData()) {
        LOGE("[Proto][CalcuDataSerialLen] Length too large, msgId=%u, serializeLen=%u, alignedLen=%u.",
            messageId, serializeLen, alignedLen);
        return -E_LENGTH_ERROR;
    }
    // Attention: return the serializeLen nor the alignedLen. Let SerialBuffer to deal with the padding
    outLength = serializeLen;
    return E_OK;
}

int ProtocolProto::SerializeMessage(SerialBuffer *inBuff, const Message *inMsg)
{
    auto payloadByteLen = inBuff->GetWritableBytesForPayload();
    if (payloadByteLen.second < sizeof(MessageHeader)) { // For equal, only msgHeader case
        LOGE("[Proto][Serialize] Length error, payload length=%u.", payloadByteLen.second);
        return -E_LENGTH_ERROR;
    }
    uint32_t dataLen = payloadByteLen.second - sizeof(MessageHeader);

    auto messageHdr = reinterpret_cast<MessageHeader *>(payloadByteLen.first);
    messageHdr->version = inMsg->GetVersion();
    messageHdr->messageType = inMsg->GetMessageType();
    messageHdr->messageId = inMsg->GetMessageId();
    messageHdr->sessionId = inMsg->GetSessionId();
    messageHdr->sequenceId = inMsg->GetSequenceId();
    messageHdr->errorNo = inMsg->GetErrorNo();
    messageHdr->dataLen = dataLen;
    HeaderConverter::ConvertHostToNet(*messageHdr, *messageHdr);

    if (dataLen == 0) {
        // For zero dataLen, we don't need to serialize data part
        return E_OK;
    }
    // If dataLen not zero, the TransformFunc of this messageId must exist, the caller's logic guarantee it
    TransformFunc function;
    if (GetTransformFunc(inMsg->GetMessageId(), function) != E_OK) {
        LOGE("[Proto][Serialize] Not register, messageId=%" PRIu32 ".", inMsg->GetMessageId());
        return -E_NOT_REGISTER;
    }
    int result = function.serializeFunc(payloadByteLen.first + sizeof(MessageHeader), dataLen, inMsg);
    if (result != E_OK) {
        LOGE("[Proto][Serialize] SerializeFunc Fail, result=%d.", result);
        return -E_SERIALIZE_ERROR;
    }
    return E_OK;
}

int ProtocolProto::DeSerializeMessage(const SerialBuffer *inBuff, Message *inMsg, bool onlyMsgHeader)
{
    auto payloadByteLen = inBuff->GetReadOnlyBytesForPayload();
    // Check version before parse field
    if (payloadByteLen.second < sizeof(uint16_t)) {
        return -E_LENGTH_ERROR;
    }
    uint16_t version = NetToHost(*(reinterpret_cast<const uint16_t *>(payloadByteLen.first)));
    if (!IsSupportMessageVersion(version)) {
        LOGE("[Proto][DeSerialize] Version=%u not support.", version);
        return -E_VERSION_NOT_SUPPORT;
    }

    if (payloadByteLen.second < sizeof(MessageHeader)) {
        LOGE("[Proto][DeSerialize] Length error, payload length=%u.", payloadByteLen.second);
        return -E_LENGTH_ERROR;
    }
    auto oriMsgHeader = reinterpret_cast<const MessageHeader *>(payloadByteLen.first);
    MessageHeader messageHdr;
    HeaderConverter::ConvertNetToHost(*oriMsgHeader, messageHdr);
    inMsg->SetVersion(version);
    inMsg->SetMessageType(messageHdr.messageType);
    inMsg->SetMessageId(messageHdr.messageId);
    inMsg->SetSessionId(messageHdr.sessionId);
    inMsg->SetSequenceId(messageHdr.sequenceId);
    inMsg->SetErrorNo(messageHdr.errorNo);
    uint32_t dataLen = payloadByteLen.second - sizeof(MessageHeader);
    if (dataLen != messageHdr.dataLen) {
        LOGE("[Proto][DeSerialize] dataLen=%u, msgDataLen=%u.", dataLen, messageHdr.dataLen);
        return -E_LENGTH_ERROR;
    }
    // It is better to check FeedbackMessage first and check onlyMsgHeader flag later
    if (IsFeedbackErrorMessage(messageHdr.errorNo)) {
        LOGI("[Proto][DeSerialize] Feedback Message with errorNo=%u.", messageHdr.errorNo);
        return E_OK;
    }
    if (onlyMsgHeader || dataLen == 0) { // Do not need to deserialize data
        return E_OK;
    }
    TransformFunc function;
    if (GetTransformFunc(inMsg->GetMessageId(), function) != E_OK) {
        LOGE("[Proto][DeSerialize] Not register, messageId=%" PRIu32 ".", inMsg->GetMessageId());
        return -E_NOT_REGISTER;
    }
    int result = function.deserializeFunc(payloadByteLen.first + sizeof(MessageHeader), dataLen, inMsg);
    if (result != E_OK) {
        LOGE("[Proto][DeSerialize] DeserializeFunc Fail, result=%d.", result);
        return -E_DESERIALIZE_ERROR;
    }
    return E_OK;
}

bool ProtocolProto::IsSupportMessageVersion(uint16_t version)
{
    return (version == MSG_VERSION_BASE || version == MSG_VERSION_EXT);
}

bool ProtocolProto::IsFeedbackErrorMessage(uint32_t errorNo)
{
    return (errorNo == E_FEEDBACK_UNKNOWN_MESSAGE || errorNo == E_FEEDBACK_COMMUNICATOR_NOT_FOUND);
}

int ProtocolProto::ParseCommPhyHeaderCheckMagicAndVersion(const uint8_t *bytes, uint32_t length)
{
    // At least magic and version should exist
    if (length < sizeof(uint16_t) + sizeof(uint16_t)) {
        LOGE("[Proto][ParsePhyCheckVer] Length of Bytes Error.");
        return -E_LENGTH_ERROR;
    }
    auto fieldPtr = reinterpret_cast<const uint16_t *>(bytes);
    uint16_t magic = NetToHost(*fieldPtr++);
    uint16_t version = NetToHost(*fieldPtr++);

    if (magic != MAGIC_CODE) {
        LOGE("[Proto][ParsePhyCheckVer] MagicCode=%u Error.", magic);
        return -E_PARSE_FAIL;
    }
    if (version != PROTOCOL_VERSION) {
        LOGE("[Proto][ParsePhyCheckVer] Version=%u Error.", version);
        return -E_VERSION_NOT_SUPPORT;
    }
    return E_OK;
}

int ProtocolProto::ParseCommPhyHeaderCheckField(const std::string &srcTarget, const CommPhyHeader &phyHeader,
    const uint8_t *bytes, uint32_t length)
{
    if (phyHeader.sourceId != Hash::HashFunc(srcTarget)) {
        LOGE("[Proto][ParsePhyCheck] SourceId Error: inSourceId=%llu, srcTarget=%s{private}, hashId=%llu.",
            ULL(phyHeader.sourceId), srcTarget.c_str(), ULL(Hash::HashFunc(srcTarget)));
        return -E_PARSE_FAIL;
    }
    if (phyHeader.packetLen != length) {
        LOGE("[Proto][ParsePhyCheck] PacketLen=%u Mismatch length=%u.", phyHeader.packetLen, length);
        return -E_PARSE_FAIL;
    }
    if (phyHeader.paddingLen > MAX_PADDING_LEN) {
        LOGE("[Proto][ParsePhyCheck] PaddingLen=%u Error.", phyHeader.paddingLen);
        return -E_PARSE_FAIL;
    }
    if (sizeof(CommPhyHeader) + phyHeader.paddingLen > phyHeader.packetLen) {
        LOGE("[Proto][ParsePhyCheck] PaddingLen Add PhyHeader Greater Than PacketLen.");
        return -E_PARSE_FAIL;
    }
    uint64_t sumResult = 0;
    int errCode = CalculateXorSum(bytes + LENGTH_BEFORE_SUM_RANGE, length - LENGTH_BEFORE_SUM_RANGE, sumResult);
    if (errCode != E_OK) {
        LOGE("[Proto][ParsePhyCheck] Calculate Sum Fail.");
        return -E_SUM_CALCULATE_FAIL;
    }
    if (phyHeader.checkSum != sumResult) {
        LOGE("[Proto][ParsePhyCheck] Sum Mismatch, checkSum=%llu, sumResult=%llu.",
            ULL(phyHeader.checkSum), ULL(sumResult));
        return -E_SUM_MISMATCH;
    }
    return E_OK;
}

int ProtocolProto::ParseCommPhyHeader(const std::string &srcTarget, const uint8_t *bytes, uint32_t length,
    ParseResult &inResult)
{
    int errCode = ParseCommPhyHeaderCheckMagicAndVersion(bytes, length);
    if (errCode != E_OK) {
        LOGE("[Proto][ParsePhy] Check Magic And Version Fail.");
        return errCode;
    }

    if (length < sizeof(CommPhyHeader)) {
        LOGE("[Proto][ParsePhy] Length of Bytes Error.");
        return -E_PARSE_FAIL;
    }
    auto phyHeaderOri = reinterpret_cast<const CommPhyHeader *>(bytes);
    CommPhyHeader phyHeader;
    HeaderConverter::ConvertNetToHost(*phyHeaderOri, phyHeader);
    errCode = ParseCommPhyHeaderCheckField(srcTarget, phyHeader, bytes, length);
    if (errCode != E_OK) {
        LOGE("[Proto][ParsePhy] Check Field Fail.");
        return errCode;
    }

    inResult.SetFrameId(phyHeader.frameId);
    inResult.SetSourceId(phyHeader.sourceId);
    inResult.SetPacketLen(phyHeader.packetLen);
    inResult.SetPaddingLen(phyHeader.paddingLen);
    inResult.SetDbVersion(phyHeader.dbIntVer);
    if ((phyHeader.packetType & PACKET_TYPE_FRAGMENTED) != 0) {
        inResult.SetFragmentFlag(true);
    } // FragmentFlag default is false
    FrameType frameType = GetFrameType(phyHeader.packetType);
    if (frameType == FrameType::INVALID_MAX_FRAME_TYPE) {
        LOGW("[Proto][ParsePhy] Unrecognized frame, pktType=%u.", phyHeader.packetType);
        return -E_FRAME_TYPE_NOT_SUPPORT;
    }
    inResult.SetFrameTypeInfo(frameType);
    return E_OK;
}

int ProtocolProto::ParseCommPhyOptHeader(const uint8_t *bytes, uint32_t length, ParseResult &inResult)
{
    if (length < sizeof(CommPhyHeader) + sizeof(CommPhyOptHeader)) {
        LOGE("[Proto][ParsePhyOpt] Length of Bytes Error.");
        return -E_LENGTH_ERROR;
    }
    auto headerOri = reinterpret_cast<const CommPhyOptHeader *>(bytes + sizeof(CommPhyHeader));
    CommPhyOptHeader phyOptHeader;
    HeaderConverter::ConvertNetToHost(*headerOri, phyOptHeader);

    // Check of CommPhyOptHeader field will be done in the procedure of FrameCombiner
    inResult.SetFrameLen(phyOptHeader.frameLen);
    inResult.SetFragCount(phyOptHeader.fragCount);
    inResult.SetFragNo(phyOptHeader.fragNo);
    return E_OK;
}

int ProtocolProto::ParseCommDivergeHeader(const uint8_t *bytes, uint32_t length, ParseResult &inResult)
{
    // Check version before parse field
    if (length < sizeof(CommPhyHeader) + sizeof(uint16_t)) {
        return -E_LENGTH_ERROR;
    }
    uint16_t version = NetToHost(*(reinterpret_cast<const uint16_t *>(bytes + sizeof(CommPhyHeader))));
    if (version != PROTOCOL_VERSION) {
        LOGE("[Proto][ParseDiverge] Version=%" PRIu16 " not support.", version);
        return -E_VERSION_NOT_SUPPORT;
    }

    if (length < sizeof(CommPhyHeader) + sizeof(CommDivergeHeader)) {
        LOGE("[Proto][ParseDiverge] Length of Bytes Error.");
        return -E_PARSE_FAIL;
    }
    auto headerOri = reinterpret_cast<const CommDivergeHeader *>(bytes + sizeof(CommPhyHeader));
    CommDivergeHeader divergeHeader;
    HeaderConverter::ConvertNetToHost(*headerOri, divergeHeader);
    if (sizeof(CommPhyHeader) + sizeof(CommDivergeHeader) + divergeHeader.payLoadLen +
        inResult.GetPaddingLen() != inResult.GetPacketLen()) {
        LOGE("[Proto][ParseDiverge] Total Length Mismatch.");
        return -E_PARSE_FAIL;
    }
    inResult.SetPayloadLen(divergeHeader.payLoadLen);
    inResult.SetCommLabel(LabelType(std::begin(divergeHeader.commLabel), std::end(divergeHeader.commLabel)));
    return E_OK;
}

int ProtocolProto::ParseCommLayerPayload(const uint8_t *bytes, uint32_t length, ParseResult &inResult)
{
    if (inResult.GetFrameTypeInfo() == FrameType::COMMUNICATION_LABEL_EXCHANGE_ACK) {
        int errCode = ParseLabelExchangeAck(bytes, length, inResult);
        if (errCode != E_OK) {
            LOGE("[Proto][ParseCommPayload] Total Length Mismatch.");
            return errCode;
        }
    } else {
        int errCode = ParseLabelExchange(bytes, length, inResult);
        if (errCode != E_OK) {
            LOGE("[Proto][ParseCommPayload] Total Length Mismatch.");
            return errCode;
        }
    }
    return E_OK;
}

int ProtocolProto::ParseLabelExchange(const uint8_t *bytes, uint32_t length, ParseResult &inResult)
{
    // Check version at very first
    if (length < sizeof(CommPhyHeader) + LABEL_VER_LEN) {
        return -E_LENGTH_ERROR;
    }
    auto fieldPtr = reinterpret_cast<const uint64_t *>(bytes + sizeof(CommPhyHeader));
    uint64_t version = NetToHost(*fieldPtr++);
    if (version != PROTOCOL_VERSION) {
        LOGE("[Proto][ParseLabel] Version=%llu not support.", ULL(version));
        return -E_VERSION_NOT_SUPPORT;
    }

    // Version, DistinctValue, SequenceId and CommLabelCount field must be exist.
    if (length < sizeof(CommPhyHeader) + LABEL_VER_LEN + DISTINCT_VALUE_LEN + SEQUENCE_ID_LEN + COMM_LABEL_COUNT_LEN) {
        LOGE("[Proto][ParseLabel] Length of Bytes Error.");
        return -E_LENGTH_ERROR;
    }
    uint64_t distinctValue = NetToHost(*fieldPtr++);
    inResult.SetLabelExchangeDistinctValue(distinctValue);
    uint64_t sequenceId = NetToHost(*fieldPtr++);
    inResult.SetLabelExchangeSequenceId(sequenceId);
    uint64_t commLabelCount = NetToHost(*fieldPtr++);
    if (length < commLabelCount || (UINT32_MAX / COMM_LABEL_LENGTH) < commLabelCount) {
        LOGE("[Proto][ParseLabel] commLabelCount=%llu invalid.", ULL(commLabelCount));
        return -E_PARSE_FAIL;
    }
    // commLabelCount is expected to be not very large
    if (length < sizeof(CommPhyHeader) + LABEL_VER_LEN + DISTINCT_VALUE_LEN + SEQUENCE_ID_LEN + COMM_LABEL_COUNT_LEN +
        commLabelCount * COMM_LABEL_LENGTH) {
        LOGE("[Proto][ParseLabel] Length of Bytes Error, commLabelCount=%llu", ULL(commLabelCount));
        return -E_LENGTH_ERROR;
    }

    // Get each commLabel
    std::set<LabelType> commLabels;
    auto bytePtr = reinterpret_cast<const uint8_t *>(fieldPtr);
    for (uint64_t i = 0; i < commLabelCount; i++) {
        // the length is checked just above
        LabelType commLabel(bytePtr + i * COMM_LABEL_LENGTH, bytePtr + (i + 1) * COMM_LABEL_LENGTH);
        if (commLabels.count(commLabel) != 0) {
            LOGW("[Proto][ParseLabel] Duplicate Label Detected, commLabel=%s.", VEC_TO_STR(commLabel));
        } else {
            commLabels.insert(commLabel);
        }
    }
    inResult.SetLatestCommLabels(commLabels);
    return E_OK;
}

int ProtocolProto::ParseLabelExchangeAck(const uint8_t *bytes, uint32_t length, ParseResult &inResult)
{
    // Check version at very first
    if (length < sizeof(CommPhyHeader) + LABEL_VER_LEN) {
        return -E_LENGTH_ERROR;
    }
    auto fieldPtr = reinterpret_cast<const uint64_t *>(bytes + sizeof(CommPhyHeader));
    uint64_t version = NetToHost(*fieldPtr++);
    if (version != PROTOCOL_VERSION) {
        LOGE("[Proto][ParseLabelAck] Version=%llu not support.", ULL(version));
        return -E_VERSION_NOT_SUPPORT;
    }

    if (length < sizeof(CommPhyHeader) + LABEL_VER_LEN + DISTINCT_VALUE_LEN + SEQUENCE_ID_LEN) {
        LOGE("[Proto][ParseLabelAck] Length of Bytes Error.");
        return -E_LENGTH_ERROR;
    }
    uint64_t distinctValue = NetToHost(*fieldPtr++);
    inResult.SetLabelExchangeDistinctValue(distinctValue);
    uint64_t sequenceId = NetToHost(*fieldPtr++);
    inResult.SetLabelExchangeSequenceId(sequenceId);
    return E_OK;
}

// Note: framePhyHeader is in network endian
// This function aims at calculating and preparing each part of each packets
int ProtocolProto::FrameFragmentation(const uint8_t *splitStartBytes, const FrameFragmentInfo &fragmentInfo,
    const CommPhyHeader &framePhyHeader, std::vector<std::pair<std::vector<uint8_t>, uint32_t>> &outPieces)
{
    // It can be guaranteed that fragCount >= 2 and also won't be too large
    if (fragmentInfo.fragCount < MIN_FRAGMENT_COUNT) {
        return -E_INVALID_ARGS;
    }
    outPieces.resize(fragmentInfo.fragCount); // Note: should use resize other than reserve
    uint32_t quotient = fragmentInfo.splitLength / fragmentInfo.fragCount;
    uint16_t remainder = fragmentInfo.splitLength % fragmentInfo.fragCount;
    uint16_t fragNo = 0; // Fragment index start from 0
    uint32_t byteOffset = 0;

    for (auto &entry : outPieces) {
        // subtract 1 for index
        uint32_t pieceFragLen = (fragNo != fragmentInfo.fragCount - 1) ? quotient : (quotient + remainder);
        uint32_t alignedFragLen = BYTE_8_ALIGN(pieceFragLen); // Add padding length
        uint32_t pieceTotalLen = alignedFragLen + sizeof(CommPhyHeader) + sizeof(CommPhyOptHeader);

        // Since exception is disabled, we have to check the vector size to assure that memory is truly allocated
        entry.first.resize(pieceTotalLen + fragmentInfo.extendHeadSize); // Note: should use resize other than reserve
        if (entry.first.size() != (pieceTotalLen + fragmentInfo.extendHeadSize)) {
            LOGE("[Proto][FrameFrag] Resize failed for length=%u", pieceTotalLen);
            return -E_OUT_OF_MEMORY;
        }

        CommPhyHeader pktPhyHeader;
        HeaderConverter::ConvertNetToHost(framePhyHeader, pktPhyHeader); // Restore to host endian

        // The sum value need to be recalculated, and the packet is fragmented.
        // The alignedFragLen is always larger than pieceFragLen
        FillPhyHeaderLenInfo(pktPhyHeader, pieceTotalLen, 0, PACKET_TYPE_FRAGMENTED, alignedFragLen - pieceFragLen);
        HeaderConverter::ConvertHostToNet(pktPhyHeader, pktPhyHeader);

        CommPhyOptHeader pktPhyOptHeader = {static_cast<uint32_t>(fragmentInfo.splitLength + sizeof(CommPhyHeader)),
            fragmentInfo.fragCount, fragNo};
        HeaderConverter::ConvertHostToNet(pktPhyOptHeader, pktPhyOptHeader);
        int err;
        FragmentPacket packet;
        uint8_t *ptrPacket = &(entry.first[0]);
        if (fragmentInfo.extendHeadSize > 0) {
            packet = {ptrPacket, fragmentInfo.extendHeadSize};
            err = FillFragmentPacketExtendHead(fragmentInfo.oringinalBytesAddr, fragmentInfo.extendHeadSize, packet);
            if (err != E_OK) {
                return err;
            }
            ptrPacket += fragmentInfo.extendHeadSize;
        }
        packet = {ptrPacket, static_cast<uint32_t>(entry.first.size()) - fragmentInfo.extendHeadSize};
        err = FillFragmentPacket(pktPhyHeader, pktPhyOptHeader, splitStartBytes + byteOffset,
            pieceFragLen, packet);
        entry.second = fragmentInfo.extendHeadSize;
        if (err != E_OK) {
            LOGE("[Proto][FrameFrag] Fill packet fail, fragCount=%" PRIu16 ", fragNo=%" PRIu16, fragmentInfo.fragCount,
                fragNo);
            return err;
        }

        fragNo++;
        byteOffset += pieceFragLen;
    }

    return E_OK;
}

int ProtocolProto::FillFragmentPacketExtendHead(uint8_t *headBytesAddr, uint32_t headLen, FragmentPacket &outPacket)
{
    if (headLen > outPacket.leftLength) {
        LOGE("[Proto][FrameFrag] headLen less than leftLength");
        return -E_INVALID_ARGS;
    }
    errno_t retCode = memcpy_s(outPacket.ptrPacket, outPacket.leftLength, headBytesAddr, headLen);
    if (retCode != EOK) {
        LOGE("memcpy error:%d", retCode);
        return -E_SECUREC_ERROR;
    }
    return E_OK;
}

// Note: phyHeader and phyOptHeader is in network endian
int ProtocolProto::FillFragmentPacket(const CommPhyHeader &phyHeader, const CommPhyOptHeader &phyOptHeader,
    const uint8_t *fragBytes, uint32_t fragLen, FragmentPacket &outPacket)
{
    if (outPacket.leftLength == 0) {
        return -E_INVALID_ARGS;
    }
    uint8_t *ptrPacket = outPacket.ptrPacket;
    uint32_t leftLength = outPacket.leftLength;

    // leftLength is guaranteed to be no smaller than the sum of phyHeaderLen + phyOptHeaderLen + fragLen
    // So, there will be no redundant check during subtraction
    errno_t retCode = memcpy_s(ptrPacket, leftLength, &phyHeader, sizeof(CommPhyHeader));
    if (retCode != EOK) {
        return -E_SECUREC_ERROR;
    }
    ptrPacket += sizeof(CommPhyHeader);
    leftLength -= sizeof(CommPhyHeader);

    retCode = memcpy_s(ptrPacket, leftLength, &phyOptHeader, sizeof(CommPhyOptHeader));
    if (retCode != EOK) {
        return -E_SECUREC_ERROR;
    }
    ptrPacket += sizeof(CommPhyOptHeader);
    leftLength -= sizeof(CommPhyOptHeader);

    retCode = memcpy_s(ptrPacket, leftLength, fragBytes, fragLen);
    if (retCode != EOK) {
        return -E_SECUREC_ERROR;
    }

    // Calculate sum and set sum field
    uint64_t sumResult = 0;
    int errCode  = CalculateXorSum(outPacket.ptrPacket + LENGTH_BEFORE_SUM_RANGE,
        outPacket.leftLength - LENGTH_BEFORE_SUM_RANGE, sumResult);
    if (errCode != E_OK) {
        return -E_SUM_CALCULATE_FAIL;
    }
    auto ptrPhyHeader = reinterpret_cast<CommPhyHeader *>(outPacket.ptrPacket);
    if (ptrPhyHeader == nullptr) {
        return -E_INVALID_ARGS;
    }
    ptrPhyHeader->checkSum = HostToNet(sumResult);

    return E_OK;
}

int ProtocolProto::GetExtendHeadDataSize(std::shared_ptr<ExtendHeaderHandle> &extendHandle, uint32_t &headSize)
{
    if (extendHandle != nullptr) {
        DBStatus status = extendHandle->GetHeadDataSize(headSize);
        if (status != DBStatus::OK) {
            LOGI("[Proto][ToSerial] get head data size failed,not permit to send");
            return -E_FEEDBACK_COMMUNICATOR_NOT_FOUND;
        }
        if (headSize > SerialBuffer::MAX_EXTEND_HEAD_LENGTH || headSize != BYTE_8_ALIGN(headSize)) {
            LOGI("[Proto][ToSerial] head data size is larger than 512 or not 8 byte align");
            return -E_FEEDBACK_COMMUNICATOR_NOT_FOUND;
        }
        return E_OK;
    }
    return E_OK;
}

int ProtocolProto::FillExtendHeadDataIfNeed(std::shared_ptr<ExtendHeaderHandle> &extendHandle, SerialBuffer *buffer,
    uint32_t headSize)
{
    if (extendHandle != nullptr && headSize > 0) {
        if (buffer == nullptr) {
            return -E_INVALID_ARGS;
        }
        LOGI("[HP_DEBUG] ProtocolProto::FillExtendHeadDataIfNeed");
        DBStatus status = extendHandle->FillHeadData(buffer->GetOringinalAddr(), headSize,
            buffer->GetSize() + headSize);
        if (status != DBStatus::OK) {
            LOGI("[Proto][ToSerial] fill head data failed");
            return -E_FEEDBACK_COMMUNICATOR_NOT_FOUND;
        }
        return E_OK;
    }
    return E_OK;
}

int ProtocolProto::GetTransformFunc(uint32_t messageId, DistributedDB::TransformFunc &function)
{
    std::shared_lock<std::shared_mutex> autoLock(msgIdMutex_);
    const auto &entry = msgIdMapFunc_.find(messageId);
    if (entry == msgIdMapFunc_.end()) {
        return -E_NOT_REGISTER;
    }
    function = entry->second;
    return E_OK;
}
} // namespace DistributedDB
