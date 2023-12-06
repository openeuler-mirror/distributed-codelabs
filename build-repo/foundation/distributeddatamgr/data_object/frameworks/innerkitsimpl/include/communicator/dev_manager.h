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
#ifndef DATA_OBJECT_DEV_MANAGER_H
#define DATA_OBJECT_DEV_MANAGER_H
#include <cstdint>
namespace OHOS {
namespace ObjectStore {
class DevManager {
public:
    static DevManager *GetInstance()
    {
        static DevManager *instance = new DevManager();
        return instance;
    }
    void RegisterDevCallback();

private:
    DevManager();
    ~DevManager();
    int32_t Init();
};
} // namespace ObjectStore
} // namespace OHOS
#endif // DATA_OBJECT_DEV_MANAGER_H