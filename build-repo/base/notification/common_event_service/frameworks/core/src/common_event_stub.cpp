/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "common_event_stub.h"
#include "common_event_publish_info.h"
#include "event_log_wrapper.h"
#include "string_ex.h"
#include "ces_inner_error_code.h"

namespace OHOS {
namespace EventFwk {
using namespace OHOS::Notification;

CommonEventStub::CommonEventStub()
{}

CommonEventStub::~CommonEventStub()
{}

int CommonEventStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        EVENT_LOGE("local descriptor is not equal to remote");
        return ERR_TRANSACTION_FAILED;
    }

    switch (code) {
        case static_cast<uint32_t>(ICommonEvent::Message::CES_PUBLISH_COMMON_EVENT): {
            std::unique_ptr<CommonEventData> event(data.ReadParcelable<CommonEventData>());
            std::unique_ptr<CommonEventPublishInfo> publishinfo(data.ReadParcelable<CommonEventPublishInfo>());
            sptr<IRemoteObject> commonEventListener = nullptr;
            bool hasLastSubscriber = data.ReadBool();
            if (hasLastSubscriber) {
                commonEventListener = data.ReadRemoteObject();
            }
            int32_t userId = data.ReadInt32();
            if (!event) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventData>");
                return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
            }
            if (!publishinfo) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventPublishInfo>");
                return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
            }
            int32_t ret = PublishCommonEvent(*event, *publishinfo, commonEventListener, userId);
            if (!reply.WriteInt32(ret)) {
                EVENT_LOGE("Failed to write reply ");
                return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_PUBLISH_COMMON_EVENT2): {
            std::unique_ptr<CommonEventData> event(data.ReadParcelable<CommonEventData>());
            std::unique_ptr<CommonEventPublishInfo> publishinfo(data.ReadParcelable<CommonEventPublishInfo>());
            sptr<IRemoteObject> commonEventListener = nullptr;
            bool hasLastSubscriber = data.ReadBool();
            if (hasLastSubscriber) {
                commonEventListener = data.ReadRemoteObject();
            }
            int32_t uid = data.ReadInt32();
            int32_t callerToken = data.ReadInt32();
            int32_t userId = data.ReadInt32();
            if (!event) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventData>");
                return ERR_INVALID_VALUE;
            }
            if (!publishinfo) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventPublishInfo>");
                return ERR_INVALID_VALUE;
            }
            bool ret = PublishCommonEvent(*event, *publishinfo, commonEventListener, uid, callerToken, userId);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply ");
                return ERR_INVALID_VALUE;
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_SUBSCRIBE_COMMON_EVENT): {
            std::unique_ptr<CommonEventSubscribeInfo> subscribeInfo(data.ReadParcelable<CommonEventSubscribeInfo>());
            if (!subscribeInfo) {
                EVENT_LOGE("Failed to ReadParcelable<CommonEventSubscribeInfo>");
                return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
            }
            bool hasSubscriber = data.ReadBool();
            if (!hasSubscriber) {
                EVENT_LOGE("no valid commonEventListener!");
                return ERR_INVALID_VALUE;
            }
            sptr<IRemoteObject> commonEventListener = data.ReadRemoteObject();
            if (commonEventListener == nullptr) {
                EVENT_LOGE("Failed to ReadParcelable<IRemoteObject>");
                return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
            }
            int32_t ret = SubscribeCommonEvent(*subscribeInfo, commonEventListener);
            if (!reply.WriteInt32(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_UNSUBSCRIBE_COMMON_EVENT): {
            bool hasSubscriber = data.ReadBool();
            if (!hasSubscriber) {
                EVENT_LOGE("no valid commonEventListener!");
                return ERR_INVALID_VALUE;
            }
            sptr<IRemoteObject> commonEventListener = data.ReadRemoteObject();
            if (commonEventListener == nullptr) {
                EVENT_LOGE("Failed to ReadParcelable<IRemoteObject>");
                return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
            }
            int32_t ret = UnsubscribeCommonEvent(commonEventListener);
            if (!reply.WriteInt32(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_NOTIFICATION_CES_COMMON_PARAM_INVALID;
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_GET_STICKY_COMMON_EVENT): {
            std::string event = Str16ToStr8(data.ReadString16());
            CommonEventData eventData;
            bool ret = GetStickyCommonEvent(event, eventData);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply ret!");
                return ERR_INVALID_VALUE;
            }
            if (!reply.WriteParcelable(&eventData)) {
                EVENT_LOGE("Failed to write reply eventData!");
                return ERR_INVALID_VALUE;
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_DUMP_STATE): {
            std::vector<std::string> result;
            uint8_t dumpType = data.ReadUint8();
            std::string event = Str16ToStr8(data.ReadString16());
            int32_t userId = data.ReadInt32();
            DumpState(dumpType, event, userId, result);
            reply.WriteInt32(result.size());
            for (auto stack : result) {
                reply.WriteString16(Str8ToStr16(stack));
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_FINISH_RECEIVER): {
            bool hasPorxy = data.ReadBool();
            if (!hasPorxy) {
                EVENT_LOGE("no valid proxy!");
                return ERR_INVALID_VALUE;
            }
            sptr<IRemoteObject> proxy = data.ReadRemoteObject();
            if (proxy == nullptr) {
                EVENT_LOGE("Failed to ReadRemoteObject");
                return ERR_INVALID_VALUE;
            }
            int32_t receiverCode = data.ReadInt32();
            std::string receiverData = Str16ToStr8(data.ReadString16());
            bool abortEvent = data.ReadBool();
            bool ret = FinishReceiver(proxy, receiverCode, receiverData, abortEvent);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_FREEZE): {
            int32_t uid = data.ReadInt32();
            bool ret = Freeze(uid);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_UNFREEZE): {
            int32_t uid = data.ReadInt32();
            bool ret = Unfreeze(uid);
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }
        case static_cast<uint32_t>(ICommonEvent::Message::CES_UNFREEZE_ALL): {
            bool ret = UnfreezeAll();
            if (!reply.WriteBool(ret)) {
                EVENT_LOGE("Failed to write reply");
                return ERR_INVALID_VALUE;
            }
            break;
        }
        default:
            EVENT_LOGW("unknown, code = %{public}u, flags= %{public}u", code, option.GetFlags());
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return NO_ERROR;
}

int32_t CommonEventStub::PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
    const sptr<IRemoteObject> &commonEventListener, const int32_t &userId)
{
    EVENT_LOGD("called");

    return ERR_OK;
}

bool CommonEventStub::PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
    const sptr<IRemoteObject> &commonEventListener, const uid_t &uid, const int32_t &callerToken,
    const int32_t &userId)
{
    EVENT_LOGD("called");

    return true;
}

int32_t CommonEventStub::SubscribeCommonEvent(
    const CommonEventSubscribeInfo &subscribeInfo, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("called");

    return ERR_OK;
}

int32_t CommonEventStub::UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::DumpState(const uint8_t &dumpType, const std::string &event, const int32_t &userId,
    std::vector<std::string> &state)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::FinishReceiver(const sptr<IRemoteObject> &proxy, const int32_t &code,
    const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::Freeze(const uid_t &uid)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::Unfreeze(const uid_t &uid)
{
    EVENT_LOGD("called");

    return true;
}

bool CommonEventStub::UnfreezeAll()
{
    EVENT_LOGD("called");

    return true;
}
}  // namespace EventFwk
}  // namespace OHOS