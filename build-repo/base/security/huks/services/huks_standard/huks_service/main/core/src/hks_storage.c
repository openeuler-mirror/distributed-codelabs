/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef _CUT_AUTHENTICATE_

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#include "hks_storage.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "hks_file_operator.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_storage_file_lock.h"
#include "hks_template.h"
#include "securec.h"

#define HKS_ENCODE_OFFSET_LEN         6
#define HKS_ENCODE_KEY_SALT_VALUE     0x3f
#define HKS_STORAGE_BAK_FLAG_TRUE     1
#define HKS_STORAGE_BAK_FLAG_FLASE    0
#define KEY_ALIAS_ANONYMOUS_LEN       4
#define KEY_ALIAS_SUFFIX_LEN          4
#define USER_ID_ROOT                  "0"

enum KeyOperation {
    KEY_OPERATION_SAVE = 0,
    KEY_OPERATION_GET = 1,
    KEY_OPERATION_DELETE = 2,
};

static bool g_setRootMainPath = false;
static bool g_setRootBakPath = false;
static char g_keyStoreMainPath[HKS_MAX_FILE_NAME_LEN + 1] = {0};
static char g_keyStoreBakPath[HKS_MAX_FILE_NAME_LEN + 1] = {0};

struct HksFileEntry {
    char *fileName;
    uint32_t fileNameLen;
};

#ifdef HKS_SUPPORT_POSIX
static void ConstructInvalidCharacter(const char input, char *output)
{
    switch (input) {
        case ':':
            *output = '#';
            return;
        case '<':
            *output = '$';
            return;
        case '>':
            *output = '%';
            return;
        case '?':
            *output = '&';
            return;
        case '\\':
            *output = '(';
            return;
        case '|':
            *output = ')';
            return;
        default:
            *output = input;
            return;
    }
}
#endif

static void ResumeInvalidCharacter(const char input, char *output)
{
    switch (input) {
        case '#':
            *output = ':';
            return;
        case '$':
            *output = '<';
            return;
        case '%':
            *output = '>';
            return;
        case '&':
            *output = '?';
            return;
        case '(':
            *output = '\\';
            return;
        case ')':
            *output = '|';
            return;
        default:
            *output = input;
            return;
    }
}

/* Encode invisible content to visible */
static int32_t ConstructName(const struct HksBlob *blob, char *targetName, uint32_t nameLen)
{
    uint32_t count = 0;

    for (uint32_t i = 0; i < blob->size; ++i) {
        if (count >= (nameLen - 1)) { /* nameLen can be guaranteed to be greater than 1 */
            return HKS_ERROR_INSUFFICIENT_DATA;
        }

        if ((blob->data[i] < '0') || (blob->data[i] > '~')) {
            targetName[count++] = '+' + (blob->data[i] >> HKS_ENCODE_OFFSET_LEN);
            targetName[count++] = '0' + (blob->data[i] & HKS_ENCODE_KEY_SALT_VALUE);
        } else {
            targetName[count++] = blob->data[i];
        }

#ifdef HKS_SUPPORT_POSIX
        ConstructInvalidCharacter(targetName[count - 1], &targetName[count - 1]);
#endif
    }

    return HKS_SUCCESS;
}

/* Decode encoded content to original content */
static int32_t ConstructBlob(const char *src, struct HksBlob *blob)
{
    uint32_t size = strlen(src);
    uint8_t *outputBlob = (uint8_t *)HksMalloc(size);
    HKS_IF_NULL_LOGE_RETURN(outputBlob, HKS_ERROR_MALLOC_FAIL, "malloc failed")

    uint32_t count = 0;
    int32_t ret = HKS_SUCCESS;
    for (uint32_t i = 0; i < size; ++i) {
        if ((src[i] >= '+') && (src[i] <= '.')) {
            uint8_t c = (uint8_t)(src[i] - '+') << HKS_ENCODE_OFFSET_LEN;
            i++;
            if (i >= size) {
                ret = HKS_ERROR_INVALID_KEY_FILE; /* keyfile name invalid */
                break;
            }
            char tmp;
            ResumeInvalidCharacter(src[i], &tmp);
            c += tmp - '0';
            outputBlob[count++] = c;
        } else {
            char tmp;
            ResumeInvalidCharacter(src[i], &tmp);
            outputBlob[count++] = tmp;
        }
    }

    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(outputBlob);
        return ret;
    }

    if (blob->size < count) {
        HKS_FREE_PTR(outputBlob);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memcpy_s(blob->data, blob->size, outputBlob, count);

    blob->size = count;
    HKS_FREE_PTR(outputBlob);
    return ret;
}

static int32_t GetPath(const char *path, const char *name, char *targetPath, uint32_t pathLen, uint32_t bakFlag)
{
    if (strncpy_s(targetPath, pathLen, path, strlen(path)) != EOK) {
        HKS_LOG_E("strncpy path failed");
        return HKS_ERROR_BAD_STATE;
    }

    if (targetPath[strlen(targetPath) - 1] != '/') {
        if (strncat_s(targetPath, pathLen, "/", strlen("/")) != EOK) {
            HKS_LOG_E("strncat slash failed");
            return HKS_ERROR_INTERNAL_ERROR;
        }
    }

    if (strncat_s(targetPath, pathLen, name, strlen(name)) != EOK) {
        HKS_LOG_E("strncat Name failed");
        return HKS_ERROR_BAD_STATE;
    }

    if (bakFlag == HKS_STORAGE_BAK_FLAG_TRUE) {
        if (strncat_s(targetPath, pathLen, ".bak", strlen(".bak")) != EOK) {
            HKS_LOG_E("strncat bak failed");
            return HKS_ERROR_BAD_STATE;
        }
    }

    return HKS_SUCCESS;
}

static int32_t GetFullPath(const char *path, const char *processName, const char *storeName,
    struct HksStoreFileInfo *fileInfo)
{
    int32_t ret = GetPath(path, processName, fileInfo->mainPath.processPath, fileInfo->mainPath.size,
        HKS_STORAGE_BAK_FLAG_FLASE);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get processPath failed, ret = %" LOG_PUBLIC "d.", ret)

    return GetPath(fileInfo->mainPath.processPath, storeName, fileInfo->mainPath.path, fileInfo->mainPath.size,
        HKS_STORAGE_BAK_FLAG_FLASE);
}

