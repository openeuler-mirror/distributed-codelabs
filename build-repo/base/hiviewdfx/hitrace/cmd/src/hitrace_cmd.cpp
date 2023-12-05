/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <getopt.h>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <zlib.h>

#include "hitrace_meter.h"
#include "hitrace_osal.h"
#include "securec.h"

using namespace std;
using namespace OHOS::HiviewDFX::HitraceOsal;

namespace {
constexpr struct option LONG_OPTIONS[] = {
    { "buffer_size",       required_argument, nullptr, 0 },
    { "trace_clock",       required_argument, nullptr, 0 },
    { "help",              no_argument,       nullptr, 0 },
    { "output",            required_argument, nullptr, 0 },
    { "time",              required_argument, nullptr, 0 },
    { "trace_begin",       no_argument,       nullptr, 0 },
    { "trace_finish",      no_argument,       nullptr, 0 },
    { "trace_dump",        no_argument,       nullptr, 0 },
    { "list_categories",   no_argument,       nullptr, 0 },
    { "overwrite",         no_argument,       nullptr, 0 },
    { nullptr,             0,                 nullptr, 0 },
};
const unsigned int CHUNK_SIZE = 65536;
const int BLOCK_SIZE = 4096;
const int SHELL_UID = 2000;
const int WAIT_MILLISECONDS = 10;

constexpr const char *TRACE_TAG_PROPERTY = "debug.hitrace.tags.enableflags";

// various operating paths of ftrace
constexpr const char *TRACING_ON_PATH = "tracing_on";
constexpr const char *TRACE_PATH = "trace";
constexpr const char *TRACE_MARKER_PATH = "trace_marker";

// support customization of some parameters
const int MIN_BUFFER_SIZE = 256;
const int MAX_BUFFER_SIZE = 307200; // 300 MB
constexpr unsigned int MAX_OUTPUT_LEN = 255;
const int PAGE_SIZE_KB = 4; // 4 KB
int g_traceDuration = 5;
int g_bufferSizeKB = 2048;
string g_clock = "boot";
bool g_overwrite = true;
string g_outputFile;
bool g_compress = false;

string g_traceRootPath;

const unsigned int START_NONE = 0;
const unsigned int START_NORMAL = 1;
const unsigned int START_ASYNC = 2;
unsigned int g_traceStart = START_NORMAL;
bool g_traceStop = true;
bool g_traceDump = true;

map<string, TagCategory> g_tagMap;
vector<uint64_t> g_userEnabledTags;
vector<string> g_kernelEnabledPaths;
}

static bool IsTraceMounted()
{
    const string debugfsPath = "/sys/kernel/debug/tracing/";
    const string tracefsPath = "/sys/kernel/tracing/";

    if (access((debugfsPath + TRACE_MARKER_PATH).c_str(), F_OK) != -1) {
        g_traceRootPath = debugfsPath;
        return true;
    }
    if (access((tracefsPath + TRACE_MARKER_PATH).c_str(), F_OK) != -1) {
        g_traceRootPath = tracefsPath;
        return true;
    }

    (void)fprintf(stderr, "Error: Did not find trace folder\n");
    return false;
}

static bool IsFileExit(const string& filename)
{
    return access((g_traceRootPath + filename).c_str(), F_OK) != -1;
}

static bool IsWritableFile(const string& filename)
{
    return access((g_traceRootPath + filename).c_str(), W_OK) != -1;
}

static bool WriteStrToFile(const string& filename, const std::string& str)
{
    ofstream out;
    out.open(g_traceRootPath + filename, ios::out);
    if (out.fail()) {
        fprintf(stderr, "Error: Did not open %s\n", filename.c_str());
        return false;
    }
    out << str;
    if (out.bad()) {
        fprintf(stderr, "Error: Did not write %s\n", filename.c_str());
        out.close();
        return false;
    }
    out.flush();
    out.close();
    return true;
}

static bool SetFtraceEnabled(const string& path, bool enabled)
{
    return WriteStrToFile(path, enabled ? "1" : "0");
}

static bool IsTagSupported(const string& name)
{
    auto it = g_tagMap.find(name);
    if (it == g_tagMap.end()) {
        return false;
    }

    TagCategory tagCategory = it->second;
    if (tagCategory.type != KERNEL) {
        g_userEnabledTags.push_back(tagCategory.tag);
        return true;
    }

    bool findPath = false;
    for (int i = 0; i < MAX_SYS_FILES; i++) {
        string path = tagCategory.SysFiles[i].path;
        if (path.size() == 0) {
            continue;
        }
        if (IsWritableFile(path)) {
            g_kernelEnabledPaths.push_back(std::move(path));
            findPath = true;
        } else if (IsFileExit(path)) {
            fprintf(stderr, "Warning: category \"%s\" requires root "
                "privileges.\n", name.c_str());
        }
    }
    return findPath;
}

