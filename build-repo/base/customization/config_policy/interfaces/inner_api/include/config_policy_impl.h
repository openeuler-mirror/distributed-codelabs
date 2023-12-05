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

#ifndef CUSTOMIZATION_CONFIG_POLICY_IMPL_H
#define CUSTOMIZATION_CONFIG_POLICY_IMPL_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // __cplusplus

// these name is used for write data, init may use it
#define CUST_KEY_POLICY_LAYER "const.cust.config_dir_layer"
#define CUST_FOLLOW_X_RULES   "const.cust.follow_x_rules"
// opkey info for sim1
#define CUST_OPKEY0           "telephony.sim.opkey0"
// opkey info for sim1
#define CUST_OPKEY1           "telephony.sim.opkey1"

// if fs need path prefix, set ROOT_PREFIX before include current file
#ifndef ROOT_PREFIX
#define ROOT_PREFIX ""
#endif
#define DEFAULT_LAYER ROOT_PREFIX"/system:"ROOT_PREFIX"/chipset:"ROOT_PREFIX"/sys_prod:"ROOT_PREFIX"/chip_prod"

#ifdef __LITEOS_M__
#define MINI_CONFIG_POLICY_BUF_SIZE 256
// for mini system, if exceed max size will not set
void SetMiniConfigPolicy(const char *policy);
__WEAK void TrigSetMiniConfigPolicy();
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // __cplusplus

#endif // CUSTOMIZATION_CONFIG_POLICY_IMPL_H
