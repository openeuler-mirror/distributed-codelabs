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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_LISTENER_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_LISTENER_H

#include <mutex>

#include "event_handler.h"
#include "common_event_subscriber.h"
#include "event_receive_stub.h"

namespace OHOS {
namespace EventFwk {
class CommonEventListener : public EventReceiveStub {
public:
    using EventHandler = OHOS::AppExecFwk::EventHandler;
    using EventRunner = OHOS::AppExecFwk::EventRunner;

    /**
     * Constructor.
     *
     * @param commonEventSubscriber the CommonEventSubscriber object
     */
    explicit CommonEventListener(const std::shared_ptr<CommonEventSubscriber> &commonEventSubscriber);

    virtual ~CommonEventListener();

    /**
     * Notifies event.
     *
     * @param data Indicates the common event data.
     * @param ordered Indicates whether it is an ordered common event.
     * @param sticky Indicates whether it is a sticky common event.
     */
    void NotifyEvent(const CommonEventData &data, const bool &ordered, const bool &sticky) override;

    /**
     * Stops to receive events.
     *
     */
    void Stop();

private:
    ErrCode Init();

    bool IsReady();

    void OnReceiveEvent(const CommonEventData &commonEventData, const bool &ordered, const bool &sticky);

private:
    std::mutex mutex_;
    std::shared_ptr<CommonEventSubscriber> commonEventSubscriber_;
    std::shared_ptr<EventRunner> runner_;
    std::shared_ptr<EventHandler> handler_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_LISTENER_H