/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils_json.h"

#include <stddef.h>

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

JsonHandle CreateJson(const char *data)
{
    cJSON *root = NULL;

    if (data != NULL) {
        root = cJSON_Parse(data);
    } else {
        root = cJSON_CreateObject();
    }
    return (void *)root;
}

void DestroyJson(JsonHandle handle)
{
    if (handle != NULL) {
        cJSON_Delete((cJSON *)handle);
    }
}

int32_t GetJsonFieldInt(JsonHandle handle, const char *field)
{
    int32_t ret = -1;

    if (handle == NULL) {
        return ret;
    }

    if (field == NULL) {
        return ((cJSON *)handle)->valueint;
    }

    cJSON *objValue = NULL;

    do {
        objValue = (cJSON *)GetJsonFieldJson(handle, field);
        if (objValue == NULL) {
            break;
        }
        if (!cJSON_IsNumber(objValue)) {
            break;
        }
        ret = objValue->valueint;
    } while (0);

    return ret;
}

uint32_t GetJsonFieldIntArray(JsonHandle handle, const char *field, int32_t *array, int32_t arrayLen)
{
    if (handle == NULL || field == NULL || array == NULL) {
        return 0;
    }

    cJSON *objValue = cJSON_GetObjectItem(handle, field);
    if (objValue == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(objValue)) {
        return 0;
    }

    int size = cJSON_GetArraySize(objValue);
    if (size > arrayLen) {
        size = arrayLen;
    }
    uint32_t index = 0;
    for (int32_t i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(objValue, i);
        if (!cJSON_IsNumber(item)) {
            continue;
        }
        array[index++] = item->valueint;
    }

    return index;
}

void AddFieldBoolToJson(JsonHandle handle, const char *field, bool value)
{
    if (handle == NULL || field == NULL) {
        return;
    }
    (void)cJSON_AddBoolToObject((cJSON *)handle, field, value);
}

const char *GetJsonFieldString(JsonHandle handle, const char *field)
{
    if (handle == NULL) {
        return NULL;
    }
    if (field == NULL) {
        return ((cJSON *)handle)->valuestring;
    }
    cJSON *objValue = NULL;
    const char *payload = NULL;

    do {
        objValue = (cJSON *)GetJsonFieldJson(handle, field);
        if (objValue == NULL) {
            break;
        }
        payload = cJSON_GetStringValue(objValue);
        if (payload == NULL) {
            break;
        }
    } while (0);
    return payload;
}

JsonHandle GetJsonFieldJson(JsonHandle handle, const char *field)
{
    return cJSON_GetObjectItem((cJSON *)handle, field);
}

JsonHandle GetJsonFieldJsonArray(JsonHandle handle, uint32_t num)
{
    return cJSON_GetArrayItem((cJSON *)handle, num);
}

int32_t GetJsonFieldJsonArraySize(JsonHandle handle)
{
    return cJSON_GetArraySize((cJSON *)handle);
}

void AddFieldIntToJson(JsonHandle handle, const char *field, int32_t value)
{
    if (handle == NULL || field == NULL) {
        return;
    }
    (void)cJSON_AddNumberToObject((cJSON *)handle, field, value);
}

void AddFieldIntArrayToJson(JsonHandle handle, const char *field, const int32_t *array, int32_t arrayLen)
{
    if (handle == NULL || field == NULL || array == NULL) {
        return;
    }
    cJSON *arrayObj = cJSON_CreateIntArray(array, arrayLen);
    if (arrayObj == NULL) {
        return;
    }
    (void)cJSON_AddItemToObject((cJSON *)handle, field, arrayObj);
}

void AddFieldStringToJson(JsonHandle handle, const char *field, const char *value)
{
    if (handle == NULL || field == NULL || value == NULL) {
        return;
    }
    (void)cJSON_AddStringToObject((cJSON *)handle, field, value);
}

void AddFieldJsonToJson(JsonHandle handle, const char *field, JsonHandle json)
{
    if (handle == NULL || field == NULL || json == NULL) {
        return;
    }
    (void)cJSON_AddItemToObject((cJSON *)handle, field, json);
}

char *ConvertJsonToString(JsonHandle handle)
{
    if (handle != NULL) {
        char *ret = cJSON_PrintUnformatted((cJSON *)handle);
        return ret;
    }
    return NULL;
}

bool CompareJsonData(JsonHandle handleA, JsonHandle handleB, bool caseSensitive)
{
    if (handleA == NULL || handleB == NULL) {
        return false;
    }
    return cJSON_Compare(handleA, handleB, caseSensitive);
}

#ifdef __cplusplus
}
#endif