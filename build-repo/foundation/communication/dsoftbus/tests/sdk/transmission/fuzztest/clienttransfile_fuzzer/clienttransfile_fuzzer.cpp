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

#include "clienttransfile_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <securec.h>

#include "client_trans_file.h"
#include "softbus_utils.h"
#include "softbus_def.h"
#include "session.h"
#include "client_trans_file_listener.h"
#include "file_adapter.h"

namespace OHOS {
    static int OnReceiveFileStarted(int sessionId, const char* files, int fileCnt)
    {
        return 0;
    }

    static void OnReceiveFileFinished(int sessionId, const char* files, int fileCnt)
    {}

    static int OnReceiveFileProcess(int sessionId, const char* firstFile, uint64_t bytesUpload, uint64_t bytesTotal)
    {
        return 0;
    }

    static int OnSendFileProcess(int sessionId, uint64_t bytesUpload, uint64_t bytesTotal)
    {
        return 0;
    }

    static int OnSendFileFinished(int sessionId, const char* firstFile)
    {
        return 0;
    }

    static void OnFileTransError(int sessionId)
    {}

    void TransOnFileChannelOpenedTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return;
        }
        const char* sessionName = reinterpret_cast<const char*>(data);
        int32_t fileport = 0;
        TransOnFileChannelOpened(sessionName, nullptr, &fileport);
    }

    void TransSetFileReceiveListenerTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return;
        }
        const IFileReceiveListener fileRecvListener = {
            .OnReceiveFileStarted = OnReceiveFileStarted,
            .OnReceiveFileProcess = OnReceiveFileProcess,
            .OnReceiveFileFinished = OnReceiveFileFinished,
            .OnFileTransError = OnFileTransError,
        };
        const char* sessionName = reinterpret_cast<const char*>(data);
        const char* rootDir = "/data/recv/";
        TransSetFileReceiveListener(sessionName, &fileRecvListener, rootDir);
    }

    void TransSetFileSendListenerTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return;
        }

        IFileSendListener sendListener = {
            .OnSendFileProcess = OnSendFileProcess,
            .OnSendFileFinished = OnSendFileFinished,
            .OnFileTransError = OnFileTransError,
        };
        const char* sessionName = reinterpret_cast<const char*>(data);
        TransSetFileSendListener(sessionName, &sendListener);
    }

    void TransGetFileListenerTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return;
        }

        FileListener fileListener;
        const char* sessionName = reinterpret_cast<const char*>(data);
        TransGetFileListener(sessionName, &fileListener);
    }

    void StartNStackXDFileServerTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size < sizeof(int32_t))) {
            return;
        }

        #define DEFAULT_KEY_LENGTH 32
        int32_t len = *(reinterpret_cast<const int32_t*>(data));
        StartNStackXDFileServer(nullptr, data, DEFAULT_KEY_LENGTH, NULL, &len);
    }

    void TransDeleteFileListenerTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size < SESSION_NAME_SIZE_MAX)) {
            return;
        }
        char tmp[SESSION_NAME_SIZE_MAX + 1] = {0};
        if (memcpy_s(tmp, sizeof(tmp) - 1, data, sizeof(tmp) - 1) != EOK) {
            return;
        }
        TransDeleteFileListener(tmp);
    }
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::TransOnFileChannelOpenedTest(data, size);
    OHOS::TransSetFileReceiveListenerTest(data, size);
    OHOS::TransSetFileSendListenerTest(data, size);
    OHOS::TransGetFileListenerTest(data, size);
    OHOS::StartNStackXDFileServerTest(data, size);
    OHOS::TransDeleteFileListenerTest(data, size);
    return 0;
}
