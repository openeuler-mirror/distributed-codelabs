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

#include "parse_util.h"

#include <cinttypes>
#include <dlfcn.h>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>
#include <algorithm>

#include "datetime_ex.h"
#include "hisysevent_adapter.h"
// #include "hitrace_meter.h"
#include "libxml/globals.h"
#include "libxml/tree.h"
#include "libxml/xmlstring.h"
#include "sam_log.h"
#include "string_ex.h"

namespace OHOS {
using std::string;

namespace {
const auto XML_TAG_PROFILE = "profile";
const auto XML_TAG_INFO = "info";
const auto XML_TAG_SYSTEM_ABILITY = "systemability";
const auto XML_TAG_PROCESS = "process";
const auto XML_TAG_LIB_PATH = "libpath";
const auto XML_TAG_NAME = "name";
const auto XML_TAG_DEPEND = "depend";
const auto XML_TAG_DEPEND_TIMEOUT = "depend-time-out";
const auto XML_TAG_RUN_ON_CREATE = "run-on-create";
const auto XML_TAG_DISTRIBUTED = "distributed";
const auto XML_TAG_DUMP_LEVEL = "dump-level";
const auto XML_TAG_CAPABILITY = "capability";
const auto XML_TAG_PERMISSION = "permission";
const auto XML_TAG_BOOT_PHASE = "bootphase";
const auto XML_TAG_SAID = "said";
}

ParseUtil::~ParseUtil()
{
    ClearResource();
}

void ParseUtil::CloseHandle(SaProfile& saProfile)
{
    if (saProfile.handle == nullptr) {
        return;
    }
    int32_t ret = dlclose(saProfile.handle);
    if (ret) {
        HILOGW("close handle failed with errno:%{public}d!", errno);
    }
    saProfile.handle = nullptr;
}

void ParseUtil::CloseSo()
{
    for (auto& saProfile : saProfiles_) {
        CloseHandle(saProfile);
    }
}

void ParseUtil::CloseSo(int32_t systemAbilityId)
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.saId == systemAbilityId) {
            CloseHandle(saProfile);
            break;
        }
    }
}

void ParseUtil::ClearResource()
{
    CloseSo();
    saProfiles_.clear();
}

void ParseUtil::OpenSo()
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.runOnCreate) {
            OpenSo(saProfile);
        }
    }
}

void ParseUtil::OpenSo(SaProfile& saProfile)
{
    if (saProfile.handle == nullptr) {
        std::string dlopenTag = ToString(saProfile.saId) + "_DLOPEN";
        // HITRACE_METER_NAME(HITRACE_TAG_SAMGR, dlopenTag);
        int64_t begin = GetTickCount();
        DlHandle handle = dlopen(Str16ToStr8(saProfile.libPath).c_str(), RTLD_NOW);
        HILOGI("[PerformanceTest] SA:%{public}d OpenSo spend %{public}" PRId64 " ms",
            saProfile.saId, GetTickCount() - begin);
        if (handle == nullptr) {
            std::vector<string> libPathVec;
            string fileName = "";
            SplitStr(Str16ToStr8(saProfile.libPath), "/", libPathVec);
            if ((libPathVec.size() > 0)) {
                fileName = libPathVec[libPathVec.size() - 1];
            }
            // ReportAddSystemAbilityFailed(saProfile.saId, fileName);
            HILOGE("dlopen %{public}s failed with errno:%s!", fileName.c_str(), dlerror());
            return;
        }
        saProfile.handle = handle;
    } else {
        HILOGI("SA:%{public}d handle is not null", saProfile.saId);
    }
}

bool ParseUtil::LoadSaLib(int32_t systemAbilityId)
{
    for (auto& saProfile : saProfiles_) {
        if (saProfile.saId == systemAbilityId) {
            OpenSo(saProfile);
            return true;
        }
    }
    return false;
}

const std::list<SaProfile>& ParseUtil::GetAllSaProfiles() const
{
    return saProfiles_;
}

bool ParseUtil::GetProfile(int32_t saId, SaProfile& saProfile)
{
    auto iter = std::find_if(saProfiles_.begin(), saProfiles_.end(), [saId](auto saProfile) {
        return saProfile.saId == saId;
    });
    if (iter != saProfiles_.end()) {
        saProfile = *iter;
        return true;
    }
    return false;
}

