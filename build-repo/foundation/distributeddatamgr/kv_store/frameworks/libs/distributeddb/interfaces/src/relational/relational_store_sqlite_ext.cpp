/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <mutex>
#include <openssl/sha.h>
#include <string>
#include <sys/time.h>
#include <thread>
#include <vector>

// using the "sqlite3sym.h" in OHOS
#ifndef USE_SQLITE_SYMBOLS
#include "sqlite3.h"
#else
#include "sqlite3sym.h"
#endif

namespace {
constexpr int E_OK = 0;
constexpr int E_ERROR = 1;
constexpr int BUSY_TIMEOUT = 2000;  // 2s.
class ValueHashCalc {
public:
    ValueHashCalc() {};
    ~ValueHashCalc()
    {
        delete context_;
        context_ = nullptr;
    }

    int Initialize()
    {
        context_ = new (std::nothrow) SHA256_CTX;
        if (context_ == nullptr) {
            return -E_ERROR;
        }

        int errCode = SHA256_Init(context_);
        if (errCode == 0) {
            return -E_ERROR;
        }
        return E_OK;
    }

    int Update(const std::vector<uint8_t> &value)
    {
        if (context_ == nullptr) {
            return -E_ERROR;
        }
        int errCode = SHA256_Update(context_, value.data(), value.size());
        if (errCode == 0) {
            return -E_ERROR;
        }
        return E_OK;
    }

    int GetResult(std::vector<uint8_t> &value)
    {
        if (context_ == nullptr) {
            return -E_ERROR;
        }

        value.resize(SHA256_DIGEST_LENGTH);
        int errCode = SHA256_Final(value.data(), context_);
        if (errCode == 0) {
            return -E_ERROR;
        }

        return E_OK;
    }

private:
    SHA256_CTX *context_ = nullptr;
};


const uint64_t MULTIPLES_BETWEEN_SECONDS_AND_MICROSECONDS = 1000000;

using Timestamp = uint64_t;
using TimeOffset = int64_t;

class TimeHelper {
public:
    constexpr static int64_t BASE_OFFSET = 10000LL * 365LL * 24LL * 3600LL * 1000LL * 1000LL * 10L; // 10000 year 100ns

    constexpr static int64_t MAX_VALID_TIME = BASE_OFFSET * 2; // 20000 year 100ns

    constexpr static uint64_t TO_100_NS = 10; // 1us to 100ns

    constexpr static Timestamp INVALID_TIMESTAMP = 0;

    // Get current system time
    static Timestamp GetSysCurrentTime()
    {
        uint64_t curTime = 0;
        int errCode = GetCurrentSysTimeInMicrosecond(curTime);
        if (errCode != E_OK) {
            return INVALID_TIMESTAMP;
        }

        std::lock_guard<std::mutex> lock(systemTimeLock_);
        // If GetSysCurrentTime in 1us, we need increase the currentIncCount_
        if (curTime == lastSystemTimeUs_) {
            // if the currentIncCount_ has been increased MAX_INC_COUNT, keep the currentIncCount_
            if (currentIncCount_ < MAX_INC_COUNT) {
                currentIncCount_++;
            }
        } else {
            lastSystemTimeUs_ = curTime;
            currentIncCount_ = 0;
        }
        return (curTime * TO_100_NS) + currentIncCount_; // Currently Timestamp is uint64_t
    }

    // Init the TimeHelper
    static void Initialize(Timestamp maxTimestamp)
    {
        std::lock_guard<std::mutex> lock(lastLocalTimeLock_);
        if (lastLocalTime_ < maxTimestamp) {
            lastLocalTime_ = maxTimestamp;
        }
    }

