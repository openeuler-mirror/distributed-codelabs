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

#include "huks_napi_attest_key_item.h"

#include "securec.h"

#include "hks_api.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"
#include "hks_type.h"
#include "huks_napi_common_item.h"

namespace HuksNapiItem {
namespace {
constexpr int HUKS_NAPI_ATTEST_KEY_MIN_ARGS = 2;
constexpr int HUKS_NAPI_ATTEST_KEY_MAX_ARGS = 3;

constexpr int INDEX_0 = 0;
constexpr int INDEX_1 = 1;
constexpr int INDEX_2 = 2;
constexpr int INDEX_3 = 3;
}  // namespace

struct AttestKeyAsyncContextT {
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;

    int32_t result = 0;
    struct HksBlob *keyAlias = nullptr;
    struct HksParamSet *paramSet = nullptr;
    struct HksCertChain *certChain = nullptr;
};
using AttestKeyAsyncContext = AttestKeyAsyncContextT *;

static AttestKeyAsyncContext CreateAttestKeyAsyncContext()
{
    AttestKeyAsyncContext context = static_cast<AttestKeyAsyncContext>(HksMalloc(sizeof(AttestKeyAsyncContextT)));
    if (context != nullptr) {
        (void)memset_s(context, sizeof(AttestKeyAsyncContextT), 0, sizeof(AttestKeyAsyncContextT));
    }
    return context;
}

static void DeleteAttestKeyAsyncContext(napi_env env, AttestKeyAsyncContext &context)
{
    if (context == nullptr) {
        return;
    }
    DeleteCommonAsyncContext(env, context->asyncWork, context->callback, context->keyAlias, context->paramSet);
    FreeHksCertChain(context->certChain);
    HksFree(context);
    context = nullptr;
}

static napi_value AttestKeyParseParams(napi_env env, napi_callback_info info, AttestKeyAsyncContext context)
{
    size_t argc = HUKS_NAPI_ATTEST_KEY_MAX_ARGS;
    napi_value argv[HUKS_NAPI_ATTEST_KEY_MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (argc < HUKS_NAPI_ATTEST_KEY_MIN_ARGS) {
        napi_throw_error(env, std::to_string(HUKS_ERR_CODE_ILLEGAL_ARGUMENT).c_str(), "no enough params input");
        HKS_LOG_E("no enough params");
        return nullptr;
    }

    size_t index = 0;
    napi_value result = ParseKeyAlias(env, argv[index], context->keyAlias);
    if (result == nullptr) {
        HKS_LOG_E("could not get alias");
        return nullptr;
    }

    index++;
    napi_value property = GetPropertyFromOptions(env, argv[index], HKS_OPTIONS_PROPERTY_PROPERTIES);
    if (property == nullptr) {
        HKS_LOG_E("attestKey get property failed");
        return nullptr;
    }
    HksParam param = { .tag = HKS_TAG_ATTESTATION_BASE64, .boolParam = true };
    result = ParseHksParamSetAndAddParam(env, property, context->paramSet, &param);
    if (result == nullptr) {
        HKS_LOG_E("could not get paramset");
        return nullptr;
    }

    index++;
    if (index < argc) {
        context->callback = GetCallback(env, argv[index]);
    }

    return GetInt32(env, 0);
}

static void InitCertChain(struct HksCertChain *certChain)
{
    certChain->certsCount = HKS_CERT_COUNT;
    certChain->certs = static_cast<struct HksBlob *>(HksMalloc(certChain->certsCount * sizeof(struct HksBlob)));
    if (certChain->certs != nullptr) {
        certChain->certs[INDEX_0].size = HKS_CERT_APP_SIZE;
        certChain->certs[INDEX_0].data = static_cast<uint8_t *>(HksMalloc(certChain->certs[INDEX_0].size));
        certChain->certs[INDEX_1].size = HKS_CERT_DEVICE_SIZE;
        certChain->certs[INDEX_1].data = static_cast<uint8_t *>(HksMalloc(certChain->certs[INDEX_1].size));
        certChain->certs[INDEX_2].size = HKS_CERT_CA_SIZE;
        certChain->certs[INDEX_2].data = static_cast<uint8_t *>(HksMalloc(certChain->certs[INDEX_2].size));
        certChain->certs[INDEX_3].size = HKS_CERT_ROOT_SIZE;
        certChain->certs[INDEX_3].data = static_cast<uint8_t *>(HksMalloc(certChain->certs[INDEX_3].size));
    }
}

static napi_value AttestKeyAsyncWork(napi_env env, AttestKeyAsyncContext context)
{
    napi_value promise = nullptr;
    if (context->callback == nullptr) {
        NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "attestKeyAsyncWork", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AttestKeyAsyncContext napiContext = static_cast<AttestKeyAsyncContext>(data);

            napiContext->certChain = static_cast<struct HksCertChain *>(HksMalloc(sizeof(struct HksCertChain)));
            if (napiContext->certChain != nullptr) {
                InitCertChain(napiContext->certChain);
            }

            napiContext->result = HksAttestKey(napiContext->keyAlias, napiContext->paramSet, napiContext->certChain);
        },
        [](napi_env env, napi_status status, void *data) {
            AttestKeyAsyncContext napiContext = static_cast<AttestKeyAsyncContext>(data);
            HksSuccessReturnResult resultData;
            SuccessReturnResultInit(resultData);
            resultData.certChain = napiContext->certChain;
            HksReturnNapiResult(env, napiContext->callback, napiContext->deferred, napiContext->result, resultData);
            DeleteAttestKeyAsyncContext(env, napiContext);
        },
        static_cast<void *>(context),
        &context->asyncWork);

    napi_status status = napi_queue_async_work(env, context->asyncWork);
    if (status != napi_ok) {
        DeleteAttestKeyAsyncContext(env, context);
        HKS_LOG_E("could not queue async work");
        return nullptr;
    }

    if (context->callback == nullptr) {
        return promise;
    } else {
        return GetNull(env);
    }
}

napi_value HuksNapiAttestKeyItem(napi_env env, napi_callback_info info)
{
    AttestKeyAsyncContext context = CreateAttestKeyAsyncContext();
    if (context == nullptr) {
        HKS_LOG_E("could not create context");
        return nullptr;
    }

    napi_value result = AttestKeyParseParams(env, info, context);
    if (result == nullptr) {
        HKS_LOG_E("could not parse params");
        DeleteAttestKeyAsyncContext(env, context);
        return nullptr;
    }

    result = AttestKeyAsyncWork(env, context);
    if (result == nullptr) {
        HKS_LOG_E("could not start async work");
        DeleteAttestKeyAsyncContext(env, context);
        return nullptr;
    }
    return result;
}
}  // namespace HuksNapiItem
