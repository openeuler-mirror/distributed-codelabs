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

/*
这段代码定义了一个名为 RefObject 的类，它是一个基类，提供了一些线程安全的对象引用计数和锁定机制，以及对象销毁时的回调函数。

类中定义了一个名为 AutoLock 的嵌套类，它是一个 RAII 风格的锁定类，用于在对象上进行锁定和解锁操作。类中还定义了一些常用的函数，如 OnLastRef、OnKill、IsKilled、KillObj、LockObj、UnlockObj 和 WaitLockedUntil。其中，OnLastRef 用于在对象销毁时执行回调函数，OnKill 用于在对象销毁时执行回调函数，并在执行回调函数前对对象进行锁定，IsKilled 用于判断对象是否已经被销毁，KillObj 用于标记对象被销毁，LockObj 和 UnlockObj 用于锁定和解锁对象，WaitLockedUntil 用于等待对象锁定，并在指定条件满足时返回。

类中还定义了一些静态的函数，如 IncObjRef、DecObjRef 和 KillAndDecObjRef，用于增加和减少对象的引用计数，以及销毁对象并减少引用计数。

类中还声明了一些虚函数，如 ~RefObject 和 GetObjectTag，分别用于销毁对象和获取对象的标记。

类中使用了 C++11 的一些新特性，如原子变量和 lambda 表达式，以及自定义的一些宏，如 DISABLE_COPY_ASSIGN_MOVE，用于禁止对象的复制、赋值和移动操作。

总体来说，这段代码定义了一个通用的对象引用计数和锁定机制的基类，可以用于实现多线程环境下的对象管理。
*/

#ifndef KV_DB_REF_OBJECT_H
#define KV_DB_REF_OBJECT_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

#include "macro_utils.h"

namespace DistributedDB {
class RefObject {
public:
    class AutoLock final {
    public:
        AutoLock(const RefObject *obj, bool unlocked = true);
        ~AutoLock();
        void Lock();
        void Unlock();

    private:
        DISABLE_COPY_ASSIGN_MOVE(AutoLock);
        const RefObject *refObj_;
        bool isLocked_;
    };

    RefObject();

    /* Invoked before this object deleted. */
    void OnLastRef(const std::function<void(void)> &callback) const;

    /* Invoked when kill object, with lock held. */
    void OnKill(const std::function<void(void)> &callback);

    bool IsKilled() const;
    void KillObj();
    void LockObj() const;
    void UnlockObj() const;
    bool WaitLockedUntil(std::condition_variable &cv,
        const std::function<bool(void)> &condition, int seconds = 0);

    /* Work as static members, avoid to 'delete this' */
    static void IncObjRef(const RefObject *obj);
    static void DecObjRef(const RefObject *obj);
    static void KillAndDecObjRef(RefObject *obj);

protected:
    virtual ~RefObject();
    virtual std::string GetObjectTag() const;

private:
    constexpr static const char * const classTag = "Class-RefObject";

    DISABLE_COPY_ASSIGN_MOVE(RefObject);

    /* A const object can also be locked/unlocked/ref()/unref() */
    mutable std::atomic<int> refCount_;
    mutable std::mutex objLock_;
    std::atomic<bool> isKilled_;
    mutable std::function<void(void)> onLast_;
    std::function<void(void)> onKill_;
};
} // namespace DistributedDB

#endif // KV_DB_REF_OBJECT_H