static string CanonicalizeSpecPath(const char* src)
{
    if (src == nullptr || strlen(src) >= PATH_MAX) {
        fprintf(stderr, "Error: CanonicalizeSpecPath failed\n");
        return "";
    }
    char resolvedPath[PATH_MAX] = { 0 };
#if defined(_WIN32)
    if (!_fullpath(resolvedPath, src, PATH_MAX)) {
        fprintf(stderr, "Error: _fullpath %s failed\n", src);
        return "";
    }
#else
    if (access(src, F_OK) == 0) {
        if (realpath(src, resolvedPath) == nullptr) {
            fprintf(stderr, "Error: realpath %s failed\n", src);
            return "";
        }
    } else {
        string fileName(src);
        if (fileName.find("..") == string::npos) {
            if (sprintf_s(resolvedPath, PATH_MAX, "%s", src) == -1) {
                fprintf(stderr, "Error: sprintf_s %s failed\n", src);
                return "";
            }
        } else {
            fprintf(stderr, "Error: find .. %s failed\n", src);
            return "";
        }
    }
#endif
    string res(resolvedPath);
    return res;
}

static string ReadFile(const string& filename)
{
    string resolvedPath = CanonicalizeSpecPath((g_traceRootPath + filename).c_str());
    ifstream fin(resolvedPath.c_str());
    if (!fin.is_open()) {
        fprintf(stderr, "open file: %s failed!\n", (g_traceRootPath + filename).c_str());
        return "";
    }

    string str((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();
    return str;
}

static bool SetBufferSize(int bufferSize)
{
    constexpr const char *currentTracerPath = "current_tracer";
    if (!WriteStrToFile(currentTracerPath, "nop")) {
        fprintf(stderr, "Error: write \"nop\" to %s\n", currentTracerPath);
    }
    constexpr const char *bufferSizePath = "buffer_size_kb";
    return WriteStrToFile(bufferSizePath, to_string(bufferSize));
}

static bool SetClock(const string& timeclock)
{
    constexpr const char *traceClockPath = "trace_clock";
    string allClocks = ReadFile(traceClockPath);
    size_t begin = allClocks.find("[");
    size_t end = allClocks.find("]");
    string newClock;
    if (begin != string::npos && end != string::npos &&
        timeclock.compare(0, timeclock.size(), allClocks, begin + 1, end - begin - 1) >= 0) {
        return true;
    } else if (allClocks.find(timeclock) != string::npos) {
        newClock = timeclock;
    } else if (allClocks.find("boot") != string::npos) {
        // boot: This is the boot clock (CLOCK_BOOTTIME) and is based on the fast monotonic clock,
        // but also accounts for time in suspend.
        newClock = "boot";
    } else if (allClocks.find("mono") != string::npos) {
        // mono: uses the fast monotonic clock (CLOCK_MONOTONIC)
        // which is monotonic and is subject to NTP rate adjustments.
        newClock = "mono";
    } else if (allClocks.find("global") != string::npos) {
        // global: is in sync with all CPUs but may be a bit slower than the local clock.
        newClock = "global";
    } else {
        fprintf(stderr, "You can set trace clock in %s\n", allClocks.c_str());
        return false;
    }
    if (newClock.size() != 0) {
        return WriteStrToFile(traceClockPath, newClock);
    }
    return true;
}

static bool SetOverWriteEnable(bool enabled)
{
    constexpr const char *overWritePath = "options/overwrite";
    return SetFtraceEnabled(overWritePath, enabled);
}

static bool SetTgidEnable(bool enabled)
{
    constexpr const char *recordTgidPath = "options/record-tgid";
    return SetFtraceEnabled(recordTgidPath, enabled);
}

static bool DisableAllFtraceEvents()
{
    bool isTrue = true;
    for (auto it = g_tagMap.begin(); it != g_tagMap.end(); ++it) {
        TagCategory tag = it->second;
        if (tag.type != KERNEL) {
            continue;
        }
        for (int i = 0; i < MAX_SYS_FILES; i++) {
            const string path = tag.SysFiles[i].path;
            if ((path.size() > 0) && IsWritableFile(path)) {
                isTrue = isTrue && SetFtraceEnabled(path, false);
            }
        }
    }
    return isTrue;
}

static bool SetProperty(const string& property, const string& value)
{
    return SetPropertyInner(property, value);
}

static bool SetTraceTagsEnabled(uint64_t tags)
{
    string value = to_string(tags);
    return SetProperty(TRACE_TAG_PROPERTY, value);
}

static bool RefreshServices()
{
    bool res = false;

    res = RefreshBinderServices();
    if (!res) {
        return res;
    }
    res = RefreshHalServices();
    return res;
}

static bool SetUserSpaceSettings()
{
    uint64_t enabledTags = 0;
    for (auto tag: g_userEnabledTags) {
        enabledTags |= tag;
    }
    return SetTraceTagsEnabled(enabledTags) && RefreshServices();
}

static bool ClearUserSpaceSettings()
{
    return SetTraceTagsEnabled(0) && RefreshServices();
}

static bool SetKernelSpaceSettings()
{
    if (!(SetBufferSize(g_bufferSizeKB) && SetClock(g_clock) &&
        SetOverWriteEnable(g_overwrite) && SetTgidEnable(true))) {
        fprintf(stderr, "Set trace kernel settings failed\n");
        return false;
    }
    if (DisableAllFtraceEvents() == false) {
        fprintf(stderr, "Pre-clear kernel tracers failed\n");
        return false;
    }
    for (const auto& path : g_kernelEnabledPaths) {
        SetFtraceEnabled(path, true);
    }
    return true;
}

static bool ClearKernelSpaceSettings()
{
    return DisableAllFtraceEvents() && SetOverWriteEnable(true) && SetBufferSize(1) && SetClock("boot");
}

static void ShowListCategory()
{
    printf("  %18s   description:\n", "tagName:");
    for (auto it = g_tagMap.begin(); it != g_tagMap.end(); ++it) {
        string key = it->first;
        TagCategory tag = it->second;
        if (IsTagSupported(key)) {
            printf("  %18s - %s\n", tag.name.c_str(), tag.description.c_str());
        }
    }
}

static void ShowHelp(const string& cmd)
{
    printf("usage: %s [options] [categories...]\n", cmd.c_str());
    printf("options include:\n"
           "  -b N               Sets the size of the buffer (KB) for storing and reading traces. The default \n"
           "                     buffer size is 2048 KB.\n"
           "  --buffer_size N    Like \"-b N\".\n"
           "  -l                 Lists available hitrace categories.\n"
           "  --list_categories  Like \"-l\".\n"
           "  -t N               Sets the hitrace running duration in seconds (5s by default), which depends on \n"
           "                     the time required for analysis.\n"
           "  --time N           Like \"-t N\".\n"
           "  --trace_clock clock\n"
           "                     Sets the type of the clock for adding a timestamp to a trace, which can be\n"
           "                     boot (default), global, mono, uptime, or perf.\n"
           "  --trace_begin      Starts capturing traces.\n"
           "  --trace_dump       Dumps traces to a specified path (stdout by default).\n"
           "  --trace_finish     Stops capturing traces and dumps traces to a specified path (stdout by default).\n"
           "  --overwrite        Sets the action to take when the buffer is full. If this option is used,\n"
           "                     the latest traces are discarded; if this option is not used (default setting),\n"
           "                     the earliest traces are discarded.\n"
           "  -o filename        Specifies the name of the target file (stdout by default).\n"
           "  --output filename\n"
           "                     Like \"-o filename\".\n"
           "  -z                 Compresses a captured trace.\n"
    );
}

template <typename T>
inline bool StrToNum(const std::string& sString, T &tX)
{
    std::istringstream iStream(sString);
    return (iStream >> tX) ? true : false;
}

static bool ParseLongOpt(const string& cmd, int optionIndex)
{
    bool isTrue = true;
    if (!strcmp(LONG_OPTIONS[optionIndex].name, "buffer_size")) {
        if (!StrToNum(optarg, g_bufferSizeKB)) {
            fprintf(stderr, "Error: buffer size is illegal input. eg: \"--buffer_size 1024\"\n");
            isTrue = false;
        } else if (g_bufferSizeKB < MIN_BUFFER_SIZE || g_bufferSizeKB > MAX_BUFFER_SIZE) {
            fprintf(stderr, "Error: buffer size must be from 256 KB to 300 MB. eg: \"--buffer_size 1024\"\n");
            isTrue = false;
        }
        g_bufferSizeKB = g_bufferSizeKB / PAGE_SIZE_KB * PAGE_SIZE_KB;
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "trace_clock")) {
        regex re("[a-zA-Z]{4,6}");
        if (regex_match(optarg, re)) {
            g_clock = optarg;
        } else {
            fprintf(stderr, "Error: \"--trace_clock\" is illegal input. eg: \"--trace_clock boot\"\n");
            isTrue = false;
        }
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "help")) {
        ShowHelp(cmd);
        isTrue = false;
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "time")) {
        if (!StrToNum(optarg, g_traceDuration)) {
            fprintf(stderr, "Error: the time is illegal input. eg: \"--time 5\"\n");
            isTrue = false;
        } else if (g_traceDuration < 1) {
            fprintf(stderr, "Error: \"-t %s\" to be greater than zero. eg: \"--time 5\"\n", optarg);
            isTrue = false;
        }
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "list_categories")) {
        ShowListCategory();
        isTrue = false;
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "output")) {
        struct stat buf;
        size_t len = strnlen(optarg, MAX_OUTPUT_LEN);
        if (len == MAX_OUTPUT_LEN || len < 1 || (stat(optarg, &buf) == 0 && (buf.st_mode & S_IFDIR))) {
            fprintf(stderr, "Error: output file is illegal\n");
            isTrue = false;
        } else {
            g_outputFile = optarg;
        }
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "overwrite")) {
        g_overwrite = false;
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "trace_begin")) {
        g_traceStart = START_ASYNC;
        g_traceStop = false;
        g_traceDump = false;
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "trace_finish")) {
        g_traceStart = START_NONE;
        g_traceStop = true;
        g_traceDump = true;
    } else if (!strcmp(LONG_OPTIONS[optionIndex].name, "trace_dump")) {
        g_traceStart = START_NONE;
        g_traceStop = false;
        g_traceDump = true;
    }
    return isTrue;
}

