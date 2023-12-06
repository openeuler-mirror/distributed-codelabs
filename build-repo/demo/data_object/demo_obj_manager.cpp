#define LOG_TAG "[OBJ_DEMO]"

#include <iostream>
#include <vector>
#include <sstream>
#include <vector>

#include "distributed_object.h"
#include "distributed_objectstore.h"
#include "distributed_kv_data_manager.h"
#include "objectstore_errors.h"
#include "hilog/log.h"

using KvDeviceInfo = OHOS::DistributedKv::DeviceInfo;
using DistributedKvDataManager = OHOS::DistributedKv::DistributedKvDataManager;
using DeviceFilterStrategy = OHOS::DistributedKv::DeviceFilterStrategy;

constexpr int MAX_RETRY_TIMES = 100;
constexpr int INTERVAL = 200;

// hilog日志打印使用
static inline OHOS::HiviewDFX::HiLogLabel LogLabel() {
    return {LOG_CORE, 0xD009999, "OBJ_DEMO"};
}

#define ZLOGD(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Debug(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGI(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Info(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGW(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Warn(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define ZLOGE(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Error(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

using namespace OHOS::ObjectStore;


class ObjectWatcherImpl : public ObjectWatcher {
public:
    bool GetDataStatus() {
        return dataChanged_;
    }

    void OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData) override {
        if (changedData.empty()) {
            ZLOGI("empty change");
            return;
        }
        ZLOGI("Data Changed! sessionId(%s), changedData(%s)", sessionid.c_str(), changedData.at(0).c_str());
        dataChanged_ = true;
    }

    ~ObjectWatcherImpl() {}
private:
    bool dataChanged_ = false;
};


class StatusNotifierImpl : public StatusNotifier {
public:
    std::string GetOnlineStatus() {
        return onlineStatus_;
    }
    void OnChanged(const std::string &sessionId,
                   const std::string &networkId, const std::string &onlineStatus) override {
        ZLOGI("OnChanged, sessionId(%s), networkId(%s), status(%s).",
            sessionId.c_str(), networkId.c_str(), onlineStatus.c_str());
        onlineStatus_ = onlineStatus;
    }
    ~StatusNotifierImpl() {}
private:
    std::string onlineStatus_ = "offline";
};

const std::string BUNDLENAME = "com.example.objdemo";
const std::string SESSIONID = "123456";

class DataObjTest {
public:
    DataObjTest() {
        objectStore = DistributedObjectStore::GetInstance(BUNDLENAME);
        if (objectStore == nullptr) {
            ZLOGE("Failed to get object store!");
            object = nullptr;
            return;
        }

        object = objectStore->CreateObject(SESSIONID);
        if (object == nullptr) {
            ZLOGE("Failed to create object, sessionId[%s]", SESSIONID.c_str());
        }

        manager_.GetDeviceList(deviceInfos_, DeviceFilterStrategy::NO_FILTER);
    }

    ~DataObjTest() {
        if (objectStore == nullptr) {
            ZLOGE("object store is null.");
            return;
        }
        objectStore->DeleteObject(SESSIONID);
        object = nullptr;
        objectStore = nullptr;
    }

    template <typename T>
    void Put(const std::string& key, const T& value) {
        if (object == nullptr) {
            ZLOGE("object is null.");
            return;
        }
        uint32_t ret = 0;
        if constexpr (std::is_same_v<T, std::string>) {
            ret = object->PutString(key, value);
        } else if constexpr (std::is_same_v<T, double>) {
            ret = object->PutDouble(key, value);
        } else if constexpr (std::is_same_v<T, bool>) {
            ret = object->PutBoolean(key, value);
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            ret = object->PutComplex(key, value);
        } else {
            ZLOGE("Unsupported data type!");
            return;
        }
        if (ret != 0) {
            ZLOGE("Failed to put(%s).", key.c_str());
        }
    }