static int32_t GetStoreRootPath(enum HksStoragePathType type, char **path)
{
    if (type == HKS_STORAGE_MAIN_PATH) {
        if (!g_setRootMainPath) {
            uint32_t len = sizeof(g_keyStoreMainPath);
            int32_t ret = HksGetStoragePath(HKS_STORAGE_MAIN_PATH, g_keyStoreMainPath, &len);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get mainStorage path fail")

            g_setRootMainPath = true;
        }
        *path = g_keyStoreMainPath;
    } else if (type == HKS_STORAGE_BACKUP_PATH) {
        if (!g_setRootBakPath) {
            uint32_t len = sizeof(g_keyStoreBakPath);
            int32_t ret = HksGetStoragePath(HKS_STORAGE_BACKUP_PATH, g_keyStoreBakPath, &len);
            HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get backup storage path fail")

            g_setRootBakPath = true;
        }
        *path = g_keyStoreBakPath;
    } else {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t MakeDirIfNotExist(const char *path)
{
    int32_t ret = HksIsDirExist(path);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("dir not exist, path = %" LOG_PUBLIC "s", path);
        ret = HksMakeDir(path);
        if (ret != HKS_SUCCESS) {
            if (ret == HKS_ERROR_ALREADY_EXISTS) {
                ret = HKS_SUCCESS;
            } else {
                HKS_LOG_E("make dir failed");
                ret = HKS_ERROR_MAKE_DIR_FAIL;
            }
        }
    }
    return ret;
}

#ifdef HKS_SUPPORT_THREAD
static HksStorageFileLock *CreateStorageFileLock(const char *path, const char *fileName)
{
    char *fullPath = HksMalloc(HKS_MAX_FILE_NAME_LEN);
    HKS_IF_NULL_RETURN(fullPath, NULL)

    int32_t ret = HksGetFileName(path, fileName, fullPath, HKS_MAX_FILE_NAME_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get full path failed, ret = %" LOG_PUBLIC "d.", ret);
        HksFree(fullPath);
        return NULL;
    }

    HksStorageFileLock *lock = HksStorageFileLockCreate(fullPath);
    HksFree(fullPath);
    return lock;
}
#endif

static int32_t HksStorageWriteFile(
    const char *path, const char *fileName, uint32_t offset, const uint8_t *buf, uint32_t len)
{
#ifdef HKS_SUPPORT_THREAD
    HksStorageFileLock *lock = CreateStorageFileLock(path, fileName);
    HksStorageFileLockWrite(lock);
    int32_t ret = HksFileWrite(path, fileName, offset, buf, len);
    HksStorageFileUnlockWrite(lock);
    HksStorageFileLockRelease(lock);
    return ret;
#else
    return HksFileWrite(path, fileName, offset, buf, len);
#endif
}

static uint32_t HksStorageReadFile(
    const char *path, const char *fileName, uint32_t offset, uint8_t *buf, uint32_t len)
{
#ifdef HKS_SUPPORT_THREAD
    HksStorageFileLock *lock = CreateStorageFileLock(path, fileName);
    HksStorageFileLockRead(lock);
    uint32_t size = HksFileRead(path, fileName, offset, buf, len);
    HksStorageFileUnlockRead(lock);
    HksStorageFileLockRelease(lock);
#else
    uint32_t size = HksFileRead(path, fileName, offset, buf, len);
#endif
    return size;
}

static int32_t HksStorageRemoveFile(const char *path, const char *fileName)
{
#ifdef HKS_SUPPORT_THREAD
    HksStorageFileLock *lock = CreateStorageFileLock(path, fileName);
    HksStorageFileLockWrite(lock);
    int32_t ret = HksFileRemove(path, fileName);
    HksStorageFileUnlockWrite(lock);
    HksStorageFileLockRelease(lock);
#else
    int32_t ret = HksFileRemove(path, fileName);
#endif
    return ret;
}

#ifdef SUPPORT_STORAGE_BACKUP
static int32_t GetBakFullPath(const char *path, const char *processName, const char *storeName,
    struct HksStoreFileInfo *fileInfo)
{
    int32_t ret = GetPath(path, processName, fileInfo->bakPath.processPath, fileInfo->bakPath.size,
        HKS_STORAGE_BAK_FLAG_TRUE);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get bakProcessPath failed, ret = %" LOG_PUBLIC "d.", ret)

    return GetPath(fileInfo->bakPath.processPath, storeName, fileInfo->bakPath.path, fileInfo->bakPath.size,
        HKS_STORAGE_BAK_FLAG_TRUE);
}

static int32_t GetBakFileName(const char *fileName, char *bakFileName, uint32_t bakFileNameLen)
{
    if (strncpy_s(bakFileName, bakFileNameLen, fileName, strlen(fileName)) != EOK) {
        HKS_LOG_E("strncpy_s fileName failed");
        return HKS_ERROR_BAD_STATE;
    }
    if (strncat_s(bakFileName, bakFileNameLen, ".bak", strlen(".bak")) != EOK) {
        HKS_LOG_E("strncat_s bak failed");
        return HKS_ERROR_BAD_STATE;
    }

    return HKS_SUCCESS;
}

static int32_t CopyKeyBlobFromSrc(const char *srcPath, const char *srcFileName,
    const char *destPath, const char *destFileName)
{
    uint32_t size = HksFileSize(srcPath, srcFileName);
    if (size == 0) {
        HKS_LOG_E("get file size failed, ret = %" LOG_PUBLIC "u.", size);
        return HKS_ERROR_FILE_SIZE_FAIL;
    }

    uint8_t *buffer = (uint8_t *)HksMalloc(size);
    HKS_IF_NULL_RETURN(buffer, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(buffer, size, 0, size);

    int32_t ret;
    do {
        size = HksStorageReadFile(srcPath, srcFileName, 0, buffer, size);
        if (size == 0) {
            HKS_LOG_E("read file failed, ret = %" LOG_PUBLIC "u.", size);
            ret = HKS_ERROR_READ_FILE_FAIL;
            break;
        }

        ret = MakeDirIfNotExist(destPath);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "makdir destPath failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = HksStorageWriteFile(destPath, destFileName, 0, buffer, size);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "file write destPath failed, ret = %" LOG_PUBLIC "d.", ret)
    } while (0);

    HKS_FREE_PTR(buffer);
    return ret;
}

static int32_t CopyKeyBlob(const struct HksStoreFileInfo *fileInfo,
    int32_t isMainFileExist, int32_t isBakFileExist)
{
    if ((isMainFileExist != HKS_SUCCESS) && (isBakFileExist != HKS_SUCCESS)) {
        return HKS_ERROR_NOT_EXIST;
    } else if ((isMainFileExist == HKS_SUCCESS) && (isBakFileExist == HKS_SUCCESS)) {
        return HKS_SUCCESS;
    }

    int32_t ret = HKS_SUCCESS;
    if (isMainFileExist != HKS_SUCCESS) {
        ret = MakeDirIfNotExist(fileInfo->mainPath.processPath);
        HKS_IF_NOT_SUCC_RETURN(ret, ret)

        ret = CopyKeyBlobFromSrc(fileInfo->bakPath.path, fileInfo->bakPath.fileName,
            fileInfo->mainPath.path, fileInfo->mainPath.fileName);
    } else if (isBakFileExist != HKS_SUCCESS) {
        ret = MakeDirIfNotExist(fileInfo->bakPath.processPath);
        HKS_IF_NOT_SUCC_RETURN(ret, ret)

        ret = CopyKeyBlobFromSrc(fileInfo->mainPath.path, fileInfo->mainPath.fileName,
            fileInfo->bakPath.path, fileInfo->bakPath.fileName);
    }

    return ret;
}
#endif

static int32_t GetKeyBlobFromFile(const char *path, const char *fileName, struct HksBlob *keyBlob)
{
    uint32_t size = HksFileSize(path, fileName);
    if (size == 0) {
        return HKS_ERROR_FILE_SIZE_FAIL;
    }

    if (keyBlob->size < size) {
        return HKS_ERROR_INSUFFICIENT_DATA;
    }

    size = HksStorageReadFile(path, fileName, 0, keyBlob->data, keyBlob->size);
    if (size == 0) {
        HKS_LOG_E("file read failed, ret = %" LOG_PUBLIC "u.", size);
        return HKS_ERROR_READ_FILE_FAIL;
    }

    keyBlob->size = size;
    return HKS_SUCCESS;
}

static int32_t SaveKeyBlob(const char *processPath, const char *path, const char *fileName,
    const struct HksBlob *keyBlob)
{
    int32_t ret = MakeDirIfNotExist(processPath);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "makedir processPath failed, ret = %" LOG_PUBLIC "d.", ret)

    ret = MakeDirIfNotExist(path);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "makedir path failed, ret = %" LOG_PUBLIC "d.", ret)

    return HksStorageWriteFile(path, fileName, 0, keyBlob->data, keyBlob->size);
}

