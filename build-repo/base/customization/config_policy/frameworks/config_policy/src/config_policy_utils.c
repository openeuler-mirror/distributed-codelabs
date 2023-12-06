/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "config_policy_utils.h"

#include <ctype.h>
#include <securec.h>
#include <stdbool.h>
#include <unistd.h>

#include "config_policy_impl.h"
#ifndef __LITEOS__
#include "init_param.h"
#endif

static const size_t MIN_APPEND_LEN = 32;
// ':' split different x rules, example:":relPath,mode[,extra][:]"
// ',' split different param for x rules
// ":-" split for key:-value
// exampe:"etc/xml/config.xml,10:etc/xml/config1.xml,100,etc/carrier/${key:-value}"
static const char SEP_FOR_X_RULE = ':';
static const char SEP_FOR_X_PARAM = ',';
static const char *SEP_FOR_X_VALUE = ":-";

typedef struct {
    size_t size;   // allocated buffer size of p
    size_t strLen; // strlen(p), less than size
    char *p;       // just init to NULL, by malloc
} StringHolder;

typedef struct {
    int segCount; // count of char * in segs
    char *orgStr;
    char *segs[1];
} SplitedStr;

#ifdef __LITEOS_M__
// The data will be used always, so prealloc to avoid memory fragment
static char gConfigPolicy[MINI_CONFIG_POLICY_BUF_SIZE] = {0};
void SetMiniConfigPolicy(const char *policy)
{
    if (gConfigPolicy[0] != 0) {
        return; // now only set once
    }
    (void)strcpy_s(gConfigPolicy, sizeof(gConfigPolicy), policy);
}

__WEAK void TrigSetMiniConfigPolicy()
{
    return;
}
#endif

/**
 * query function, the memory is allocated in function
 * @return value, or NULL if not exist or strlen(value) is 0
 * @NOTICE caller should free the returned value
 */
typedef char *(*QueryFunc)(const char *key);
static char *ExpandStr(char *src, const char *def, QueryFunc queryFunc);

static inline size_t Min(size_t a, size_t b)
{
    return a < b ? a : b;
}

static inline size_t Max(size_t a, size_t b)
{
    return a > b ? a : b;
}

static void FreeIf(void *p)
{
    if (p != NULL) {
        free(p);
    }
}

#ifndef __LITEOS__
static char *CustGetSystemParam(const char *name)
{
    char *value = NULL;
    unsigned int len = 0;

    if (SystemGetParameter(name, NULL, &len) != 0 || len == 0) {
        return NULL;
    }
    value = (char *)calloc(len, sizeof(char));
    if (value != NULL && SystemGetParameter(name, value, &len) == 0 && value[0]) {
        return value;
    }
    FreeIf(value);
    return NULL;
}

static char *GetOpkeyPath(int type)
{
    char *result = NULL;
    const char *opKeyDir = "etc/carrier/";
    const char *opKeyName = CUST_OPKEY0;
    if (type == FOLLOWX_MODE_SIM_1) {
        opKeyName = CUST_OPKEY0;
    } else if (type == FOLLOWX_MODE_SIM_2) {
        opKeyName = CUST_OPKEY1;
    } else {
        unsigned int len = 0;
        if (SystemGetParameter(CUST_OPKEY0, NULL, &len) == 0 && len > 0) {
            opKeyName = CUST_OPKEY0;
        } else if (SystemGetParameter(CUST_OPKEY1, NULL, &len) == 0 && len > 0) {
            opKeyName = CUST_OPKEY1;
        }
    }
    char *opKeyValue = CustGetSystemParam(opKeyName);
    if (opKeyValue == NULL) {
        return NULL;
    }
    size_t bufSize = strlen(opKeyDir) + strlen(opKeyValue) + 1;
    bufSize = Min(bufSize, MAX_PATH_LEN);
    result = (char *)calloc(bufSize, sizeof(char));
    if (result != NULL && sprintf_s(result, bufSize, "%s%s", opKeyDir, opKeyValue) > 0) {
        FreeIf(opKeyValue);
        return result;
    }
    FreeIf(opKeyValue);
    FreeIf(result);
    return NULL;
}
#endif // __LITEOS__

