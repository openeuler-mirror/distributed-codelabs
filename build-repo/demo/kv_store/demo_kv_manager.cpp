#define LOG_TAG "[KV_DEMO]"

#include "log_print.h"
#include "distributed_kv_data_manager.h"
#include "directory_ex.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>

const int FILE_PERMISSION = 0777;

using namespace OHOS::DistributedKv;

class KvStoreTest {
public:
    KvStoreTest() {
        OHOS::DistributedKv::DistributedKvDataManager kvDataManager;
        OHOS::DistributedKv::AppId appId = {"app_demo2"};
        OHOS::DistributedKv::StoreId storedId = {"store_demo2"};
        OHOS::DistributedKv::Options options;

        options.autoSync = true;
        options.kvStoreType = SINGLE_VERSION;
        options.encrypt = false;
        options.createIfMissing = true;
        options.baseDir = std::string("/data/app/el1/0/database/app_demo2");
        options.area = EL1;
        mkdir(options.baseDir.c_str(), (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH));
        OHOS::ChangeModeDirectory(options.baseDir, FILE_PERMISSION);

        Status result = kvDataManager.GetSingleKvStore(options, appId, storedId, kvStore);
        ZLOGI("Get kvStore instance result is: %d", result);
    }

    void Put(std::string key, std::string value) {
        Status result = kvStore->Put(key, value);
        ZLOGI("Put result is %d", result);
    }

    std::string Get(std::string key) {
        OHOS::DistributedKv::Value v;
        Status result = kvStore->Get(key, v);
        ZLOGI("Get result is %d", result);
        return v.ToString();
    }

private:
    std::shared_ptr<OHOS::DistributedKv::SingleKvStore> kvStore;
};

void Run() {
    KvStoreTest kvs;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "get") {
            std::string key;
            std::getline(iss, key, '"');
            std::getline(iss, key, '"');
            std::string value = kvs.Get(key);
            std::cout << "Get value is: " << value << std::endl;
        } else if (cmd == "put") {
            std::string key, value;
            std::getline(iss, key, '"');
            std::getline(iss, key, '"');
            std::getline(iss, value, '"');
            std::getline(iss, value, '"');
            kvs.Put(key, value);
            std::cout << "Put value success" << std::endl;
        } else if (cmd == "exit" || cmd == "quit") {
            break;
        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
        }
    }
}

int main(int argc, char const *argv[])
{
    // 使用提示
    // std::cout << "Welcome to Distributed KV Store Demo!" << std::endl;
    // std::cout << "Usage:" << std::endl;
    // std::cout << "- To get a value: get \"key\"" << std::endl;
    // std::cout << "- To put a value: put \"key\" \"value\"" << std::endl;
    // std::cout << "- To exit the program: exit or quit" << std::endl;

    Run();
    return 0;
}