static int32_t DeleteKeyBlob(const struct HksStoreFileInfo *fileInfo)
{
    int32_t isMainFileExist = HksIsFileExist(fileInfo->mainPath.path, fileInfo->mainPath.fileName);
#ifndef SUPPORT_STORAGE_BACKUP
    HKS_IF_NOT_SUCC_RETURN(isMainFileExist, HKS_ERROR_NOT_EXIST)

    int32_t ret = HksStorageRemoveFile(fileInfo->mainPath.path, fileInfo->mainPath.fileName);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "delete key remove file failed, ret = %" LOG_PUBLIC "d.", ret)
#else
    int32_t isBakFileExist = HksIsFileExist(fileInfo->bakPath.path, fileInfo->bakPath.fileName);
    if ((isMainFileExist != HKS_SUCCESS) && (isBakFileExist != HKS_SUCCESS)) {
        return HKS_ERROR_NOT_EXIST;
    }

    int32_t ret = HKS_SUCCESS;
    if (isMainFileExist == HKS_SUCCESS) {
        ret = HksStorageRemoveFile(fileInfo->mainPath.path, fileInfo->mainPath.fileName);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "delete key remove file failed, ret = %" LOG_PUBLIC "d.", ret)
    }

    if (isBakFileExist == HKS_SUCCESS) {
        ret = HksStorageRemoveFile(fileInfo->bakPath.path, fileInfo->bakPath.fileName);
        HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "delete key remove bakfile failed, ret = %" LOG_PUBLIC "d.", ret)
    }
#endif

    return ret;
}

static int32_t GetKeyBlob(const struct HksStoreFileInfo *fileInfo, struct HksBlob *keyBlob)
{
    int32_t isMainFileExist = HksIsFileExist(fileInfo->mainPath.path, fileInfo->mainPath.fileName);
#ifndef SUPPORT_STORAGE_BACKUP
    HKS_IF_NOT_SUCC_RETURN(isMainFileExist, HKS_ERROR_NOT_EXIST)

    int32_t ret = GetKeyBlobFromFile(fileInfo->mainPath.path, fileInfo->mainPath.fileName, keyBlob);
#else
    int32_t isBakFileExist = HksIsFileExist(fileInfo->bakPath.path, fileInfo->bakPath.fileName);
    if ((isMainFileExist != HKS_SUCCESS) && (isBakFileExist != HKS_SUCCESS)) {
        return HKS_ERROR_NOT_EXIST;
    }

    int32_t ret = HKS_SUCCESS;
    if (isMainFileExist == HKS_SUCCESS) {
        ret = GetKeyBlobFromFile(fileInfo->mainPath.path, fileInfo->mainPath.fileName, keyBlob);
    } else if (isBakFileExist == HKS_SUCCESS) {
        ret = GetKeyBlobFromFile(fileInfo->bakPath.path, fileInfo->bakPath.fileName, keyBlob);
    }

    if (CopyKeyBlob(fileInfo, isMainFileExist, isBakFileExist) != HKS_SUCCESS) {
        HKS_LOG_W("CopyKeyBlob failed");
    }
#endif

    return ret;
}

static int32_t GetKeyBlobSize(const struct HksStoreFileInfo *fileInfo, uint32_t *keyBlobSize)
{
    int32_t isMainFileExist = HksIsFileExist(fileInfo->mainPath.path, fileInfo->mainPath.fileName);
#ifndef SUPPORT_STORAGE_BACKUP
    HKS_IF_NOT_SUCC_RETURN(isMainFileExist, HKS_ERROR_NOT_EXIST)

    uint32_t size = HksFileSize(fileInfo->mainPath.path, fileInfo->mainPath.fileName);
    if (size == 0) {
        return HKS_ERROR_FILE_SIZE_FAIL;
    }
    *keyBlobSize = size;
#else
    int32_t isBakFileExist = HksIsFileExist(fileInfo->bakPath.path, fileInfo->bakPath.fileName);
    if ((isMainFileExist != HKS_SUCCESS) && (isBakFileExist != HKS_SUCCESS)) {
        return HKS_ERROR_NOT_EXIST;
    }

    uint32_t size = 0;
    if (isMainFileExist == HKS_SUCCESS) {
        size = HksFileSize(fileInfo->mainPath.path, fileInfo->mainPath.fileName);
    } else if (isBakFileExist == HKS_SUCCESS) {
        size = HksFileSize(fileInfo->bakPath.path, fileInfo->bakPath.fileName);
    }

    if (size == 0) {
        return HKS_ERROR_FILE_SIZE_FAIL;
    }

    *keyBlobSize = size;
    if (CopyKeyBlob(fileInfo, isMainFileExist, isBakFileExist) != HKS_SUCCESS) {
        HKS_LOG_W("CopyKeyBlob failed");
    }
#endif

    return HKS_SUCCESS;
}

static int32_t IsKeyBlobExist(const struct HksStoreFileInfo *fileInfo)
{
    int32_t isMainFileExist = HksIsFileExist(fileInfo->mainPath.path, fileInfo->mainPath.fileName);
#ifndef SUPPORT_STORAGE_BACKUP
    HKS_IF_NOT_SUCC_RETURN(isMainFileExist, HKS_ERROR_NOT_EXIST)
#else
    int32_t isBakFileExist = HksIsFileExist(fileInfo->bakPath.path, fileInfo->bakPath.fileName);
    if ((isMainFileExist != HKS_SUCCESS) && (isBakFileExist != HKS_SUCCESS)) {
        return HKS_ERROR_NOT_EXIST;
    }

    if (CopyKeyBlob(fileInfo, isMainFileExist, isBakFileExist) != HKS_SUCCESS) {
        HKS_LOG_W("CopyKeyBlob failed");
    }
#endif

    return HKS_SUCCESS;
}

