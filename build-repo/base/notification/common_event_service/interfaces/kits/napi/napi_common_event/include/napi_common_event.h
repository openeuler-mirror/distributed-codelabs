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

#ifndef BASE_NOTIFICATION_CES_STANDARD_CESFWK_KITS_NAPI_COMMON_EVENT_MANAGER_INCLUDE_COMMON_EVENT_MANAGER_H
#define BASE_NOTIFICATION_CES_STANDARD_CESFWK_KITS_NAPI_COMMON_EVENT_MANAGER_INCLUDE_COMMON_EVENT_MANAGER_H

#include "common_event_constant.h"
#include "common_event_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace EventManagerFwkNapi {
using namespace OHOS::EventFwk;

static const int32_t CREATE_MAX_PARA = 2;
static const int32_t SUBSCRIBE_MAX_PARA = 2;
static const int32_t UNSUBSCRIBE_MAX_PARA = 2;
static const int32_t PUBLISH_MAX_PARA_BY_PUBLISHDATA = 3;
static const int32_t PUBLISH_MAX_PARA_BY_USERID = 4;
static const int8_t NO_ERROR = 0;
static const int8_t ERR_CES_FAILED = 1;

class SubscriberInstance;
struct AsyncCallbackInfoSubscribe;

struct subscriberInstanceInfo {
    std::vector<AsyncCallbackInfoSubscribe *> asyncCallbackInfo;
    std::shared_ptr<AsyncCommonEventResult> commonEventResult = nullptr;
};

static thread_local napi_ref g_CommonEventSubscriber = nullptr;
static std::map<std::shared_ptr<SubscriberInstance>, subscriberInstanceInfo> subscriberInstances;
static std::mutex subscriberInsMutex;

struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int32_t errorCode = NO_ERROR;
};

struct AsyncCallbackInfoSubscribe {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    int32_t errorCode = NO_ERROR;
};

struct AsyncCallbackInfoUnsubscribe {
    explicit AsyncCallbackInfoUnsubscribe();
    ~AsyncCallbackInfoUnsubscribe();
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback = nullptr;
    size_t argc = 0;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    int32_t errorCode = NO_ERROR;
};

struct AsyncCallbackInfoSubscribeInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    std::vector<std::string> events;
    std::string permission;
    std::string deviceId;
    int32_t userId = UNDEFINED_USER;
    int32_t priority = 0;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoOrderedCommonEvent {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    bool isOrdered = false;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoStickyCommonEvent {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    bool isSticky = false;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoGetCode {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    int32_t code = 0;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoSetCode {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    int32_t code = 0;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoGetData {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    std::string data;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoSetData {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    std::string data;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoSetCodeAndData {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    int32_t code = 0;
    std::string data;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoAbort {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoClearAbort {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoGetAbort {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    bool abortEvent = false;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoFinish {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
    CallbackPromiseInfo info;
};

struct AsyncCallbackInfoCreate {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref subscribeInfo = nullptr;
    CallbackPromiseInfo info;
    napi_value result = nullptr;
};

struct AsyncCallbackInfoPublish {
    napi_env env = nullptr;
    napi_async_work asyncWork;
    napi_ref callback = nullptr;
    CommonEventData commonEventData;
    CommonEventPublishInfo commonEventPublishInfo;
    int32_t userId = UNDEFINED_USER;
    int32_t errorCode = NO_ERROR;
};

struct CommonEventPublishDataByjs {
    std::string bundleName;
    std::string data;
    int32_t code = 0;
    std::vector<std::string> subscriberPermissions;
    bool isOrdered = false;
    bool isSticky = false;
    AAFwk::WantParams wantParams;
};

struct CommonEventDataWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    std::shared_ptr<bool> valid;
    Want want;
    int32_t code = 0;
    std::string data;
};

class SubscriberInstance : public CommonEventSubscriber {
public:
    SubscriberInstance(const CommonEventSubscribeInfo &sp);
    virtual ~SubscriberInstance();

    void OnReceiveEvent(const CommonEventData &data) override;

    void SetEnv(const napi_env &env);
    void SetCallbackRef(const napi_ref &ref);
    unsigned long long GetID();

private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
    std::shared_ptr<bool> valid_;
    std::atomic_ullong id_;
    static std::atomic_ullong subscriberID_;
};

class SubscriberInstanceWrapper {
public:
    SubscriberInstanceWrapper(const CommonEventSubscribeInfo &info);
    std::shared_ptr<SubscriberInstance> GetSubscriber();

private:
    std::shared_ptr<SubscriberInstance> subscriber = nullptr;
};

napi_value NapiGetNull(napi_env env);

napi_value GetCallbackErrorValue(napi_env env, int32_t errorCode);

napi_value ParseParametersByCreateSubscriber(
    const napi_env &env, const napi_value (&argv)[CREATE_MAX_PARA], const size_t &argc, napi_ref &callback);

void SetCallback(const napi_env &env, const napi_ref &callbackIn, const int32_t &errorCode, const napi_value &result);

void SetCallback(const napi_env &env, const napi_ref &callbackIn, const napi_value &result);

void SetPromise(const napi_env &env, const napi_deferred &deferred, const int32_t &errorCode, const napi_value &result);

void ReturnCallbackPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result);

void PaddingAsyncCallbackInfoCreateSubscriber(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoCreate *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value CreateSubscriber(napi_env env, napi_callback_info info);

napi_value ParseParametersByGetSubscribeInfo(
    const napi_env &env, const size_t &argc, const napi_value (&argv)[1], napi_ref &callback);

void SetEventsResult(const napi_env &env, const std::vector<std::string> &events, napi_value &commonEventSubscribeInfo);

void SetPublisherPermissionResult(
    const napi_env &env, const std::string &permission, napi_value &commonEventSubscribeInfo);

void SetPublisherDeviceIdResult(const napi_env &env, const std::string &deviceId, napi_value &commonEventSubscribeInfo);

void SetPublisherUserIdResult(const napi_env &env, const int32_t &userId, napi_value &commonEventSubscribeInfo);

void SetPublisherPriorityResult(const napi_env &env, const int32_t &priority, napi_value &commonEventSubscribeInfo);

void PaddingAsyncCallbackInfoGetSubscribeInfo(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSubscribeInfo *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

void PaddingNapiCreateAsyncWorkCallbackInfo(AsyncCallbackInfoSubscribeInfo *&asynccallbackinfo);

void SetNapiResult(const napi_env &env, const AsyncCallbackInfoSubscribeInfo *asynccallbackinfo, napi_value &result);

napi_value GetSubscribeInfo(napi_env env, napi_callback_info info);

napi_value ParseParametersByIsOrderedCommonEvent(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoIsOrderedCommonEvent(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoOrderedCommonEvent *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value IsOrderedCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByIsStickyCommonEvent(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoIsStickyCommonEvent(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoStickyCommonEvent *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value IsStickyCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByGetCode(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoGetCode(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetCode *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

std::shared_ptr<AsyncCommonEventResult> GetAsyncResult(const SubscriberInstance *objectInfo);

napi_value GetCode(napi_env env, napi_callback_info info);

napi_value ParseParametersBySetCode(
    const napi_env &env, const napi_value (&argv)[2], size_t argc, int32_t &code, napi_ref &callback);

void PaddingAsyncCallbackInfoSetCode(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetCode *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value SetCode(napi_env env, napi_callback_info info);

napi_value ParseParametersByGetData(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoGetData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value GetData(napi_env env, napi_callback_info info);

napi_value ParseParametersBySetData(
    const napi_env &env, const napi_value (&argv)[2], size_t argc, std::string &data, napi_ref &callback);

void PaddingAsyncCallbackInfoSetData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value SetData(napi_env env, napi_callback_info info);

napi_value ParseParametersBySetCodeAndData(const napi_env &env, const napi_value (&argv)[3],
    size_t argc, int32_t &code, std::string &data, napi_ref &callback);

void PaddingAsyncCallbackInfoSetCodeAndData(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoSetCodeAndData *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value SetCodeAndData(napi_env env, napi_callback_info info);

napi_value ParseParametersByAbort(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoAbort(const napi_env &env, const size_t &argc, AsyncCallbackInfoAbort *&asynccallbackinfo,
    const napi_ref &callback, napi_value &promise);

napi_value AbortCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByClearAbort(
    const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoClearAbort(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoClearAbort *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value ClearAbortCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByGetAbort(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoGetAbort(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoGetAbort *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value GetAbortCommonEvent(napi_env env, napi_callback_info info);

napi_value ParseParametersByFinish(const napi_env &env, const napi_value (&argv)[1], size_t argc, napi_ref &callback);

void PaddingAsyncCallbackInfoFinish(const napi_env &env, const size_t &argc,
    AsyncCallbackInfoFinish *&asynccallbackinfo, const napi_ref &callback, napi_value &promise);

napi_value FinishCommonEvent(napi_env env, napi_callback_info info);

std::shared_ptr<SubscriberInstance> GetSubscriber(const napi_env &env, const napi_value &value);

napi_value ParseParametersBySubscribe(const napi_env &env, const napi_value (&argv)[SUBSCRIBE_MAX_PARA],
    std::shared_ptr<SubscriberInstance> &subscriber, napi_ref &callback);

napi_value Subscribe(napi_env env, napi_callback_info info);

napi_value GetBundlenameByPublish(const napi_env &env, const napi_value &value, std::string &bundleName);

napi_value GetDataByPublish(const napi_env &env, const napi_value &value, std::string &data);

napi_value GetCodeByPublish(const napi_env &env, const napi_value &value, int32_t &code);

napi_value GetSubscriberPermissionsByPublish(
    const napi_env &env, const napi_value &value, std::vector<std::string> &subscriberPermissions);

napi_value GetIsOrderedByPublish(const napi_env &env, const napi_value &value, bool &isOrdered);

napi_value GetIsStickyByPublish(const napi_env &env, const napi_value &value, bool &isSticky);

napi_value GetParametersByPublish(const napi_env &env, const napi_value &value, AAFwk::WantParams &wantParams);

napi_value ParseParametersByPublish(const napi_env &env, const napi_value (&argv)[PUBLISH_MAX_PARA_BY_PUBLISHDATA],
    const size_t &argc, std::string &event, CommonEventPublishDataByjs &commonEventPublishData, napi_ref &callback);

void PaddingCallbackInfoPublish(Want &want, AsyncCallbackInfoPublish *&asynccallbackinfo,
    const CommonEventPublishDataByjs &commonEventPublishDatajs);

napi_value Publish(napi_env env, napi_callback_info info);

napi_value ParseParametersByPublishAsUser(const napi_env &env, const napi_value (&argv)[PUBLISH_MAX_PARA_BY_USERID],
    const size_t &argc, std::string &event, int32_t &userId, CommonEventPublishDataByjs &commonEventPublishData,
    napi_ref &callback);

napi_value PublishAsUser(napi_env env, napi_callback_info info);

napi_value GetSubscriberByUnsubscribe(
    const napi_env &env, const napi_value &value, std::shared_ptr<SubscriberInstance> &subscriber, bool &isFind);

napi_value ParseParametersByUnsubscribe(const napi_env &env, const size_t &argc,
    const napi_value (&argv)[UNSUBSCRIBE_MAX_PARA], std::shared_ptr<SubscriberInstance> &subscriber,
    napi_ref &callback);

void NapiDeleteSubscribe(const napi_env &env, std::shared_ptr<SubscriberInstance> &subscriber);

napi_value Unsubscribe(napi_env env, napi_callback_info info);

napi_value GetEventsByCreateSubscriber(const napi_env &env, const napi_value &argv, std::vector<std::string> &events);

napi_value GetPublisherPermissionByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::string &permission, bool &hasProperty);

napi_value GetPublisherDeviceIdByCreateSubscriber(
    const napi_env &env, const napi_value &argv, std::string &publisherDeviceId, bool &hasProperty);

napi_value GetUserIdByCreateSubscriber(const napi_env &env, const napi_value &argv, CommonEventSubscribeInfo &info);

napi_value GetPriorityByCreateSubscriber(const napi_env &env, const napi_value &argv, int &priority, bool &hasProperty);

napi_value CommonEventSubscriberConstructor(napi_env env, napi_callback_info info);

napi_value CommonEventSubscriberInit(napi_env env, napi_value exports);

napi_value CommonEventManagerInit(napi_env env, napi_value exports);

void NapiThrow(napi_env env, int32_t errCode);
}  // namespace EventManagerFwkNapi
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_CES_STANDARD_CESFWK_KITS_NAPI_COMMON_EVENT_INCLUDE_COMMON_EVENT_H
