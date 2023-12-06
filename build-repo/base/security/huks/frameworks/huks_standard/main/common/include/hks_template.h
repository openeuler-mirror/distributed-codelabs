/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HKS_TEMPLATE_H
#define HKS_TEMPLATE_H

#undef HKS_NULL_POINTER

#ifdef __cplusplus
#define HKS_NULL_POINTER nullptr
#else
#define HKS_NULL_POINTER NULL
#endif

#define HKS_IF_NOT_SUCC_LOGE_RETURN(RESULT, ERROR_CODE, LOG_MESSAGE, ...) \
if ((RESULT) != HKS_SUCCESS) { \
    HKS_LOG_E(LOG_MESSAGE, ##__VA_ARGS__); \
    return (ERROR_CODE); \
}

#define HKS_IF_NOT_SUCC_LOGE_BREAK(RESULT, LOG_MESSAGE, ...) \
if ((RESULT) != HKS_SUCCESS) { \
    HKS_LOG_E(LOG_MESSAGE, ##__VA_ARGS__); \
    break; \
}

#define HKS_IF_NOT_SUCC_BREAK(RESULT, ...) \
if ((RESULT) != HKS_SUCCESS) { \
    break; \
}

#define HKS_IF_NOT_SUCC_LOGE(RESULT, LOG_MESSAGE, ...) \
if ((RESULT) != HKS_SUCCESS) { \
    HKS_LOG_E(LOG_MESSAGE, ##__VA_ARGS__); \
}

#define HKS_IF_NOT_SUCC_RETURN(RESULT, ERROR_CODE) \
if ((RESULT) != HKS_SUCCESS) { \
    return (ERROR_CODE); \
}

#define HKS_IF_NULL_LOGE_RETURN(OBJECT, ERROR_CODE, LOG_MESSAGE, ...) \
if ((OBJECT) == HKS_NULL_POINTER) { \
    HKS_LOG_E(LOG_MESSAGE, ##__VA_ARGS__); \
    return (ERROR_CODE); \
}

#define HKS_IF_NULL_LOGE_BREAK(OBJECT, LOG_MESSAGE, ...) \
if ((OBJECT) == HKS_NULL_POINTER) { \
    HKS_LOG_E(LOG_MESSAGE, ##__VA_ARGS__); \
    break; \
}

#define HKS_IF_NULL_RETURN(OBJECT, ERROR_CODE) \
if ((OBJECT) == HKS_NULL_POINTER) { \
    return (ERROR_CODE); \
}

#define HKS_IF_NULL_BREAK(OBJECT) \
if ((OBJECT) == HKS_NULL_POINTER) { \
    break; \
}
#endif /* HKS_TEMPLATE_H */
