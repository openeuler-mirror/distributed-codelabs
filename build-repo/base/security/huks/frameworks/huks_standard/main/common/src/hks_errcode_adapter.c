/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hks_errcode_adapter.h"

#include <stddef.h>

#include "hks_type.h"

static const char *g_convertErrMsg = "HksConvertErrCode Failed.";

static struct HksError g_errCodeTable[] = {
    {
        .innerErrCode = HKS_SUCCESS,
        .hksResult = {
            .errorCode = HKS_SUCCESS,
            .errorMsg = "Success.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_NO_PERMISSION,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_PERMISSION_FAIL,
            .errorMsg = "Permission check failed. Apply for the required permissions first.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_ARGUMENT,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid parameters.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INSUFFICIENT_DATA,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Some input parameters are not set.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_BUFFER_TOO_SMALL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Insufficient buffer size.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_NULL_POINTER,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "The parameter value cannot be null.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_PUBLIC_KEY,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid public key.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_KEY_INFO,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid key information.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_PARAM_NOT_EXIST,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "The parameter does not exist.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_NEW_ROOT_KEY_MATERIAL_EXIST,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "The root key material already exists.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_WRAPPED_FORMAT,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "The wrapped key data is in invalid format.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_AUTH_TYP_FAILED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Failed to obtain the authentication type. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_CHALLENGE_TYPE_FAILED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Failed to obtain the challenge type. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_ACCESS_TYPE_FAILED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Failed to obtain the access type. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_AUTH_TOKEN_FAILED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Failed to obtain the authentication token. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_TIME_OUT,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid timeout parameter.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_AUTH_TYPE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid authentication type.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_CHALLENGE_TYPE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid challenge type.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_ACCESS_TYPE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid access type.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_AUTH_TOKEN,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid authentication token.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_SECURE_SIGN_TYPE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ILLEGAL_ARGUMENT,
            .errorMsg = "Invalid secure sign type.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_API_NOT_SUPPORTED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_NOT_SUPPORTED_API,
            .errorMsg = "This API is not supported.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_NOT_SUPPORTED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FEATURE_NOT_SUPPORTED,
            .errorMsg = "The feature is not support.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_USER_AUTH_TYPE_NOT_SUPPORT,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FEATURE_NOT_SUPPORTED,
            .errorMsg = "The user authentication type is not supported.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_ALG_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the algorithm. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_KEY_SIZE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the key size. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_PADDING_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the padding algorithm. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_PURPOSE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the key purpose. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_DIGEST_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the digest algorithm. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_MODE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the cipher mode. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_NONCE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the nonce. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_AAD_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the AAD. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_IV_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the IV. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_AE_TAG_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the AEAD. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_SALT_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the salt value. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CHECK_GET_ITERATION_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_MISSING_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Failed to obtain the number of iterations. It is not set in ParamSet.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_ALGORITHM,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid algorithm.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_KEY_SIZE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid key size.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_PADDING,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid padding algorithm.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_PURPOSE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid key purpose.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_MODE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid cipher mode.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_DIGEST,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid digest algorithm.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_SIGNATURE_SIZE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid signature size.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_IV,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid IV.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_AAD,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid AAD.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_NONCE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid nonce.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_AE_TAG,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid AE.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_SALT,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid salt value.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_ITERATION,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid iteration count.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_USAGE_OF_KEY,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INVALID_CRYPTO_ALG_ARGUMENT,
            .errorMsg = "Invalid key purpose.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_STORAGE_FAILURE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Insufficient storage space.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_FILE_SIZE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Invalid file size.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_READ_FILE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Failed to read the file.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_WRITE_FILE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Failed to write the file.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_REMOVE_FILE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Failed to remove the file.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_OPEN_FILE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Failed to open the file.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CLOSE_FILE_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Failed to close the file.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_MAKE_DIR_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Failed to create the directory.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INVALID_KEY_FILE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_FILE_OPERATION_FAIL,
            .errorMsg = "Failed to read the key from an invalid key file.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_IPC_MSG_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_COMMUNICATION_FAIL,
            .errorMsg = "Failed to get message from IPC.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_COMMUNICATION_TIMEOUT,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_COMMUNICATION_FAIL,
            .errorMsg = "IPC timed out.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_IPC_INIT_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_COMMUNICATION_FAIL,
            .errorMsg = "IPC initialization failed.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_UNKNOWN_ERROR,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_COMMUNICATION_FAIL,
            .errorMsg = "IPC async call failed.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CRYPTO_ENGINE_ERROR,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_CRYPTO_FAIL,
            .errorMsg = "Crypto engine error.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_KEY_AUTH_PERMANENTLY_INVALIDATED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_KEY_AUTH_PERMANENTLY_INVALIDATED,
            .errorMsg = "This credential is invalidated permanently.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_KEY_AUTH_VERIFY_FAILED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_KEY_AUTH_VERIFY_FAILED,
            .errorMsg = "The authentication token verification failed.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_KEY_AUTH_TIME_OUT,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_KEY_AUTH_TIME_OUT,
            .errorMsg = "This authentication token timed out.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_SESSION_REACHED_LIMIT,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_SESSION_LIMIT,
            .errorMsg = "The number of key operation sessions has reached the limit.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_NOT_EXIST,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_ITEM_NOT_EXIST,
            .errorMsg = "The entity does not exist.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_FAILURE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_EXTERNAL_ERROR,
            .errorMsg = "System external error.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_BAD_STATE,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_EXTERNAL_ERROR,
            .errorMsg = "System external error.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INTERNAL_ERROR,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_EXTERNAL_ERROR,
            .errorMsg = "System external error.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_CREDENTIAL_NOT_EXIST,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_CREDENTIAL_NOT_EXIST,
            .errorMsg = "The credential does not exist.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_INSUFFICIENT_MEMORY,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INSUFFICIENT_MEMORY,
            .errorMsg = "Insufficient memory.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_MALLOC_FAIL,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_INSUFFICIENT_MEMORY,
            .errorMsg = "Malloc failed.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_GET_USERIAM_SECINFO_FAILED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_CALL_SERVICE_FAILED,
            .errorMsg = "Failed to obtain the security information via UserIAM.",
            .data = NULL
        }
    }, {
        .innerErrCode = HKS_ERROR_GET_USERIAM_AUTHINFO_FAILED,
        .hksResult = {
            .errorCode = HUKS_ERR_CODE_CALL_SERVICE_FAILED,
            .errorMsg = "Failed to obtain the authentication information via UserIAM.",
            .data = NULL
        }
    }
};

/**
 * Convert ErrCode.
 * Convert internal error code to formal error code and return.
 * Return HUKS_ERR_CODE_EXTERNAL_ERROR in case of converting failed.
 */
struct HksResult HksConvertErrCode(int32_t ret)
{
    struct HksResult result = {HUKS_ERR_CODE_EXTERNAL_ERROR, g_convertErrMsg, NULL};
    uint32_t i = 0;
    uint32_t uErrCodeCount = sizeof(g_errCodeTable) / sizeof(g_errCodeTable[0]);
    for (; i < uErrCodeCount; ++i) {
        if (ret == g_errCodeTable[i].innerErrCode) {
            return g_errCodeTable[i].hksResult;
        }
    }
    return result;
}

