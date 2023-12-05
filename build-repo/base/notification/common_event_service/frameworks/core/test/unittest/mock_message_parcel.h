/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef MOCK_MESSAGE_PARCEL_H
#define MOCK_MESSAGE_PARCEL_H

#include "common_event.h"
#include "common_event_death_recipient.h"
#include "common_event_stub.h"
#include "common_event_proxy.h"
#include "common_event_data.h"
#include "common_event_subscriber.h"
#include "event_receive_proxy.h"
#include "common_event_publish_info.h"
#include "matching_skills.h"

#include "event_log_wrapper.h"
#include "string_ex.h"
#include "ces_inner_error_code.h"
#include "iremote_broker.h"

#include <gtest/gtest.h>

class MessageParcel {
public:
    MessageParcel() = default;
    virtual ~MessageParcel() = default;

    bool WriteInterfaceToken(std::string descriptor)
    {
        if (flag_) {
            return true;
        }
        return false;
    }

    static void SetFlag(bool flag);

    static bool flag_;
};

bool MessageParcel::flag_ = false;
void MessageParcel::SetFlag(bool flag)
{
    flag_ = flag;
}

#endif  // MOCK_MESSAGE_PARCEL_H