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
#ifdef RELATIONAL_STORE
#include "relational_store_manager.h"

#include <thread>

#include "auto_launch.h"
#include "relational_store_instance.h"
#include "db_common.h"
#include "db_dfx_adapter.h"
#include "param_check_utils.h"
#include "log_print.h"
#include "db_errno.h"
#include "kv_store_errno.h"
#include "relational_store_changed_data_impl.h"
#include "relational_store_delegate_impl.h"
#include "runtime_context.h"
#include "platform_specific.h"

namespace DistributedDB {
namespace {
const int GET_CONNECT_RETRY = 3;
const int RETRY_GET_CONN_INTER = 30;
}

RelationalStoreManager::RelationalStoreManager(const std::string &appId, const std::string &userId, int32_t instanceId)
    : appId_(appId),
      userId_(userId),
      instanceId_(instanceId)
{}

static RelationalStoreConnection *GetOneConnectionWithRetry(const RelationalDBProperties &properties, int &errCode)
{
    for (int i = 0; i < GET_CONNECT_RETRY; i++) {
        auto conn = RelationalStoreInstance::GetDatabaseConnection(properties, errCode);
        if (conn != nullptr) {
            return conn;
        }
        if (errCode == -E_STALE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_GET_CONN_INTER));
        } else {
            return nullptr;
        }
    }
    return nullptr;
}

DB_API DBStatus RelationalStoreManager::OpenStore(const std::string &path, const std::string &storeId,
    const RelationalStoreDelegate::Option &option, RelationalStoreDelegate *&delegate)
{
    if (delegate != nullptr) {
        LOGE("[RelationalStoreMgr] Invalid delegate!");
        return INVALID_ARGS;
    }

    std::string canonicalDir;
    if (!ParamCheckUtils::CheckDataDir(path, canonicalDir)) {
        return INVALID_ARGS;
    }

    if (!ParamCheckUtils::CheckStoreParameter(storeId, appId_, userId_) || path.empty()) {
        return INVALID_ARGS;
    }

    RelationalDBProperties properties;
    properties.SetStringProp(RelationalDBProperties::DATA_DIR, canonicalDir);
    properties.SetIdentifier(userId_, appId_, storeId, instanceId_);
    properties.SetBoolProp(RelationalDBProperties::SYNC_DUAL_TUPLE_MODE, option.syncDualTupleMode);
    if (option.isEncryptedDb) {
        if (!ParamCheckUtils::CheckEncryptedParameter(option.cipher, option.passwd) || option.iterateTimes == 0) {
            return INVALID_ARGS;
        }
        properties.SetCipherArgs(option.cipher, option.passwd, option.iterateTimes);
    }

    int errCode = E_OK;
    auto *conn = GetOneConnectionWithRetry(properties, errCode);
    if (errCode == -E_INVALID_PASSWD_OR_CORRUPTED_DB) {
        DBDfxAdapter::ReportFault( { DBDfxAdapter::EVENT_OPEN_DATABASE_FAILED, userId_, appId_, storeId, errCode } );
    }
    if (conn == nullptr) {
        return TransferDBErrno(errCode);
    }

    delegate = new (std::nothrow) RelationalStoreDelegateImpl(conn, path);
    if (delegate == nullptr) {
        conn->Close();
        return DB_ERROR;
    }
    conn->RegisterObserverAction([option, storeId, this](const std::string &changedDevice) {
        RelationalStoreChangedDataImpl data(changedDevice);
        data.SetStoreProperty({userId_, appId_, storeId});
        if (option.observer) {
            LOGD("begin to observer on changed, changedDevice=%s", STR_MASK(changedDevice));
            option.observer->OnChange(data);
        }
    });
    return OK;
}

DBStatus RelationalStoreManager::CloseStore(RelationalStoreDelegate *store)
{
    if (store == nullptr) {
        return INVALID_ARGS;
    }

    auto storeImpl = static_cast<RelationalStoreDelegateImpl *>(store);
    DBStatus status = storeImpl->Close();
    if (status == BUSY) {
        LOGD("NbDelegateImpl is busy now.");
        return BUSY;
    }
    storeImpl->SetReleaseFlag(true);
    delete store;
    store = nullptr;
    return OK;
}

std::string RelationalStoreManager::GetDistributedTableName(const std::string &device, const std::string &tableName)
{
    if (device.empty() || tableName.empty()) {
        return {};
    }
    return DBCommon::GetDistributedTableName(device, tableName);
}

void RelationalStoreManager::SetAutoLaunchRequestCallback(const AutoLaunchRequestCallback &callback)
{
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(callback, DBType::DB_RELATION);
}

std::string RelationalStoreManager::GetRelationalStoreIdentifier(const std::string &userId, const std::string &appId,
    const std::string &storeId, bool syncDualTupleMode)
{
    if (!ParamCheckUtils::CheckStoreParameter(storeId, appId, userId, syncDualTupleMode)) {
        return "";
    }
    if (syncDualTupleMode) {
        return DBCommon::TransferHashString(appId + "-" + storeId);
    }
    return DBCommon::TransferHashString(DBCommon::GenerateIdentifierId(storeId, appId, userId));
}
} // namespace DistributedDB
#endif