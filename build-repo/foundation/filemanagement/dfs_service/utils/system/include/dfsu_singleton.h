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

#ifndef DFSU_SINGLETON_H
#define DFSU_SINGLETON_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include "nocopyable.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
#define DECLARE_SINGLETON(MyClass)                \
public:                                           \
    ~MyClass();                                   \
    MyClass(const MyClass&) = delete;             \
    MyClass& operator=(const MyClass&) = delete;  \
                                                  \
private:                                          \
    friend DfsuSingleton<MyClass>;                \
    MyClass();

template<typename T>
class DfsuSingleton : public NoCopyable {
public:
    static std::shared_ptr<T> GetInstance();

protected:
    /**
     * @note We depend on the IPC manager to serialize the start and the stop procedure
     */
    virtual void StartInstance() = 0;

    /**
     * @note Be very careful when freeing memory! Threads may call stop and other member functions simultaneously
     */
    virtual void StopInstance() = 0;
};

/**
 * @brief
 *
 * @tparam T
 * @return T&
 *
 * @note We use call_once to ensure the atomicity of new() and start()
 * @note Memory leaking of T is exactly what we want. Now T will be available along the program's life-time
 */
template<typename T>
std::shared_ptr<T> DfsuSingleton<T>::GetInstance()
{
    static std::shared_ptr<T> *dummy = nullptr;
    static std::once_flag once;
    std::call_once(once, []() mutable {
        dummy = new std::shared_ptr<T>(new T());
        (*dummy)->StartInstance();
    });
    return *dummy;
}
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFSU_SINGLETON_H