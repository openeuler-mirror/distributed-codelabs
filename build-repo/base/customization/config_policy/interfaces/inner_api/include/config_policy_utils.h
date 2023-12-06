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

#ifndef CUSTOMIZATION_CONFIG_POLICY_UTILS_H
#define CUSTOMIZATION_CONFIG_POLICY_UTILS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // __cplusplus

#define MAX_CFG_POLICY_DIRS_CNT   32   // max number of directories
#define MAX_PATH_LEN              128  // max length of a filepath

// follow X(carrier/network/PLMN/...) usage
// 1. etc/cust/followx_file_list.cfg can be in any layer, except follow x dir
// 2. file format:
//    1)line begin with '#' is comment
//    2)every line is a item, item begin with "follow_rule," is a follow rule
//    3)follow rule item has 3~4 segs, every segs split by ',', as below
//      follow_rule,relPath,follow_mode,follow_rule_add_path
//      >> if follow mode is USER_DEFINE, the next segs is a user defined follow rule
//      >> the follow_rule_add_path can contains param variant
// 3. if a relPath has multi follow rules, use the highest priority rule
// LIMIT: the max size of concatenated all follow rule is PARAM_CONST_VALUE_LEN_MAX(4096)
// example:
//    follow_rule,etc/xml/config.xml,10
//    follow_rule,etc/xml/config1.xml,100,etc/carrier2/${keyname}
//    follow_rule,etc/xml/config2.xml,100,etc/carrier/${keyname:-value}
// Follow rule in followx_file_list.cfg, which stored in param variant
#define FOLLOWX_MODE_DEFAULT      0
// Not use any follow rule, even exsit followx_file_list.cfg
#define FOLLOWX_MODE_NO_FOLLOW    1
// Follow rule by default slot
#define FOLLOWX_MODE_SIM_DEFAULT  10
// Follow rule by slot 1
#define FOLLOWX_MODE_SIM_1        11
// Follow rule by slot 2
#define FOLLOWX_MODE_SIM_2        12
// User defined follow rule, get follow_rule_add_path from @param extra
// Notice: Follow rule in followx_file_list.cfg will be ignored.
#define FOLLOWX_MODE_USER_DEFINE  100

// Config Files
struct CfgFiles {
    char *paths[MAX_CFG_POLICY_DIRS_CNT];
};

// Config Directories
struct CfgDir {
    char *paths[MAX_CFG_POLICY_DIRS_CNT];
    char *realPolicyValue;
};

typedef struct CfgFiles CfgFiles;
typedef struct CfgDir CfgDir;

// free struct CfgFiles allocated by GetCfgFiles()
void FreeCfgFiles(CfgFiles *res);

// free struct CfgDir allocated by GetCfgDirList()
void FreeCfgDirList(CfgDir *res);

// get the highest priority config file
// pathSuffixStr: the relative path of the config file, e.g. "etc/xml/config.xml"
// buf: recommended buffer length is MAX_PATH_LEN
// followMode: 0/1/10/11/12/100, see FOLLOWX_MODE_*
// extra: User defined follow rule, get follow_rule_add_path
// return: path of the highest priority config file, return '\0' when such a file is not found
char *GetOneCfgFileEx(const char *pathSuffix, char *buf, unsigned int bufLength, int followMode, const char *extra);

// get the highest priority config file
// pathSuffixStr: the relative path of the config file, e.g. "etc/xml/config.xml"
// buf: recommended buffer length is MAX_PATH_LEN
// return: path of the highest priority config file, return '\0' when such a file is not found
char *GetOneCfgFile(const char *pathSuffix, char *buf, unsigned int bufLength);

// get config files, ordered by priority from low to high
// pathSuffixStr: the relative path of the config file, e.g. "etc/xml/config.xml"
// return: paths of config files
// CAUTION: please use FreeCfgFiles() to avoid memory leak.
CfgFiles *GetCfgFiles(const char *pathSuffix);

// get config files, ordered by priority from low to high
// pathSuffixStr: the relative path of the config file, e.g. "etc/xml/config.xml"
// followMode: 0/1/10/11/12/100, see FOLLOWX_MODE_*
// extra: User defined follow rule, get follow_rule_add_path
// return: paths of config files
// CAUTION: please use FreeCfgFiles() to avoid memory leak.
CfgFiles *GetCfgFilesEx(const char *pathSuffix, int followMode, const char *extra);

// get config directories, ordered by priority from low to high
// return: paths of config directories
// CAUTION: please use FreeCfgDirList() to avoid memory leak.
CfgDir *GetCfgDirList(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // __cplusplus

#endif // CUSTOMIZATION_CONFIG_POLICY_UTILS_H
