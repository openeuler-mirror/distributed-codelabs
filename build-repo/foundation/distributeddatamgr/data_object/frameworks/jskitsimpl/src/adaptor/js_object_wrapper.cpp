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

#include "js_object_wrapper.h"

#include "logger.h"
namespace OHOS::ObjectStore {
JSObjectWrapper::JSObjectWrapper(DistributedObjectStore *objectStore, DistributedObject *object)
    : objectStore_(objectStore), object_(object)
{
}

JSObjectWrapper::~JSObjectWrapper()
{
    LOG_INFO("JSObjectWrapper::~JSObjectWrapper");
    std::unique_lock<std::shared_mutex> cacheLock(watchMutex_);
    if (watcher_ != nullptr) {
        watcher_ = nullptr;
    }
    LOG_INFO("JSObjectWrapper::~JSObjectWrapper end");
}

DistributedObject *JSObjectWrapper::GetObject()
{
    return object_;
}

bool JSObjectWrapper::AddWatch(napi_env env, const char *type, napi_value handler)
{
    std::unique_lock<std::shared_mutex> cacheLock(watchMutex_);
    if (watcher_ == nullptr) {
        watcher_ = std::make_unique<JSWatcher>(env, objectStore_, object_);
        if (watcher_ == nullptr) {
            LOG_ERROR("JSObjectWrapper::new JSWatcher fail");
            return false;
        }
    }
    return watcher_->On(type, handler);
}

void JSObjectWrapper::DeleteWatch(napi_env env, const char *type, napi_value handler)
{
    std::unique_lock<std::shared_mutex> cacheLock(watchMutex_);
    if (watcher_ != nullptr) {
        watcher_->Off(type, handler);
        LOG_INFO("JSObjectWrapper::DeleteWatch %{public}s", type);
    } else {
        LOG_ERROR("JSObjectWrapper::DeleteWatch watcher_ is null");
    }
}

bool JSObjectWrapper::isUndefined(char *value)
{
    std::string tmpStr = value;
    auto it = std::find(undefinedProperties.begin(), undefinedProperties.end(), tmpStr);
    if (it == undefinedProperties.end()) {
        return false;
    }
    return true;
}

void JSObjectWrapper::AddUndefined(char *value)
{
    std::string tmpStr = value;
    if (std::find(undefinedProperties.begin(), undefinedProperties.end(), tmpStr) == undefinedProperties.end()) {
        undefinedProperties.push_back(tmpStr);
    }
}

void JSObjectWrapper::DeleteUndefined(char *value)
{
    std::string tmpStr = value;
    auto it = std::find(undefinedProperties.begin(), undefinedProperties.end(), tmpStr);
    if (it != undefinedProperties.end()) {
        undefinedProperties.erase(it);
    }
}

void JSObjectWrapper::DestroyObject()
{
    object_ = nullptr;
}

void JSObjectWrapper::SetObjectId(const std::string &objectId)
{
    objectId_ = objectId;
}

std::string JSObjectWrapper::GetObjectId()
{
    return objectId_;
}

} // namespace OHOS::ObjectStore