/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#ifndef HI_SYS_EVENT_H
#define HI_SYS_EVENT_H
#include "hisysevent_c.h"

#ifdef __cplusplus

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "def.h"
#include "write_controller.h"

/*
 * Usage: define string macro "DOMAIN_MASKS" to disable one or more components.
 *     Method1: add macro in this header file, e.g.          #define DOMAIN_MASKS "AAFWK|APPEXECFWK|ACCOUNT"
 *     Method1: addd cflags in build.gn file, e.g.           -D DOMAIN_MASKS="AAFWK|APPEXECFWK|ACCOUNT"
 */
namespace OHOS {
namespace HiviewDFX {
// init domain masks
#ifndef DOMAIN_MASKS
#define DOMAIN_MASKS ""
#endif

static constexpr char DOMAIN_MASKS_DEF[] = DOMAIN_MASKS;

// split domain masks by '|', then compare with str
template<const std::string_view& str, const std::string_view& masks, std::string::size_type pos = 0>
struct IsMaskedImpl {
    inline static constexpr auto newpos = masks.find('|', pos);
    inline static constexpr bool value =
        IsMaskedImpl<str, masks, (newpos != std::string_view::npos) ? pos + 1 : newpos>::value ||
        (pos != newpos && pos != masks.size() &&
            masks.substr(pos, (newpos != std::string_view::npos) ? newpos - pos : str.size() - pos).compare(str) == 0);
};

template<const std::string_view& str, const std::string_view& masks>
struct IsMaskedImpl<str, masks, std::string_view::npos> {
    inline static constexpr bool value = false;
};

template<const char* domain, const char* domainMasks>
struct IsMaskedCvt {
    inline static constexpr std::string_view domainView {domain};
    inline static constexpr std::string_view domainMasksView {domainMasks};
    inline static constexpr bool value = IsMaskedImpl<domainView, domainMasksView>::value;
};

template<const char* domain>
inline static constexpr bool isMasked = IsMaskedCvt<domain, DOMAIN_MASKS_DEF>::value;

class HiSysEvent {
public:
    friend class NapiHiSysEventAdapter;
    // system event domain list
    class Domain {
    public:
        static constexpr char AAFWK[] = "AAFWK";
        static constexpr char ACCESS_TOKEN[] = "ACCESS_TOKEN";
        static constexpr char ACCESSIBILITY[] = "ACCESSIBILITY";
        static constexpr char ACCOUNT[] = "ACCOUNT";
        static constexpr char ACE[] = "ACE";
        static constexpr char AI[] = "AI";
        static constexpr char APPEXECFWK[] = "APPEXECFWK";
        static constexpr char AV_SESSION[] = "AV_SESSION";
        static constexpr char BARRIER_FREE[] = "BARRIERFREE";
        static constexpr char BIOMETRICS[] = "BIOMETRICS";
        static constexpr char BLUETOOTH[] = "BLUETOOTH";
        static constexpr char BUNDLE_MANAGER[] = "BUNDLE_MANAGER";
        static constexpr char CAST_ENGINE[] = "CASTENGINE";
        static constexpr char CCRUNTIME[] = "CCRUNTIME";
        static constexpr char CERT_MANAGER[] = "CERT_MANAGER";
        static constexpr char COMMONEVENT[] = "COMMONEVENT";
        static constexpr char COMMUNICATION[] = "COMMUNICATION";
        static constexpr char CUSTOMIZATION_CONFIG[] = "CUST_CONFIG";
        static constexpr char CUSTOMIZATION_EDM[] = "CUST_EDM";
        static constexpr char DEVELOPTOOLS[] = "DEVELOPTOOLS";
        static constexpr char DEVICE_AUTH[] = "DEVICE_AUTH";
        static constexpr char DEVICE_PROFILE[] = "DEVICE_PROFILE";
        static constexpr char DISTRIBUTED_AUDIO[] = "DISTAUDIO";
        static constexpr char DISTRIBUTED_CAMERA[] = "DISTCAMERA";
        static constexpr char DISTRIBUTED_DATAMGR[] = "DISTDATAMGR";
        static constexpr char DISTRIBUTED_DEVICE_MANAGER[] = "DISTDM";
        static constexpr char DISTRIBUTED_HARDWARE_FWK[] = "DISTHWFWK";
        static constexpr char DISTRIBUTED_INPUT[] = "DISTINPUT";
        static constexpr char DISTRIBUTED_SCHEDULE[] = "DISTSCHEDULE";
        static constexpr char DISTRIBUTED_SCREEN[] = "DISTSCREEN";
        static constexpr char DLP[] = "DLP";
        static constexpr char DLP_CRE_SERVICE[] = "DLP_CRE_SERVICE";
        static constexpr char DSLM[] = "DSLM";
        static constexpr char DSOFTBUS[] = "DSOFTBUS";
        static constexpr char GLOBAL[] = "GLOBAL";
        static constexpr char GLOBAL_I18N[] = "GLOBAL_I18N";
        static constexpr char GLOBAL_RESMGR[] = "GLOBAL_RESMGR";
        static constexpr char GRAPHIC[] = "GRAPHIC";
        static constexpr char HIVIEWDFX[] = "HIVIEWDFX";
        static constexpr char HUKS[] = "HUKS";
        static constexpr char IAWARE[] = "IAWARE";
        static constexpr char INPUTMETHOD[] = "INPUTMETHOD";
        static constexpr char INTELLI_ACCESSORIES[] = "INTELLIACC";
        static constexpr char INTELLI_TV[] = "INTELLITV";
        static constexpr char ISHARE[] = "ISHARE";
        static constexpr char IVI_HARDWARE[] = "IVIHARDWARE";
        static constexpr char LOCATION[] = "LOCATION";
        static constexpr char MEDICAL_SENSOR[] = "MEDICAL_SENSOR";
        static constexpr char MISCDEVICE[] = "MISCDEVICE";
        static constexpr char MSDP[] = "MSDP";
        static constexpr char MULTI_MEDIA[] = "MULTIMEDIA";
        static constexpr char MULTI_MODAL_INPUT[] = "MULTIMODALINPUT";
        static constexpr char NETMANAGER_STANDARD[] = "NETMANAGER";
        static constexpr char NOTIFICATION[] = "NOTIFICATION";
        static constexpr char PASTEBOARD[] = "PASTEBOARD";
        static constexpr char POWERMGR[] = "POWERMGR";
        static constexpr char REQUEST[] = "REQUEST";
        static constexpr char ROUTER[] = "ROUTER";
        static constexpr char SAMGR[] = "SAMGR";
        static constexpr char SECURITY_GUARD[] = "SECURITY_GUARD";
        static constexpr char SENSOR[] = "SENSOR";
        static constexpr char SOURCE_CODE_TRANSFORMER[] = "SRCTRANSFORMER";
        static constexpr char STARTUP[] = "STARTUP";
        static constexpr char TELEPHONY[] = "TELEPHONY";
        static constexpr char THEME[] = "THEME";
        static constexpr char TIME[] = "TIME";
        static constexpr char UPDATE[] = "UPDATE";
        static constexpr char USB[] = "USB";
        static constexpr char USERIAM_PIN[] = "USERIAM_PIN";
        static constexpr char USERIAM_FWK[] = "USERIAM_FWK";
        static constexpr char WEARABLE[] = "WEARABLE";
        static constexpr char WEARABLE_HARDWARE[] = "WEARABLEHW";
        static constexpr char WEBVIEW[] = "WEBVIEW";
        static constexpr char WINDOW_MANAGER[] = "WINDOWMANAGER";
        static constexpr char OTHERS[] = "OTHERS";
    };

