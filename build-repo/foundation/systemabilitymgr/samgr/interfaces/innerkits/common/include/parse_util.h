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

#ifndef SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_PARSE_UTIL_H
#define SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_PARSE_UTIL_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "sa_profiles.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"

namespace OHOS {
class ParseUtil {
public:
    ~ParseUtil();
    bool ParseSaProfiles(const std::string& profilePath);

    const std::list<SaProfile>& GetAllSaProfiles() const;
    bool GetProfile(int32_t saId, SaProfile& saProfile);
    void ClearResource();
    void OpenSo();
    void CloseSo(int32_t systemAbilityId);
    bool LoadSaLib(int32_t systemAbilityId);
    bool ParseTrustConfig(const std::string& profilePath, std::map<std::u16string, std::set<int32_t>>& values);
    void RemoveSaProfile(int32_t saId);
    bool CheckPathExist(const std::string& profilePath);
    std::u16string GetProcessName() const;
private:
    void CloseSo();
    void OpenSo(SaProfile& saProfile);
    void CloseHandle(SaProfile& saProfile);
    bool ParseSystemAbility(const xmlNode& rootNode, const std::u16string& processName);
    bool ParseProcess(const xmlNodePtr& rootNode, std::u16string& processName);
    void ParseSAProp(const std::string& nodeName, const std::string& nodeContent, SaProfile& saProfile);
    bool CheckRootTag(const xmlNodePtr& rootNodePtr);
    bool ParseTrustConfigInner(const xmlNodePtr& rootNodePtr, std::map<std::u16string, std::set<int32_t>>& values);
    bool ParseSaId(const xmlNodePtr& rootNode, int32_t& saId);
    std::string GetRealPath(const std::string& profilePath) const;
    std::list<SaProfile> saProfiles_;
    std::u16string procName_;
};
} // namespace OHOS

#endif // SAMGR_INTERFACE_INNERKITS_COMMOM_INCLUDE_PARSE_UTIL_H
