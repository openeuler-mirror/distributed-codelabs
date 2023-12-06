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

#include "bus_center_mock.h"
#include "softbus_log.h"
#include "softbus_error_code.h"
#include "securec.h"

/* implement related global function of bus center */
int32_t LnnGetLocalStrInfo(InfoKey key, char *info, uint32_t len)
{
    return BusCenterMock::GetMock()->LnnGetLocalStrInfo(key, info, len);
}

int32_t LnnConvertDeviceTypeToId(const char *deviceType, uint16_t *typeId)
{
    return BusCenterMock::GetMock()->LnnConvertDeviceTypeToId(deviceType, typeId);
}

int32_t LnnGetLocalByteInfo(InfoKey key, uint8_t *info, uint32_t len)
{
    return BusCenterMock::GetMock()->LnnGetLocalByteInfo(key, info, len);
}

bool LnnIsDefaultOhosAccount()
{
    return BusCenterMock::GetMock()->LnnIsDefaultOhosAccount();
}

/* definition for class BusCenterMock */
BusCenterMock::BusCenterMock()
{
    mock.store(this);
}

BusCenterMock::~BusCenterMock()
{
    mock.store(nullptr);
}

int32_t BusCenterMock::ActionOfLnnGetLocalStrInfo(InfoKey key, char *out, uint32_t outSize)
{
    if (key == STRING_KEY_DEV_NAME) {
        if (strcpy_s(out, outSize, deviceName.c_str()) != EOK) {
            return SOFTBUS_ERR;
        }
        return SOFTBUS_OK;
    }
    if (key == STRING_KEY_DEV_TYPE) {
        if (strcpy_s(out, outSize, TYPE_PHONE) != EOK) {
            return SOFTBUS_ERR;
        }
        return SOFTBUS_OK;
    }
    if (key == STRING_KEY_DEV_UDID) {
        if (strcpy_s(out, outSize, deviceUDID.c_str()) != EOK) {
            return SOFTBUS_ERR;
        }
        return SOFTBUS_OK;
    }
    return SOFTBUS_ERR;
}

int32_t BusCenterMock::ActionOfLnnConvertDeviceTypeToId(const char *deviceType, uint16_t *typeId)
{
    *typeId = TYPE_PHONE_ID;
    return SOFTBUS_OK;
}

int32_t BusCenterMock::ActionOfLnnGetLocalByteInfo(InfoKey key, uint8_t *out, uint32_t outSize)
{
    if (key == BYTE_KEY_ACCOUNT_HASH) {
        if (memcpy_s(out, outSize, accountHash, SHA_256_HASH_LEN) != EOK) {
            return SOFTBUS_ERR;
        }
        return SOFTBUS_OK;
    }
    return SOFTBUS_ERR;
}

bool BusCenterMock::ActionOfLnnIsDefaultOhosAccount()
{
    return false;
}

void BusCenterMock::SetupSuccessStub()
{
    EXPECT_CALL(*this, LnnGetLocalStrInfo).WillRepeatedly(BusCenterMock::ActionOfLnnGetLocalStrInfo);
    EXPECT_CALL(*this, LnnConvertDeviceTypeToId).WillRepeatedly(BusCenterMock::ActionOfLnnConvertDeviceTypeToId);
    EXPECT_CALL(*this, LnnGetLocalByteInfo).WillRepeatedly(BusCenterMock::ActionOfLnnGetLocalByteInfo);
    EXPECT_CALL(*this, LnnIsDefaultOhosAccount).WillRepeatedly(BusCenterMock::ActionOfLnnIsDefaultOhosAccount);
}