    enum EventType {
        FAULT     = 1,    // system fault event
        STATISTIC = 2,    // system statistic event
        SECURITY  = 3,    // system security event
        BEHAVIOR  = 4     // system behavior event
    };

public:
    /**
     * @deprecated
     * @brief write system event
     * @param domain    system event domain name
     * @param eventName system event name
     * @param type      system event type
     * @param keyValues system event parameter name or value
     * @return 0 success, other fail
     */
    template<typename... Types>
    static int Write(const std::string &domain, const std::string &eventName,
        EventType type, Types... keyValues)
    {
        return InnerWrite(domain, eventName, type, keyValues...);
    }

    template<const char* domain, typename... Types, std::enable_if_t<!isMasked<domain>>* = nullptr>
    static int Write(const char* func, int64_t line, const std::string& eventName,
        EventType type, Types... keyValues)
    {
        ControlParam param {
#ifdef HISYSEVENT_PERIOD
            .period = HISYSEVENT_PERIOD,
#else
            .period = HISYSEVENT_DEFAULT_PERIOD,
#endif
#ifdef HISYSEVENT_THRESHOLD
            .threshold = HISYSEVENT_THRESHOLD,
#else
            .threshold = HISYSEVENT_DEFAULT_THRESHOLD,
#endif
        };
        if (controller.CheckLimitWritingEvent(param, domain, eventName.c_str(), func, line)) {
            return ERR_WRITE_IN_HIGH_FREQ;
        }
        return InnerWrite(std::string(domain), eventName, type, keyValues...);
    }

