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

#include "auth_common.h"
#include "cJSON.h"
#include "softbus_adapter_file.h"
 
#define TEST_JSON "{\"hichain\":0}"
 
const char *g_FileName = "/system/bin/auth_config.txt";
#define MAX_READ_LEN (1024)
 
typedef struct {
    int32_t hichainEnable;
} AuthConfig;
 
static AuthConfig g_authConfig;
      
int32_t AuthConfigInit()
{       
    g_authConfig.hichainEnable = 0; 

    char readbuf[MAX_READ_LEN + 1] = {"\0"};
    int32_t maxLen = MAX_READ_LEN;  
    int32_t ret = SoftBusReadFullFile(g_FileName, readbuf, maxLen);
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_AUTH, SOFTBUS_LOG_ERROR, "SoftBusReadFullFile fail.");
        return ret;
    }         
    cJSON *data = cJSON_Parse((char *)readbuf); 
    if (data == NULL) {
        SoftBusLog(SOFTBUS_LOG_AUTH, SOFTBUS_LOG_ERROR, "cJSON_Parse fail.");
        return SOFTBUS_ERR;
    }         
    if (!GetJsonObjectInt32Item(data, "hichain", &g_authConfig.hichainEnable)) { 
        SoftBusLog(SOFTBUS_LOG_AUTH, SOFTBUS_LOG_ERROR, "GetJsonObjectInt32Item fail.");
        cJSON_Delete(data);
        return SOFTBUS_ERR;
    }   
        
    SoftBusLog(SOFTBUS_LOG_AUTH, SOFTBUS_LOG_INFO, "hichain: %d", g_authConfig.hichainEnable);
    cJSON_Delete(data);
    return SOFTBUS_OK;
}     
      
int32_t AuthConfigDeInit() 
{     
    return SOFTBUS_OK;
}       
        
bool GetHichainEnable()                                                                                                                                                                                   {    
    return  (g_authConfig.hichainEnable != 0);
}
