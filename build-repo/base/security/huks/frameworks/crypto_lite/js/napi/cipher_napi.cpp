/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "cipher.h"
#include "cipher_log.h"
#include "securec.h"

namespace OHOS::Ace::Napi {
namespace {
}

struct CallbackContext {
    napi_ref callbackSuccess = nullptr;
    napi_ref callbackFail = nullptr;
    napi_ref callbackComplete = nullptr;
};

struct CommonNapiValue {
    napi_env env = nullptr;
    napi_async_work work = nullptr;
    napi_value action_napi = nullptr;
    napi_value text_napi = nullptr;
    napi_value key_napi = nullptr;
    napi_value transformation_napi = nullptr;
};

struct RsaAsyncContext {
    CommonNapiValue *commonNapi = nullptr;
    CallbackContext *callback = nullptr;
    RsaKeyData *rsaKey = nullptr;
    RsaData *textIn = nullptr;
    RsaData *textOut = nullptr;
    int32_t ret = 0;
};

struct AesAsyncContext {
    CommonNapiValue *commonNapi = nullptr;
    napi_value iv_napi = nullptr;
    napi_value ivOffset_napi = nullptr;
    napi_value ivLen_napi = nullptr;
    CallbackContext *callback = nullptr;
    char *key = nullptr;
    char *textIn = nullptr;
    char *textOut = nullptr;
    char *action = nullptr;
    char *transformation = nullptr;
    char *ivBuf = nullptr;
    int32_t ivOffset = 0;
    int32_t ivLen = 0;
    int32_t ret = 0;
};

static const char g_failCode[] = "System error";
static const int ERROR_CODE = 200;
static const int FAIL_ARG = 2;
static const int IV_LEN = 16;

#define SELF_FREE_PTR(PTR, FREE_FUNC) \
{ \
    if ((PTR) != nullptr) { \
        FREE_FUNC(PTR); \
        (PTR) = nullptr; \
    } \
}

#define CIPHER_FREE_PTR(p) SELF_FREE_PTR(p, free)

static int32_t GetString(napi_env env, napi_value object, char **element, size_t *len)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, object, &valueType);
    if (status != napi_ok) {
        return ERROR_CODE_GENERAL;
    }

    if (valueType != napi_string) {
        *element = nullptr;
        CIPHER_LOG_E("input is not a string-type object");
        return ERROR_SUCCESS;
    }
        
    status = napi_get_value_string_utf8(env, object, nullptr, 0, len);
    if (status != napi_ok) {
        return ERROR_CODE_GENERAL;
    }

    *element = static_cast<char *>(malloc(*len + 1));
    if (*element == nullptr) {
        CIPHER_LOG_E("malloc element fail");
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(*element, *len + 1, 0, *len + 1);
    size_t result = 0;
    status = napi_get_value_string_utf8(env, object, *element, *len + 1, &result);
    if (status != napi_ok) {
        return ERROR_CODE_GENERAL;
    }
    return ERROR_SUCCESS;
}

static int32_t GetCallbackFuncProperty(napi_env env, napi_value object, napi_value *successFunc,
    napi_value *failFunc, napi_value *completeFunc)
{
    napi_status status = napi_get_named_property(env, object, "success", successFunc);
    if (status != napi_ok || *successFunc == nullptr) {
        CIPHER_LOG_E("get success property fail");
        return ERROR_CODE_GENERAL;
    }
    status = napi_get_named_property(env, object, "fail", failFunc);
    if (status != napi_ok || *failFunc == nullptr) {
        CIPHER_LOG_E("get fail property fail");
        return ERROR_CODE_GENERAL;
    }
    status = napi_get_named_property(env, object, "complete", completeFunc);
    if (status != napi_ok || *completeFunc == nullptr) {
        CIPHER_LOG_E("get complete property fail");
        return ERROR_CODE_GENERAL;
    }
    return ERROR_SUCCESS;
}

static int32_t CreateCallbackReference(napi_env env, napi_value successFunc, napi_value failFunc,
    napi_value completeFunc, CallbackContext *callback)
{
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, successFunc, &valueType);
    if (status != napi_ok) {
        return ERROR_CODE_GENERAL;
    }
    if (valueType != napi_function) {
        return ERROR_CODE_GENERAL;
    }

    status = napi_create_reference(env, successFunc, 1, &callback->callbackSuccess);
    if (status != napi_ok) {
        CIPHER_LOG_E("create success reference fail");
        return ERROR_CODE_GENERAL;
    }

    status = napi_create_reference(env, failFunc, 1, &callback->callbackFail);
    if (status != napi_ok) {
        CIPHER_LOG_E("create fail reference fail");
        return ERROR_CODE_GENERAL;
    }

    status = napi_create_reference(env, completeFunc, 1, &callback->callbackComplete);
    if (status != napi_ok) {
        CIPHER_LOG_E("create complete reference fail");
        return ERROR_CODE_GENERAL;
    }

    return ERROR_SUCCESS;
}