    template<const char* domain, typename... Types, std::enable_if_t<isMasked<domain>>* = nullptr>
    inline static constexpr int Write(const char*, int64_t, const std::string&, EventType, Types...)
    {
        // do nothing
        return ERR_DOMAIN_MASKED;
    }

private:
    class EventBase {
    public:
        EventBase(const std::string &domain, const std::string &eventName, int type)
            : retCode_(0), keyCnt_(0), domain_(domain), eventName_(eventName), type_(type)
            {};
        ~EventBase() {}
    public:
        int retCode_;
        unsigned int keyCnt_;
        std::stringstream jsonStr_;
        const std::string domain_;
        const std::string eventName_;
        const int type_;
    };

private:
    template<typename... Types>
    static int InnerWrite(const std::string &domain, const std::string &eventName,
        EventType type, Types... keyValues)
    {
        EventBase eventBase(domain, eventName, type);
        eventBase.jsonStr_ << "{";
        WritebaseInfo(eventBase);
        if (IsError(eventBase)) {
            ExplainRetCode(eventBase);
            return eventBase.retCode_;
        }

        InnerWrite(eventBase, keyValues...);
        if (IsError(eventBase)) {
            ExplainRetCode(eventBase);
            return eventBase.retCode_;
        }
        eventBase.jsonStr_ << "}";

        SendSysEvent(eventBase);
        return eventBase.retCode_;
    }

    template<typename T>
    static void AppendData(EventBase &eventBase, const std::string &key, T value)
    {
        if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
            return;
        }
        if (UpdateAndCheckKeyNumIsOver(eventBase)) {
            return;
        }
        eventBase.jsonStr_ << "\"" << key << "\":";
        AppendValue(eventBase, value);
        eventBase.jsonStr_ << ",";
    }

    template<typename T>
    static void AppendArrayData(EventBase &eventBase, const std::string &key, const std::vector<T> &value)
    {
        if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
            return;
        }
        if (UpdateAndCheckKeyNumIsOver(eventBase)) {
            return;
        }
        if (value.empty()) {
            eventBase.jsonStr_ << "\"" << key << "\":[],";
            return;
        }
        IsWarnAndUpdate(CheckArraySize(value.size()), eventBase);

