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

#ifndef OHOS_STARTUP_SYSPARAM_ERRNO_H
#define OHOS_STARTUP_SYSPARAM_ERRNO_H
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/* --------------------------------------------------------------------------------------------*
 * Definition of error code. The error codes are applicable to both the application and kernel
 * -------------------------------------------------------------------------------------------- */
enum OHOSStartUpSysParamErrorCode {
    EC_SUCCESS = 0,  /* OK or No error */
    EC_FAILURE = -1, /* Execution failed */
    EC_INVALID = -9, /* Invalid argument */
    EC_SYSTEM_ERR = -10, /* system error */

    SYSPARAM_INVALID_INPUT = -401, /* Input parameter is missing or invalid */
    SYSPARAM_NOT_FOUND = -14700101, /* System parameter can not be found. */
    SYSPARAM_INVALID_VALUE = -14700102, /* System parameter value is invalid */
    SYSPARAM_PERMISSION_DENIED = -14700103, /* System permission operation permission denied */
    SYSPARAM_SYSTEM_ERROR = -14700104, /* System internal error including out of memory, deadlock etc. */
    SYSPARAM_WAIT_TIMEOUT = -14700105, /* System wait parameter value change time out. */
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif // OHOS_STARTUP_SYSPARAM_ERRNO_H