static int32_t ReadAesData(napi_env env, AesAsyncContext *context)
{
    size_t len = 0;
    int32_t ret = GetString(env, context->commonNapi->action_napi, &context->action, &len);
    if (ret != ERROR_SUCCESS || context->action == nullptr) {
        CIPHER_LOG_E("get action fail");
        return ret;
    }

    len = 0;
    ret = GetString(env, context->commonNapi->text_napi, &context->textIn, &len);
    if (ret != ERROR_SUCCESS || context->textIn == nullptr) {
        CIPHER_LOG_E("get text fail");
        return ret;
    }

    len = 0;
    ret = GetString(env, context->commonNapi->key_napi, &context->key, &len);
    if (ret != ERROR_SUCCESS || context->key == nullptr) {
        CIPHER_LOG_E("get key fail");
        return ret;
    }

    len = 0;
    ret = GetString(env, context->commonNapi->transformation_napi, &context->transformation, &len);
    if (ret != ERROR_SUCCESS) {
        CIPHER_LOG_E("get transformition fail");
        return ret;
    }

    (void)context->ivLen_napi;
    context->ivLen = IV_LEN;

    len = 0;
    ret = GetString(env, context->iv_napi, &context->ivBuf, &len);
    if (ret != ERROR_SUCCESS) {
        CIPHER_LOG_E("get ivBuf fail");
        return ret;
    }

    (void)context->ivOffset_napi;
    context->ivOffset = 0;
    return ret;
}

static int32_t GetCommonProperties(napi_env env, napi_value object, CommonNapiValue *commonNapi)
{
    napi_status status = napi_get_named_property(env, object, "action", &commonNapi->action_napi);
    if (status != napi_ok || commonNapi->action_napi == nullptr) {
        CIPHER_LOG_E("get action property fail");
        return ERROR_CODE_GENERAL;
    }
    status = napi_get_named_property(env, object, "text", &commonNapi->text_napi);
    if (status != napi_ok || commonNapi->text_napi == nullptr) {
        CIPHER_LOG_E("get text property fail");
        return ERROR_CODE_GENERAL;
    }
    status = napi_get_named_property(env, object, "key", &commonNapi->key_napi);
    if (status != napi_ok || commonNapi->key_napi == nullptr) {
        CIPHER_LOG_E("get key property fail");
        return ERROR_CODE_GENERAL;
    }
    status = napi_get_named_property(env, object, "transformation", &commonNapi->transformation_napi);
    if (status != napi_ok) {
        CIPHER_LOG_E("get transformation property fail");
        return ERROR_CODE_GENERAL;
    }
    return ERROR_SUCCESS;
}

static int32_t GetAesProperties(napi_env env, napi_value object, AesAsyncContext *context)
{
    int32_t ret = GetCommonProperties(env, object, context->commonNapi);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    napi_status status = napi_get_named_property(env, object, "iv", &context->iv_napi);
    if (status != napi_ok) {
        CIPHER_LOG_E("get iv property fail");
        return ERROR_CODE_GENERAL;
    }
    status = napi_get_named_property(env, object, "ivOffset", &context->ivOffset_napi);
    if (status != napi_ok || context->ivOffset_napi == nullptr) {
        CIPHER_LOG_E("get ivOffset property fail");
        return ERROR_CODE_GENERAL;
    }
    status = napi_get_named_property(env, object, "ivLen", &context->ivLen_napi);
    if (status != napi_ok || context->ivLen_napi == nullptr) {
        CIPHER_LOG_E("get ivLen property fail");
        return ERROR_CODE_GENERAL;
    }
    return ERROR_SUCCESS;
}