    static Timestamp GetTime(TimeOffset timeOffset)
    {
        Timestamp currentSysTime = GetSysCurrentTime();
        Timestamp currentLocalTime = currentSysTime + timeOffset;
        std::lock_guard<std::mutex> lock(lastLocalTimeLock_);
        if (currentLocalTime <= lastLocalTime_ || currentLocalTime > MAX_VALID_TIME) {
            lastLocalTime_++;
            currentLocalTime = lastLocalTime_;
        } else {
            lastLocalTime_ = currentLocalTime;
        }
        return currentLocalTime;
    }

private:
    static int GetCurrentSysTimeInMicrosecond(uint64_t &outTime)
    {
        struct timeval rawTime;
        int errCode = gettimeofday(&rawTime, nullptr);
        if (errCode < 0) {
            return -E_ERROR;
        }
        outTime = static_cast<uint64_t>(rawTime.tv_sec) * MULTIPLES_BETWEEN_SECONDS_AND_MICROSECONDS +
            static_cast<uint64_t>(rawTime.tv_usec);
        return E_OK;
    }

    static std::mutex systemTimeLock_;
    static Timestamp lastSystemTimeUs_;
    static Timestamp currentIncCount_;
    static const uint64_t MAX_INC_COUNT = 9; // last bit from 0-9

    static Timestamp lastLocalTime_;
    static std::mutex lastLocalTimeLock_;
};

std::mutex TimeHelper::systemTimeLock_;
Timestamp TimeHelper::lastSystemTimeUs_ = 0;
Timestamp TimeHelper::currentIncCount_ = 0;
Timestamp TimeHelper::lastLocalTime_ = 0;
std::mutex TimeHelper::lastLocalTimeLock_;

struct TransactFunc {
    void (*xFunc)(sqlite3_context*, int, sqlite3_value**) = nullptr;
    void (*xStep)(sqlite3_context*, int, sqlite3_value**) = nullptr;
    void (*xFinal)(sqlite3_context*) = nullptr;
    void(*xDestroy)(void*) = nullptr;
};

int RegisterFunction(sqlite3 *db, const std::string &funcName, int nArg, void *uData, TransactFunc &func)
{
    if (db == nullptr) {
        return -E_ERROR;
    }
    return sqlite3_create_function_v2(db, funcName.c_str(), nArg, SQLITE_UTF8 | SQLITE_DETERMINISTIC, uData,
        func.xFunc, func.xStep, func.xFinal, func.xDestroy);
}

int CalcValueHash(const std::vector<uint8_t> &value, std::vector<uint8_t> &hashValue)
{
    ValueHashCalc hashCalc;
    int errCode = hashCalc.Initialize();
    if (errCode != E_OK) {
        return -E_ERROR;
    }

    errCode = hashCalc.Update(value);
    if (errCode != E_OK) {
        return -E_ERROR;
    }

    errCode = hashCalc.GetResult(hashValue);
    if (errCode != E_OK) {
        return -E_ERROR;
    }

    return E_OK;
}

void CalcHashKey(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
    // 1 means that the function only needs one parameter, namely key
    if (ctx == nullptr || argc != 1 || argv == nullptr) {
        return;
    }
    auto keyBlob = static_cast<const uint8_t *>(sqlite3_value_blob(argv[0]));
    if (keyBlob == nullptr) {
        sqlite3_result_error(ctx, "Parameters is invalid.", -1);
        return;
    }
    int blobLen = sqlite3_value_bytes(argv[0]);
    std::vector<uint8_t> value(keyBlob, keyBlob + blobLen);
    std::vector<uint8_t> hashValue;
    int errCode = CalcValueHash(value, hashValue);
    if (errCode != E_OK) {
        sqlite3_result_error(ctx, "Get hash value error.", -1);
        return;
    }
    sqlite3_result_blob(ctx, hashValue.data(), hashValue.size(), SQLITE_TRANSIENT);
    return;
}

int RegisterCalcHash(sqlite3 *db)
{
    TransactFunc func;
    func.xFunc = &CalcHashKey;
    return RegisterFunction(db, "calc_hash", 1, nullptr, func);
}

void GetSysTime(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
    if (ctx == nullptr || argc != 1 || argv == nullptr) { // 1: function need one parameter
        return;
    }
    int timeOffset = static_cast<int64_t>(sqlite3_value_int64(argv[0]));
    sqlite3_result_int64(ctx, (sqlite3_int64)TimeHelper::GetTime(timeOffset));
}

int RegisterGetSysTime(sqlite3 *db)
{
    TransactFunc func;
    func.xFunc = &GetSysTime;
    return RegisterFunction(db, "get_sys_time", 1, nullptr, func);
}

int ResetStatement(sqlite3_stmt *&stmt)
{
    if (stmt == nullptr || sqlite3_finalize(stmt) != SQLITE_OK) {
        return -E_ERROR;
    }
    stmt = nullptr;
    return E_OK;
}

int GetStatement(sqlite3 *db, const std::string &sql, sqlite3_stmt *&stmt)
{
    int errCode = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (errCode != SQLITE_OK) {
        (void)ResetStatement(stmt);
        return -E_ERROR;
    }
    return E_OK;
}

int ExecuteRawSQL(sqlite3 *db, const std::string &sql)
{
    if (db == nullptr) {
        return -E_ERROR;
    }
    char *errMsg = nullptr;
    int errCode = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errCode != SQLITE_OK) {
        errCode = -E_ERROR;
    }