static SplitedStr *SplitStr(char *str, char delim)
{
    int segCount = 1;
    for (char *p = str; *p != '\0'; p++) {
        (*p == delim) ? segCount++ : 0;
    }
    SplitedStr *result = (SplitedStr *)calloc(sizeof(SplitedStr) + sizeof(char *) * segCount, 1);
    if (result == NULL) {
        return NULL;
    }
    result->segCount = segCount;
    result->orgStr = str;

    char *nextDelim = NULL;
    char delimStr[] = {delim, 0};
    result->segs[0] = strtok_s(str, delimStr, &nextDelim);
    for (int index = 1; index < segCount; index++) {
        result->segs[index] = strtok_s(NULL, delimStr, &nextDelim);
    }
    return result;
}

static void FreeSplitedStr(SplitedStr *p)
{
    if (p != NULL) {
        FreeIf(p->orgStr);
        p->orgStr = NULL;
        free(p);
    }
}

#ifndef __LITEOS__
// get follow x rule from param variant
// *mode: read from contains param variant, mode is output param.
// return: extra path rule.
// followPath format ":relPath,mode[,extra][:]"
// example: ":etc/xml/config.xml,10:etc/xml/config1.xml,100,etc/carrier/${key:-value}"
static char *GetFollowXRule(const char *relPath, int *mode)
{
    char *followRule = CustGetSystemParam(CUST_FOLLOW_X_RULES);
    if (followRule == NULL) {
        return NULL;
    }

    size_t bufSize = strlen(relPath) + sizeof(":,") + 1;
    bufSize = Min(bufSize, MAX_PATH_LEN);
    char *search = (char *)calloc(bufSize, sizeof(char));
    if (search == NULL || sprintf_s(search, bufSize, ":%s,", relPath) == -1) {
        FreeIf(search);
        FreeIf(followRule);
        return NULL;
    }

    char *addPath = NULL;
    char *item = strstr(followRule, search);
    if (item) {
        item++; // skip delim ':', goto ":relPath,mode[,extra][:]"
        char *endItem = strchr(item, SEP_FOR_X_RULE);
        char *nextItem = endItem + 1;
        while (endItem && nextItem && *nextItem == '-') {
            endItem = strchr(nextItem, SEP_FOR_X_RULE);
            nextItem = endItem + 1;
        }
        if (endItem) {
            *endItem = '\0';
        }
        char *modeStr = strchr(item, SEP_FOR_X_PARAM);
        if (modeStr) {
            modeStr++;
            *mode = atoi(modeStr);
        }
        if (*mode == FOLLOWX_MODE_USER_DEFINE && modeStr) {
            addPath = strchr(modeStr, SEP_FOR_X_PARAM);
            if (addPath) {
                addPath++; // skip ',' get extra info
            }
        }
    }

    char *result = (addPath && *addPath) ? strdup(addPath) : NULL;
    FreeIf(followRule);
    FreeIf(search);
    return result;
}

static SplitedStr *GetFollowXPathByMode(const char *relPath, int followMode, const char *extra)
{
    char *followXPath = NULL;
    char *modePathFromCfg = NULL;
    const char *extraPath = extra;
    if (followMode == FOLLOWX_MODE_DEFAULT) {
        modePathFromCfg = GetFollowXRule(relPath, &followMode);
        if (followMode == FOLLOWX_MODE_USER_DEFINE && modePathFromCfg && *modePathFromCfg) {
            extraPath = modePathFromCfg;
        }
    }

    switch (followMode) {
        case FOLLOWX_MODE_SIM_DEFAULT:
            followXPath = GetOpkeyPath(FOLLOWX_MODE_SIM_DEFAULT);
            break;
        case FOLLOWX_MODE_SIM_1:
            followXPath = GetOpkeyPath(FOLLOWX_MODE_SIM_1);
            break;
        case FOLLOWX_MODE_SIM_2:
            followXPath = GetOpkeyPath(FOLLOWX_MODE_SIM_2);
            break;
        case FOLLOWX_MODE_USER_DEFINE:
            followXPath = (extraPath && *extraPath) ? strdup(extraPath) : NULL;
            break;
        default:
            break;
    }
    char *expandVal = (followXPath && *followXPath) ? ExpandStr(followXPath, "-x-", CustGetSystemParam) : NULL;
    FreeIf(followXPath);
    FreeIf(modePathFromCfg);
    SplitedStr *result = expandVal ? SplitStr(expandVal, ',') : NULL;
    return result;
}
#else
static SplitedStr *GetFollowXPathByMode(const char *relPath, int followMode, const char *extra)
{
    return NULL;
}
#endif