static bool ParseOpt(int opt, char** argv, int optIndex)
{
    bool isTrue = true;
    switch (opt) {
        case 'b': {
            if (!StrToNum(optarg, g_bufferSizeKB)) {
                fprintf(stderr, "Error: buffer size is illegal input. eg: \"--buffer_size 1024\"\n");
                isTrue = false;
            } else if (g_bufferSizeKB < MIN_BUFFER_SIZE || g_bufferSizeKB > MAX_BUFFER_SIZE) {
                fprintf(stderr, "Error: buffer size must be from 256 KB to 300 MB. eg: \"--buffer_size 1024\"\n");
                isTrue = false;
            }
            g_bufferSizeKB = g_bufferSizeKB / PAGE_SIZE_KB * PAGE_SIZE_KB;
            break;
        }
        case 'h':
            ShowHelp(argv[0]);
            isTrue = false;
            break;
        case 'l':
            ShowListCategory();
            isTrue = false;
            break;
        case 't': {
            if (!StrToNum(optarg, g_traceDuration)) {
                fprintf(stderr, "Error: the time is illegal input. eg: \"--time 5\"\n");
                isTrue = false;
            } else if (g_traceDuration < 1) {
                fprintf(stderr, "Error: \"-t %s\" to be greater than zero. eg: \"--time 5\"\n", optarg);
                isTrue = false;
            }
            break;
        }
        case 'o': {
            struct stat buf;
            size_t len = strnlen(optarg, MAX_OUTPUT_LEN);
            if (len == MAX_OUTPUT_LEN || len < 1 || (stat(optarg, &buf) == 0 && (buf.st_mode & S_IFDIR))) {
                fprintf(stderr, "Error: output file is illegal\n");
                isTrue = false;
            } else {
                g_outputFile = optarg;
            }
            break;
        }
        case 'z':
            g_compress = true;
            break;
        case 0: // long options
            isTrue = ParseLongOpt(argv[0], optIndex);
            break;
        default:
            ShowHelp(argv[0]);
            isTrue = false;
            break;
    }
    return isTrue;
}