    template <typename T>
    bool Get(const std::string& key, T& value) {
        if (object == nullptr) {
            ZLOGE("object is null.");
            return false;
        }

        uint32_t ret = 0;
        if constexpr (std::is_same_v<T, std::string>) {
            ret = object->GetString(key, value);
        } else if constexpr (std::is_same_v<T, double>) {
            ret = object->GetDouble(key, value);
        } else if constexpr (std::is_same_v<T, bool>) {
            ret = object->GetBoolean(key, value);
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            ret = object->GetComplex(key, value);
        } else {
            ZLOGE("Unsupported data type!");
            return false;
        }

        if (ret != 0) {
            ZLOGE("Failed to get(%s).", key.c_str());
            return false;
        }
        return true;
    }

    OHOS::ObjectStore::Type GetType(const std::string &key) {
        OHOS::ObjectStore::Type type = OHOS::ObjectStore::Type::TYPE_STRING;
        if (object->GetType(key, type) != OHOS::ObjectStore::SUCCESS) {
            ZLOGE("Failed to get type of %s", key.c_str());
        }
        return type;
    }

    std::string &GetSessionId() {
        return object->GetSessionId();
    }

    void Save() {
        manager_.GetDeviceList(deviceInfos_, DeviceFilterStrategy::NO_FILTER);
        if (deviceInfos_.empty()) {
            ZLOGE("no other online device.");
            return;
        }
        uint32_t ret = object->Save(deviceInfos_[0].deviceId);
        if (ret != OHOS::ObjectStore::SUCCESS) {
            ZLOGE("Failed to Save to device: %s.", deviceInfos_[0].deviceId.c_str());
        }
    }

    void RevokeSave() {
        uint32_t ret = object->RevokeSave();
        if (ret != OHOS::ObjectStore::SUCCESS) {
            ZLOGE("Failed to RevokeSave to device.");
        }
    }

    void Watch() {
        auto watcherPtr = std::make_shared<ObjectWatcherImpl>();
        uint32_t ret = objectStore->Watch(object, watcherPtr);
        if (ret != OHOS::ObjectStore::SUCCESS) {
            ZLOGE("Failed to watch.");
        }
    }

    void UnWatch() {
        uint32_t ret = objectStore->UnWatch(object);
        if (ret != OHOS::ObjectStore::SUCCESS) {
            ZLOGE("Failed to unwatch");
        }
    }

    void SetStatusNotifier() {
        auto notifierPtr = std::make_shared<StatusNotifierImpl>();
        uint32_t ret = objectStore->SetStatusNotifier(notifierPtr);
        if (ret != OHOS::ObjectStore::SUCCESS) {
            ZLOGE("Failed to set status notifier.");
        }
    }

    void NotifyCachedStatus() {
        std::string sessionId = GetSessionId();
        objectStore->NotifyCachedStatus(sessionId);
    }
private:
    DistributedObjectStore *objectStore;
    DistributedObject *object;
    std::vector <KvDeviceInfo> deviceInfos_;
    DistributedKvDataManager manager_;
};

// 单纯进行Put相关测试
static void PutTest(DataObjTest *obj) {
    obj->Put("name", std::string("zhangsan"));
    obj->Put("salary", 100.5);
    obj->Put("isTrue", true);
    std::string str = "hello world";
    std::vector<uint8_t> vec(str.begin(), str.end());
    obj->Put("list", vec);

    obj->Save();
    obj->RevokeSave();
    // getchar();
}

// 单纯进行Get相关测试
static void GetTest(DataObjTest *obj) {
    // DataObjTest obj;
    bool isTrue;
    if (obj->Get("isTrue", isTrue)) {
        ZLOGI("Get isTrue is %d.", isTrue);
    }

    std::string name;
    if (obj->Get("name", name)) {
        ZLOGI("Get name is %s.", name.c_str());
    }

    double salary;
    if (obj->Get("salary", salary)) {
        ZLOGI("Get salary is %f.", salary);
    }

    std::vector<uint8_t> vec;
    std::string str = "";
    if (obj->Get("list", vec)) {
        for (char ch : vec) {
            str.push_back(ch);
        }
        ZLOGI("Get list is %s.", str.c_str());
    }

    OHOS::ObjectStore::Type type = obj->GetType("salary");
    ZLOGI("Get type is %d.", type);
    // getchar();
}