static char *TrimInplace(char *str, bool moveToStart)
{
    char *src = str;
    while (isspace(*src)) {
        src++;
    }
    for (int i = strlen(src) - 1; i >= 0 && isspace(src[i]); i--) {
        src[i] = '\0';
    }

    char *res = moveToStart ? str : src;
    if (moveToStart && src != str) {
        size_t len = strlen(src) + 1;
        if (memmove_s(str, len, src, len) != EOK) {
            return NULL;
        }
    }
    return (*res != '\0') ? res : NULL;
}

static bool EnsureHolderSpace(StringHolder *holder, size_t leastSize)
{
    if (holder->size < leastSize) {
        size_t allocSize = Max(leastSize * 2, MIN_APPEND_LEN);
        char *newPtr = (char *)calloc(allocSize, sizeof(char));
        if (newPtr == NULL) {
            allocSize = leastSize;
            newPtr = (char *)calloc(allocSize, sizeof(char));
            if (newPtr == NULL) {
                return false;
            }
        }
        if (holder->p != NULL && memcpy_s(newPtr, allocSize, holder->p, holder->strLen) != EOK) {
            FreeIf(newPtr);
            return false;
        }
        FreeIf(holder->p);
        holder->p = newPtr;
        holder->size = allocSize;
    }
    return true;
}

static bool AppendStr(StringHolder *holder, const char *s)
{
    size_t leastSize = holder->strLen + strlen(s) + 1;
    if (!EnsureHolderSpace(holder, leastSize)) {
        return false;
    }
    if (strcat_s(holder->p, holder->size, s) != EOK) {
        return false;
    }
    holder->strLen = leastSize - 1;
    return true;
}

static char *ExpandStr(char *src, const char *def, QueryFunc queryFunc)
{
    bool ok = true;
    StringHolder sh = { 0 };
    char *copyCur = NULL;
    char *searchCur = NULL;
    char *end = src + strlen(src);
    for (copyCur = searchCur = src; ok && searchCur < end;) {
        char *varStart = NULL;
        char *varEnd = NULL;
        char *find = strchr(searchCur, '$');
        if (!find) {
            ok = ok && AppendStr(&sh, copyCur);
            break;
        } else if ((varStart = strchr(find, '{')) && (varStart == find + 1) && (varEnd = strchr(find, '}')) &&
            varEnd - varStart > 0) { // Get user defined name
            *find = *varEnd = 0;
            ok = ok && AppendStr(&sh, copyCur);
            char *name = find + 2;
            char *defVal = strstr(name, SEP_FOR_X_VALUE);
            if (defVal) {
                *defVal = 0;                                                   // cut for name end
                defVal = TrimInplace(defVal + strlen(SEP_FOR_X_VALUE), false); // skip ":-", get value
            }
            char *value = queryFunc(name);
            if (value || defVal || def) {
                ok = ok && AppendStr(&sh, value ? value : (defVal ? defVal : def));
                FreeIf(value);
            } else {
                errno = EINVAL;
                ok = false;
                break;
            }
            copyCur = searchCur = varEnd + 1;
        } else {
            searchCur = find + 1;
        }
    }
    if (!ok) {
        FreeIf(sh.p);
        sh.p = NULL;
    }
    return sh.p;
}

static void GetCfgDirRealPolicyValue(CfgDir *res)
{
    if (res == NULL) {
        return;
    }
#ifdef __LITEOS_M__
    if (gConfigPolicy[0] == '\0') {
        TrigSetMiniConfigPolicy();
    }
    if (gConfigPolicy[0] != '\0') {
        res->realPolicyValue = strdup(gConfigPolicy);
    }
#elif defined(__LITEOS__)
    // use default, now do nothing
#else
    res->realPolicyValue = CustGetSystemParam(CUST_KEY_POLICY_LAYER);
#endif
    if (res->realPolicyValue != NULL && res->realPolicyValue[0]) {
        return;
    }
    res->realPolicyValue = strdup(DEFAULT_LAYER);
}

void FreeCfgFiles(CfgFiles *res)
{
    if (res == NULL) {
        return;
    }
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT; i++) {
        if (res->paths[i] != NULL) {
            free(res->paths[i]);
            res->paths[i] = NULL;
        }
    }
    free(res);
}

