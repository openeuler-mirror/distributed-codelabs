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

#include <map>
#include <securec.h>
#include <session.h>
#include <string>

#include "softbus_bus_center.h"

static std::map<std::string, std::map<std::string, const ISessionListener *>> sessionListeners;
static std::map<std::string, INodeStateCb *> callbacks_;

int OpenSession(const char *mySessionName, const char *peerSessionName, const char *peerDeviceId, const char *groupId,
    const SessionAttribute *attr)
{
    return -1;
}

int SendBytes(int sessionId, const void *data, unsigned int len)
{
    return -1;
}

int RemoveSessionServer(const char *pkgName, const char *sessionName)
{
    const char *invalidSessionName = "INVALID_SESSION_NAME";
    const char *closeFailedSessionName = "REMOVE_FAILED_SESSION_NAME";
    if (!strcmp(sessionName, invalidSessionName)) {
        return -1;
    }
    if (!strcmp(sessionName, closeFailedSessionName)) {
        return -1;
    }
    return 0;
}
int GetMySessionName(int sessionId, char *sessionName, unsigned int len)
{
    return -1;
}
int GetPeerSessionName(int sessionId, char *sessionName, unsigned int len)
{
    return 0;
}
int GetPeerDeviceId(int sessionId, char *devId, unsigned int len)
{
    return 0;
}
void CloseSession(int sessionId){};
int GetSessionSide(int sessionId)
{
    return 1;
};

int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    return -1;
}

int32_t GetAllNodeDeviceInfo(const char *pkgName, NodeBasicInfo **info, int32_t *infoNum)
{
    return -1;
}

int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId, NodeDeviceInfoKey key, uint8_t *info,
                       int32_t infoLen)
{
    return -1;
}

int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener)
{
    const char *invalidSessionName = "INVALID_SESSION_NAME";
    if (!strcmp(sessionName, invalidSessionName)) {
        return -1;
    }
    return 0;
}