static void Run() {
    DataObjTest obj;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        std::string key;
        iss >> cmd;
        iss >> key;
        if (cmd == "PutDouble") {
            double value = 0;
            iss >> value;
            obj.Put(key, value);
        } else if (cmd == "PutStr") {
            std::string value = "";
            std::getline(iss, value, '"');
            std::getline(iss, value, '"');
            obj.Put(key, value);
        } else if (cmd == "PutBool") {
            bool value = false;
            iss >> value;
            obj.Put(key, value);
        } else if (cmd == "GetDouble") {
            double value = 0;
            if (obj.Get(key, value)) {
                ZLOGI("Succeed to get value(%f) of %s.", value, key.c_str());
            }
        } else if (cmd == "GetStr") {
            std::string value = "";
            if (obj.Get(key, value)) {
                ZLOGI("Succeed to get value(%s) of %s.", value.c_str(), key.c_str());
            }
        } else if (cmd == "GetBool") {
            bool value = false;
            if (obj.Get(key, value)) {
                ZLOGI("Succeed to get value(%d) of %s.", value, key.c_str());
            }
        } else if (cmd == "GetSeesionId") {
            ZLOGI("Succeed to get session id(%s).", obj.GetSessionId().c_str());
        } else if (cmd == "Watch") {
            obj.Watch();
        } else if (cmd == "UnWatch") {
            obj.UnWatch();
        } else if (cmd == "Save") {
            obj.Save();
        } else if (cmd == "RevokeSave") {
            obj.RevokeSave();
        } else if (cmd == "SetStatusNotifier") {
            obj.SetStatusNotifier();
        } else if (cmd == "NotifyCachedStatus") {
            obj.NotifyCachedStatus();
        } else if (cmd == "exit" || cmd == "quit") {
            break;
        } else {
            ZLOGE("Unknown command: %s.", cmd.c_str());
        }
    }
}

// static void TestObjStore() {
//     ZLOGI("*****start****** \n");
//     std::string bundleName = "default";
//     std::string sessionId = "123456";
//     DistributedObjectStore *objectStore = DistributedObjectStore::GetInstance(bundleName);
//     if (objectStore == nullptr)
//         ZLOGI("*****objectStore== nullptr \n");

//     uint32_t ret;
//     DistributedObject *object = objectStore->CreateObject(sessionId);
//     if (object == nullptr)
//         ZLOGI("*****object== nullptr \n");
//     auto watcherPtr = std::shared_ptr<ObjectWatcher>();
//     ret = objectStore->Watch(object, watcherPtr);
//     ZLOGI("*****objectStore->Watch ret value = %d ", ret);
//     ret = objectStore->UnWatch(object);
//     ZLOGI("*****objectStore->UnWatch ret value = %d ", ret);

//     ret = object->PutString("name", "zhangsan");
//     ZLOGI("*****PutString ret value = %d ", ret);
//     ret = object->PutDouble("salary", 100.5);
//     ZLOGI("*****PutDouble ret value = %d ", ret);
//     ret = object->PutBoolean("isTrue", true);
//     ZLOGI("*****PutBoolean ret value = %d ", ret);

//     bool isTrue;
//     ret = object->GetBoolean("isTrue", isTrue);
//     ZLOGI("*****isTrue value = %d ", isTrue);

//     std::string name;
//     ret = object->GetString("name", name);
//     ZLOGI("*****name value = %s ", name.c_str());

//     double salary;
//     ret = object->GetDouble("salary", salary);
//     // ZLOGI("*****salary value = %lf ", salary);
//     std::cout << "Salary is " << salary << std::endl;

//     ret = object->Save("local");
//     ZLOGI("*****Save ret value = %d ", ret);
//     ret = object->RevokeSave();
//     ZLOGI("*****RevokeSave ret value = %d ", ret);

//     std::string getSessionId = object->GetSessionId();
//     ZLOGI("*****getSessionId ret value = %s ", getSessionId.c_str());

//     ret = objectStore->DeleteObject(sessionId);
//     ZLOGI("*****objectStore->DeleteObject ret value = %d ", ret);
//     getchar();
// }

int main(int argc, char const *argv[]) {
    Run();
    return 0;
}