void ParseUtil::RemoveSaProfile(int32_t saId)
{
    saProfiles_.remove_if([saId] (auto saInfo) -> bool { return saInfo.saId == saId; });
}

void ParseUtil::ParseSAProp(const string& nodeName, const string& nodeContent, SaProfile& saProfile)
{
    if (nodeName == XML_TAG_NAME) {
        StrToInt(nodeContent.c_str(), saProfile.saId);
    } else if (nodeName == XML_TAG_LIB_PATH) {
        saProfile.libPath = Str8ToStr16(nodeContent);
    } else if (nodeName == XML_TAG_DEPEND) {
        saProfile.dependSa.emplace_back(Str8ToStr16(nodeContent));
    } else if (nodeName == XML_TAG_DEPEND_TIMEOUT) {
        StrToInt(nodeContent.c_str(), saProfile.dependTimeout);
    } else if (nodeName == XML_TAG_RUN_ON_CREATE) {
        std::istringstream(nodeContent) >> std::boolalpha >> saProfile.runOnCreate;
    } else if (nodeName == XML_TAG_DISTRIBUTED) {
        std::istringstream(nodeContent) >> std::boolalpha >> saProfile.distributed;
    } else if (nodeName == XML_TAG_DUMP_LEVEL) {
        std::stringstream ss(nodeContent);
        ss >> saProfile.dumpLevel;
    } else if (nodeName == XML_TAG_CAPABILITY) {
        saProfile.capability = Str8ToStr16(nodeContent);
    } else if (nodeName == XML_TAG_PERMISSION) {
        saProfile.permission = Str8ToStr16(nodeContent);
    } else if (nodeName == XML_TAG_BOOT_PHASE) {
        saProfile.bootPhase = Str8ToStr16(nodeContent);
    }
}

bool ParseUtil::ParseSystemAbility(const xmlNode& rootNode, const std::u16string& process)
{
    auto currNodePtr = rootNode.xmlChildrenNode;
    if (currNodePtr == nullptr) {
        return false;
    }
    SaProfile saProfile;
    saProfile.process = process;
    for (; currNodePtr != nullptr; currNodePtr = currNodePtr->next) {
        if (currNodePtr->name == nullptr || currNodePtr->type == XML_COMMENT_NODE) {
            continue;
        }
        auto contentPtr = xmlNodeGetContent(currNodePtr);
        if (contentPtr == nullptr) {
            continue;
        }
        string nodeName(reinterpret_cast<const char*>(currNodePtr->name));
        string nodeContent(reinterpret_cast<char*>(contentPtr));
        ParseSAProp(nodeName, nodeContent, saProfile);
        xmlFree(contentPtr);
    }
    saProfiles_.emplace_back(saProfile);
    return true;
}

bool ParseUtil::ParseProcess(const xmlNodePtr& rootNode, std::u16string& processName)
{
    if (rootNode->name == nullptr || rootNode->type == XML_COMMENT_NODE) {
        return false;
    }
    auto contentPtr = xmlNodeGetContent(rootNode);
    if (contentPtr == nullptr) {
        return false;
    }
    string nodeContent(reinterpret_cast<char*>(contentPtr));
    processName = Str8ToStr16(nodeContent);
    xmlFree(contentPtr);
    return true;
}

