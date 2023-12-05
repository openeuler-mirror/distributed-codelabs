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

#include "hisysevent_json_decorator.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "hilog/log.h"
#include "hisysevent.h"
#include "json_flatten_parser.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT_JSON_DECORATOR" };
constexpr char ARRY_SIZE[] = "arrsize";
constexpr char DECORATE_PREFIX[] = "\033[31m";
constexpr char DECORATE_SUFFIX[] = "\033[0m";
constexpr char HISYSEVENT_YAML_DEF_JSON_PATH[] = "/system/etc/hiview/hisysevent.def";
constexpr char LEVEL[] = "level";
constexpr char LEVEL_[] = "level_";
constexpr char TYPE[] = "type";
constexpr int BASE_INDEX = 0;
constexpr int DOMAIN_INDEX = 1;
constexpr int NAME_INDEX = 2;
const char* INNER_BUILD_KEYS[] = {
    "__BASE", "domain_", "name_", "type_", "level_", "tag_",
    "time_", "tz_", "pid_", "tid_", "uid_", "traceid_",
    "id_", "spanid_", "pspanid_", "trace_flag_", "info_", "seq_" };
const std::string VALID_LEVELS[] = { "CRITICAL", "MINOR" };
const std::map<std::string, int> EVENT_TYPE_MAP = {{"FAULT", 1}, {"STATISTIC", 2}, {"SECURITY", 3}, {"BEHAVIOR", 4} };
}

HiSysEventJsonDecorator::HiSysEventJsonDecorator()
{
    std::ifstream fin(HISYSEVENT_YAML_DEF_JSON_PATH, std::ifstream::binary);
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    JSONCPP_STRING errs;
    if (!parseFromStream(jsonRBuilder, fin, &root, &errs)) {
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(fin, root)) {
#endif
        HiLog::Error(LABEL, "parse json file failed, please check the style of json file: %{public}s.",
            HISYSEVENT_YAML_DEF_JSON_PATH);
    } else {
        isRootValid = true;
    }
}

bool HiSysEventJsonDecorator::CheckAttrDecorationNeed(const Json::Value& eventJson, const std::string& key,
    const Json::Value& standard)
{
    auto ret = CheckAttrValidity(eventJson, key, standard);
    decoratedMarks[key] = ret;
    return ret != Validity::KV_BOTH_VALID;
}

Validity HiSysEventJsonDecorator::CheckAttrValidity(const Json::Value& eventJson, const std::string& key,
    const Json::Value& standard)
{
    if (!standard.isMember(key)) {
        return Validity::KEY_INVALID;
    }
    bool ret = false;
    if (standard[key].isMember(ARRY_SIZE)) {
        if (!eventJson[key].isArray() || eventJson[key].size() > standard[key][ARRY_SIZE].asUInt()) {
            return Validity::VALUE_INVALID;
        }
        ret = JudgeDataType(standard[key][TYPE].asString(), eventJson[key][0]);
        return ret ? Validity::KV_BOTH_VALID : Validity::VALUE_INVALID;
    }
    ret = JudgeDataType(standard[key][TYPE].asString(), eventJson[key]);
    return ret ? Validity::KV_BOTH_VALID : Validity::VALUE_INVALID;
}

Validity HiSysEventJsonDecorator::CheckLevelValidity(const Json::Value& baseInfo)
{
    if (!baseInfo.isMember(LEVEL)) {
        HiLog::Error(LABEL, "level not defined in __BASE");
        return Validity::KEY_INVALID;
    }
    std::string levelDes = baseInfo[LEVEL].asString();
    if (std::any_of(std::begin(VALID_LEVELS), std::end(VALID_LEVELS), [&levelDes] (auto& level) {
        return level == levelDes;
    })) {
        return Validity::KV_BOTH_VALID;
    }
    return Validity::VALUE_INVALID;
}

bool HiSysEventJsonDecorator::CheckEventDecorationNeed(const Json::Value& eventJson,
    BaseInfoHandler baseJsonInfoHandler, ExtensiveInfoHander extensiveJsonInfoHandler)
{
    if (!isRootValid || !root.isObject() || !eventJson.isObject()) {
        return true;
    }
    std::string domain = eventJson[INNER_BUILD_KEYS[DOMAIN_INDEX]].asString();
    std::string name = eventJson[INNER_BUILD_KEYS[NAME_INDEX]].asString();
    if (!root.isMember(domain)) {
        return true;
    }
    Json::Value definedDomain = root[domain];
    if (!definedDomain.isObject() || !definedDomain.isMember(name)) {
        return true;
    }
    Json::Value definedName = definedDomain[name];
    if (!definedName.isObject()) {
        return true;
    }
    auto baseInfoNeed = false;
    if (definedName.isMember(INNER_BUILD_KEYS[BASE_INDEX])) {
        baseInfoNeed = baseJsonInfoHandler(definedName[INNER_BUILD_KEYS[BASE_INDEX]]);
    }
    auto extensiveInfoNeed = extensiveJsonInfoHandler(eventJson, definedName);
    return baseInfoNeed || extensiveInfoNeed;
}

