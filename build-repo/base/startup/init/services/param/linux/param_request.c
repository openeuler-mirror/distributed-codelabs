/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "init_param.h"
#include "param_request.h"
#include "parameter.h"
#include "param_utils.h"

#include <errno.h>
#include <stdatomic.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/un.h>
#include <stdio.h>

#include "beget_ext.h"

static void ClearEnv(ParamRequestMsg* pmsg, ParamRespMsg* respmsg, int fd)
{
    if (pmsg != NULL)
        free(pmsg);
    if (respmsg != NULL)
        free(respmsg);
    if (fd > 0)
        close(fd);
}

static int GetClientSocket()
{
    int cfd = socket(PF_UNIX, SOCK_STREAM, 0);
    BEGET_ERROR_CHECK(cfd > 0, return -1, "Failed to create client socket");

    struct sockaddr_un serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sun_family = PF_UNIX;
	strncpy(serverAddr.sun_path, PIPE_NAME, strlen(PIPE_NAME));
	if (connect(cfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("Failed to connect");
		return -1;
	}

    return cfd;
}

static struct ParamRequestMsg* GetRequestMsg(uint32_t type, uint32_t size)
{
    uint32_t data_alloc_size = size;
    if (data_alloc_size > PARAM_VALUE_LEN_MAX || data_alloc_size == 0)
        data_alloc_size = PARAM_VALUE_LEN_MAX;
    struct ParamRequestMsg *pmsg;
    if (type == GET_PARAMETER) {
        pmsg = (struct ParamRequestMsg*)malloc(sizeof(struct ParamRequestMsg));
        BEGET_ERROR_CHECK(pmsg != NULL, return NULL, "Failed to malloc ParamRequestMsg");
        bzero(pmsg, sizeof(struct ParamRequestMsg));
    } else if (type == SET_PARAMETER) {
        pmsg = (struct ParamRequestMsg*)malloc(sizeof(struct ParamRequestMsg) + data_alloc_size);
        BEGET_ERROR_CHECK(pmsg != NULL, return NULL, "Failed to malloc ParamRequestMsg");
        bzero(pmsg, sizeof(struct ParamRequestMsg) + data_alloc_size);
    } else if (type == WAIT_PARAMETER) {
        pmsg = (struct ParamRequestMsg*)malloc(sizeof(struct ParamRequestMsg) + data_alloc_size);
        BEGET_ERROR_CHECK(pmsg != NULL, return NULL, "Failed to malloc ParamRequestMsg");
        bzero(pmsg, sizeof(struct ParamRequestMsg) + data_alloc_size);
    }
    pmsg->datasize = data_alloc_size;
    pmsg->type = type;
    return pmsg;
}

static struct ParamRespMsg* StartRequest(int fd, struct ParamRequestMsg* pmsg)
{
    int ret = send(fd, pmsg, sizeof(struct ParamRequestMsg) + pmsg->datasize, 0);
    BEGET_ERROR_CHECK(ret > 0, return NULL, "Failed to send msg");

    struct ParamRespMsg* respmsg = (struct ParamRespMsg*)malloc(sizeof(struct ParamRespMsg) + PARAM_VALUE_LEN_MAX);
    BEGET_ERROR_CHECK(respmsg != NULL, return NULL, "Failed to malloc ParamRespMsg");

    bzero(respmsg, sizeof(struct ParamRespMsg) + PARAM_VALUE_LEN_MAX);
    ret = recv(fd, respmsg, sizeof(struct ParamRespMsg) + PARAM_VALUE_LEN_MAX, 0);
    BEGET_ERROR_CHECK(ret > 0, free(respmsg);return NULL, "Failed to recv msg");
    return respmsg;
}

int SystemSetParameter(const char *name, const char *value)
{
    BEGET_ERROR_CHECK(name != NULL, return -1, "Invalid name");
    BEGET_ERROR_CHECK(value != NULL, return -1, "Invalid value");
    BEGET_ERROR_CHECK(CheckParamName(name, 0) == 0, return -1, "Invalid name");

    int fd = GetClientSocket();
    if (fd < 0)
        return -1;
    struct ParamRequestMsg* pmsg = GetRequestMsg(SET_PARAMETER, strlen(value));
    if (pmsg == NULL) {
        close(fd);
        return -1;
    }

    strncpy(pmsg->key, name, sizeof(pmsg->key));
	strncpy(pmsg->data, value, pmsg->datasize);
    int ret;
    struct ParamRespMsg* respmsg = StartRequest(fd, pmsg);
    if (respmsg == NULL) {
        ret = -1;
    } else {
        if (respmsg->flag == 0) {
            ret = 0;
        } else {
            ret = -1;
        }
    }

    ClearEnv(pmsg, respmsg, fd);
    return ret;
}

int SystemReadParam(const char *name, char *value, uint32_t *len)
{
    BEGET_ERROR_CHECK(name != NULL, return -1, "Invalid name");
    BEGET_ERROR_CHECK(*len <=  PARAM_BUFFER_MAX, return -1, "Invalid len");

    int fd = GetClientSocket();
    if (fd < 0)
        return -1;
    struct ParamRequestMsg* pmsg = GetRequestMsg(GET_PARAMETER, *len);
    BEGET_ERROR_CHECK(pmsg != NULL, close(fd);return -1, "Invalid pmsg");

    strncpy(pmsg->key, name, sizeof(pmsg->key));
    int ret;
    struct ParamRespMsg* respmsg = StartRequest(fd, pmsg);
    if (respmsg == NULL) {
        ret = -1;
    } else {
        if (respmsg->flag == 0 && respmsg->datasize > 0) {
            if (value == NULL) {
                *len = respmsg->datasize + 1;
                ret = 0;
            } else {
                strncpy(value, respmsg->data, *len);
                ret = 0;
            }
        } else {
            ret = -1;
        }
    }

    ClearEnv(pmsg, respmsg, fd);
    return ret;
}

int SystemWaitParameter(const char *name, const char *value, int32_t timeout)
{
    BEGET_ERROR_CHECK(name != NULL, return -1, "Invalid name");
    BEGET_ERROR_CHECK(value != NULL, return -1, "Invalid value");
    BEGET_ERROR_CHECK(CheckParamName(name, 0) == 0, return -1, "Invalid name");
    
    int ret;
    int fd = GetClientSocket();
    if (fd < 0)
        return -1;
        
    struct ParamRequestMsg* pmsg = GetRequestMsg(WAIT_PARAMETER, strlen(value) + 1);
    BEGET_ERROR_CHECK(pmsg != NULL, close(fd);return -1, "Invalid pmsg");

    pmsg->timeout = timeout;
    strncpy(pmsg->key, name, sizeof(pmsg->key));
    strncpy(pmsg->data, value, sizeof(pmsg->datasize));
    struct ParamRespMsg* respmsg = StartRequest(fd, pmsg);
    if (respmsg == NULL) {
        ret = -1;
    } else {
        ret = respmsg->flag;
    }

    ClearEnv(pmsg, respmsg, fd);
    return ret;
}