static void IsInvalidOpt(int argc, char** argv)
{
    for (int i = optind; i < argc; i++) {
        if (!IsTagSupported(argv[i])) {
            fprintf(stderr, "Error: \"%s\" is not support category on this device\n", argv[i]);
            exit(-1);
        }
    }
}

static bool HandleOpt(int argc, char** argv)
{
    bool isTrue = true;
    int opt = 0;
    int optionIndex = 0;
    string shortOption = "b:c:hlo:t:z";
    int argcSize = argc;
    while (isTrue && argcSize-- > 0) {
        opt = getopt_long(argc, argv, shortOption.c_str(), LONG_OPTIONS, &optionIndex);
        if (opt < 0) {
            IsInvalidOpt(argc, argv);
            break;
        }
        isTrue = ParseOpt(opt, argv, optionIndex);
    }
    return isTrue;
}

static bool TruncateFile(const string& path)
{
    int fd = creat((g_traceRootPath + path).c_str(), 0);
    if (fd == -1) {
        fprintf(stderr, "Error: clear %s, errno: %d\n", (g_traceRootPath + path).c_str(), errno);
        return false;
    }
    close(fd);
    fd = -1;
    return true;
}

static bool ClearTrace()
{
    return TruncateFile(TRACE_PATH);
}

static bool StartTrace()
{
    if (!SetFtraceEnabled(TRACING_ON_PATH, true)) {
        return false;
    }
    ClearTrace();
    printf("capturing trace...\n");
    fflush(stdout);
    return true;
}

static void WaitForTraceDone(void)
{
    struct timespec ts = {0, 0};
    ts.tv_sec = g_traceDuration;
    ts.tv_nsec = 0;
    while ((nanosleep(&ts, &ts) == -1) && (errno == EINTR)) {}
}

static bool StopTrace()
{
    return SetFtraceEnabled(TRACING_ON_PATH, false);
}

