#include "hilog/log.h"

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <securec.h>
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>
#include <functional>

#include "log_utils.h"
#include "hilog_common.h"
#include "log_timestamp.h"
#include "vsnprintf_s_p.h"
#include "log_print.h"

using namespace std;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
#define HILOG_VA_ARGS_PROCESS(ret, level) \
    do { \
        va_list args; \
        va_start(args, fmt); \
        (ret) = ::HiLogPrintArgs(label.type, (level), label.domain, label.tag, fmt, args); \
        va_end(args); \
    } while (0)

int HiPrintLog(HilogMsg& header, const char *tag, uint16_t tagLen, const char *fmt, uint16_t fmtLen)
{
    LogContent content = {
        .level = header.level,
        .type = header.type,
        .pid = header.pid,
        .tid = header.tid,
        .domain = header.domain,
        .tv_sec = header.tv_sec,
        .tv_nsec = header.tv_nsec,
        .mono_sec = header.mono_sec,
        .tag = tag,
        .log = fmt,
    };
    LogFormat format = {
        .colorful = false,
        .timeFormat = FormatTime::TIME,
        .timeAccuFormat = FormatTimeAccu::MSEC,
        .year = false,
        .zone = false,
    };
    LogPrintWithFormat(content, format);
    return RET_SUCCESS;
}

int HiLogPrintArgs(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *fmt, va_list ap)
{
    if ((tag == nullptr)) {
        return -1;
    }

    HilogMsg header = {0};
    struct timespec ts = {0};
    (void)clock_gettime(CLOCK_REALTIME, &ts);
    struct timespec ts_mono = {0};
    (void)clock_gettime(CLOCK_MONOTONIC, &ts_mono);
    header.tv_sec = static_cast<uint32_t>(ts.tv_sec);
    header.tv_nsec = static_cast<uint32_t>(ts.tv_nsec);
    header.mono_sec = static_cast<uint32_t>(ts_mono.tv_sec);

    char buf[MAX_LOG_LEN] = {0};
    char *logBuf = buf;
    int traceBufLen = 0;
    int ret;
    bool priv = false;

#ifdef __clang__
/* code specific to clang compiler */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#elif __GNUC__
/* code for GNU C compiler */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    ret = vsnprintfp_s(logBuf, MAX_LOG_LEN - traceBufLen, MAX_LOG_LEN - traceBufLen - 1, priv, fmt, ap);
#ifdef __clang__
#pragma clang diagnostic pop
#elif __GNUC__
#pragma GCC diagnostic pop
#endif

    /* fill header info */
    auto tagLen = strnlen(tag, MAX_TAG_LEN - 1);
    auto logLen = strnlen(buf, MAX_LOG_LEN - 1);
    header.type = type;
    header.level = level;
    header.pid = getpid();
    header.tid = static_cast<uint32_t>(syscall(SYS_gettid));
    header.domain = domain;

    return HiPrintLog(header, tag, tagLen + 1, buf, logLen + 1);

}

int HiLog::Debug(const HiLogLabel &label, const char *fmt, ...)
{
    int ret;
    HILOG_VA_ARGS_PROCESS(ret, LOG_DEBUG);
    return ret;
}

int HiLog::Info(const HiLogLabel &label, const char *fmt, ...)
{
    int ret;
    HILOG_VA_ARGS_PROCESS(ret, LOG_INFO);
    return ret;
}

int HiLog::Warn(const HiLogLabel &label, const char *fmt, ...)
{
    int ret;
    HILOG_VA_ARGS_PROCESS(ret, LOG_WARN);
    return ret;
}

int HiLog::Error(const HiLogLabel &label, const char *fmt, ...)
{
    int ret;
    HILOG_VA_ARGS_PROCESS(ret, LOG_ERROR);
    return ret;
}

int HiLog::Fatal(const HiLogLabel &label, const char *fmt, ...)
{
    int ret;
    HILOG_VA_ARGS_PROCESS(ret, LOG_FATAL);
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS

int HiLogPrint(LogType type, LogLevel level, unsigned int domain, const char *tag, const char *fmt, ...)
{
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = HiLogPrintArgs(type, level, domain, tag, fmt, ap);
    va_end(ap);
    return ret;
}
