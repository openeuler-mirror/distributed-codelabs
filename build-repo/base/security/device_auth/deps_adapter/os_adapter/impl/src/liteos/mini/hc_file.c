/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "hc_file.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "hal_error.h"
#include "hc_log.h"
#include "securec.h"
#include "utils_file.h"

static int HcFileOpenRead(const char *path)
{
    int ret = UtilsFileOpen(path, O_RDONLY, 0);
    LOGI("ret = %d", ret);
    return ret;
}

static int HcFileOpenWrite(const char *path)
{
    int ret = UtilsFileOpen(path, O_RDWR_FS | O_CREAT_FS | O_TRUNC_FS, 0);
    LOGI("ret = %d", ret);
    return ret;
}

int HcFileOpen(const char *path, int mode, FileHandle *file)
{
    if (path == NULL || file == NULL) {
        return -1;
    }
    if (strcpy_s(file->filePath, MAX_FILE_PATH_SIZE, path) != EOK) {
        LOGE("Failed to copy filePath!");
        return HAL_FAILED;
    }
    if (mode == MODE_FILE_READ) {
        file->fileHandle.fd = HcFileOpenRead(path);
    } else {
        file->fileHandle.fd = HcFileOpenWrite(path);
    }
    if (file->fileHandle.fd == HAL_FAILED) {
        return HAL_FAILED;
    }
    return HAL_SUCCESS;
}

int HcFileSize(FileHandle file)
{
    int fileSize = 0;
    int ret = UtilsFileStat(file.filePath, (unsigned int *)&fileSize);
    LOGI("ret = %d, fileSize = %d\n", ret, fileSize);
    if (ret == HAL_SUCCESS) {
        return fileSize;
    }
    return HAL_FAILED;
}

int HcFileRead(FileHandle file, void *dst, int dstSize)
{
    int fp = file.fileHandle.fd;
    if (fp  == HAL_FAILED || dstSize < 0 || dst == NULL) {
        return HAL_FAILED;
    }
    int ret = UtilsFileRead(fp, (char *)dst, dstSize);
    LOGI("ret = %d", ret);
    return ret;
}

int HcFileWrite(FileHandle file, const void *src, int srcSize)
{
    int fp = file.fileHandle.fd;
    if (fp  == HAL_FAILED || srcSize < 0 || src == NULL) {
        return HAL_FAILED;
    }
    int ret = UtilsFileWrite(fp, src, srcSize);
    LOGI("ret = %d", ret);
    return ret;
}

void HcFileClose(FileHandle file)
{
    int fp = file.fileHandle.fd;
    if (fp < 0) {
        return;
    }
    int ret = UtilsFileClose(fp);
    LOGI("ret = %d", ret);
}

void HcFileRemove(const char *path)
{
    if (path == NULL) {
        LOGE("Invalid file path");
        return;
    }
    int ret = UtilsFileDelete(path);
    LOGI("File delete result:%d", ret);
}

void HcFileGetSubFileName(const char *path, StringVector *nameVec)
{
    /* Since the liteOS device does not support retrieving files in the directory, the default file is used. */
    (void)path;
    HcString subFileName = CreateString();
    if (!StringSetPointer(&subFileName, "hcgroup.dat")) {
        LOGE("Failed to copy subFileName!");
        DeleteString(&subFileName);
        return;
    }
    if (nameVec->pushBackT(nameVec, subFileName) == NULL) {
        LOGE("Failed to push name!");
        DeleteString(&subFileName);
    }
}