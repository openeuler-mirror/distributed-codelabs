/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "hks_condition.h"

#include "hks_mem.h"
#include "hks_template.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HksCondition {
    bool notified;
    bool waited;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

int32_t HksConditionWait(HksCondition *condition)
{
    HKS_IF_NULL_RETURN(condition, -1)

    int32_t ret = pthread_mutex_lock(&condition->mutex);
    if (ret != 0) {
        return ret;
    }
    if (condition->notified) {
        condition->notified = false;
        pthread_mutex_unlock(&condition->mutex);
        return 0;
    } else {
        condition->waited = true;
        ret = pthread_cond_wait(&condition->cond, &condition->mutex);
        condition->waited = false;
        condition->notified = false;
        pthread_mutex_unlock(&condition->mutex);
        return ret;
    }
}

int32_t HksConditionNotify(HksCondition *condition)
{
    HKS_IF_NULL_RETURN(condition, -1)

    int32_t ret = pthread_mutex_lock(&condition->mutex);
    if (ret != 0) {
        return ret;
    }

    if (!condition->waited) {
        condition->notified = true;
    } else {
        condition->notified = false;
    }
    ret = pthread_cond_signal(&condition->cond);
    pthread_mutex_unlock(&condition->mutex);
    return ret;
}

HksCondition *HksConditionCreate(void)
{
    HksCondition *condition = (HksCondition *)HksMalloc(sizeof(HksCondition));
    HKS_IF_NULL_RETURN(condition, NULL)
    condition->notified = false;
    condition->waited = false;
    int32_t ret = pthread_mutex_init(&condition->mutex, NULL);
    if (ret != 0) {
        HksFree(condition);
        return NULL;
    }

    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    ret = pthread_cond_init(&condition->cond, &attr);
    pthread_condattr_destroy(&attr);
    if (ret != 0) {
        pthread_mutex_destroy(&condition->mutex);
        HksFree(condition);
        return NULL;
    }
    return condition;
}

void HksConditionDestroy(HksCondition* condition)
{
    if (condition == NULL) {
        return;
    }
    pthread_mutex_destroy(&condition->mutex);
    pthread_cond_destroy(&condition->cond);
    HksFree(condition);
}

#ifdef __cplusplus
}
#endif