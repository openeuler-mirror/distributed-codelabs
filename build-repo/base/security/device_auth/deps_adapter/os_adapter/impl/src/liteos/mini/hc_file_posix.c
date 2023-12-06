/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "hc_log.h"
#include "hc_types.h"
#include "securec.h"

#define MAX_FOLDER_NAME_SIZE 128
#define DEFAULT_FILE_PERMISSION 0600

static int32_t CreateDirectory(const char *filePath)
{
    int32_t ret;
    errno_t eno;
    char *chPtr = NULL;
    char dirCache[MAX_FOLDER_NAME_SIZE];

    chPtr = (char *)filePath;
    while ((chPtr = strchr(chPtr, '/')) != NULL) {
        unsigned long len = (unsigned long)((uintptr_t)chPtr - (uintptr_t)filePath);
        if (len == 0ul) {
            chPtr++;
            continue;
        }
        eno = memcpy_s(dirCache, sizeof(dirCache), filePath, len);
        if (eno != EOK) {
            LOGE("memory copy failed");
            return -1;
        }
        dirCache[len] = 0;
        if (access(dirCache, F_OK) == 0) {
            chPtr++;
            continue;
        }
        DIR *dir = opendir(dirCache);
        if (dir == NULL) {
            ret = mkdir(dirCache, DEFAULT_FILE_PERMISSION);
            if (ret != 0) {
                LOGE("make dir failed, err code %d, errno = %d", ret, errno);
                return -1;
            }
        } else {
            closedir(dir);
        }
        chPtr++;
    }
    return 0;
}

static int HcFileOpenRead(const char *path)
{
    return open(path, O_RDONLY);
}

static int HcFileOpenWrite(const char *path)
{
    if (access(path, F_OK) != 0) {
        int32_t ret = CreateDirectory(path);
        if (ret != 0) {
            return -1;
        }
    }
    return open(path, O_RDWR | O_CREAT | O_TRUNC);
}

int HcFileOpen(const char *path, int mode, FileHandle *file)
{
    if (path == NULL || file == NULL) {
        return -1;
    }
    if (mode == MODE_FILE_READ) {
        file->fileHandle.fd = HcFileOpenRead(path);
    } else {
        file->fileHandle.fd = HcFileOpenWrite(path);
    }
    if (file->fileHandle.fd == -1) {
        LOGE("[OS]: file open failed, errno = %d", errno);
        return -1;
    } else {
        return 0;
    }
}

int HcFileSize(FileHandle file)
{
    int fp = file.fileHandle.fd;
    int size = lseek(fp, 0, SEEK_END);
    (void)lseek(fp, 0, SEEK_SET);
    return size;
}

int HcFileRead(FileHandle file, void *dst, int dstSize)
{
    int fp = file.fileHandle.fd;
    if (fp == -1 || dstSize < 0 || dst == NULL) {
        return -1;
    }

    char *dstBuffer = (char *)dst;
    int total = 0;
    while (total < dstSize) {
        int readCount = read(fp, dstBuffer + total, dstSize - total);
        if (readCount < 0 || readCount > (dstSize - total)) {
            LOGE("read size error, errno = %d", errno);
            return -1;
        }
        if (readCount == 0) {
            LOGE("read size = 0, errno = %d", errno);
            return total;
        }
        total += readCount;
    }

    return total;
}

int HcFileWrite(FileHandle file, const void *src, int srcSize)
{
    int fp = file.fileHandle.fd;
    if (fp == -1 || srcSize < 0 || src == NULL) {
        return -1;
    }

    const char *srcBuffer = (const char *)src;
    int total = 0;
    while (total < srcSize) {
        int writeCount = write(fp, srcBuffer + total, srcSize - total);
        if (writeCount < 0 || writeCount > (srcSize - total)) {
            LOGE("write size error, errno = %d", errno);
            return -1;
        }
        total += writeCount;
    }
    return total;
}

void HcFileClose(FileHandle file)
{
    int fp = file.fileHandle.fd;
    if (fp == -1) {
        return;
    }

    int res = close(fp);
    if (res != 0) {
        LOGW("close file failed, res = %d", res);
    }
}

void HcFileRemove(const char *path)
{
    if (path == NULL) {
        LOGE("Invalid file path");
        return;
    }
    int res = unlink(path);
    if (res != 0) {
        LOGW("delete file failed, res = %d", res);
    }
}

void HcFileGetSubFileName(const char *path, StringVector *nameVec)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    if ((dir = opendir(path)) == NULL) {
        LOGI("opendir failed!");
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
            continue;
        }
        HcString subFileName = CreateString();
        if (!StringSetPointer(&subFileName, entry->d_name)) {
            LOGE("Failed to copy subFileName!");
            DeleteString(&subFileName);
            continue;
        }
        if (nameVec->pushBackT(nameVec, subFileName) == NULL) {
            LOGE("Failed to push path to pathVec!");
            DeleteString(&subFileName);
        }
    }
    if (closedir(dir) < 0) {
        LOGE("Failed to close file");
    }
}
