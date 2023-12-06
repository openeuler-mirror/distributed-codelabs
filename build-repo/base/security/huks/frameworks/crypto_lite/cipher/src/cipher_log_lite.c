/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "cipher_log.h"

#include "securec.h"

#include "log.h"


#define MAX_LOG_BUFF_LEN 512

void CipherLog(uint32_t logLevel, const char *funcName, uint32_t lineNo, const char *format, ...)
{
    char *buf = (char *)malloc(MAX_LOG_BUFF_LEN);
    if (buf == NULL) {
        HILOG_ERROR(HILOG_MODULE_SCY, "Cipher log malloc fail");
        return;
    }
    (void)memset_s(buf, MAX_LOG_BUFF_LEN, 0, MAX_LOG_BUFF_LEN);

    va_list ap;
    va_start(ap, format);
    int32_t ret = vsnprintf_s(buf, MAX_LOG_BUFF_LEN, MAX_LOG_BUFF_LEN - 1, format, ap);
    va_end(ap);
    if (ret < 0) {
        HILOG_ERROR(HILOG_MODULE_SCY, "Cipher log concatenate error.");
        free(buf);
        buf = NULL;
        return;
    }

    switch (logLevel) {
        case CIPHER_LOG_LEVEL_E:
#if defined(__LINUX__)
            HILOG_ERROR(HILOG_MODULE_SCY, "%s[%u]: %s\n", funcName, lineNo, buf);
#else
            HILOG_ERROR(HILOG_MODULE_SCY, "%{public}s[%{public}u]: %{public}s\n", funcName, lineNo, buf);
#endif
            break;
        default:
            free(buf);
            buf = NULL;
            return;
    }

    free(buf);
    buf = NULL;
}