bool ParseUtil::ParseSaProfiles(const string& profilePath)
{
    HILOGD("xmlFile:%{private}s", profilePath.c_str());
    std::string realPath = GetRealPath(profilePath);
    if (!CheckPathExist(realPath.c_str())) {
        HILOGE("bad profile path!");
        return false;
    }
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> ptrDoc(
        xmlReadFile(realPath.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);

    if (ptrDoc == nullptr) {
        HILOGE("xmlReadFile error!");
        return false;
    }
    xmlNodePtr rootNodePtr = xmlDocGetRootElement(ptrDoc.get());
    if (!CheckRootTag(rootNodePtr)) {
        HILOGW("wrong root element tag!");
        return false;
    }
    bool isParseCorrect = false;
    xmlNodePtr currNodePtr = rootNodePtr->xmlChildrenNode;
    std::u16string process = u"";
    for (; currNodePtr != nullptr; currNodePtr = currNodePtr->next) {
        if (currNodePtr->name == nullptr || currNodePtr->type == XML_COMMENT_NODE) {
            continue;
        }

        string nodeName(reinterpret_cast<const char*>(currNodePtr->name));
        HILOGD("profile nodeName:%{public}s", nodeName.c_str());
        if (nodeName == XML_TAG_PROCESS && process.empty()) {
            if (!ParseProcess(currNodePtr, process)) {
                HILOGW("profile %{public}s wrong tag!", currNodePtr->name);
                return false;
            }
        }
        if (nodeName == XML_TAG_SYSTEM_ABILITY) {
            if (!ParseSystemAbility(*currNodePtr, process)) {
                HILOGW("profile %{public}s wrong tag!", currNodePtr->name);
                return false;
            }
            isParseCorrect = true;
        }
    }
    procName_ = process;
    return isParseCorrect;
}

std::u16string ParseUtil::GetProcessName() const
{
    return procName_;
}

std::string ParseUtil::GetRealPath(const string& profilePath) const
{
    char path[PATH_MAX] = {'\0'};
    if (realpath(profilePath.c_str(), path) == nullptr) {
        HILOGE("get real path fail");
        return "";
    }
    std::string realPath(path);
    return realPath;
}

bool ParseUtil::CheckPathExist(const string& profilePath)
{
    std::ifstream profileStream(profilePath.c_str());
    return profileStream.good();
}

bool ParseUtil::ParseTrustConfig(const string& profilePath,
    std::map<std::u16string, std::set<int32_t>>& values)
{
    HILOGD("config path:%{private}s", profilePath.c_str());
    std::string realPath = GetRealPath(profilePath);
    if (!CheckPathExist(realPath.c_str())) {
        HILOGE("bad profile path!");
        return false;
    }
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(realPath.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        HILOGE("ParseTrustConfig xmlReadFile error!");
        return false;
    }
    xmlNodePtr rootNodePtr = xmlDocGetRootElement(docPtr.get());
    if (!CheckRootTag(rootNodePtr)) {
        HILOGW("ParseTrustConfig wrong root element tag!");
        return false;
    }

    return ParseTrustConfigInner(rootNodePtr, values);
}

bool ParseUtil::CheckRootTag(const xmlNodePtr& rootNodePtr)
{
    if (rootNodePtr == nullptr || rootNodePtr->name == nullptr ||
        (xmlStrcmp(rootNodePtr->name, reinterpret_cast<const xmlChar*>(XML_TAG_PROFILE)) != 0 &&
        xmlStrcmp(rootNodePtr->name, reinterpret_cast<const xmlChar*>(XML_TAG_INFO)) != 0)) {
        return false;
    }
    return true;
}

bool ParseUtil::ParseTrustConfigInner(const xmlNodePtr& rootNodePtr,
    std::map<std::u16string, std::set<int32_t>>& values)
{
    xmlNodePtr currNodePtr = rootNodePtr->xmlChildrenNode;
    if (currNodePtr == nullptr) {
        return false;
    }
    std::u16string processName = u"";
    for (; currNodePtr != nullptr; currNodePtr = currNodePtr->next) {
        if (currNodePtr->name == nullptr || currNodePtr->type == XML_COMMENT_NODE) {
            continue;
        }

        string nodeName(reinterpret_cast<const char*>(currNodePtr->name));
        HILOGD("ParseTrustConfigInner profile nodeName:%{public}s", nodeName.c_str());

        if (nodeName == XML_TAG_NAME && processName.empty()) {
            // parse process name
            if (!ParseProcess(currNodePtr, processName)) {
                HILOGE("ParseTrustConfigInner wrong name tag!");
                return false;
            }
        } else if (nodeName == XML_TAG_SAID) {
            // parse said
            int32_t saId = -1;
            if (!ParseSaId(currNodePtr, saId)) {
                HILOGE("ParseTrustConfigInner wrong said tag!");
                continue;
            }
            auto& saIds = values[processName];
            saIds.emplace(saId);
        }
    }
    return true;
}

bool ParseUtil::ParseSaId(const xmlNodePtr& rootNode, int32_t& saId)
{
    if (rootNode->name == nullptr || rootNode->type == XML_COMMENT_NODE) {
        return false;
    }
    auto contentPtr = xmlNodeGetContent(rootNode);
    if (contentPtr == nullptr) {
        return false;
    }
    string nodeContent(reinterpret_cast<char*>(contentPtr));
    bool ret = StrToInt(nodeContent.c_str(), saId);
    xmlFree(contentPtr);
    return ret;
}
} // namespace OHOS