static void DumpCompressedTrace(int traceFd, int outFd)
{
    z_stream zs { nullptr };
    int flush = Z_NO_FLUSH;
    ssize_t bytesWritten;
    ssize_t bytesRead;
    if (memset_s(&zs, sizeof(zs), 0, sizeof(zs)) != 0) {
        fprintf(stderr, "Error: zip stream buffer init failed\n");
        return;
    }
    int ret = deflateInit(&zs, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "Error: initializing zlib: %d\n", ret);
        return;
    }
    std::unique_ptr<uint8_t[]>  in = std::make_unique<uint8_t[]>(CHUNK_SIZE);
    std::unique_ptr<uint8_t[]>  out = std::make_unique<uint8_t[]>(CHUNK_SIZE);
    if (!in || !out) {
        fprintf(stderr, "Error: couldn't allocate buffers\n");
        return;
    }
    zs.next_out = reinterpret_cast<Bytef*>(out.get());
    zs.avail_out = CHUNK_SIZE;

    do {
        if (zs.avail_in == 0 && flush == Z_NO_FLUSH) {
            bytesRead = TEMP_FAILURE_RETRY(read(traceFd, in.get(), CHUNK_SIZE));
            if (bytesRead == 0) {
                flush = Z_FINISH;
            } else if (bytesRead == -1) {
                fprintf(stderr, "Error: reading trace, errno: %d\n", errno);
                break;
            } else {
                zs.next_in = reinterpret_cast<Bytef*>(in.get());
                zs.avail_in = bytesRead;
            }
        }
        if (zs.avail_out == 0) {
            bytesWritten = TEMP_FAILURE_RETRY(write(outFd, out.get(), CHUNK_SIZE));
            if (bytesWritten < CHUNK_SIZE) {
                fprintf(stderr, "Error: writing deflated trace, errno: %d\n", errno);
                break;
            }
            zs.next_out = reinterpret_cast<Bytef*>(out.get());
            zs.avail_out = CHUNK_SIZE;
        }
        ret = deflate(&zs, flush);
        if (flush == Z_FINISH && ret == Z_STREAM_END) {
            size_t have = CHUNK_SIZE - zs.avail_out;
            bytesWritten = TEMP_FAILURE_RETRY(write(outFd, out.get(), have));
            if (static_cast<size_t>(bytesWritten) < have) {
                fprintf(stderr, "Error: writing deflated trace, errno: %d\n", errno);
            }
            break;
        } else if (ret != Z_OK) {
            if (ret == Z_ERRNO) {
                fprintf(stderr, "Error: deflate failed with errno %d\n", errno);
            } else {
                fprintf(stderr, "Error: deflate failed return %d\n", ret);
            }
            break;
        }
    } while (ret == Z_OK);

    ret = deflateEnd(&zs);
    if (ret != Z_OK) {
        fprintf(stderr, "error cleaning up zlib: %d\n", ret);
    }
}

static void DumpTrace(int outFd, const string& path)
{
    string resolvedPath = CanonicalizeSpecPath((g_traceRootPath + path).c_str());
    int traceFd = open(resolvedPath.c_str(), O_RDWR);
    if (traceFd == -1) {
        fprintf(stderr, "error opening %s, errno: %d\n", path.c_str(), errno);
        return;
    }
    ssize_t bytesWritten;
    ssize_t bytesRead;
    if (g_compress) {
        DumpCompressedTrace(traceFd, outFd);
    } else {
        char buffer[BLOCK_SIZE];
        do {
            bytesRead = TEMP_FAILURE_RETRY(read(traceFd, buffer, BLOCK_SIZE));
            if ((bytesRead == 0) || (bytesRead == -1)) {
                break;
            }
            bytesWritten = TEMP_FAILURE_RETRY(write(outFd, buffer, bytesRead));
        } while (bytesWritten > 0);
    }
    close(traceFd);
}

static bool MarkOthersClockSync()
{
    constexpr unsigned int bufferSize = 128; // buffer size
    char buffer[bufferSize] = { 0 };
    string resolvedPath = CanonicalizeSpecPath((g_traceRootPath + TRACE_MARKER_PATH).c_str());
    int fd = open(resolvedPath.c_str(), O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "Error: opening %s, errno: %d\n", TRACE_MARKER_PATH, errno);
        return false;
    }

    struct timespec mts = {0, 0};
    struct timespec rts = {0, 0};
    if (clock_gettime(CLOCK_REALTIME, &rts) == -1) {
        fprintf(stderr, "Error: get realtime, errno: %d\n", errno);
        close(fd);
        return false;
    } else if (clock_gettime(CLOCK_MONOTONIC, &mts) == -1) {
        fprintf(stderr, "Error: get parent_ts, errno: %d\n", errno);
        close(fd);
        return false;
    }
    constexpr unsigned int nanoSeconds = 1000000000; // seconds converted to nanoseconds
    constexpr unsigned int nanoToMill = 1000000; // millisecond converted to nanoseconds
    constexpr float nanoToSecond = 1000000000.0f; // consistent with the ftrace timestamp format
    int len = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
        "trace_event_clock_sync: realtime_ts=%" PRId64 "\n",
        static_cast<int64_t>((rts.tv_sec * nanoSeconds + rts.tv_nsec) / nanoToMill));
    if (len < 0) {
        fprintf(stderr, "Error: entering data into buffer, errno: %d\n", errno);
        close(fd);
        return false;
    }
    if (write(fd, buffer, len) < 0) {
        fprintf(stderr, "Warning: writing clock sync marker, errno: %d\n", errno);
        fprintf(stderr, "the buffer is not enough, please increase the buffer\n");
    }
    len = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, "trace_event_clock_sync: parent_ts=%f\n",
        static_cast<float>(((static_cast<float>(mts.tv_sec)) * nanoSeconds + mts.tv_nsec) / nanoToSecond));
    if (len < 0) {
        fprintf(stderr, "Error: entering data into buffer, errno: %d\n", errno);
        close(fd);
        return false;
    }
    if (write(fd, buffer, len) < 0) {
        fprintf(stderr, "Warning: writing clock sync marker, errno: %d\n", errno);
        fprintf(stderr, "the buffer is not enough, please increase the buffer\n");
    }
    close(fd);
    return true;
}