std::string HiSysEventJsonDecorator::Decorate(Validity validity, std::string& key, std::string& value)
{
    std::stringstream ss;
    switch (validity) {
        case Validity::KEY_INVALID:
            ss << "\"" << DECORATE_PREFIX << key << DECORATE_SUFFIX << "\":" << value;
            break;
        case Validity::VALUE_INVALID:
            ss << "\"" << key << "\":" << DECORATE_PREFIX << value << DECORATE_SUFFIX;
            break;
        case Validity::KV_BOTH_VALID:
            ss << "\"" << key << "\":" << value;
            break;
        default:
            break;
    }
    std::string ret = ss.str();
    ss.clear();
    return ret;
}

std::string HiSysEventJsonDecorator::DecorateEventJsonStr(const HiSysEventRecord& record)
{
    std::string origin = record.AsJson();
    decoratedMarks.clear(); // reset marked keys.
    if (!isRootValid) {
        HiLog::Error(LABEL, "root json value is not valid, failed to decorate.");
        return origin;
    }
    Json::Value eventJson;
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    std::unique_ptr<Json::CharReader> const reader(jsonRBuilder.newCharReader());
    JSONCPP_STRING errs;
    if (!reader->parse(origin.data(), origin.data() + origin.size(), &eventJson, &errs)) {
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(origin, eventJson)) {
#endif
        HiLog::Error(LABEL, "parse json file failed, please check the style of json file: %{public}s.",
            origin.c_str());
        return origin;
    }
    auto needDecorate = CheckEventDecorationNeed(eventJson,
        [this] (const Json::Value& definedBase) {
            auto levelValidity = this->CheckLevelValidity(definedBase);
            this->decoratedMarks[LEVEL_] = levelValidity;
            return levelValidity != Validity::KV_BOTH_VALID;
        }, [this] (const Json::Value& eventJson, const Json::Value& definedName) {
                auto attrList = eventJson.getMemberNames();
                bool ret = false;
                for (auto it = attrList.cbegin(); it != attrList.cend(); it++) {
                    std::string key = *it;
                    if (std::find_if(std::cbegin(INNER_BUILD_KEYS), std::cend(INNER_BUILD_KEYS),
                        [&ret, &eventJson, &key, &definedName] (const char* ele) {
                            return (key.compare(ele) == 0);
                        }) == std::cend(INNER_BUILD_KEYS)) {
                        ret = this->CheckAttrDecorationNeed(eventJson, key, definedName) || ret;
                    }
                }
                return ret;
            });
    if (!needDecorate) {
        HiLog::Debug(LABEL, "no need to decorate this event json string.");
        return origin;
    }
    HiLog::Debug(LABEL, "all invalid key or value will be high-lighted with red color.");
    return DecorateJsonStr(origin, decoratedMarks);
}

std::string HiSysEventJsonDecorator::DecorateJsonStr(const std::string& origin, DecorateMarks marks)
{
    if (marks.empty()) {
        return origin;
    }
    JsonFlattenParser parser(origin);
    return parser.Print([this, &marks] (KV& kv) -> std::string {
        auto iter = marks.find(kv.first);
        return this->Decorate((iter == marks.end() ? Validity::KV_BOTH_VALID : iter->second), kv.first, kv.second);
    });
}

bool HiSysEventJsonDecorator::JudgeDataType(const std::string &dataType, const Json::Value &eventJson)
{
    if (dataType.compare("BOOL") == 0) {
        return eventJson.isBool() || (eventJson.isInt() && (eventJson.asInt() == 0 || eventJson.asInt() == 1));
    } else if ((dataType.compare("INT8") == 0) || (dataType.compare("INT16") == 0) ||
        (dataType.compare("INT32") == 0)) {
        return eventJson.isInt();
    } else if (dataType.compare("INT64") == 0) {
        return eventJson.isInt64();
    } else if ((dataType.compare("UINT8") == 0) || (dataType.compare("UINT16") == 0) ||
        (dataType.compare("UINT32") == 0)) {
        return eventJson.isUInt();
    } else if (dataType.compare("UINT64") == 0) {
        return eventJson.isUInt64();
    } else if ((dataType.compare("FLOAT") == 0) || (dataType.compare("DOUBLE") == 0)) {
        return eventJson.isDouble();
    } else if (dataType.compare("STRING") == 0) {
        return eventJson.isString();
    } else {
        return false;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
