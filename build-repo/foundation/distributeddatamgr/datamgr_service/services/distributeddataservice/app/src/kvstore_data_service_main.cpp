#include <iostream>
#include "distributed_kv_data_manager.h"

OHOS::DistributedKv::DistributedKvDataManager kvDataManager;
OHOS::DistributedKv::Options options;
OHOS::DistributedKv::AppId appId = {"app_demo"};
OHOS::DistributedKv::StoreId storedId = {"store_demo"};
std::shared_ptr<OHOS::DistributedKv::SingleKvStore> kvStore;

static void GetSingleKvStore()
{
    OHOS::DistributedKv::Status result = kvDataManager.GetSingleKvStore(options, appId, storedId, kvStore);
    std::cout << "result is " << result << std::endl;
}

int main(int argc, char const *argv[])
{
    options.baseDir = "/tmp";
    GetSingleKvStore();
    return 0;
}