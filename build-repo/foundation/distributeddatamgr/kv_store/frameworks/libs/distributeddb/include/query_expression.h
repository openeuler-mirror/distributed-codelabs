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

#ifndef DISTRIBUTEDDB_QUERY_EXPRESSION_H
#define DISTRIBUTEDDB_QUERY_EXPRESSION_H

#include <string>
#include <vector>
#include <list>
#include <set>

#include "types_export.h"

namespace DistributedDB {
enum class QueryValueType: int32_t {
    VALUE_TYPE_INVALID = -1,
    VALUE_TYPE_NULL,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_LONG,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_STRING,
};

// value will const, it will influence query object id
// use high pos bit to distinguish operator type
enum class QueryObjType : uint32_t {
    OPER_ILLEGAL = 0x0000,
    EQUALTO = 0x0101,
    NOT_EQUALTO,
    GREATER_THAN,
    LESS_THAN,
    GREATER_THAN_OR_EQUALTO,
    LESS_THAN_OR_EQUALTO,
    LIKE = 0x0201,
    NOT_LIKE,
    IS_NULL,
    IS_NOT_NULL,
    IN = 0x0301,
    NOT_IN,
    QUERY_BY_KEY_PREFIX = 0x0401,
    BEGIN_GROUP = 0x0501,
    END_GROUP,
    AND = 0x0601,
    OR,
    LIMIT = 0x0701,
    ORDERBY,
    SUGGEST_INDEX = 0x0801,
    IN_KEYS = 0x0901,
};

struct QueryObjNode {
    QueryObjType operFlag = QueryObjType::OPER_ILLEGAL;
    std::string fieldName {};
    QueryValueType type = QueryValueType::VALUE_TYPE_INVALID;
    std::vector<FieldValue> fieldValue = {};
    bool IsValid()
    {
        return operFlag != QueryObjType::OPER_ILLEGAL &&
               type != QueryValueType::VALUE_TYPE_INVALID;
    }
};

enum class WriteTimeSort : int32_t {
    TIMESTAMP_ASC = 1,
    TIMESTAMP_DESC
};

class QueryExpression final {
public:
    DB_SYMBOL QueryExpression();
    DB_SYMBOL ~QueryExpression() {};

    void EqualTo(const std::string &field, const QueryValueType type, const FieldValue &value);

    void NotEqualTo(const std::string &field, const QueryValueType type, const FieldValue &value);

    void GreaterThan(const std::string &field, const QueryValueType type, const FieldValue &value);

    void LessThan(const std::string &field, const QueryValueType type, const FieldValue &value);

    void GreaterThanOrEqualTo(const std::string &field, const QueryValueType type, const FieldValue &value);

    void LessThanOrEqualTo(const std::string &field, const QueryValueType type, const FieldValue &value);

    void OrderBy(const std::string &field, bool isAsc);

    void Limit(int number, int offset);

    void Like(const std::string &field, const std::string &value);
    void NotLike(const std::string &field, const std::string &value);

    void In(const std::string &field, const QueryValueType type, const std::vector<FieldValue> &values);
    void NotIn(const std::string &field, const QueryValueType type, const std::vector<FieldValue> &values);

    void IsNull(const std::string &field);
    void IsNotNull(const std::string &field);

    void And();

    void Or();

    void BeginGroup();

    void EndGroup();

    void Reset();

    void QueryByPrefixKey(const std::vector<uint8_t> &key);

    void QueryBySuggestIndex(const std::string &indexName);

    std::vector<uint8_t> GetPreFixKey() const;

    void SetTableName(const std::string &tableName);
    const std::string &GetTableName();
    bool IsTableNameSpecified() const;

    std::string GetSuggestIndex() const;

    const std::set<Key> &GetKeys() const;
    void InKeys(const std::set<Key> &keys);

    const std::list<QueryObjNode> &GetQueryExpression();

    void SetErrFlag(bool flag);
    bool GetErrFlag();

    int GetSortType() const;
    void SetSortType(bool isAsc);

private:
    void AssemblyQueryInfo(const QueryObjType queryOperType, const std::string &field,
        const QueryValueType type, const std::vector<FieldValue> &value, bool isNeedFieldPath);

    std::list<QueryObjNode> queryInfo_;
    bool errFlag_ = true;
    std::vector<uint8_t> prefixKey_;
    std::string suggestIndex_;
    std::string tableName_;
    bool isTableNameSpecified_;
    std::set<Key> keys_;
    int sortType_ = 0;
};

// specialize for double
class GetQueryValueType {
public:
    static QueryValueType GetFieldTypeAndValue(const double &queryValue, FieldValue &fieldValue)
    {
        fieldValue.doubleValue = queryValue;
        return QueryValueType::VALUE_TYPE_DOUBLE;
    }
    static QueryValueType GetFieldTypeAndValue(const int &queryValue, FieldValue &fieldValue)
    {
        fieldValue.integerValue = queryValue;
        return QueryValueType::VALUE_TYPE_INTEGER;
    }
    static QueryValueType GetFieldTypeAndValue(const int64_t &queryValue, FieldValue &fieldValue)
    {
        fieldValue.longValue = queryValue;
        return QueryValueType::VALUE_TYPE_LONG;
    }
    static QueryValueType GetFieldTypeAndValue(const bool &queryValue, FieldValue &fieldValue)
    {
        fieldValue.boolValue = queryValue;
        return QueryValueType::VALUE_TYPE_BOOL;
    }
    static QueryValueType GetFieldTypeAndValue(const std::string &queryValue, FieldValue &fieldValue)
    {
        fieldValue.stringValue = queryValue;
        return QueryValueType::VALUE_TYPE_STRING;
    }
    static QueryValueType GetFieldTypeAndValue(const char *queryValue, FieldValue &fieldValue)
    {
        if (queryValue == nullptr) {
            return QueryValueType::VALUE_TYPE_STRING;
        }
        fieldValue.stringValue = queryValue;
        return QueryValueType::VALUE_TYPE_STRING;
    }
};
}
#endif