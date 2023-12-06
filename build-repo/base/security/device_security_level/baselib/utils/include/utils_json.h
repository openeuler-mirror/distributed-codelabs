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

#ifndef SEC_UTILS_JSON_H
#define SEC_UTILS_JSON_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *JsonHandle;

JsonHandle CreateJson(const char *data);
void DestroyJson(JsonHandle handle);

int32_t GetJsonFieldInt(JsonHandle handle, const char *field);
uint32_t GetJsonFieldIntArray(JsonHandle handle, const char *field, int32_t *array, int32_t arrayLen);
const char *GetJsonFieldString(JsonHandle handle, const char *field);
JsonHandle GetJsonFieldJson(JsonHandle handle, const char *field);

JsonHandle GetJsonFieldJsonArray(JsonHandle handle, uint32_t num);
int32_t GetJsonFieldJsonArraySize(JsonHandle handle);

void AddFieldIntToJson(JsonHandle handle, const char *field, int32_t value);
void AddFieldIntArrayToJson(JsonHandle handle, const char *field, const int32_t *array, int32_t arrayLen);
void AddFieldBoolToJson(JsonHandle handle, const char *field, bool value);
void AddFieldStringToJson(JsonHandle handle, const char *field, const char *value);
void AddFieldJsonToJson(JsonHandle handle, const char *field, JsonHandle json);

char *ConvertJsonToString(JsonHandle handle);

bool CompareJsonData(JsonHandle handleA, JsonHandle handleB, bool caseSensitive);

#ifdef __cplusplus
}
#endif

#endif // SEC_UTILS_JSON_H