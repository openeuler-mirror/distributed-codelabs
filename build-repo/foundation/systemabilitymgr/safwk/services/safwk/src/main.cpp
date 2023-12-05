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
#include <csignal>
#include <sys/prctl.h>
#include <vector>

#include "errors.h"
#include "local_ability_manager.h"
#include "parameter.h"
#include "safwk_log.h"
#include "securec.h"
#include "string_ex.h"
#include "system_ability_definition.h"

using namespace OHOS;
using std::string;

namespace {
const string TAG = "SaMain";

using ProcessNameSetFunc = std::function<void(const string&)>;

constexpr auto DEFAULT_XML = "/system/usr/default.xml";
// The pid name can be up to 16 bytes long, including the terminating null byte.
// So need to set the max length of pid name to 15 bytes.
constexpr size_t MAX_LEN_PID_NAME = 15;

constexpr int PROFILE_INDEX = 1;
constexpr int SAID_INDEX = 2;
constexpr int DEFAULT_SAID = -1;
constexpr int DEFAULT_LOAD = 1;
constexpr int ONDEMAND_LOAD = 2;
}

static void StartMemoryHook(const string& processName)
{
    const int paramBufLen = 128;
    const char defaultValue[paramBufLen] = { 0 };
    const char targetPrefix[] = "startup:";
    const int targetPrefixLen = 8;
    char paramValue[paramBufLen] = { 0 };
    int retParam = GetParameter("libc.hook_mode", defaultValue, paramValue, sizeof(paramValue));
    if (retParam <= 0 || strncmp(paramValue, targetPrefix, targetPrefixLen) != 0) {
        return;
    }

    if (processName.find(paramValue + targetPrefixLen) == 0) {
        const int hookSignal = 36;
        HILOGI(TAG, "raise hook signal %{public}d to %{public}s", hookSignal, processName.c_str());
        raise(hookSignal);
    }
}

static void SetProcName(const string& filePath, const ProcessNameSetFunc& setProcessName)
{
    std::vector<string> strVector;
    SplitStr(filePath, "/", strVector);
    auto vectorSize = strVector.size();
    if (vectorSize > 0) {
        auto& fileName = strVector[vectorSize - 1];
        auto dotPos = fileName.find(".");
        if (dotPos == string::npos) {
            return;
        }
        if (dotPos > MAX_LEN_PID_NAME) {
            dotPos = MAX_LEN_PID_NAME;
        }
        string profileName = fileName.substr(0, dotPos);
        int32_t ret = prctl(PR_SET_NAME, profileName.c_str());
        if (ret != 0) {
            HILOGI(TAG, "call the system API prctl failed!");
        }
        setProcessName(profileName);
        StartMemoryHook(profileName);
    }
}

// check argv size with SAID_INDEX before using the function
static int32_t ParseSaId(char *argv[])
{
    string saIdStr(argv[SAID_INDEX]);
    int32_t saId = DEFAULT_SAID;
    if (!StrToInt(saIdStr, saId)) {
        return DEFAULT_SAID;
    }
    return saId;
}

static bool CheckSaId(int32_t saId)
{
    return (saId >= FIRST_SYS_ABILITY_ID) && (saId <= LAST_SYS_ABILITY_ID);
}

int main(int argc, char *argv[])
{
    HILOGI(TAG, "[PerformanceTest] SAFWK main entry process starting!");
    auto setProcessName = [argc, argv](const string& name) -> void {
        uintptr_t start = reinterpret_cast<uintptr_t>(argv[0]);
        uintptr_t end = reinterpret_cast<uintptr_t>(strchr(argv[argc - 1], 0));
        uintptr_t argvSize = end - start;

        if (memset_s(argv[0], argvSize, 0, argvSize) != EOK) {
            HILOGW(TAG, "failed to clear argv:%s", strerror(errno));
            return;
        }
        if (strcpy_s(argv[0], argvSize, name.c_str()) != EOK) {
            HILOGW(TAG, "failed to set process name:%s", strerror(errno));
            return;
        }
        HILOGI(TAG, "Set process name to %{public}s", argv[0]);
    };
    // Load ondemand system abilities related shared libraries from specific xml-format profile
    // when this process starts.
    int32_t saId = DEFAULT_SAID;
    if (argc > ONDEMAND_LOAD) {
        saId = ParseSaId(argv);
        if (!CheckSaId(saId)) {
            HILOGE(TAG, "saId is invalid!");
            return 0;
        }
    }
    // Load default system abilities related shared libraries from specific xml-format profile,
    // when this process starts.
    string profilePath(DEFAULT_XML);
    if (argc > DEFAULT_LOAD) {
        string filePath(argv[PROFILE_INDEX]);
        if (filePath.empty() || filePath.find(".xml") == string::npos) {
            HILOGE(TAG, "profile file path is invalid!");
            return 0;
        }
        SetProcName(filePath, setProcessName);
        profilePath = std::move(filePath);
    }
    LocalAbilityManager::GetInstance().DoStartSAProcess(profilePath, saId);
    return 0;
}