static void InitDiskSupportTags()
{
    g_tagMap["disk"] = { "disk", "Disk I/O", 0, KERNEL, {
        { "events/f2fs/f2fs_sync_file_enter/enable" },
        { "events/f2fs/f2fs_sync_file_exit/enable" },
        { "events/f2fs/f2fs_write_begin/enable" },
        { "events/f2fs/f2fs_write_end/enable" },
        { "events/ext4/ext4_da_write_begin/enable" },
        { "events/ext4/ext4_da_write_end/enable" },
        { "events/ext4/ext4_sync_file_enter/enable" },
        { "events/ext4/ext4_sync_file_exit/enable" },
        { "events/block/block_rq_issue/enable" },
        { "events/block/block_rq_complete/enable" },
    }};
    g_tagMap["mmc"] = { "mmc", "eMMC commands", 0, KERNEL, {
        { "events/mmc/enable" },
    }};
    g_tagMap["ufs"] = { "ufs", "UFS commands", 0, KERNEL, {
        { "events/ufs/enable" },
    }};
}

static void InitHardwareSupportTags()
{
    g_tagMap["irq"] = { "irq", "IRQ Events", 0, KERNEL, {
        { "events/irq/enable" },
        { "events/ipi/enable" },
    }};
    g_tagMap["irqoff"] = { "irqoff", "IRQ-disabled code section tracing", 0, KERNEL, {
        { "events/preemptirq/irq_enable/enable" },
        { "events/preemptirq/irq_disable/enable" },
    }};
    InitDiskSupportTags();
    g_tagMap["i2c"] = { "i2c", "I2C Events", 0, KERNEL, {
        { "events/i2c/enable" },
        { "events/i2c/i2c_read/enable" },
        { "events/i2c/i2c_write/enable" },
        { "events/i2c/i2c_result/enable" },
        { "events/i2c/i2c_reply/enable" },
        { "events/i2c/smbus_read/enable" },
        { "events/i2c/smbus_write/enable" },
        { "events/i2c/smbus_result/enable" },
        { "events/i2c/smbus_reply/enable" },
    }};
    g_tagMap["regulators"] = { "regulators", "Voltage and Current Regulators", 0, KERNEL, {
        { "events/regulator/enable" },
    }};
    g_tagMap["membus"] = { "membus", "Memory Bus Utilization", 0, KERNEL, {
        { "events/memory_bus/enable" },
    }};
}

static void InitCpuSupportTags()
{
    g_tagMap["freq"] = { "freq", "CPU Frequency", 0, KERNEL, {
        { "events/power/cpu_frequency/enable" },
        { "events/power/clock_set_rate/enable" },
        { "events/power/clock_disable/enable" },
        { "events/power/clock_enable/enable" },
        { "events/clk/clk_set_rate/enable" },
        { "events/clk/clk_disable/enable" },
        { "events/clk/clk_enable/enable" },
        { "events/power/cpu_frequency_limits/enable" },
    }};
    g_tagMap["idle"] = { "idle", "CPU Idle", 0, KERNEL, {
        { "events/power/cpu_idle/enable" },
    }};
    g_tagMap["load"] = { "load", "CPU Load", 0, KERNEL, {
        { "events/cpufreq_interactive/enable" },
    }};
}

static void InitKernelSupportTags()
{
    g_tagMap["sched"] = { "sched", "CPU Scheduling", 0, KERNEL, {
        { "events/sched/sched_switch/enable" },
        { "events/sched/sched_wakeup/enable" },
        { "events/sched/sched_wakeup_new/enable" },
        { "events/sched/sched_waking/enable" },
        { "events/sched/sched_blocked_reason/enable" },
        { "events/sched/sched_pi_setprio/enable" },
        { "events/sched/sched_process_exit/enable" },
        { "events/cgroup/enable" },
        { "events/oom/oom_score_adj_update/enable" },
        { "events/task/task_rename/enable" },
        { "events/task/task_newtask/enable" },
    }};
    g_tagMap["preemptoff"] = { "preemptoff", "Preempt-disabled code section tracing", 0, KERNEL, {
        { "events/preemptirq/preempt_enable/enable" },
        { "events/preemptirq/preempt_disable/enable" },
    }};

    g_tagMap["binder"] = { "binder", "Binder kernel Info", 0, KERNEL, {
        { "events/binder/binder_transaction/enable" },
        { "events/binder/binder_transaction_received/enable" },
        { "events/binder/binder_transaction_alloc_buf/enable" },
        { "events/binder/binder_set_priority/enable" },
        { "events/binder/binder_lock/enable" },
        { "events/binder/binder_locked/enable" },
        { "events/binder/binder_unlock/enable" },
    }};

    g_tagMap["sync"] = { "sync", "Synchronization", 0, KERNEL, {
        // linux kernel > 4.9
        { "events/dma_fence/enable" },
    }};
    g_tagMap["workq"] = { "workq", "Kernel Workqueues", 0, KERNEL, {
        { "events/workqueue/enable" },
    }};
    g_tagMap["memreclaim"] = { "memreclaim", "Kernel Memory Reclaim", 0, KERNEL, {
        { "events/vmscan/mm_vmscan_direct_reclaim_begin/enable" },
        { "events/vmscan/mm_vmscan_direct_reclaim_end/enable" },
        { "events/vmscan/mm_vmscan_kswapd_wake/enable" },
        { "events/vmscan/mm_vmscan_kswapd_sleep/enable" },
        { "events/lowmemorykiller/enable" },
    }};
    g_tagMap["pagecache"] = { "pagecache", "Page cache", 0, KERNEL, {
        { "events/filemap/enable" },
    }};
    g_tagMap["memory"] = { "memory", "Memory", 0, KERNEL, {
        { "events/kmem/rss_stat/enable" },
        { "events/kmem/ion_heap_grow/enable" },
        { "events/kmem/ion_heap_shrink/enable" },
    }};
    InitCpuSupportTags();
    InitHardwareSupportTags();
}