void FreeCfgDirList(CfgDir *res)
{
    if (res == NULL) {
        return;
    }
    if (res->realPolicyValue != NULL) {
        free(res->realPolicyValue);
        res->realPolicyValue = NULL;
    }
    free(res);
}

char *GetOneCfgFileEx(const char *pathSuffix, char *buf, unsigned int bufLength, int followMode, const char *extra)
{
    if (pathSuffix == NULL || buf == NULL || bufLength < MAX_PATH_LEN) {
        return NULL;
    }
    *buf = '\0';
    CfgDir *dirs = GetCfgDirList();
    if (dirs == NULL) {
        return NULL;
    }

    SplitedStr *result = GetFollowXPathByMode(pathSuffix, followMode, extra);
    for (size_t i = MAX_CFG_POLICY_DIRS_CNT; i > 0; i--) {
        if (dirs->paths[i - 1] == NULL) {
            continue;
        }
        // check follow x
        for (int j = (result ? result->segCount : 0); j > 0; j--) {
            if (result->segs[j - 1] &&
                snprintf_s(buf, bufLength, bufLength - 1, "%s/%s/%s", dirs->paths[i - 1], result->segs[j - 1],
                pathSuffix) > 0 &&
                access(buf, F_OK) == 0) {
                break;
            }
            *buf = '\0';
        }
        if (*buf != '\0') {
            break;
        }
        if (snprintf_s(buf, bufLength, bufLength - 1, "%s/%s", dirs->paths[i - 1], pathSuffix) > 0 &&
            access(buf, F_OK) == 0) {
            break;
        }
        *buf = '\0';
    }
    FreeCfgDirList(dirs);
    FreeSplitedStr(result);
    return (*buf != '\0') ? buf : NULL;
}

char *GetOneCfgFile(const char *pathSuffix, char *buf, unsigned int bufLength)
{
    return GetOneCfgFileEx(pathSuffix, buf, bufLength, FOLLOWX_MODE_DEFAULT, NULL);
}

CfgFiles *GetCfgFilesEx(const char *pathSuffix, int followMode, const char *extra)
{
    if (pathSuffix == NULL) {
        return NULL;
    }
    char buf[MAX_PATH_LEN] = {0};
    CfgDir *dirs = GetCfgDirList();
    if (dirs == NULL) {
        return NULL;
    }
    CfgFiles *files = (CfgFiles *)(malloc(sizeof(CfgFiles)));
    if (files == NULL) {
        FreeCfgDirList(dirs);
        return NULL;
    }

    SplitedStr *result = GetFollowXPathByMode(pathSuffix, followMode, extra);
    (void)memset_s(files, sizeof(CfgFiles), 0, sizeof(CfgFiles));
    int index = 0;
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT && index < MAX_CFG_POLICY_DIRS_CNT; i++) {
        if (dirs->paths[i] == NULL) {
            continue;
        }
        if (snprintf_s(buf, MAX_PATH_LEN, MAX_PATH_LEN - 1, "%s/%s", dirs->paths[i], pathSuffix) > 0 &&
            access(buf, F_OK) == 0) {
            files->paths[index++] = strdup(buf);
        }
        for (int j = 0; result && j < result->segCount; j++) {
            if (result->segs[j] &&
                snprintf_s(buf, MAX_PATH_LEN, MAX_PATH_LEN - 1, "%s/%s/%s", dirs->paths[i], result->segs[j],
                pathSuffix) > 0 &&
                access(buf, F_OK) == 0) {
                files->paths[index++] = strdup(buf);
            }
        }
    }
    FreeCfgDirList(dirs);
    FreeSplitedStr(result);
    return files;
}

CfgFiles *GetCfgFiles(const char *pathSuffix)
{
    return GetCfgFilesEx(pathSuffix, FOLLOWX_MODE_DEFAULT, NULL);
}

CfgDir *GetCfgDirList()
{
    CfgDir *res = (CfgDir *)(malloc(sizeof(CfgDir)));
    if (res == NULL) {
        return NULL;
    }
    (void)memset_s(res, sizeof(CfgDir), 0, sizeof(CfgDir));
    GetCfgDirRealPolicyValue(res);
    char *next = res->realPolicyValue;
    if (next == NULL) {
        free(res);
        return NULL;
    }
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT; i++) {
        res->paths[i] = next;
        next = strchr(next, ':');
        if (next == NULL) {
            break;
        }
        *next = 0;
        next += 1;
    }
    return res;
}
