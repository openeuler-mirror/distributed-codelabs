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

#include "clienttransproxyfilemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cinttypes>
#include <limits>
#include <string>
#include <unistd.h>

#include "client_trans_proxy_file_manager.h"
#include "client_trans_pending.h"
#include "client_trans_proxy_file_common.h"
#include "client_trans_session_manager.h"
#include "client_trans_tcp_direct_message.h"
#include "securec.h"
#include "softbus_adapter_errcode.h"
#include "softbus_adapter_file.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_timer.h"
#include "softbus_app_info.h"
#include "softbus_conn_interface.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_type_def.h"
#include "softbus_utils.h"
#include "trans_server_proxy.h"

namespace OHOS {
void ClientTransProxyFileManagerTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    int32_t channelId = *(reinterpret_cast<const int32_t*>(data));
    const char **sFileList = nullptr;
    const char **dFileList = nullptr;
    uint32_t fileCnt = *(reinterpret_cast<const uint32_t*>(data));
    int32_t sessionId = *(reinterpret_cast<const int32_t*>(data));
    const FileFrame oneFrame = {0};

    ProxyChannelSendFile(channelId, sFileList, dFileList, fileCnt);

    ProcessRecvFileFrameData(sessionId, channelId, &oneFrame);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::ClientTransProxyFileManagerTest(data, size);
    return 0;
}
