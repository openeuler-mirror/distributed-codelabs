/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "dev_slinfo_list.h"
#include "dev_slinfo_adpt.h"
#include "dev_slinfo_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DATASL_API __attribute__ ((visibility ("default")))

static int32_t GetHighestSecLevelByUdid(DEVSLQueryParams *queryParams, uint32_t *dataLevel)
{
    int32_t ret;
    int32_t devLevel = DEFAULT_DEV_SEC_LEVEL;

    if ((queryParams->udidLen <= 0u) || (queryParams->udidLen > 64u)) {
        return DEVSL_ERR_BAD_PARAMETERS;
    }

    ret = GetDeviceSecLevelByUdid(queryParams->udid, queryParams->udidLen, &devLevel);
    if (ret != DEVSL_SUCCESS) {
        *dataLevel = DATA_SEC_LEVEL0;
        DATA_SEC_LOG_ERROR("GetHighestSecLevelByUdid: get device security level failed, error code: %d", ret);
        return ret;
    }

    *dataLevel = GetDataSecLevelByDevSecLevel(devLevel);
    return ret;
}

DATASL_API int32_t DATASL_OnStart(void)
{
    int32_t ret;

    DATA_SEC_LOG_INFO("Enter DATASL_Onstart...");
    ret = StartDevslEnv();
    if (ret != DEVSL_SUCCESS) {
        DATA_SEC_LOG_ERROR("DATASL_Onstart: StartDevslEnv - failed, error code is %d", ret);
        FinishDevslEnv();
        return DEVSL_ERROR;
    }
    DATA_SEC_LOG_INFO("DATASL_Onstart done!");
    return DEVSL_SUCCESS;
}

DATASL_API void DATASL_OnStop(void)
{
    DATA_SEC_LOG_INFO("Enter DATASL_OnStop...");
    FinishDevslEnv();
    DATA_SEC_LOG_INFO("DATASL_OnStop done!");
    return;
}

DATASL_API int32_t DATASL_GetHighestSecLevel(DEVSLQueryParams *queryParams, uint32_t *levelInfo)
{
    int32_t ret;
    DATA_SEC_LOG_INFO("Enter DATASL_GetHighestSecLevel...");
    if ((queryParams == NULL) || (levelInfo == NULL)) {
        return DEVSL_ERR_BAD_PARAMETERS;
    }

    ret = GetHighestSecLevelByUdid(queryParams, levelInfo);

    DATA_SEC_LOG_INFO("DATASL_GetHighestSecLevel done!");
    return ret;
}

static int32_t GetHighestSecLevelByUdidAsync(DEVSLQueryParams *queryParams)
{
    DATA_SEC_LOG_INFO("Enter GetHighestSecLevelByUdidAsync...");
    int32_t ret;

    ret = GetDeviceSecLevelByUdidAsync(queryParams->udid, queryParams->udidLen);
    if (ret != DEVSL_SUCCESS) {
        DATA_SEC_LOG_ERROR("GetHighestSecLevelByUdidAsync: get device security level failed, error code: %d", ret);
        return ret;
    }

    DATA_SEC_LOG_INFO("GetHighestSecLevelByUdidAsync done!");
    return ret;
}

DATASL_API int32_t DATASL_GetHighestSecLevelAsync(DEVSLQueryParams *queryParams, HigestSecInfoCallback *callback)
{
    DATA_SEC_LOG_INFO("Enter DATASL_GetHighestSecLevelAsync...");
    int32_t ret;

    if ((queryParams == NULL) || (callback == NULL) || (queryParams->udidLen <= 0u) || (queryParams->udidLen > 64u)) {
        return DEVSL_ERR_BAD_PARAMETERS;
    }

    ret = UpdateCallbackListParams(queryParams, callback);
    if (ret != DEVSL_SUCCESS) {
        DATA_SEC_LOG_ERROR("DATASL_GetHighestSecLevelAsync: Update Callback list params failed, error code: %d", ret);
        return ret;
    }

    ret = GetHighestSecLevelByUdidAsync(queryParams);
    DATA_SEC_LOG_INFO("DATASL_GetHighestSecLevelAsync done!");
    return ret;
}

#ifdef __cplusplus
}
#endif