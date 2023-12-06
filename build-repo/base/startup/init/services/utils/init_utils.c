/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "init_utils.h"

#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "beget_ext.h"
#include "securec.h"
#include "service_control.h"

#define MAX_BUF_SIZE  1024
#define MAX_SMALL_BUFFER 3096

#define MAX_JSON_FILE_LEN 102400    // max init.cfg size 100KB
#define CONVERT_MICROSEC_TO_SEC(x) ((x) / 1000 / 1000.0)
#ifndef DT_DIR
#define DT_DIR 4
#endif

#define THOUSAND_UNIT_INT 1000
#define THOUSAND_UNIT_FLOAT 1000.0

float ConvertMicrosecondToSecond(int x)
{
    return ((x / THOUSAND_UNIT_INT) / THOUSAND_UNIT_FLOAT);
}

#ifndef __LITEOS_M__
static bool CheckDigit(const char *name)
{
    size_t nameLen = strlen(name);
    for (size_t i = 0; i < nameLen; ++i) {
        if (!isdigit(name[i])) {
            return false;
        }
    }
    return true;
}
#endif

int StringToUint(const char *name, unsigned int *value)
{
    errno = 0;
    *value = (unsigned int)strtoul(name, 0, DECIMAL_BASE);
    BEGET_CHECK_RETURN_VALUE(errno == 0, -1);
    return 0;
}

char *ReadFileToBuf(const char *configFile)
{
    char *buffer = NULL;
    FILE *fd = NULL;
    struct stat fileStat = {0};
    BEGET_CHECK_RETURN_VALUE(configFile != NULL && *configFile != '\0', NULL);
    do {
        if (stat(configFile, &fileStat) != 0 ||
            fileStat.st_size <= 0 || fileStat.st_size > MAX_JSON_FILE_LEN) {
            BEGET_LOGE("Unexpected config file \" %s \", check if it exist. if exist, check file size", configFile);
            break;
        }
        fd = fopen(configFile, "r");
        if (fd == NULL) {
            BEGET_LOGE("Open %s failed. err = %d", configFile, errno);
            break;
        }
        buffer = (char*)malloc((size_t)(fileStat.st_size + 1));
        if (buffer == NULL) {
            BEGET_LOGE("Failed to allocate memory for config file, err = %d", errno);
            break;
        }

        if (fread(buffer, fileStat.st_size, 1, fd) != 1) {
            free(buffer);
            buffer = NULL;
            break;
        }
        buffer[fileStat.st_size] = '\0';
    } while (0);

    if (fd != NULL) {
        (void)fclose(fd);
        fd = NULL;
    }
    return buffer;
}

char *ReadFileData(const char *fileName)
{
    if (fileName == NULL) {
        return NULL;
    }
    char *buffer = NULL;
    int fd = -1;
    fd = open(fileName, O_RDONLY);
    BEGET_ERROR_CHECK(fd >= 0, return NULL, "Failed to read file %s", fileName);
    buffer = (char *)calloc(1, MAX_SMALL_BUFFER); // fsmanager not create, can not get fileStat st_size
    BEGET_ERROR_CHECK(buffer != NULL, close(fd);
        return NULL, "Failed to allocate memory for %s", fileName);
    ssize_t readLen = read(fd, buffer, MAX_SMALL_BUFFER - 1);
    BEGET_ERROR_CHECK((readLen > 0) && (readLen < (MAX_SMALL_BUFFER - 1)), close(fd);
        free(buffer);
        return NULL, "Failed to read data for %s", fileName);
    buffer[readLen] = '\0';
    if (fd != -1) {
        close(fd);
    }
    return buffer;
}

int SplitString(char *srcPtr, const char *del, char **dstPtr, int maxNum)
{
    BEGET_CHECK_RETURN_VALUE(srcPtr != NULL && dstPtr != NULL && del != NULL, -1);
    char *buf = NULL;
    dstPtr[0] = strtok_r(srcPtr, del, &buf);
    int counter = 0;
    while ((counter < maxNum) && (dstPtr[counter] != NULL)) {
        counter++;
        if (counter >= maxNum) {
            break;
        }
        dstPtr[counter] = strtok_r(NULL, del, &buf);
    }
    return counter;
}

void FreeStringVector(char **vector, int count)
{
    if (vector != NULL) {
        for (int i = 0; i < count; i++) {
            if (vector[i] != NULL) {
                free(vector[i]);
            }
        }
        free(vector);
    }
}