static void InitAllSupportTags()
{
    // OHOS
    g_tagMap["ohos"] = { "ohos", "OpenHarmony", HITRACE_TAG_OHOS, USER, {}};
    g_tagMap["ability"] = { "ability", "Ability Manager", HITRACE_TAG_ABILITY_MANAGER, USER, {}};
    g_tagMap["zcamera"] = { "zcamera", "OpenHarmony Camera Module", HITRACE_TAG_ZCAMERA, USER, {}};
    g_tagMap["zmedia"] = { "zmedia", "OpenHarmony Media Module", HITRACE_TAG_ZMEDIA, USER, {}};
    g_tagMap["zimage"] = { "zimage", "OpenHarmony Image Module", HITRACE_TAG_ZIMAGE, USER, {}};
    g_tagMap["zaudio"] = { "zaudio", "OpenHarmony Audio Module", HITRACE_TAG_ZAUDIO, USER, {}};
    g_tagMap["distributeddatamgr"] = { "distributeddatamgr", "Distributed Data Manager",
        HITRACE_TAG_DISTRIBUTEDDATA, USER, {}};
    g_tagMap["mdfs"] = { "mdfs", "Mobile Distributed File System", HITRACE_TAG_MDFS, USER, {}};
    g_tagMap["graphic"] = { "graphic", "Graphic Module", HITRACE_TAG_GRAPHIC_AGP, USER, {}};
    g_tagMap["ace"] = { "ace", "ACE development framework", HITRACE_TAG_ACE, USER, {}};
    g_tagMap["notification"] = { "notification", "Notification Module", HITRACE_TAG_NOTIFICATION, USER, {}};
    g_tagMap["misc"] = { "misc", "Misc Module", HITRACE_TAG_MISC, USER, {}};
    g_tagMap["multimodalinput"] = { "multimodalinput", "Multimodal Input Module",
        HITRACE_TAG_MULTIMODALINPUT, USER, {}};
    g_tagMap["sensors"] = { "sensors", "Sensors Module", HITRACE_TAG_SENSORS, USER, {}};
    g_tagMap["msdp"] = { "msdp", "Multimodal Sensor Data Platform", HITRACE_TAG_MSDP, USER, {}};
    g_tagMap["dsoftbus"] = { "dsoftbus", "Distributed Softbus", HITRACE_TAG_DSOFTBUS, USER, {}};
    g_tagMap["rpc"] = { "rpc", "RPC and IPC", HITRACE_TAG_RPC, USER, {}};
    g_tagMap["ark"] = { "ark", "ARK Module", HITRACE_TAG_ARK, USER, {}};
    g_tagMap["window"] = { "window", "Window Manager", HITRACE_TAG_WINDOW_MANAGER, USER, {}};
    g_tagMap["accessibility"] = { "accessibility", "Accessibility Manager",
        HITRACE_TAG_ACCESSIBILITY_MANAGER, USER, {}};
    g_tagMap["account"] = { "account", "Account Manager", HITRACE_TAG_ACCOUNT_MANAGER, USER, {}};
    g_tagMap["dhfwk"] = { "dhfwk", "Distributed Hardware FWK", HITRACE_TAG_DISTRIBUTED_HARDWARE_FWK, USER, {}};
    g_tagMap["dscreen"] = { "dscreen", "Distributed Screen", HITRACE_TAG_DISTRIBUTED_SCREEN, USER, {}};
    g_tagMap["daudio"] = { "daudio", "Distributed Audio", HITRACE_TAG_DISTRIBUTED_AUDIO, USER, {}};
    g_tagMap["dinput"] = { "dinput", "Distributed Input", HITRACE_TAG_DISTRIBUTED_INPUT, USER, {}};
    g_tagMap["devicemanager"] = { "devicemanager", "Device Manager", HITRACE_TAG_DEVICE_MANAGER, USER, {}};
    g_tagMap["deviceprofile"] = { "deviceprofile", "Device Profile", HITRACE_TAG_DEVICE_PROFILE, USER, {}};
    g_tagMap["dsched"] = { "dsched", "Distributed Schedule", HITRACE_TAG_DISTRIBUTED_SCHEDULE, USER, {}};
    g_tagMap["huks"] = { "huks", "Universal KeyStore", HITRACE_TAG_HUKS, USER, {}};
    g_tagMap["dlpcre"] = { "dlpcre", "Dlp Credential Service", HITRACE_TAG_DLP_CREDENTIAL, USER, {}};
    g_tagMap["samgr"] = { "samgr", "samgr", HITRACE_TAG_SAMGR, USER, {}};
    g_tagMap["app"] = { "app", "APP Module", HITRACE_TAG_APP, USER, {}};
    g_tagMap["dcamera"] = { "dcamera", "Distributed Camera", HITRACE_TAG_DISTRIBUTED_CAMERA, USER, {}};
    g_tagMap["zbinder"] = { "zbinder", "OpenHarmony binder communication", 0, KERNEL, {
        { "events/zbinder/enable" },
    }};
    g_tagMap["gresource"] = { "gresource", "Global Resource Manager", HITRACE_TAG_GLOBAL_RESMGR, USER, {}};
    g_tagMap["power"] = { "power", "Power Manager", HITRACE_TAG_POWER, USER, {}};
    g_tagMap["bluetooth"] = { "bluetooth", "communicatio bluetooth", HITRACE_TAG_BLUETOOTH, USER, {}};
    g_tagMap["filemanagement"] = { "filemanagement", "filemanagement", HITRACE_TAG_FILEMANAGEMENT, USER, {}};
    g_tagMap["dslm"] = {"dslm", "device security level", HITRACE_TAG_DLSM, USER, {}};
    g_tagMap["useriam"] = {"useriam", "useriam", HITRACE_TAG_USERIAM, USER, {}};
    g_tagMap["nweb"] = {"nweb", "NWEB Module", HITRACE_TAG_NWEB, USER, {}};
    g_tagMap["net"] = {"net", "net", HITRACE_TAG_NET, USER, {}};
    g_tagMap["accesscontrol"] = {"accesscontrol", "Access Control Module", HITRACE_TAG_ACCESS_CONTROL, USER, {}};
    g_tagMap["interconn"] = {"interconn", "Interconnection subsystem", HITRACE_TAG_INTERCONNECTION, USER, {}};

    g_tagMap["commonlibrary"] = {"commonlibrary", "commonlibrary subsystem", HITRACE_TAG_COMMONLIBRARY, USER, {}};
    // Kernel os
    InitKernelSupportTags();
}

