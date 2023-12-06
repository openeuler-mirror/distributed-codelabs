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

#ifndef DISTRIBUTED_KV_DATA_MANAGER_H
#define DISTRIBUTED_KV_DATA_MANAGER_H

#include <functional>
#include "kvstore.h"
#include "kvstore_death_recipient.h"
#include "kvstore_observer.h"
#include "single_kvstore.h"
#include "types.h"
#include "device_status_change_listener.h"

namespace OHOS {
namespace DistributedKv {
class API_EXPORT DistributedKvDataManager final {
public:
    API_EXPORT DistributedKvDataManager();

    API_EXPORT ~DistributedKvDataManager();

    // Open kvstore instance with the given storeId, creating it if needed.
    // It is allowed to open the same kvstore concurrently
    // multiple times, but only one KvStoreImpl will be created.
    // Parameters:
    // options: the config of the kvstore, including encrypt,
    // create if needed and whether need sync between devices.
    // appId: the name of the application.
    // :storeId: the name of the kvstore.
    // callback: including status and KvStore instance returned by this call.
    // callback will return:
    // if Options.createIfMissing is false and kvstore has not been created before,
    // STORE_NOT_FOUND and nullptr,
    // if storeId is not valid, INVALID_ARGUMENT and nullptr,
    // if appId has no permission, PERMISSION_DENIED and nullptr,
    // otherwise, SUCCESS and the unipue_ptr of kvstore, which client can use to operate kvstore, will be returned.
    API_EXPORT Status GetSingleKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                                       std::shared_ptr<SingleKvStore> &singleKvStore);

    // get all existed kvstore names.
    API_EXPORT Status GetAllKvStoreId(const AppId &appId, std::vector<StoreId> &storeIds);

    // WARNING: try to close a KvStore while other thread(s) still using it may cause process crash.
    // Disconnect kvstore instance from kvstoreimpl with the given storeId,
    // if all kvstore created for a single kvsotreimpl, kvstoreimpl and resource below will be freed.
    // before this call, all KvStoreSnapshot must be released firstly,
    // otherwise this call will fail.
    // after this call, kvstore and kvstoresnapshot become invalid.
    // call to it will return nullptr exception.
    // Parameters:
    // appId: the name of the application.
    // storeId: the name of the kvstore.
    API_EXPORT Status CloseKvStore(const AppId &appId, const StoreId &storeId);

    // WARNING: try to close a KvStore while other thread(s) still using it may cause process crash.
    //
    // Disconnect kvstore instance from kvstoreimpl.
    // if all kvstore created for a single kvsotreimpl, kvstoreimpl and resource below will be freed.
    // before this call, all KvStoreResultSet must be released firstly,
    // otherwise this call will fail.
    // after this call, kvstore and KvStoreResultSet become invalid.
    // call to it will return nullptr exception.
    // Parameters:
    // appId: the name of the application.
    // kvStorePtr: the pointer of the kvstore.
    API_EXPORT Status CloseKvStore(const AppId &appId, std::shared_ptr<SingleKvStore> &kvStore);

    // WARNING: try to close a KvStore while other thread(s) still using it may cause process crash.
    // close all opened kvstores for this appId.
    API_EXPORT Status CloseAllKvStore(const AppId &appId);

    // delete kvstore file with the given storeId.
    // client should first close all connections to it and then delete it,
    // otherwise delete may return error.
    // after this call, kvstore and kvstoresnapshot become invalid.
    // call to it will return error.
    // Parameters:
    // appId: the name of the application.
    // storeId: the name of the kvstore.
    API_EXPORT Status DeleteKvStore(const AppId &appId, const StoreId &storeId, const std::string &path = "");

    // delete all kvstore.
    API_EXPORT Status DeleteAllKvStore(const AppId &appId, const std::string &path = "");

    API_EXPORT void RegisterKvStoreServiceDeathRecipient(std::shared_ptr<KvStoreDeathRecipient> deathRecipient);

    API_EXPORT void UnRegisterKvStoreServiceDeathRecipient(std::shared_ptr<KvStoreDeathRecipient> deathRecipient);

    // Subscribe device status change, like online or offline.
    // Client should override AppDeviceChangeListener and register it by this function, observer->OnDeviceChanged
    // will be called on remote device status change.
    // Parameters:
    //     observer: callback for device status change event.
    // Return:
    //     Status of this subscribe operation.
    API_EXPORT Status StartWatchDeviceChange(std::shared_ptr<DeviceStatusChangeListener> observer);

    // Unsubscribe device status change, like online or offline.
    // client should override AppDeviceChangeListener and register it by calling this function, then
    // observer->OnDeviceChanged will no longer be called on remote device status change.
    // Parameters:
    //     observer: callback for device status change event.
    // Return:
    //     Status of this unsubscribe operation.
    API_EXPORT Status StopWatchDeviceChange(std::shared_ptr<DeviceStatusChangeListener> observer);

    // Get all connected devices.
    // Client can use this method to retrieve all devices that have already connected,
    // and then call StartWatchDeviceChange to watch device status change later.
    // Parameters:
    //     deviceInfoList: list of all connected device will be returned by this parameter.
    // Return:
    //     Status of this get device list operation.
    API_EXPORT Status GetDeviceList(std::vector<DeviceInfo> &deviceInfoList, DeviceFilterStrategy strategy);

    // Get device.
    // Client can use this method to retrieve local device,
    // Parameters:
    //     localDevice: DeviceInfo will be returned by this parameter.
    // Return:
    //     Status of this get device operation.
    API_EXPORT Status GetLocalDevice(DeviceInfo &localDevice);
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // DISTRIBUTED_KV_DATA_MANAGER_H