    if (errMsg != nullptr) {
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }
    return errCode;
}

int StepWithRetry(sqlite3_stmt *stmt)
{
    if (stmt == nullptr) {
        return -E_ERROR;
    }
    int errCode = sqlite3_step(stmt);
    if (errCode != SQLITE_DONE && errCode != SQLITE_ROW) {
        return -E_ERROR;
    }
    return errCode;
}

int GetColumnTestValue(sqlite3_stmt *stmt, int index, std::string &value)
{
    if (stmt == nullptr) {
        return -E_ERROR;
    }
    const unsigned char *val = sqlite3_column_text(stmt, index);
    value = (val != nullptr) ? std::string(reinterpret_cast<const char *>(val)) : std::string();
    return E_OK;
}

int GetCurrentMaxTimestamp(sqlite3 *db, Timestamp &maxTimestamp)
{
    if (db == nullptr) {
        return -E_ERROR;
    }
    std::string checkTableSql = "SELECT name FROM sqlite_master WHERE type = 'table' AND " \
        "name LIKE 'naturalbase_rdb_aux_%_log';";
    sqlite3_stmt *checkTableStmt = nullptr;
    int errCode = GetStatement(db, checkTableSql, checkTableStmt);
    if (errCode != E_OK) {
        return -E_ERROR;
    }
    while ((errCode = StepWithRetry(checkTableStmt)) != SQLITE_DONE) {
        if (errCode != SQLITE_ROW) {
            ResetStatement(checkTableStmt);
            return -E_ERROR;
        }
        std::string logTablename;
        GetColumnTestValue(checkTableStmt, 0, logTablename);
        if (logTablename.empty()) {
            continue;
        }

        std::string getMaxTimestampSql = "SELECT MAX(timestamp) FROM " + logTablename + ";";
        sqlite3_stmt *getTimeStmt = nullptr;
        errCode = GetStatement(db, getMaxTimestampSql, getTimeStmt);
        if (errCode != E_OK) {
            continue;
        }
        errCode = StepWithRetry(getTimeStmt);
        if (errCode != SQLITE_ROW) {
            ResetStatement(getTimeStmt);
            continue;
        }
        auto tableMaxTimestamp = static_cast<Timestamp>(sqlite3_column_int64(getTimeStmt, 0));
        maxTimestamp = (maxTimestamp > tableMaxTimestamp) ? maxTimestamp : tableMaxTimestamp;
        ResetStatement(getTimeStmt);
    }
    ResetStatement(checkTableStmt);
    return E_OK;
}

