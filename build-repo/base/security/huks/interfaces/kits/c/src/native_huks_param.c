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

#include "native_huks_param.h"

#include "hks_param.h"
#include "hks_errcode_adapter.h"

static struct OH_Huks_Result ConvertParamResult(int32_t ret)
{
    struct HksResult result = HksConvertErrCode(ret);
    return *((struct OH_Huks_Result *)(&result));
}

struct OH_Huks_Result OH_Huks_InitParamSet(struct OH_Huks_ParamSet **paramSet)
{
    int32_t result = HksInitParamSet((struct HksParamSet **) paramSet);
    return ConvertParamResult(result);
}

struct OH_Huks_Result OH_Huks_AddParams(struct OH_Huks_ParamSet *paramSet,
    const struct OH_Huks_Param *params, uint32_t paramCnt)
{
    int32_t result = HksAddParams((struct HksParamSet *) paramSet,
        (const struct HksParam *) params, paramCnt);
    return ConvertParamResult(result);
}

struct OH_Huks_Result OH_Huks_BuildParamSet(struct OH_Huks_ParamSet **paramSet)
{
    int32_t result = HksBuildParamSet((struct HksParamSet **) paramSet);
    return ConvertParamResult(result);
}

void OH_Huks_FreeParamSet(struct OH_Huks_ParamSet **paramSet)
{
    HksFreeParamSet((struct HksParamSet **) paramSet);
}

struct OH_Huks_Result OH_Huks_CopyParamSet(const struct OH_Huks_ParamSet *fromParamSet,
    uint32_t fromParamSetSize, struct OH_Huks_ParamSet **paramSet)
{
    int32_t result = HksGetParamSet((const struct HksParamSet *) fromParamSet,
        fromParamSetSize, (struct HksParamSet **) paramSet);
    return ConvertParamResult(result);
}

struct OH_Huks_Result OH_Huks_GetParam(const struct OH_Huks_ParamSet *paramSet, uint32_t tag,
    struct OH_Huks_Param **param)
{
    int32_t result = HksGetParam((const struct HksParamSet *) paramSet, tag, (struct HksParam **) param);
    return ConvertParamResult(result);
}

struct OH_Huks_Result OH_Huks_FreshParamSet(struct OH_Huks_ParamSet *paramSet, bool isCopy)
{
    int32_t result = HksFreshParamSet((struct HksParamSet *) paramSet, isCopy);
    return ConvertParamResult(result);
}

struct OH_Huks_Result OH_Huks_IsParamSetTagValid(const struct OH_Huks_ParamSet *paramSet)
{
    int32_t result = HksCheckParamSetTag((const struct HksParamSet *) paramSet);
    return ConvertParamResult(result);
}

struct OH_Huks_Result OH_Huks_IsParamSetValid(const struct OH_Huks_ParamSet *paramSet, uint32_t size)
{
    int32_t result = HksCheckParamSet((const struct HksParamSet *) paramSet, size);
    return ConvertParamResult(result);
}

struct OH_Huks_Result OH_Huks_CheckParamMatch(const struct OH_Huks_Param *baseParam, const struct OH_Huks_Param *param)
{
    int32_t result = HksCheckParamMatch((const struct HksParam *) baseParam, (const struct HksParam *) param);
    return ConvertParamResult(result);
}