static void InterruptExit(int signo)
{
    _exit(-1);
}

int main(int argc, char **argv)
{
    setgid(SHELL_UID);
    (void)signal(SIGKILL, InterruptExit);
    (void)signal(SIGINT, InterruptExit);

    if (!IsTraceMounted()) {
        exit(-1);
    }

    InitAllSupportTags();

    if (!HandleOpt(argc, argv)) {
        exit(-1);
    }

    if (g_traceStart != START_NONE) {
        if (!SetKernelSpaceSettings()) {
            ClearKernelSpaceSettings();
            exit(-1);
        }
    }

    bool isTrue = true;
    if (g_traceStart != START_NONE) {
        isTrue = isTrue && StartTrace();
        if (!SetUserSpaceSettings()) {
            ClearKernelSpaceSettings();
            ClearUserSpaceSettings();
            exit(-1);
        }
        if (g_traceStart == START_ASYNC) {
            return isTrue ? 0 : -1;
        }
        WaitForTraceDone();
    }

    // following is dump and stop handling
    isTrue = isTrue && MarkOthersClockSync();

    if (g_traceStop) {
        // clear user tags first and sleep a little to let apps already be notified.
        ClearUserSpaceSettings();
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MILLISECONDS));
        isTrue = isTrue && StopTrace();
    }

    if (isTrue && g_traceDump) {
        int outFd = STDOUT_FILENO;
        if (g_outputFile.size() > 0) {
            printf("write trace to %s\n", g_outputFile.c_str());
            string resolvedPath = CanonicalizeSpecPath(g_outputFile.c_str());
            outFd = open(resolvedPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        }
        if (outFd == -1) {
            fprintf(stderr, "Failed to open file '%s', err=%d", g_outputFile.c_str(), errno);
            isTrue = false;
        } else {
            dprintf(outFd, "TRACE:\n");
            DumpTrace(outFd, TRACE_PATH);
            if (outFd != STDOUT_FILENO) {
                close(outFd);
                outFd = -1;
            }
        }
        ClearTrace();
    }

    if (g_traceStop) {
        // clear kernel setting including clock type after dump(MUST) and tracing_on is off.
        ClearKernelSpaceSettings();
    }
    return isTrue ? 0 : -1;
}
