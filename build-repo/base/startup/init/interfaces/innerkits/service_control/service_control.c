/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "service_control.h"


static int StartProcess(const char *name, const char *extArgv[], int extArgc)
{
    return -1;
}

static int StopProcess(const char *serviceName)
{
    return -1;
}

static int GetCurrentServiceStatus(const char *serviceName, ServiceStatus *status)
{
    return -1;
}

static int RestartProcess(const char *serviceName, const char *extArgv[], int extArgc)
{
    return -1;
}

int ServiceControlWithExtra(const char *serviceName, int action, const char *extArgv[], int extArgc)
{
    return -1;
}

int ServiceControl(const char *serviceName, int action)
{
    return -1;
}

static int GetProcessInfo(const char *serviceName, char *nameBuffer, char *valueBuffer, ServiceStatus status)
{
    return -1;
}

int ServiceWaitForStatus(const char *serviceName, ServiceStatus status, int waitTimeout)
{
    return -1;
}

int ServiceSetReady(const char *serviceName)
{
    return -1;
}

int StartServiceByTimer(const char *serviceName, uint64_t timeout)
{
    return -1;
}

int StopServiceTimer(const char *serviceName)
{
    return -1;
}
