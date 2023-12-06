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
#include <stdio.h>
#include <string.h>
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_hidumper.h"
#include "fillpinc.h"
#include "nstackx.h"
#include "nstackx_dfile.h"
#include "softbus_hidumper_nstack.h"

#define SOFTBUS_DSTREAM_MODULE_NAME "dstream"
#define SOFTBUS_DSTREAM_MODULE_HELP "List all the dump item of dstream"
#define SOFTBUS_DFILE_MODULE_NAME "dfile"
#define SOFTBUS_DFILE_MODULE_HELP "List all the dump item of dfile"
#define SOFTBUS_DFINDLER_MODULE_NAME "dfinder"
#define SOFTBUS_DFINDLER_MODULE_HELP "List all the dump item of dfinder"
#define SOFTBUS_DMSG_MODULE_NAME "dmsg"
#define SOFTBUS_DMSG_MODULE_HELP "List all the dump item of dmsg"

#define SOFTBUF_NSTACK_DUMP_BUF_LEN (2048)

void SoftBufNstackDumpFunc(void *softObj, const char *data, uint32_t len)
{
    int fd = *(int *)softObj;
    size_t dataLen = strnlen(data, SOFTBUF_NSTACK_DUMP_BUF_LEN);
    if (dataLen == 0 || dataLen == SOFTBUF_NSTACK_DUMP_BUF_LEN || dataLen != len) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR,
            "SoftBufNstackDumpFunc len error, data strlen %d, len %d.", dataLen, len);
        return;
    }
    SOFTBUS_DPRINTF(fd, "%s", data);
}

static int32_t SoftBusNStackDstreamDumpHander(int fd, int32_t argc, const char **argv)
{
    if (fd < 0 || argc < 0 || argv == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "SoftBusNStackDstreamDumpHander invalid input");
        return SOFTBUS_ERR;
    }
#ifdef FILLP_ENHANCED
    if (FtDfxHiDumper((uint32_t)argc, argv, &fd, SoftBufNstackDumpFunc) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "call FtDfxHiDumper failed!");
        return SOFTBUS_ERR;
    }
#endif
    return SOFTBUS_OK;
}
static int32_t SoftBusNStackDfileDumpHander(int fd, int32_t argc, const char **argv)
{
    if (fd < 0 || argc < 0 || argv == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "SoftBusNStackDfileDumpHander invalid input");
        return SOFTBUS_ERR;
    }
#ifdef FILLP_ENHANCED
    if (NSTACKX_DFileDump((uint32_t)argc, argv, &fd, SoftBufNstackDumpFunc) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "call NSTACKX_DFileDump failed!");
        return SOFTBUS_ERR;
    }
#endif
    return SOFTBUS_OK;
}
static int32_t SoftBusNStackDumpDfinderHander(int fd, int32_t argc, const char **argv)
{
    if (fd < 0 || argc < 0 || argv == NULL) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "NSTACKX_DFinderDump invalid input!");
        return SOFTBUS_ERR;
    }
#ifdef FILLP_ENHANCED
    if (NSTACKX_DFinderDump(argv, (uint32_t)argc, &fd, SoftBufNstackDumpFunc) != 0) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "call NSTACKX_DFinderDump failed!");
        return SOFTBUS_ERR;
    }
#endif
    return SOFTBUS_OK;
}

static int32_t SoftBusNStackDmsgDumpHander(int fd, int32_t argc, const char **argv)
{
    if (fd < 0 || argc < 0 || argv == NULL) {
        return SOFTBUS_ERR;
    }

    return SOFTBUS_OK;
}

int32_t SoftBusNStackHiDumperInit(void)
{
    int32_t ret = SoftBusRegHiDumperHandler(SOFTBUS_DSTREAM_MODULE_NAME, SOFTBUS_DSTREAM_MODULE_HELP,
        &SoftBusNStackDstreamDumpHander);
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "SoftBusNStackHiDumperInit regist dstream handler fail");
        return ret;
    }

    ret = SoftBusRegHiDumperHandler(SOFTBUS_DFILE_MODULE_NAME, SOFTBUS_DFILE_MODULE_HELP,
        &SoftBusNStackDfileDumpHander);
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "SoftBusNStackHiDumperInit regist dstream handler fail");
        return ret;
    }

    ret = SoftBusRegHiDumperHandler(SOFTBUS_DFINDLER_MODULE_NAME, SOFTBUS_DFINDLER_MODULE_HELP,
        &SoftBusNStackDumpDfinderHander);
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "SoftBusNStackHiDumperInit regist dstream handler fail");
        return ret;
    }

    ret = SoftBusRegHiDumperHandler(SOFTBUS_DMSG_MODULE_NAME, SOFTBUS_DMSG_MODULE_HELP,
        &SoftBusNStackDmsgDumpHander);
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "SoftBusNStackHiDumperInit regist dstream handler fail");
        return ret;
    }
    return ret;
}
