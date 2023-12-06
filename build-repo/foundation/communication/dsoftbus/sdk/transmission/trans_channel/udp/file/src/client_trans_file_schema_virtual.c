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

#include <securec.h>

#include "client_trans_file.h"
#include "client_trans_file_listener.h"
#include "file_adapter.h"
#include "nstackx_dfile.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_utils.h"

int32_t TransFileSchemaInit(void)
{
    return SOFTBUS_OK;
}

void TransFileSchemaDeinit(void)
{
}

int32_t CheckFileSchema(int32_t sessionId, FileSchemaListener *fileSchemaListener)
{
    (void)sessionId;
    (void)fileSchemaListener;
    return SOFTBUS_OK;
}

int32_t SetSchemaCallback(FileSchema fileSchema, const char *sFileList[], uint32_t fileCnt)
{
    (void)fileSchema;
    (void)sFileList;
    (void)fileCnt;
    return SOFTBUS_OK;
}