        unsigned int index = 0;
        unsigned int arrayMax = GetArrayMax();
        eventBase.jsonStr_ << "\"" << key << "\":[";
        for (auto item = value.begin(); item != value.end(); item++) {
            index++;
            if (index > arrayMax) {
                break;
            }
            AppendValue(eventBase, *item);
            eventBase.jsonStr_ << ",";
        }
        if (eventBase.jsonStr_.tellp() != 0) {
            eventBase.jsonStr_.seekp(-1, std::ios_base::end);
        }
        eventBase.jsonStr_ << "],";
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, bool value, Types... keyValues)
    {
        AppendData<bool>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const char value, Types... keyValues)
    {
        AppendData<short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const unsigned char value, Types... keyValues)
    {
        AppendData<unsigned short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const short value, Types... keyValues)
    {
        AppendData<short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const unsigned short value, Types... keyValues)
    {
        AppendData<unsigned short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const int value, Types... keyValues)
    {
        AppendData<int>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const unsigned int value, Types... keyValues)
    {
        AppendData<unsigned int>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const long value, Types... keyValues)
    {
        AppendData<long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const unsigned long value, Types... keyValues)
    {
        AppendData<unsigned long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const long long value, Types... keyValues)
    {
        AppendData<long long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const unsigned long long value, Types... keyValues)
    {
        AppendData<unsigned long long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const float value, Types... keyValues)
    {
        AppendData<float>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const double value, Types... keyValues)
    {
        AppendData<double>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const std::string &value, Types... keyValues)
    {
        AppendData(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase, const std::string &key, const char *value, Types... keyValues)
    {
        AppendData(eventBase, key, std::string(value));
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<bool> &value, Types... keyValues)
    {
        AppendArrayData<bool>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<char> &value, Types... keyValues)
    {
        AppendArrayData(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned char> &value, Types... keyValues)
    {
        AppendArrayData(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<short> &value, Types... keyValues)
    {
        AppendArrayData<short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned short> &value, Types... keyValues)
    {
        AppendArrayData<unsigned short>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<int> &value, Types... keyValues)
    {
        AppendArrayData<int>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned int> &value, Types... keyValues)
    {
        AppendArrayData<unsigned int>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<long> &value, Types... keyValues)
    {
        AppendArrayData<long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned long> &value, Types... keyValues)
    {
        AppendArrayData<unsigned long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<long long> &value, Types... keyValues)
    {
        AppendArrayData<long long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<unsigned long long> &value, Types... keyValues)
    {
        AppendArrayData<unsigned long long>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<float> &value, Types... keyValues)
    {
        AppendArrayData<float>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<double> &value, Types... keyValues)
    {
        AppendArrayData<double>(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase &eventBase,
        const std::string &key, const std::vector<std::string> &value, Types... keyValues)
    {
        AppendArrayData(eventBase, key, value);
        InnerWrite(eventBase, keyValues...);
    }

    template<typename T>
    static void AppendValue(EventBase &eventBase, const T item)
    {
        eventBase.jsonStr_ << item;
    }

    static void AppendValue(EventBase &eventBase, const std::string &item);
    static void AppendValue(EventBase &eventBase, const char item);
    static void AppendValue(EventBase &eventBase, const signed char item);
    static void AppendValue(EventBase &eventBase, const unsigned char item);
    static void AppendHexData(EventBase &eventBase, const std::string &key, uint64_t value);
    static void InnerWrite(EventBase &eventBase);
    static void InnerWrite(EventBase &eventBase, HiSysEventParam params[], size_t size);
    static void WritebaseInfo(EventBase &eventBase);

    static int CheckKey(const std::string &key);
    static int CheckValue(const std::string &value);
    static int CheckArraySize(unsigned long size);
    static bool IsErrorAndUpdate(int retCode, EventBase &eventBase);
    static bool IsWarnAndUpdate(int retCode, EventBase &eventBase);
    static bool UpdateAndCheckKeyNumIsOver(EventBase &eventBase);
    static bool IsError(EventBase &eventBase);
    static void ExplainRetCode(EventBase &eventBase);

    static unsigned int GetArrayMax();
    static void SendSysEvent(EventBase &eventBase);

    static void AppendInvalidParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendBoolParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendInt8Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendUint8Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendInt16Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendUint16Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendInt32Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendUint32Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendInt64Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendUint64Param(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendFloatParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendDoubleParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendStringParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);

    template<typename T>
    static void AppendArrayParam(HiSysEvent::EventBase &eventBase, const std::string &key,
        const T *array, size_t arraySize);
    static void AppendBoolArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendInt8ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendUint8ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendInt16ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendUint16ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendInt32ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendUint32ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendInt64ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendUint64ArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendFloatArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendDoubleArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendStringArrayParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);
    static void AppendParam(HiSysEvent::EventBase &eventBase, const HiSysEventParam &param);

    static WriteController controller;
};

// macro interface
#define HiSysEventWrite(domain, eventName, type, ...) \
({ \
    int ret = OHOS::HiviewDFX::ERR_DOMAIN_MASKED; \
    if constexpr (!OHOS::HiviewDFX::isMasked<domain>) { \
        ret = OHOS::HiviewDFX::HiSysEvent::Write<domain>(__FUNCTION__, __LINE__, \
            eventName, type, ##__VA_ARGS__); \
    } \
    ret; \
})
} // namespace HiviewDFX
} // namespace OHOS

#endif // __cplusplus
#endif // HI_SYS_EVENT_H
