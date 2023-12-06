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

#ifndef KEY_AGREE_SESSION_MANAGER_H
#define KEY_AGREE_SESSION_MANAGER_H

#include "key_agree_sdk.h"
#include "key_agree_session.h"

typedef struct {
    uint32_t (*addSession)(KeyAgreeSession *sessionId, SpekeSession *spekeSession);
    uint32_t (*deleteSession)(KeyAgreeSession *sessionId);
    SpekeSession *(*getSession)(KeyAgreeSession *sessionId);
} KeyAgreeSessionManager;

KeyAgreeSessionManager *GetManagerInstance(void);

#endif