char **SplitStringExt(char *buffer, const char *del, int *returnCount, int maxItemCount)
{
    BEGET_CHECK_RETURN_VALUE((maxItemCount >= 0) && (buffer != NULL) && (del != NULL) && (returnCount != NULL), NULL);
    // Why is this number?
    // Now we use this function to split a string with a given delimiter
    // We do not know how many sub-strings out there after splitting.
    // 50 is just a guess value.
    const int defaultItemCounts = 50;
    int itemCounts = maxItemCount;

    if (maxItemCount > defaultItemCounts) {
        itemCounts = defaultItemCounts;
    }
    char **items = (char **)malloc(sizeof(char*) * itemCounts);
    BEGET_ERROR_CHECK(items != NULL, return NULL, "No enough memory to store items");
    char *rest = NULL;
    char *p = strtok_r(buffer, del, &rest);
    int count = 0;
    while (p != NULL) {
        if (count > itemCounts - 1) {
            itemCounts += (itemCounts / 2) + 1; // 2 Request to increase the original memory by half.
            BEGET_LOGV("Too many items,expand size");
            char **expand = (char **)(realloc(items, sizeof(char *) * itemCounts));
            BEGET_ERROR_CHECK(expand != NULL, FreeStringVector(items, count);
                return NULL, "Failed to expand memory");
            items = expand;
        }
        size_t len = strlen(p);
        items[count] = (char *)malloc(len + 1);
        BEGET_CHECK(items[count] != NULL, FreeStringVector(items, count);
            return NULL);
        if (strncpy_s(items[count], len + 1, p, len) != EOK) {
            BEGET_LOGE("Copy string failed");
            FreeStringVector(items, count);
            return NULL;
        }
        items[count][len] = '\0';
        count++;
        p = strtok_r(NULL, del, &rest);
    }
    *returnCount = count;
    return items;
}

void WaitForFile(const char *source, unsigned int maxSecond)
{
    BEGET_ERROR_CHECK(maxSecond <= WAIT_MAX_SECOND, maxSecond = WAIT_MAX_SECOND, "WaitForFile max time is 5s");
    struct stat sourceInfo = {0};
    unsigned int waitTime = 500000;
    /* 500ms interval, check maxSecond*2 times total */
    unsigned int maxCount = maxSecond * 2;
    unsigned int count = 0;
    while ((stat(source, &sourceInfo) < 0) && (errno == ENOENT) && (count < maxCount)) {
        usleep(waitTime);
        count++;
    }
    BEGET_CHECK_ONLY_ELOG(count != maxCount, "wait for file:%s failed after %d second.", source, maxSecond);
    return;
}

char *GetRealPath(const char *source)
{
    BEGET_CHECK_RETURN_VALUE(source != NULL, NULL);
    char *path = realpath(source, NULL);
    if (path == NULL) {
        BEGET_ERROR_CHECK(errno == ENOENT, return NULL, "Failed to resolve %s real path err=%d", source, errno);
    }
    return path;
}

int MakeDir(const char *dir, mode_t mode)
{
    int rc = -1;
    if (dir == NULL || *dir == '\0') {
        errno = EINVAL;
        return rc;
    }
    rc = mkdir(dir, mode);
    BEGET_ERROR_CHECK(!(rc < 0 && errno != EEXIST), return rc,
        "Create directory \" %s \" failed, err = %d", dir, errno);
    // create dir success or it already exist.
    return 0;
}

int MakeDirRecursive(const char *dir, mode_t mode)
{
    int rc = -1;
    char buffer[PATH_MAX] = {0};
    const char *p = NULL;
    if (dir == NULL || *dir == '\0') {
        errno = EINVAL;
        return rc;
    }
    p = dir;
    char *slash = strchr(dir, '/');
    while (slash != NULL) {
        int gap = slash - p;
        p = slash + 1;
        if (gap == 0) {
            slash = strchr(p, '/');
            continue;
        }
        if (gap < 0) { // end with '/'
            break;
        }
        BEGET_CHECK_RETURN_VALUE(memcpy_s(buffer, PATH_MAX, dir, p - dir - 1) == 0, -1);
        rc = MakeDir(buffer, mode);
        BEGET_CHECK_RETURN_VALUE(rc >= 0, rc);
        slash = strchr(p, '/');
    }
    return MakeDir(dir, mode);
}

