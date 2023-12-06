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
#include "query_expression.h"
#include "log_print.h"
#include "schema_utils.h"
#include "db_errno.h"

namespace DistributedDB {
namespace {
    const int MAX_OPR_TIMES = 256;
} // namespace

void QueryExpression::AssemblyQueryInfo(const QueryObjType queryOperType, const std::string& field,
    const QueryValueType type, const std::vector<FieldValue> &values, bool isNeedFieldPath = true)
{
    if (queryInfo_.size() > MAX_OPR_TIMES) {
        SetErrFlag(false);
        LOGE("Operate too much times!");
        return;
    }

    if (!GetErrFlag()) {
        LOGE("Illegal data node!");
        return;
    }

    FieldPath outPath;
    if (isNeedFieldPath) {
        if (SchemaUtils::ParseAndCheckFieldPath(field, outPath) != E_OK) {
            SetErrFlag(false);
            LOGE("Field path illegal!");
            return;
        }
    }
    std::string formatedField;
    if (isTableNameSpecified_) { // remove '$.' prefix in relational query
        for (auto it = outPath.begin(); it < outPath.end(); ++it) {
            if (it != outPath.begin()) {
                formatedField += ".";
            }
            formatedField += *it;
        }
    } else {
        formatedField = field;
    }
    queryInfo_.emplace_back(QueryObjNode{queryOperType, formatedField, type, values});
}

QueryExpression::QueryExpression()
    : errFlag_(true),
      tableName_("sync_data"), // default kv type store table name
      isTableNameSpecified_(false) // default no specify for kv type store table name
{}

void QueryExpression::EqualTo(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::EQUALTO, field, type, fieldValues);
}

void QueryExpression::NotEqualTo(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::NOT_EQUALTO, field, type, fieldValues);
}

void QueryExpression::GreaterThan(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    if (type == QueryValueType::VALUE_TYPE_BOOL) {
        LOGD("Prohibit the use of bool for comparison!");
        SetErrFlag(false);
    }
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::GREATER_THAN, field, type, fieldValues);
}

void QueryExpression::LessThan(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    if (type == QueryValueType::VALUE_TYPE_BOOL) {
        LOGD("Prohibit the use of bool for comparison!");
        SetErrFlag(false);
    }
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::LESS_THAN, field, type, fieldValues);
}

void QueryExpression::GreaterThanOrEqualTo(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    if (type == QueryValueType::VALUE_TYPE_BOOL) {
        LOGD("Prohibit the use of bool for comparison!");
        SetErrFlag(false);
    }
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::GREATER_THAN_OR_EQUALTO, field, type, fieldValues);
}

void QueryExpression::LessThanOrEqualTo(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    if (type == QueryValueType::VALUE_TYPE_BOOL) {
        LOGD("Prohibit the use of bool for comparison!");
        SetErrFlag(false);
    }
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::LESS_THAN_OR_EQUALTO, field, type, fieldValues);
}

void QueryExpression::OrderBy(const std::string& field, bool isAsc)
{
    FieldValue fieldValue;
    fieldValue.boolValue = isAsc;
    std::vector<FieldValue> fieldValues{fieldValue};
    AssemblyQueryInfo(QueryObjType::ORDERBY, field, QueryValueType::VALUE_TYPE_BOOL, fieldValues);
}

void QueryExpression::Like(const std::string& field, const std::string &value)
{
    FieldValue fieldValue;
    fieldValue.stringValue = value;
    std::vector<FieldValue> fieldValues{fieldValue};
    AssemblyQueryInfo(QueryObjType::LIKE, field, QueryValueType::VALUE_TYPE_STRING, fieldValues);
}

void QueryExpression::NotLike(const std::string& field, const std::string &value)
{
    FieldValue fieldValue;
    fieldValue.stringValue = value;
    std::vector<FieldValue> fieldValues{fieldValue};
    AssemblyQueryInfo(QueryObjType::NOT_LIKE, field, QueryValueType::VALUE_TYPE_STRING, fieldValues);
}

void QueryExpression::Limit(int number, int offset)
{
    FieldValue fieldNumber;
    fieldNumber.integerValue = number;
    FieldValue fieldOffset;
    fieldOffset.integerValue = offset;
    std::vector<FieldValue> fieldValues{fieldNumber, fieldOffset};
    AssemblyQueryInfo(QueryObjType::LIMIT, std::string(), QueryValueType::VALUE_TYPE_INTEGER, fieldValues, false);
}