int ClearTheLogAfterDropTable(void *db, int actionCode, const char *tblName,
    const char *useLessParam, const char *schemaName, const char *triggerName)
{
    (void)useLessParam;
    (void)triggerName;
    if (actionCode != SQLITE_DROP_TABLE) {
        return SQLITE_OK;
    }
    if (db == nullptr || tblName == nullptr || schemaName == nullptr) {
        return SQLITE_DENY;
    }
    auto filepath = sqlite3_db_filename(static_cast<sqlite3 *>(db), schemaName);
    if (filepath == nullptr) {
        return SQLITE_DENY;
    }
    auto filename = std::string(filepath);
    std::thread th([filename, tableName = std::string(tblName), dropTimeStamp = TimeHelper::GetTime(0)] {
        sqlite3 *db = nullptr;
        (void)sqlite3_open(filename.c_str(), &db);
        if (db == nullptr) {
            return;
        }

        if (sqlite3_busy_timeout(db, BUSY_TIMEOUT) != SQLITE_OK) {
            sqlite3_close(db);
            return;
        }

        sqlite3_stmt *stmt = nullptr;
        std::string logTblName = "naturalbase_rdb_aux_" + std::string(tableName) + "_log";
        std::string sql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" + logTblName + "';";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            (void)sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        bool isLogTblExists = false;
        if (sqlite3_step(stmt) == SQLITE_ROW && static_cast<bool>(sqlite3_column_int(stmt, 0))) {
            isLogTblExists = true;
        }
        (void)sqlite3_finalize(stmt);
        stmt = nullptr;

        if (isLogTblExists) {
            RegisterGetSysTime(db);
            sql = "UPDATE " + logTblName + " SET flag=0x03, timestamp=get_sys_time(0) "
                  "WHERE flag&0x03=0x02 AND timestamp<" + std::to_string(dropTimeStamp);
            (void)sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
        }
        sqlite3_close(db);
        return;
    });
    th.detach();
    return SQLITE_OK;
}

void PostHandle(sqlite3 *db)
{
    Timestamp currentMaxTimestamp = 0;
    (void)GetCurrentMaxTimestamp(db, currentMaxTimestamp);
    TimeHelper::Initialize(currentMaxTimestamp);
    RegisterCalcHash(db);
    RegisterGetSysTime(db);
    (void)sqlite3_set_authorizer(db, &ClearTheLogAfterDropTable, db);
    (void)sqlite3_busy_timeout(db, BUSY_TIMEOUT);
    std::string recursiveTrigger = "PRAGMA recursive_triggers = ON;";
    (void)ExecuteRawSQL(db, recursiveTrigger);
}
}

SQLITE_API int sqlite3_open_relational(const char *filename, sqlite3 **ppDb)
{
    int err = sqlite3_open(filename, ppDb);
    if (err != SQLITE_OK) {
        return err;
    }
    PostHandle(*ppDb);
    return err;
}

SQLITE_API int sqlite3_open16_relational(const void *filename, sqlite3 **ppDb)
{
    int err = sqlite3_open16(filename, ppDb);
    if (err != SQLITE_OK) {
        return err;
    }
    PostHandle(*ppDb);
    return err;
}

SQLITE_API int sqlite3_open_v2_relational(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs)
{
    int err = sqlite3_open_v2(filename, ppDb, flags, zVfs);
    if (err != SQLITE_OK) {
        return err;
    }
    PostHandle(*ppDb);
    return err;
}

// hw export the symbols
#ifdef SQLITE_DISTRIBUTE_RELATIONAL
#if defined(__GNUC__)
#  define EXPORT_SYMBOLS  __attribute__ ((visibility ("default")))
#elif defined(_MSC_VER)
    #  define EXPORT_SYMBOLS  __declspec(dllexport)
#else
#  define EXPORT_SYMBOLS
#endif

struct sqlite3_api_routines_relational {
    int (*open)(const char *, sqlite3 **);
    int (*open16)(const void *, sqlite3 **);
    int (*open_v2)(const char *, sqlite3 **, int, const char *);
};

typedef struct sqlite3_api_routines_relational sqlite3_api_routines_relational;
static const sqlite3_api_routines_relational sqlite3HwApis = {
#ifdef SQLITE_DISTRIBUTE_RELATIONAL
    sqlite3_open_relational,
    sqlite3_open16_relational,
    sqlite3_open_v2_relational
#else
    0,
    0,
    0
#endif
};

EXPORT_SYMBOLS const sqlite3_api_routines_relational *sqlite3_export_relational_symbols = &sqlite3HwApis;
#endif