static int32_t DataInit(char **data, uint32_t size)
{
    *data = (char *)HksMalloc(size);
    HKS_IF_NULL_RETURN(*data, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(*data, size, 0, size);
    return HKS_SUCCESS;
}

static int32_t FileInfoInit(struct HksStoreFileInfo *fileInfo)
{
    fileInfo->mainPath.size = HKS_MAX_FILE_NAME_LEN;
    /* if one param init fail, others free by caller function */
    int32_t ret = DataInit(&fileInfo->mainPath.processPath, fileInfo->mainPath.size);
    ret += DataInit(&fileInfo->mainPath.path, fileInfo->mainPath.size);
    ret += DataInit(&fileInfo->mainPath.fileName, fileInfo->mainPath.size);

#ifdef SUPPORT_STORAGE_BACKUP
    fileInfo->bakPath.size = HKS_MAX_FILE_NAME_LEN;
    ret += DataInit(&fileInfo->bakPath.processPath, fileInfo->bakPath.size);
    ret += DataInit(&fileInfo->bakPath.path, fileInfo->bakPath.size);
    ret += DataInit(&fileInfo->bakPath.fileName, fileInfo->bakPath.size);
#endif

    return ret;
}

static void FileInfoFree(struct HksStoreFileInfo *fileInfo)
{
    HKS_FREE_PTR(fileInfo->mainPath.processPath);
    HKS_FREE_PTR(fileInfo->mainPath.path);
    HKS_FREE_PTR(fileInfo->mainPath.fileName);
    fileInfo->mainPath.size = 0;

#ifdef SUPPORT_STORAGE_BACKUP
    HKS_FREE_PTR(fileInfo->bakPath.processPath);
    HKS_FREE_PTR(fileInfo->bakPath.path);
    HKS_FREE_PTR(fileInfo->bakPath.fileName);
    fileInfo->bakPath.size = 0;
#endif
}

static int32_t MakeSubPath(const char *mainPath, const char *tmpPath, char *outPath, uint32_t outPathLen)
{
    if (strncpy_s(outPath, outPathLen, mainPath, strlen(mainPath)) != EOK) {
        return HKS_ERROR_INTERNAL_ERROR;
    }
    if (strncat_s(outPath, outPathLen, "/", strlen("/")) != EOK) {
        return HKS_ERROR_INTERNAL_ERROR;
    }
    if (strncat_s(outPath, outPathLen, tmpPath, strlen(tmpPath)) != EOK) {
        return HKS_ERROR_INTERNAL_ERROR;
    }
    HKS_LOG_E("MakeSubPath outPath = %" LOG_PUBLIC "s", outPath);
    return HKS_SUCCESS;
}

static int32_t MakeUserAndProcessNamePath(const char *mainRootPath, char *userProcess,
    int32_t processLen, const struct HksProcessInfo *processInfo)
{
    char workPath[HKS_MAX_DIRENT_FILE_LEN] = "";
    char *user = (char *)HksMalloc(HKS_PROCESS_INFO_LEN);
    HKS_IF_NULL_RETURN(user, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(user, HKS_PROCESS_INFO_LEN, 0, HKS_PROCESS_INFO_LEN);

    int32_t ret = ConstructName(&processInfo->userId, user, HKS_PROCESS_INFO_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("construct name failed, ret = %" LOG_PUBLIC "d.", ret);
        HksFree(user);
        return ret;
    }

    if (memcpy_s(userProcess, processLen, user, strlen(user)) != EOK) {
        HksFree(user);
        return HKS_ERROR_INSUFFICIENT_MEMORY;
    }
    HksFree(user);

    ret = MakeSubPath(mainRootPath, userProcess, workPath, HKS_MAX_DIRENT_FILE_LEN);
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_INTERNAL_ERROR)

    ret = MakeDirIfNotExist(workPath);
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_NO_PERMISSION)

    if (strncat_s(userProcess, HKS_PROCESS_INFO_LEN, "/", strlen("/")) != EOK) {
        return HKS_ERROR_INTERNAL_ERROR;
    }
    if (strncat_s(userProcess, HKS_PROCESS_INFO_LEN, (const char *)processInfo->processName.data,
        processInfo->processName.size) != EOK) {
        return HKS_ERROR_INTERNAL_ERROR;
    }

    ret = MakeSubPath(mainRootPath, userProcess, workPath, HKS_MAX_DIRENT_FILE_LEN);
    HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_INTERNAL_ERROR)

    HKS_IF_NOT_SUCC_RETURN(MakeDirIfNotExist(workPath), HKS_ERROR_NO_PERMISSION)
    return HKS_SUCCESS;
}

#ifdef SUPPORT_STORAGE_BACKUP
static int32_t GetStoreBakPath(const struct HksProcessInfo *processInfo,
    const char *storageName, struct HksStoreFileInfo *fileInfo, char *userProcess, char *workPath)
{
    int32_t ret;
    char *bakRootPath = NULL;
    ret = GetStoreRootPath(HKS_STORAGE_BACKUP_PATH, &bakRootPath);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get root path failed")

    ret = MakeDirIfNotExist(bakRootPath);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, HKS_ERROR_NO_PERMISSION, "makedir backup path failed")

    (void)memset_s(workPath, HKS_MAX_DIRENT_FILE_LEN, 0, HKS_MAX_DIRENT_FILE_LEN);

    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(&processInfo->userId), HKS_ERROR_NULL_POINTER, "processInfo->userId is NULL")

    if ((processInfo->userId.size == strlen(USER_ID_ROOT)) &&
        (HksMemCmp(processInfo->userId.data, USER_ID_ROOT, sizeof(USER_ID_ROOT)) == 0)) {
        if (memcpy_s(userProcess, HKS_PROCESS_INFO_LEN, processInfo->processName.data,
            processInfo->processName.size) != EOK) {
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }

        ret = MakeSubPath(bakRootPath, userProcess, workPath, HKS_MAX_DIRENT_FILE_LEN);
        HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_INTERNAL_ERROR)

        ret = MakeDirIfNotExist(workPath);
        HKS_IF_NOT_SUCC_RETURN(ret, ret)
    } else {
        ret = MakeUserAndProcessNamePath(bakRootPath, userProcess, HKS_PROCESS_INFO_LEN, processInfo);
        HKS_IF_NOT_SUCC_RETURN(ret HKS_ERROR_INTERNAL_ERROR)
    }

    ret = GetBakFullPath(bakRootPath, userProcess, storageName, fileInfo);
    HKS_IF_NOT_SUCC_LOGE(ret, "get backup full path failed, ret = %" LOG_PUBLIC "d.", ret)
    return ret;
}
#endif