static int32_t GetAesInput(napi_env env, napi_value object, AesAsyncContext *context)
{
    napi_value successFunc = nullptr;
    napi_value failFunc = nullptr;
    napi_value completeFunc = nullptr;
    napi_valuetype valueType = napi_undefined;
    context->commonNapi = static_cast<CommonNapiValue *>(malloc(sizeof(struct CommonNapiValue)));
    if (context->commonNapi == nullptr) {
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(context->commonNapi, sizeof(struct CommonNapiValue), 0, sizeof(struct CommonNapiValue));

    napi_status status = napi_typeof(env, object, &valueType);
    if (status != napi_ok) {
        return ERROR_CODE_GENERAL;
    }
    if (valueType != napi_object) {
        return ERROR_CODE_GENERAL;
    }

    int32_t ret = GetAesProperties(env, object, context);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    ret = GetCallbackFuncProperty(env, object, &successFunc, &failFunc, &completeFunc);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    context->callback = static_cast<CallbackContext *>(malloc(sizeof(struct CallbackContext)));
    if (context->callback == nullptr) {
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(context->callback, sizeof(struct CallbackContext), 0, sizeof(struct CallbackContext));

    ret = CreateCallbackReference(env, successFunc, failFunc, completeFunc, context->callback);
    if (ret != ERROR_SUCCESS) {
    }

    return ReadAesData(env, context);
}

static int32_t AesExcute(AesAsyncContext *asyncContext)
{
    AesCryptContext aes = { { nullptr, nullptr, 0, 0, 0 }, CIPHER_AES_ECB, { nullptr, nullptr, 0, 0 } };
    AesIvMode iv = { nullptr, nullptr, 0, 0 };
    iv.ivBuf = asyncContext->ivBuf;
    iv.ivLen = asyncContext->ivLen;
    iv.ivOffset = asyncContext->ivOffset;
    iv.transformation = asyncContext->transformation;

    int ret = InitAesCryptData(asyncContext->action, asyncContext->textIn, asyncContext->key, &iv, &aes);
    if (ret != ERROR_SUCCESS) {
        CIPHER_LOG_E("InitAesCryptData fail, ret is %d", ret);
        return ret;
    }

    ret = AesCrypt(&aes);
    if (ret != ERROR_SUCCESS) {
        CIPHER_LOG_E("AesCrypt fail, ret is %d", ret);
    }
    asyncContext->textOut = static_cast<char *>(malloc(strlen(aes.data.text) + 1));
    if (asyncContext->textOut == nullptr) {
        DeinitAesCryptData(&aes);
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(asyncContext->textOut, strlen(aes.data.text) + 1, 0, strlen(aes.data.text) + 1);
    (void)memcpy_s(asyncContext->textOut, strlen(aes.data.text) + 1, aes.data.text, strlen(aes.data.text));
    DeinitAesCryptData(&aes);

    return ret;
}

static int32_t ReadRsaData(napi_env env, RsaAsyncContext *context)
{
    context->rsaKey = static_cast<RsaKeyData *>(malloc(sizeof(RsaKeyData)));
    if (context->rsaKey == nullptr) {
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(context->rsaKey, sizeof(RsaKeyData), 0, sizeof(RsaKeyData));

    context->rsaKey->trans = nullptr;
    context->textIn = static_cast<RsaData *>(malloc(sizeof(RsaData)));
    if (context->textIn == nullptr) {
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(context->textIn, sizeof(RsaData), 0, sizeof(RsaData));

    context->textOut = static_cast<RsaData *>(malloc(sizeof(RsaData)));
    if (context->textOut == nullptr) {
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(context->textOut, sizeof(RsaData), 0, sizeof(RsaData));

    context->textOut->data = nullptr;
    context->textOut->length = 0;

    size_t len = 0;
    int32_t ret = GetString(env, context->commonNapi->action_napi, &context->rsaKey->action, &len);
    if (ret != ERROR_SUCCESS || context->rsaKey->action == nullptr) {
        CIPHER_LOG_E("get action fail");
        return ret;
    }

    ret = GetString(env, context->commonNapi->text_napi, &context->textIn->data, &context->textIn->length);
    if (ret != ERROR_SUCCESS || context->textIn->data == nullptr) {
        CIPHER_LOG_E("get textIn fail");
        return ret;
    }

    ret = GetString(env, context->commonNapi->key_napi, &context->rsaKey->key, &context->rsaKey->keyLen);
    if (ret != ERROR_SUCCESS || context->rsaKey->key == nullptr) {
        CIPHER_LOG_E("get key fail");
        return ret;
    }

    len = 0;
    ret = GetString(env, context->commonNapi->transformation_napi, &context->rsaKey->trans, &len);
    if (ret != ERROR_SUCCESS) {
        CIPHER_LOG_E("get trans fail");
    }
    return ret;
}

static int32_t GetRsaInput(napi_env env, napi_value object, RsaAsyncContext *context)
{
    napi_value successFunc = nullptr;
    napi_value failFunc = nullptr;
    napi_value completeFunc = nullptr;
    napi_valuetype valueType = napi_undefined;
    context->commonNapi = static_cast<CommonNapiValue *>(malloc(sizeof(struct CommonNapiValue)));
    if (context->commonNapi == nullptr) {
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(context->commonNapi, sizeof(struct CommonNapiValue), 0, sizeof(struct CommonNapiValue));

    napi_status status = napi_typeof(env, object, &valueType);
    if (status != napi_ok) {
        return ERROR_CODE_GENERAL;
    }
    if (valueType != napi_object) {
        return ERROR_CODE_GENERAL;
    }

    int32_t ret = GetCommonProperties(env, object, context->commonNapi);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    ret = GetCallbackFuncProperty(env, object, &successFunc, &failFunc, &completeFunc);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }

    context->callback = static_cast<CallbackContext *>(malloc(sizeof(struct CallbackContext)));
    if (context->callback == nullptr) {
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(context->callback, sizeof(struct CallbackContext), 0, sizeof(struct CallbackContext));

    ret = CreateCallbackReference(env, successFunc, failFunc, completeFunc, context->callback);
    if (ret != ERROR_SUCCESS) {
    }

    return ReadRsaData(env, context);
}

static int32_t RsaExcute(RsaAsyncContext *asyncContext)
{
    if ((asyncContext->rsaKey->key == nullptr) || (asyncContext->textIn->data == nullptr)) {
        return ERROR_CODE_GENERAL;
    }

    int32_t ret = RsaCrypt(asyncContext->rsaKey, asyncContext->textIn, asyncContext->textOut);
    if ((ret != ERROR_SUCCESS) || (asyncContext->textOut->length == 0)) {
        return ERROR_CODE_GENERAL;
    }

    asyncContext->textOut->data = static_cast<char *>(malloc(asyncContext->textOut->length));
    if (asyncContext->textOut->data == nullptr) {
        return ERROR_CODE_GENERAL;
    }
    (void)memset_s(asyncContext->textOut->data, asyncContext->textOut->length, 0, asyncContext->textOut->length);

    ret = RsaCrypt(asyncContext->rsaKey, asyncContext->textIn, asyncContext->textOut);
    if (ret != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

void SetComplete(napi_env env, CallbackContext *asyncContext)
{
    napi_value callback = nullptr;
    napi_value ret = 0;
    napi_get_reference_value(env, asyncContext->callbackComplete, &callback);
    napi_call_function(env, nullptr, callback, 0, nullptr, &ret);
    napi_delete_reference(env, asyncContext->callbackComplete);
}

void SetSuccess(napi_env env, char *textOut, size_t textLength, CallbackContext *asyncContext)
{
    napi_value callback = nullptr;
    napi_value ret;

    napi_value result = nullptr;
    napi_value returnObj = nullptr;
    napi_create_object(env, &returnObj);
    napi_create_string_utf8(env, textOut, textLength, &result);
    napi_set_named_property(env, returnObj, "text", result);

    napi_get_reference_value(env, asyncContext->callbackSuccess, &callback);
    napi_call_function(env, nullptr, callback, 1, &returnObj, &ret);
    napi_delete_reference(env, asyncContext->callbackSuccess);
}

void SetFail(napi_env env, CallbackContext *asyncContext)
{
    napi_value callback = nullptr;
    napi_value ret;

    napi_value result = nullptr;
    napi_create_string_utf8(env, g_failCode, sizeof(g_failCode), &result);

    napi_value errorCode = nullptr;
    napi_create_int32(env, ERROR_CODE, &errorCode);
    
    napi_value params[FAIL_ARG] = { result, errorCode };
    napi_get_reference_value(env, asyncContext->callbackFail, &callback);
    napi_call_function(env, nullptr, callback, FAIL_ARG, params, &ret);
    napi_delete_reference(env, asyncContext->callbackFail);
}

static void DeleteRsaAsyncContext(napi_env env, RsaAsyncContext *context)
{
    if (context == nullptr) {
        return;
    }

    CIPHER_FREE_PTR(context->commonNapi);

    CIPHER_FREE_PTR(context->callback);

    CIPHER_FREE_PTR(context->textIn->data);
    CIPHER_FREE_PTR(context->textIn);

    if (context->textOut->data != nullptr) {
        (void)memset_s(context->textOut->data, context->textOut->length, 0, context->textOut->length);
    }
    CIPHER_FREE_PTR(context->textOut->data);
    CIPHER_FREE_PTR(context->textOut);

    CIPHER_FREE_PTR(context->rsaKey->trans);
    CIPHER_FREE_PTR(context->rsaKey->action);
    if (context->rsaKey->key != nullptr) {
        (void)memset_s(context->rsaKey->key, context->rsaKey->keyLen, 0, context->rsaKey->keyLen);
    }
    CIPHER_FREE_PTR(context->rsaKey->key);
    CIPHER_FREE_PTR(context->rsaKey);
}

static void DeleteAesAsyncContext(napi_env env, AesAsyncContext *context)
{
    if (context == nullptr) {
        return;
    }

    CIPHER_FREE_PTR(context->commonNapi);

    CIPHER_FREE_PTR(context->callback);

    if (context->key != nullptr) {
        (void)memset_s(context->key, strlen(context->key), 0, strlen(context->key));
    }
    CIPHER_FREE_PTR(context->key);
    CIPHER_FREE_PTR(context->textIn);
    CIPHER_FREE_PTR(context->action);

    CIPHER_FREE_PTR(context->ivBuf);
    CIPHER_FREE_PTR(context->transformation);

    if (context->textOut != nullptr) {
        (void)memset_s(context->textOut, strlen(context->textOut), 0, strlen(context->textOut));
    }
    CIPHER_FREE_PTR(context->textOut);
}

static napi_value JSCipherRsa(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);
    auto rsaAsyncContext = new RsaAsyncContext();

    rsaAsyncContext->ret = GetRsaInput(env, argv[0], rsaAsyncContext);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JSCipherRsa", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void *data) {
            RsaAsyncContext *asyncContext = (RsaAsyncContext *)data;
            if (asyncContext->ret == ERROR_SUCCESS) {
                asyncContext->ret = RsaExcute(asyncContext);
            }
        },

        [](napi_env env, napi_status status, void *data) {
            RsaAsyncContext *asyncContext = (RsaAsyncContext *)data;
            if (asyncContext->ret != ERROR_SUCCESS) {
                SetFail(env, asyncContext->callback);
                SetComplete(env, asyncContext->callback);
                napi_delete_reference(env, asyncContext->callback->callbackSuccess);
            } else {
                SetSuccess(env, asyncContext->textOut->data, asyncContext->textOut->length, asyncContext->callback);
                SetComplete(env, asyncContext->callback);
                napi_delete_reference(env, asyncContext->callback->callbackFail);
            }
            napi_delete_async_work(env, asyncContext->commonNapi->work);
            DeleteRsaAsyncContext(env, asyncContext);
            delete asyncContext;
        },
        reinterpret_cast<void *>(rsaAsyncContext),
        &rsaAsyncContext->commonNapi->work);
        napi_queue_async_work(env, rsaAsyncContext->commonNapi->work);
        return nullptr;
}

static napi_value JSCipherAes(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);
    auto aesAsyncContext = new AesAsyncContext();

    aesAsyncContext->ret = GetAesInput(env, argv[0], aesAsyncContext);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JSCipherAes", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void *data) {
            AesAsyncContext *asyncContext = (AesAsyncContext *)data;
            if (asyncContext->ret == ERROR_SUCCESS) {
                asyncContext->ret = AesExcute(asyncContext);
            }
        },

        [](napi_env env, napi_status status, void *data) {
            AesAsyncContext *asyncContext = (AesAsyncContext *)data;
            if (asyncContext->ret != ERROR_SUCCESS) {
                SetFail(env, asyncContext->callback);
                SetComplete(env, asyncContext->callback);
                napi_delete_reference(env, asyncContext->callback->callbackSuccess);
            } else {
                SetSuccess(env, asyncContext->textOut, static_cast<size_t>(strlen(asyncContext->textOut)),
                    asyncContext->callback);
                SetComplete(env, asyncContext->callback);
                napi_delete_reference(env, asyncContext->callback->callbackFail);
            }
            napi_delete_async_work(env, asyncContext->commonNapi->work);
            DeleteAesAsyncContext(env, asyncContext);
            delete asyncContext;
        },
        reinterpret_cast<void *>(aesAsyncContext),
        &aesAsyncContext->commonNapi->work);
        napi_queue_async_work(env, aesAsyncContext->commonNapi->work);
        return nullptr;
}


static napi_value CipherExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor cipherDesc[] = {
        DECLARE_NAPI_FUNCTION("aes", JSCipherAes),
        DECLARE_NAPI_FUNCTION("rsa", JSCipherRsa),
    };
    NAPI_CALL(env, napi_define_properties(
        env, exports, sizeof(cipherDesc) / sizeof(cipherDesc[0]), cipherDesc));
    return exports;
}

static napi_module CipherModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = CipherExport,
    .nm_modname = "cipher",
    .nm_priv = reinterpret_cast<void *>(0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void CipherRegister()
{
    napi_module_register(&CipherModule);
}
}
