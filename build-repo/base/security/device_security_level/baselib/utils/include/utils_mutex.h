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

#ifndef SEC_UTILS_COMMON_MUTEX_H
#define SEC_UTILS_COMMON_MUTEX_H

#include <pthread.h>

#include "utils_log.h"

#define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define RECURSIVE_MUTEX_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP

#define INITED_MUTEX      \
    {                     \
        MUTEX_INITIALIZER \
    }

#define IRECURSIVE_INITED_MUTEX                \
    {                                          \
        PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
    }

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mutex {
    pthread_mutex_t mutex;
} Mutex;

inline static void InitMutex(Mutex *mutex)
{
    (void)pthread_mutex_init(&mutex->mutex, NULL);
}

inline static void InitRecursiveMutex(Mutex *mutex)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    (void)pthread_mutex_init(&mutex->mutex, &attr);
}

inline static void LockMutex(Mutex *mutex)
{
    int ret = pthread_mutex_lock(&(mutex->mutex));
    if (ret != 0) {
        SECURITY_LOG_ERROR("pthread_mutex_lock error");
    }
}

inline static void UnlockMutex(Mutex *mutex)
{
    int ret = pthread_mutex_unlock(&(mutex->mutex));
    if (ret != 0) {
        SECURITY_LOG_ERROR("pthread_mutex_unlock error");
    }
}

#ifdef __cplusplus
}
#endif

#endif // SEC_UTILS_COMMON_MUTEX_H