static int32_t GetStorePath(const struct HksProcessInfo *processInfo,
    const char *storageName, struct HksStoreFileInfo *fileInfo)
{
    char *mainRootPath = NULL;
    char userProcess[HKS_PROCESS_INFO_LEN] = "";
    char workPath[HKS_MAX_DIRENT_FILE_LEN] = "";

    int32_t ret = GetStoreRootPath(HKS_STORAGE_MAIN_PATH, &mainRootPath);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get root path failed")

    ret = MakeDirIfNotExist(mainRootPath);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "makedir main path failed")

    HKS_IF_NOT_SUCC_LOGE_RETURN(CheckBlob(&processInfo->userId), HKS_ERROR_NULL_POINTER, "processInfo->userId is NULL")

    if ((processInfo->userId.size == strlen(USER_ID_ROOT)) &&
        (HksMemCmp(processInfo->userId.data, USER_ID_ROOT, strlen(USER_ID_ROOT)) == 0)) {
        if (memcpy_s(userProcess, HKS_PROCESS_INFO_LEN, processInfo->processName.data,
            processInfo->processName.size) != EOK) {
            return HKS_ERROR_INSUFFICIENT_MEMORY;
        }

        ret = MakeSubPath(mainRootPath, userProcess, workPath, HKS_MAX_DIRENT_FILE_LEN);
        HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_INTERNAL_ERROR)

        ret = MakeDirIfNotExist(workPath);
        HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_NO_PERMISSION)
    } else {
        ret = MakeUserAndProcessNamePath(mainRootPath, userProcess, HKS_PROCESS_INFO_LEN, processInfo);
        HKS_IF_NOT_SUCC_RETURN(ret, HKS_ERROR_INTERNAL_ERROR)
    }

    ret = GetFullPath(mainRootPath, userProcess, storageName, fileInfo);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get full path failed, ret = %" LOG_PUBLIC "d.", ret)

#ifdef SUPPORT_STORAGE_BACKUP
    ret = GetStoreBakPath(processInfo, storageName, fileInfo, userProcess, workPath);
#endif

    return ret;
}

static void SaveProcessInfo(uint8_t *processData, int32_t dataLen,
    const struct HksBlob userData, struct HksProcessInfo *processInfo)
{
    processInfo->processName.data = processData;
    processInfo->processName.size = (uint32_t)dataLen;
    (void)memcpy_s(&processInfo->userId, sizeof(struct HksBlob), &userData, sizeof(struct HksBlob));
}

static int32_t GetFileInfo(const struct HksProcessInfo *processInfo,
    const struct HksBlob *keyAlias, uint32_t storageType, struct HksStoreFileInfo *fileInfo)
{
    int32_t ret = FileInfoInit(fileInfo);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "hks file info init failed, ret = %" LOG_PUBLIC "d.", ret)

    char *name = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
    HKS_IF_NULL_RETURN(name, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(name, HKS_MAX_FILE_NAME_LEN, 0, HKS_MAX_FILE_NAME_LEN);

    ret = ConstructName(&processInfo->processName, name, HKS_MAX_FILE_NAME_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("construct name failed, ret = %" LOG_PUBLIC "d.", ret);
        HKS_FREE_PTR(name);
        return ret;
    }

    ret = HKS_ERROR_NOT_SUPPORTED;
    struct HksProcessInfo info;
    SaveProcessInfo((uint8_t *)name, strlen(name), processInfo->userId, &info);
    if (storageType == HKS_STORAGE_TYPE_KEY) {
        ret = GetStorePath(&info, HKS_KEY_STORE_KEY_PATH, fileInfo);
    } else if (storageType == HKS_STORAGE_TYPE_CERTCHAIN) {
        ret = GetStorePath(&info, HKS_KEY_STORE_CERTCHAIN_PATH, fileInfo);
    } else if (storageType == HKS_STORAGE_TYPE_ROOT_KEY) {
        ret = GetStorePath(&info, HKS_KEY_STORE_ROOT_KEY_PATH, fileInfo);
    }
    HKS_FREE_PTR(name);

    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get store path failed, ret = %" LOG_PUBLIC "d.", ret)

    ret = ConstructName(keyAlias, fileInfo->mainPath.fileName, fileInfo->mainPath.size);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "construct name failed, ret = %" LOG_PUBLIC "d.", ret)

#ifdef SUPPORT_STORAGE_BACKUP
    ret = GetBakFileName(fileInfo->mainPath.fileName, fileInfo->bakPath.fileName, fileInfo->bakPath.size);
    HKS_IF_NOT_SUCC_LOGE(ret, "get backup file name failed, ret = %" LOG_PUBLIC "d.", ret)
#endif

    return ret;
}

int32_t HksGetFileInfo(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias, uint32_t storageType,
    struct HksStoreFileInfo *fileInfo)
{
    return GetFileInfo(processInfo, keyAlias, storageType, fileInfo);
}

/*
 * keyAlias: xxxxxxxxxxxxxxxxxxx********************xxxxxxxxxxxxxxxxxx
 *                              |<- anonymous len ->||<- suffix len ->|
 *           |<----------------- keyAlias len ----------------------->|
 */
