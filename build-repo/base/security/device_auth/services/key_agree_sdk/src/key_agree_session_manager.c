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

#include "key_agree_session_manager.h"

#include "hc_log.h"
#include "hc_mutex.h"
#include "hc_vector.h"
#include "key_agree_sdk.h"
#include "key_agree_session.h"

typedef struct {
    KeyAgreeSession *session;
    SpekeSession *spekeSession;
} SessionNode;

DECLARE_HC_VECTOR(SessionNodeVec, SessionNode);
IMPLEMENT_HC_VECTOR(SessionNodeVec, SessionNode, 1);

static SessionNodeVec g_SessionNodeVec;
static bool g_IsinitSessionNodeVec = false;
static HcMutex *g_sessionMutex = NULL;

static int32_t CreateKeyAgreeSessionMgr(void)
{
    if (g_sessionMutex == NULL) {
        g_sessionMutex = (HcMutex *)HcMalloc(sizeof(HcMutex), 0);
        if (g_sessionMutex == NULL) {
            LOGE("Alloc sessionMutex failed");
            return HC_ERR_ALLOC_MEMORY;
        }
        if (InitHcMutex(g_sessionMutex) != HC_SUCCESS) {
            LOGE("Init mutex failed");
            HcFree(g_sessionMutex);
            g_sessionMutex = NULL;
            return HC_ERROR;
        }
    }
    g_sessionMutex->lock(g_sessionMutex);
    if (!g_IsinitSessionNodeVec) {
        g_IsinitSessionNodeVec = true;
        g_SessionNodeVec = CREATE_HC_VECTOR(SessionNodeVec);
    }
    g_sessionMutex->unlock(g_sessionMutex);
    return HC_SUCCESS;
}

static void DestroyKeyAgreeSessionMgr(void)
{
    if (g_SessionNodeVec.size(&g_SessionNodeVec) == 0) {
        LOGI("Session vec is empty, destroy vec");
        DESTROY_HC_VECTOR(SessionNodeVec, &g_SessionNodeVec);
        g_IsinitSessionNodeVec = false;
        if (g_sessionMutex != NULL) {
            DestroyHcMutex(g_sessionMutex);
            HcFree(g_sessionMutex);
            g_sessionMutex = NULL;
        }
        return;
    }
    LOGI("Session vec is not empty, do nothing");
}

static uint32_t AddSessionInner(KeyAgreeSession *session, SpekeSession *spekeSession)
{
    if (session == NULL) {
        LOGE("Can not get session or sessionId!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (g_IsinitSessionNodeVec == false) {
        LOGI("Manager is not create, start create session manager!");
        if (CreateKeyAgreeSessionMgr() != HC_SUCCESS) {
            LOGE("Create Session Mgr fail!");
            return HC_ERROR;
        }
    }
    SessionNode sessionNode;
    sessionNode.session = session;
    sessionNode.spekeSession = spekeSession;
    g_SessionNodeVec.pushBack(&g_SessionNodeVec, &sessionNode);
    return HC_SUCCESS;
}

static uint32_t DeleteSessionInner(KeyAgreeSession *session)
{
    if (session == NULL) {
        LOGE("Can not get session or sessionId");
        return HC_ERR_INVALID_PARAMS;
    }
    if (g_IsinitSessionNodeVec == false) {
        LOGE("Manager is not create!");
        return HC_ERROR;
    }
    uint32_t index;
    SessionNode *sessionNode = NULL;
    FOR_EACH_HC_VECTOR(g_SessionNodeVec, index, sessionNode) {
        if ((sessionNode != NULL) && (sessionNode->session != NULL) &&
            (sessionNode->session->sessionId == session->sessionId)) {
            SessionNode tempSessionNode;
            HC_VECTOR_POPELEMENT(&g_SessionNodeVec, &tempSessionNode, index);
            return HC_SUCCESS;
        }
    }
    DestroyKeyAgreeSessionMgr();
    return HC_ERROR;
}

static SpekeSession *GetSessionInner(KeyAgreeSession *session)
{
    if (session == NULL) {
        LOGE("Can not get session or sessionId");
        return NULL;
    }
    if (g_IsinitSessionNodeVec == false) {
        LOGE("Manager is not create!");
        return NULL;
    }
    uint32_t index;
    SessionNode *sessionNode = NULL;
    FOR_EACH_HC_VECTOR(g_SessionNodeVec, index, sessionNode) {
        if ((sessionNode != NULL) && (sessionNode->session != NULL) &&
            (sessionNode->session->sessionId == session->sessionId)) {
            return sessionNode->spekeSession;
        }
    }
    return NULL;
}

static KeyAgreeSessionManager g_sessionManager = {
    .addSession = AddSessionInner,
    .deleteSession = DeleteSessionInner,
    .getSession = GetSessionInner,
};

KeyAgreeSessionManager *GetManagerInstance(void)
{
    return &g_sessionManager;
}