void QueryExpression::IsNull(const std::string& field)
{
    AssemblyQueryInfo(QueryObjType::IS_NULL, field, QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>());
}

void QueryExpression::IsNotNull(const std::string& field)
{
    AssemblyQueryInfo(QueryObjType::IS_NOT_NULL, field, QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>());
}

void QueryExpression::In(const std::string& field, const QueryValueType type, const std::vector<FieldValue> &values)
{
    AssemblyQueryInfo(QueryObjType::IN, field, type, values);
}

void QueryExpression::NotIn(const std::string& field, const QueryValueType type, const std::vector<FieldValue> &values)
{
    AssemblyQueryInfo(QueryObjType::NOT_IN, field, type, values);
}

void QueryExpression::And()
{
    AssemblyQueryInfo(QueryObjType::AND, std::string(), QueryValueType::VALUE_TYPE_NULL,
        std::vector<FieldValue>(), false);
}

void QueryExpression::Or()
{
    AssemblyQueryInfo(QueryObjType::OR, std::string(), QueryValueType::VALUE_TYPE_NULL,
        std::vector<FieldValue>(), false);
}

void QueryExpression::QueryByPrefixKey(const std::vector<uint8_t> &key)
{
    queryInfo_.emplace_front(QueryObjNode{QueryObjType::QUERY_BY_KEY_PREFIX, std::string(),
        QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>()});
    prefixKey_ = key;
}

void QueryExpression::QueryBySuggestIndex(const std::string &indexName)
{
    queryInfo_.emplace_back(QueryObjNode{QueryObjType::SUGGEST_INDEX, indexName,
        QueryValueType::VALUE_TYPE_STRING, std::vector<FieldValue>()});
    suggestIndex_ = indexName;
}

void QueryExpression::InKeys(const std::set<Key> &keys)
{
    queryInfo_.emplace_front(QueryObjNode{QueryObjType::IN_KEYS, std::string(), QueryValueType::VALUE_TYPE_NULL,
        std::vector<FieldValue>()});
    keys_ = keys;
}

const std::list<QueryObjNode> &QueryExpression::GetQueryExpression()
{
    if (!GetErrFlag()) {
        queryInfo_.clear();
        queryInfo_.emplace_back(QueryObjNode{QueryObjType::OPER_ILLEGAL});
        LOGE("Query operate illegal!");
    }
    return queryInfo_;
}

std::vector<uint8_t> QueryExpression::GetPreFixKey() const
{
    return prefixKey_;
}

void QueryExpression::SetTableName(const std::string &tableName)
{
    tableName_ = tableName;
    isTableNameSpecified_ = true;
}

const std::string &QueryExpression::GetTableName()
{
    return tableName_;
}

bool QueryExpression::IsTableNameSpecified() const
{
    return isTableNameSpecified_;
}

std::string QueryExpression::GetSuggestIndex() const
{
    return suggestIndex_;
}

const std::set<Key> &QueryExpression::GetKeys() const
{
    return keys_;
}

void QueryExpression::BeginGroup()
{
    queryInfo_.emplace_back(QueryObjNode{QueryObjType::BEGIN_GROUP, std::string(),
        QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>()});
}

void QueryExpression::EndGroup()
{
    queryInfo_.emplace_back(QueryObjNode{QueryObjType::END_GROUP, std::string(),
        QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>()});
}

void QueryExpression::Reset()
{
    errFlag_ = true;
    queryInfo_.clear();
    prefixKey_.clear();
    prefixKey_.shrink_to_fit();
    suggestIndex_.clear();
    keys_.clear();
}

void QueryExpression::SetErrFlag(bool flag)
{
    errFlag_ = flag;
}

bool QueryExpression::GetErrFlag()
{
    return errFlag_;
}

int QueryExpression::GetSortType() const
{
    return sortType_;
}

void QueryExpression::SetSortType(bool isAsc)
{
    WriteTimeSort sortType = isAsc ? WriteTimeSort::TIMESTAMP_ASC : WriteTimeSort::TIMESTAMP_DESC;
    sortType_ = static_cast<int>(sortType);
}
} // namespace DistributedDB