static int32_t RecordKeyOperation(uint32_t operation, const char *path, const char *keyAlias)
{
    uint32_t bufSize = strlen(keyAlias) + 1;
    char *outKeyAlias = (char *)HksMalloc(bufSize);
    HKS_IF_NULL_RETURN(outKeyAlias, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(outKeyAlias, bufSize, 0, bufSize);

    uint32_t keyAliasLen = strlen(keyAlias);
    for (uint32_t i = 0; i < keyAliasLen; ++i) {
        if ((keyAliasLen < (i + 1 + KEY_ALIAS_ANONYMOUS_LEN + KEY_ALIAS_SUFFIX_LEN)) &&
            ((i + 1 + KEY_ALIAS_SUFFIX_LEN) <= keyAliasLen)) {
            outKeyAlias[i] = '*';
        } else {
            outKeyAlias[i] = keyAlias[i];
        }
    }
    outKeyAlias[keyAliasLen] = '\0';

    int32_t ret = HKS_SUCCESS;
    switch (operation) {
        case KEY_OPERATION_SAVE:
            HKS_LOG_I("generate key or certchain, storage path: %" LOG_PUBLIC "s, key alias: %" LOG_PUBLIC "s",
                path, outKeyAlias);
            break;
        case KEY_OPERATION_GET:
            HKS_LOG_I("use key, storage path: %" LOG_PUBLIC "s, key alias: %" LOG_PUBLIC "s", path, outKeyAlias);
            break;
        case KEY_OPERATION_DELETE:
            HKS_LOG_I("delete key or certchain, storage path: %" LOG_PUBLIC "s, key alias: %" LOG_PUBLIC "s",
                path, outKeyAlias);
            break;
        default:
            ret = HKS_ERROR_INVALID_ARGUMENT;
    }

    HKS_FREE_PTR(outKeyAlias);
    return ret;
}

int32_t HksStoreKeyBlob(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    enum HksStorageType storageType, const struct HksBlob *keyBlob)
{
    /* params have been checked by caller functions */
    struct HksStoreFileInfo fileInfo = { 0 };

    int32_t ret;
    do {
        ret = GetFileInfo(processInfo, keyAlias, storageType, &fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks get file info failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = RecordKeyOperation(KEY_OPERATION_SAVE, fileInfo.mainPath.path, fileInfo.mainPath.fileName);
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = SaveKeyBlob(fileInfo.mainPath.processPath, fileInfo.mainPath.path, fileInfo.mainPath.fileName, keyBlob);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks save key blob failed, ret = %" LOG_PUBLIC "d.", ret)

#ifdef SUPPORT_STORAGE_BACKUP
        ret = SaveKeyBlob(fileInfo.bakPath.processPath, fileInfo.bakPath.path, fileInfo.bakPath.fileName, keyBlob);
        HKS_IF_NOT_SUCC_LOGE(ret, "hks save bak key blob failed, ret = %" LOG_PUBLIC "d.", ret)
#endif
    } while (0);

    FileInfoFree(&fileInfo);
    return ret;
}

int32_t HksStoreDeleteKeyBlob(const struct HksProcessInfo *processInfo,
    const struct HksBlob *keyAlias, uint32_t storageType)
{
    /* params have been checked by caller functions */
    struct HksStoreFileInfo fileInfo = { 0 };

    int32_t ret;
    do {
        ret = GetFileInfo(processInfo, keyAlias, storageType, &fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks get file info failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = RecordKeyOperation(KEY_OPERATION_DELETE, fileInfo.mainPath.path, fileInfo.mainPath.fileName);
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = DeleteKeyBlob(&fileInfo);
    } while (0);

    FileInfoFree(&fileInfo);
    return ret;
}

int32_t HksStoreIsKeyBlobExist(const struct HksProcessInfo *processInfo,
    const struct HksBlob *keyAlias, uint32_t storageType)
{
    /* params have been checked by caller functions */
    struct HksStoreFileInfo fileInfo = { 0 };

    int32_t ret;
    do {
        ret = GetFileInfo(processInfo, keyAlias, storageType, &fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks get file info failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = IsKeyBlobExist(&fileInfo);
        HKS_IF_NOT_SUCC_LOGE(ret, "check is key exist, ret = %" LOG_PUBLIC "d.", ret)
    } while (0);

    FileInfoFree(&fileInfo);
    return ret;
}

int32_t HksStoreGetKeyBlob(const struct HksProcessInfo *processInfo,
    const struct HksBlob *keyAlias, uint32_t storageType, struct HksBlob *keyBlob)
{
    /* params have been checked by caller functions */
    struct HksStoreFileInfo fileInfo = { 0 };

    int32_t ret;
    do {
        ret = GetFileInfo(processInfo, keyAlias, storageType, &fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks get file info failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = RecordKeyOperation(KEY_OPERATION_GET, fileInfo.mainPath.path, fileInfo.mainPath.fileName);
        HKS_IF_NOT_SUCC_BREAK(ret)

        ret = GetKeyBlob(&fileInfo, keyBlob);
        HKS_IF_NOT_SUCC_LOGE(ret, "hks get keyblob failed, ret = %" LOG_PUBLIC "d.", ret)
    } while (0);

    FileInfoFree(&fileInfo);
    return ret;
}

int32_t HksStoreGetKeyBlobSize(const struct HksProcessInfo *processInfo, const struct HksBlob *keyAlias,
    uint32_t storageType, uint32_t *keyBlobSize)
{
    /* params have been checked by caller functions */
    struct HksStoreFileInfo fileInfo = { 0 };

    int32_t ret;
    do {
        ret = GetFileInfo(processInfo, keyAlias, storageType, &fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks get file info failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = GetKeyBlobSize(&fileInfo, keyBlobSize);
        HKS_IF_NOT_SUCC_LOGE(ret, "hks get keyblob size failed, ret = %" LOG_PUBLIC "d.", ret)
    } while (0);

    FileInfoFree(&fileInfo);
    return ret;
}

static int32_t GetFileCount(const char *path, uint32_t *fileCount)
{
    if ((path == NULL) || (fileCount == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    void *dir = HksOpenDir(path);
    if (dir == NULL) {
        HKS_LOG_W("can't open directory");
        *fileCount = 0;
        return HKS_SUCCESS;
    }

    uint32_t count = 0;
    struct HksFileDirentInfo dire = {{0}};
    int32_t ret = HksGetDirFile(dir, &dire);
    while (ret == HKS_SUCCESS) {
        count++;
        ret = HksGetDirFile(dir, &dire);
    }
    (void)HksCloseDir(dir);
    *fileCount = count;

    return HKS_SUCCESS;
}

static int32_t GetFileNameList(const char *path, struct HksFileEntry *fileNameList, uint32_t *fileCount)
{
    if ((path == NULL) || (fileCount == NULL) || (fileNameList == NULL)) {
        return HKS_ERROR_NULL_POINTER;
    }

    void *dir = HksOpenDir(path);
    if (dir == NULL) {
        HKS_LOG_W("can't open directory");
        *fileCount = 0;
        return HKS_SUCCESS;
    }

    struct HksFileDirentInfo dire = {{0}};
    int32_t ret = HksGetDirFile(dir, &dire);
    uint32_t count = 0;
    while (ret == HKS_SUCCESS) {
        count++;
        uint32_t nameLen = strlen(dire.fileName);
        if ((*fileCount < count) || (fileNameList[count - 1].fileNameLen < (nameLen + 1))) {
            HKS_LOG_E("the input params are wrong and too small");
            break;
        }

        if (strncpy_s(fileNameList[count - 1].fileName, fileNameList[count - 1].fileNameLen,
            dire.fileName, nameLen) != EOK) {
            HKS_LOG_E("failed to copy the string");
            break;
        }
        fileNameList[count - 1].fileName[nameLen] = '\0';
        ret = HksGetDirFile(dir, &dire);
    }
    (void)HksCloseDir(dir);
    *fileCount = count;

    return HKS_SUCCESS;
}

static int32_t GetFilePath(const struct HksProcessInfo *processInfo,
    uint32_t storageType, struct HksStoreFileInfo *fileInfo)
{
    char *name = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
    HKS_IF_NULL_RETURN(name, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(name, HKS_MAX_FILE_NAME_LEN, 0, HKS_MAX_FILE_NAME_LEN);

    int32_t ret = ConstructName(&processInfo->processName, name, HKS_MAX_FILE_NAME_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("construct name failed, ret = %" LOG_PUBLIC "d.", ret);
        HKS_FREE_PTR(name);
        return ret;
    }

    struct HksProcessInfo info;
    SaveProcessInfo((uint8_t *)name, strlen(name), processInfo->userId, &info);
    if (storageType == HKS_STORAGE_TYPE_KEY) {
        ret = GetStorePath(&info, HKS_KEY_STORE_KEY_PATH, fileInfo);
    } else if (storageType == HKS_STORAGE_TYPE_CERTCHAIN) {
        ret = GetStorePath(&info, HKS_KEY_STORE_CERTCHAIN_PATH, fileInfo);
    } else {
        ret = HKS_ERROR_NOT_SUPPORTED;
    }

    HKS_FREE_PTR(name);
    return ret;
}

static void FileNameListFree(struct HksFileEntry **fileNameList, uint32_t keyCount)
{
    if (fileNameList != NULL && *fileNameList != NULL) {
        for (uint32_t i = 0; i < keyCount; ++i) {
            HKS_FREE_PTR((*fileNameList)[i].fileName);
        }
        HKS_FREE_PTR(*fileNameList);
    }
}

static int32_t FileNameListInit(struct HksFileEntry **fileNameList, uint32_t keyCount)
{
    if (((UINT32_MAX / (sizeof(struct HksFileEntry))) < keyCount) || (keyCount == 0)) {
        HKS_LOG_E("keyCount too big or is zero.");
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    uint32_t totalSize = keyCount * sizeof(struct HksFileEntry);
    *fileNameList = (struct HksFileEntry *)HksMalloc(totalSize);
    HKS_IF_NULL_LOGE_RETURN(*fileNameList, HKS_ERROR_MALLOC_FAIL, "malloc file name list failed.")

    (void)memset_s(*fileNameList, totalSize, 0, totalSize);
    int32_t ret = HKS_SUCCESS;

    for (uint32_t i = 0; i < keyCount; ++i) {
        (*fileNameList)[i].fileNameLen = HKS_MAX_FILE_NAME_LEN;
        (*fileNameList)[i].fileName = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
        if ((*fileNameList)[i].fileName == NULL) {
            HKS_LOG_E("malloc failed.");
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }
    }

    if (ret != HKS_SUCCESS) {
        FileNameListFree(fileNameList, keyCount);
    }
    return ret;
}

static int32_t GetAndCheckFileCount(const char *path, uint32_t *fileCount, const uint32_t *inputCount)
{
    int32_t ret = GetFileCount(path, fileCount);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get storage file count, ret = %" LOG_PUBLIC "d.", ret)

    if (*inputCount < *fileCount) {
        HKS_LOG_E("listCount space not enough");
        ret = HKS_ERROR_BUFFER_TOO_SMALL;
    }

    return ret;
}

static int32_t GetKeyAliasByProcessName(const struct HksStoreFileInfo *fileInfo, struct HksKeyInfo *keyInfoList,
    uint32_t *listCount)
{
    uint32_t fileCount;
    int32_t ret = GetAndCheckFileCount(fileInfo->mainPath.path, &fileCount, listCount);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    if (fileCount == 0) {
        *listCount = 0;
        return HKS_SUCCESS;
    }

    struct HksFileEntry *fileNameList = NULL;
    ret = FileNameListInit(&fileNameList, fileCount);
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "init file name list failed.")

    uint32_t realFileCount = fileCount;
    do {
        ret = GetFileNameList(fileInfo->mainPath.path, fileNameList, &realFileCount);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get file name list failed, ret = %" LOG_PUBLIC "d", ret)

        for (uint32_t i = 0; i < realFileCount; ++i) {
            ret = ConstructBlob(fileNameList[i].fileName, &(keyInfoList[i].alias));
            HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "construct blob failed, ret = %" LOG_PUBLIC "d", ret)
        }
    } while (0);

    FileNameListFree(&fileNameList, fileCount);
    HKS_IF_NOT_SUCC_RETURN(ret, ret)

    *listCount = realFileCount;
    return ret;
}

int32_t HksGetKeyAliasByProcessName(const struct HksProcessInfo *processInfo, struct HksKeyInfo *keyInfoList,
    uint32_t *listCount)
{
    /* params have been checked by caller functions */
    struct HksStoreFileInfo fileInfo = { 0 };

    int32_t ret;
    do {
        ret = FileInfoInit(&fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks file info init failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = GetFilePath(processInfo, HKS_STORAGE_TYPE_KEY, &fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks get file info failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = GetKeyAliasByProcessName(&fileInfo, keyInfoList, listCount);
        HKS_IF_NOT_SUCC_LOGE(ret, "get key alias by processName failed, ret = %" LOG_PUBLIC "d.", ret)
    } while (0);

    FileInfoFree(&fileInfo);
    return ret;
}

int32_t HksGetKeyCountByProcessName(const struct HksProcessInfo *processInfo, uint32_t *fileCount)
{
    /* params have been checked by caller functions */
    struct HksStoreFileInfo fileInfo = { 0 };

    int32_t ret;
    do {
        ret = FileInfoInit(&fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks file info init failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = GetFilePath(processInfo, HKS_STORAGE_TYPE_KEY, &fileInfo);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "hks get file info failed, ret = %" LOG_PUBLIC "d.", ret)

        ret = GetFileCount(fileInfo.mainPath.path, fileCount);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "get storage file count failed, ret = %" LOG_PUBLIC "d.", ret)
    } while (0);

    FileInfoFree(&fileInfo);
    return ret;
}

static int32_t DestroyType(const char *storePath, const char *typePath, uint32_t bakFlag)
{
    char *destroyPath = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
    HKS_IF_NULL_RETURN(destroyPath, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(destroyPath, HKS_MAX_FILE_NAME_LEN, 0, HKS_MAX_FILE_NAME_LEN);

    int32_t ret = GetPath(storePath, typePath, destroyPath, HKS_MAX_FILE_NAME_LEN, bakFlag);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Get Path failed! ret = 0x%" LOG_PUBLIC "X", ret);
        HKS_FREE_PTR(destroyPath);
        return ret;
    }

    ret = HksIsDirExist(destroyPath);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(destroyPath);
        return HKS_SUCCESS;
    }

    ret = HksRemoveDir(destroyPath);
    HKS_IF_NOT_SUCC_LOGE(ret, "Destroy dir failed! ret = 0x%" LOG_PUBLIC "X", ret)

    HKS_FREE_PTR(destroyPath);
    return ret;
}

static int32_t StoreDestroy(const char *processNameEncoded, uint32_t bakFlag)
{
    int32_t ret;
    char *rootPath = NULL;
    if (bakFlag == HKS_STORAGE_BAK_FLAG_TRUE) {
        ret = GetStoreRootPath(HKS_STORAGE_BACKUP_PATH, &rootPath);
    } else {
        ret = GetStoreRootPath(HKS_STORAGE_MAIN_PATH, &rootPath);
    }
    HKS_IF_NOT_SUCC_LOGE_RETURN(ret, ret, "get root path failed")

    char *storePath = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
    HKS_IF_NULL_RETURN(storePath, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(storePath, HKS_MAX_FILE_NAME_LEN, 0, HKS_MAX_FILE_NAME_LEN);

    ret = GetPath(rootPath, processNameEncoded, storePath, HKS_MAX_FILE_NAME_LEN, bakFlag);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Get Path failed! ret = 0x%" LOG_PUBLIC "X", ret);
        HKS_FREE_PTR(storePath);
        return ret;
    }

    ret = DestroyType(storePath, HKS_KEY_STORE_ROOT_KEY_PATH, bakFlag);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("Destroy info dir failed! ret = 0x%" LOG_PUBLIC "X", ret); /* continue delete */
    }

    ret = DestroyType(storePath, HKS_KEY_STORE_KEY_PATH, bakFlag);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("Destroy key dir failed! ret = 0x%" LOG_PUBLIC "X", ret); /* continue delete */
    }

    ret = DestroyType(storePath, HKS_KEY_STORE_CERTCHAIN_PATH, bakFlag);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_I("Destroy certchain dir failed! ret = 0x%" LOG_PUBLIC "X", ret); /* continue delete */
    }

    HKS_FREE_PTR(storePath);
    return HKS_SUCCESS;
}

int32_t HksStoreDestroy(const struct HksBlob *processName)
{
    char *name = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
    HKS_IF_NULL_RETURN(name, HKS_ERROR_MALLOC_FAIL)

    (void)memset_s(name, HKS_MAX_FILE_NAME_LEN, 0, HKS_MAX_FILE_NAME_LEN);

    int32_t ret;
    do {
        ret = ConstructName(processName, name, HKS_MAX_FILE_NAME_LEN);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "Construct process name failed! ret = 0x%" LOG_PUBLIC "X.", ret)

        ret = StoreDestroy(name, HKS_STORAGE_BAK_FLAG_FLASE);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "Hks destroy dir failed! ret = 0x%" LOG_PUBLIC "X.", ret)

#ifdef SUPPORT_STORAGE_BACKUP
        ret = StoreDestroy(name, HKS_STORAGE_BAK_FLAG_TRUE);
        HKS_IF_NOT_SUCC_LOGE_BREAK(ret, "Hks destroy back dir failed! ret = 0x%" LOG_PUBLIC "X.", ret)
#endif
    } while (0);

    HKS_FREE_PTR(name);
    return ret;
}

#ifndef __LITEOS_M__
static int32_t ConstructUserIdPath(const char *userId, char *userIdPath, uint32_t pathLen)
{
    if (strncpy_s(userIdPath, pathLen, HKS_KEY_STORE_PATH, strlen(HKS_KEY_STORE_PATH)) != EOK) {
        HKS_LOG_E("copy key store path failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    if (strncat_s(userIdPath, pathLen, "/", strlen("/")) != EOK) {
        HKS_LOG_E("catecnate character / failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    if (strncat_s(userIdPath, pathLen, userId, strlen(userId)) != EOK) {
        HKS_LOG_E("catecnate userId failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    if (strncat_s(userIdPath, pathLen, "\0", strlen("\0")) != EOK) {
        HKS_LOG_E("catecnate character 0 at the end failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t ConstructUidPath(const char *userId, const char *uid, char *uidPath, uint32_t pathLen)
{
    if (strncpy_s(uidPath, pathLen, HKS_KEY_STORE_PATH, strlen(HKS_KEY_STORE_PATH)) != EOK) {
        HKS_LOG_E("copy key store path failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    if (strncat_s(uidPath, pathLen, "/", strlen("/")) != EOK) {
        HKS_LOG_E("catecnate character / 1 failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    if (strncat_s(uidPath, pathLen, userId, strlen(userId)) != EOK) {
        HKS_LOG_E("catecnate userId failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    if (strncat_s(uidPath, pathLen, "/", strlen("/")) != EOK) {
        HKS_LOG_E("catecnate character / 2 failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    if (strncat_s(uidPath, pathLen, uid, strlen(uid)) != EOK) {
        HKS_LOG_E("catecnate uid failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    if (strncat_s(uidPath, pathLen, "\0", strlen("\0")) != EOK) {
        HKS_LOG_E("catecnate character 0 at the end failed.");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    return HKS_SUCCESS;
}

int32_t HksServiceDeleteUserIDKeyAliasFile(const struct HksBlob processName)
{
    char *userData = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
    HKS_IF_NULL_LOGE_RETURN(userData, HKS_ERROR_MALLOC_FAIL, "malloc user data failed")

    (void)memset_s(userData, HKS_MAX_FILE_NAME_LEN, 0, HKS_MAX_FILE_NAME_LEN);

    int32_t ret = ConstructName(&processName, userData, HKS_MAX_FILE_NAME_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(userData);
        HKS_LOG_E("construct user id name failed, ret = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    char userProcess[HKS_MAX_DIRENT_FILE_LEN] = "";
    ret = ConstructUserIdPath(userData, userProcess, HKS_MAX_DIRENT_FILE_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(userData);
        HKS_LOG_E("catecnate UserIdPath failed");
        return ret;
    }

    HKS_LOG_I("delete path: %" LOG_PUBLIC "s", userProcess);

    ret = HksDeleteDir(userProcess);
    HKS_FREE_PTR(userData);
    return ret;
}

int32_t HksServiceDeleteUIDKeyAliasFile(const struct HksProcessInfo processInfo)
{
    char *userData = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
    HKS_IF_NULL_LOGE_RETURN(userData, HKS_ERROR_MALLOC_FAIL, "malloc user data failed")

    (void)memset_s(userData, HKS_MAX_FILE_NAME_LEN, 0, HKS_MAX_FILE_NAME_LEN);

    int32_t ret = ConstructName(&processInfo.userId, userData, HKS_MAX_FILE_NAME_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(userData);
        HKS_LOG_E("construct user id name failed, ret = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    char *uidData = (char *)HksMalloc(HKS_MAX_FILE_NAME_LEN);
    if (uidData == NULL) {
        HKS_FREE_PTR(userData);
        HKS_LOG_E("malloc user data failed");
        return HKS_ERROR_MALLOC_FAIL;
    }
    (void)memset_s(uidData, HKS_MAX_FILE_NAME_LEN, 0, HKS_MAX_FILE_NAME_LEN);

    ret = ConstructName(&processInfo.processName, uidData, HKS_MAX_FILE_NAME_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(userData);
        HKS_FREE_PTR(uidData);
        HKS_LOG_E("construct uid name failed, ret = %" LOG_PUBLIC "d", ret);
        return ret;
    }

    char userProcess[HKS_MAX_DIRENT_FILE_LEN] = "";
    ret = ConstructUidPath(userData, uidData, userProcess, HKS_MAX_DIRENT_FILE_LEN);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(userData);
        HKS_FREE_PTR(uidData);
        HKS_LOG_E("catecnate uidPath failed");
        return ret;
    }

    HKS_LOG_I("delete path : %" LOG_PUBLIC "s", userProcess);

    ret = HksDeleteDir(userProcess);
    HKS_FREE_PTR(userData);
    HKS_FREE_PTR(uidData);
    return ret;
}
#endif
#endif /* _CUT_AUTHENTICATE_ */