void CheckAndCreateDir(const char *fileName)
{
#ifndef __LITEOS_M__
    if (fileName == NULL || *fileName == '\0') {
        return;
    }
    char *path = strndup(fileName, strrchr(fileName, '/') - fileName);
    if (path == NULL) {
        return;
    }
    if (access(path, F_OK) == 0) {
        free(path);
        return;
    }
    MakeDirRecursive(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    free(path);
#else
    (void)fileName;
#endif
}

int CheckAndCreatFile(const char *file, mode_t mode)
{
    if (access(file, F_OK) == 0) {
        BEGET_LOGW("File \' %s \' already exist", file);
        return 0;
    } else {
        if (errno == ENOENT) {
            CheckAndCreateDir(file);
            int fd = open(file, O_CREAT, mode);
            if (fd < 0) {
                BEGET_LOGE("Failed create %s, err=%d", file, errno);
                return -1;
            } else {
                BEGET_LOGI("Success create %s", file);
                close(fd);
            }
        } else {
            BEGET_LOGW("Failed to access \' %s \', err = %d", file, errno);
            return -1;
        }
    }
    return 0;
}

int StringToInt(const char *str, int defaultValue)
{
    if (str == NULL || *str == '\0') {
        return defaultValue;
    }
    errno = 0;
    int value = (int)strtoul(str, NULL, DECIMAL_BASE);
    return (errno != 0) ? defaultValue : value;
}

int ReadFileInDir(const char *dirPath, const char *includeExt,
    int (*processFile)(const char *fileName, void *context), void *context)
{
    BEGET_CHECK_RETURN_VALUE(dirPath != NULL && processFile != NULL, -1);
    DIR *pDir = opendir(dirPath);
    BEGET_ERROR_CHECK(pDir != NULL, return -1, "Read dir :%s failed.%d", dirPath, errno);
    char *fileName = calloc(1, MAX_BUF_SIZE);
    BEGET_ERROR_CHECK(fileName != NULL, closedir(pDir);
        return -1, "Failed to malloc for %s", dirPath);

    struct dirent *dp;
    uint32_t count = 0;
    while ((dp = readdir(pDir)) != NULL) {
        if (dp->d_type == DT_DIR) {
            continue;
        }
        if (includeExt != NULL) {
            char *tmp = strstr(dp->d_name, includeExt);
            if (tmp == NULL) {
                continue;
            }
            if (strcmp(tmp, includeExt) != 0) {
                continue;
            }
        }
        int ret = snprintf_s(fileName, MAX_BUF_SIZE, MAX_BUF_SIZE - 1, "%s/%s", dirPath, dp->d_name);
        if (ret <= 0) {
            BEGET_LOGE("Failed to get file name for %s", dp->d_name);
            continue;
        }
        struct stat st;
        if (stat(fileName, &st) == 0) {
            count++;
            processFile(fileName, context);
        }
    }
    BEGET_LOGV("ReadFileInDir dirPath %s %d", dirPath, count);
    free(fileName);
    closedir(pDir);
    return 0;
}

int StringReplaceChr(char *strl, char oldChr, char newChr)
{
    BEGET_ERROR_CHECK(strl != NULL, return -1, "Invalid parament");
    char *p = strl;
    while (*p != '\0') {
        if (*p == oldChr) {
            *p = newChr;
        }
        p++;
    }
    BEGET_LOGV("strl is %s", strl);
    return 0;
}

BEGET_LOCAL_API int StringToLL(const char *str, long long int *out)
{
    BEGET_ERROR_CHECK(str != NULL && out != NULL, return -1, "Invalid parament");
    const char *s = str;
    while (isspace(*s)) {
        s++;
    }

    size_t len = strlen(str);
    int positiveHex = (len > 1 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'));
    int negativeHex = (len > 2 && s[0] == '-' && s[1] == '0' && (s[2] == 'x' || s[2] == 'X')); // 2: shorttest
    int base = (positiveHex || negativeHex) ? HEX_BASE : DECIMAL_BASE;
    char *end = NULL;
    errno = 0;
    *out = strtoll(s, &end, base);
    if (errno != 0) {
        BEGET_LOGE("StringToLL %s err = %d", str, errno);
        return -1;
    }
    BEGET_CHECK(!(s == end || *end != '\0'), return -1);
    return 0;
}

BEGET_LOCAL_API int StringToULL(const char *str, unsigned long long int *out)
{
    BEGET_ERROR_CHECK(str != NULL && out != NULL, return -1, "Invalid parament");
    const char *s = str;
    while (isspace(*s)) {
        s++;
    }
    BEGET_CHECK(s[0] != '-', return -1);
    int base = (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) ? HEX_BASE : DECIMAL_BASE;
    char *end = NULL;
    errno = 0;
    *out = strtoull(s, &end, base);
    if (errno != 0) {
        BEGET_LOGE("StringToULL %s err = %d", str, errno);
        return -1;
    }
    BEGET_CHECK(end != s, return -1);
    BEGET_CHECK(*end == '\0', return -1);
    return 0;
}

void TrimTail(char *str, char c)
{
    char *end = str + strlen(str) - 1;
    while (end >= str && *end == c) {
        *end = '\0';
        end--;
    }
}

char *TrimHead(char *str, char c)
{
    char *head = str;
    const char *end = str + strlen(str);
    while (head < end && *head == c) {
        *head = '\0';
        head++;
    }
    return head;
}
