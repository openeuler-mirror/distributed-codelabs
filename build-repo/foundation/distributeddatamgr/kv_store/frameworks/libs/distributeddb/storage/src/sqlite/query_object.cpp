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

#include "query_object.h"
#include "db_errno.h"
#include "get_query_info.h"
#include "log_print.h"

namespace DistributedDB {
namespace {
const int INVALID_LIMIT = INT_MAX;
const int LIMIT_FIELD_VALUE_SIZE = 2;
}

QueryObject::QueryObject()
    : isValid_(true),
      initialized_(false),
      limit_(INVALID_LIMIT),
      offset_(0),
      hasOrderBy_(false),
      hasLimit_(false),
      hasPrefixKey_(false),
      hasInKeys_(false),
      orderByCounts_(0)
{
}

void QueryObject::GetAttrFromQueryObjNodes()
{
    for (const auto &iter : queryObjNodes_) {
        SymbolType symbolType = SqliteQueryHelper::GetSymbolType(iter.operFlag);
        if (iter.operFlag == QueryObjType::LIMIT) {
            hasLimit_ = true;
            if (iter.fieldValue.size() == LIMIT_FIELD_VALUE_SIZE) {
                limit_ = iter.fieldValue[0].integerValue;
                offset_ = iter.fieldValue[1].integerValue;
            }
        } else if (iter.operFlag == QueryObjType::ORDERBY) {
            hasOrderBy_ = true;
        } else if (symbolType == PREFIXKEY_SYMBOL) {
            hasPrefixKey_ = true;
        } else if (symbolType == IN_KEYS_SYMBOL) {
            hasInKeys_ = true;
        }
    }
}

QueryObject::QueryObject(const Query &query)
    : initialized_(false),
      limit_(INVALID_LIMIT),
      offset_(0),
      hasOrderBy_(false),
      hasLimit_(false),
      hasPrefixKey_(false),
      hasInKeys_(false),
      orderByCounts_(0)
{
    QueryExpression queryExpressions = GetQueryInfo::GetQueryExpression(query);
    queryObjNodes_ = queryExpressions.GetQueryExpression();
    GetAttrFromQueryObjNodes();
    isValid_ = queryExpressions.GetErrFlag();
    prefixKey_ = queryExpressions.GetPreFixKey();
    suggestIndex_ = queryExpressions.GetSuggestIndex();
    tableName_ = queryExpressions.GetTableName();
    isTableNameSpecified_ = queryExpressions.IsTableNameSpecified();
    keys_ = queryExpressions.GetKeys();
    sortType_ = static_cast<SortType>(queryExpressions.GetSortType());
}

QueryObject::QueryObject(const std::list<QueryObjNode> &queryObjNodes, const std::vector<uint8_t> &prefixKey,
    const std::set<Key> &keys)
    : queryObjNodes_(queryObjNodes),
      prefixKey_(prefixKey),
      keys_(keys),
      isValid_(true),
      initialized_(false),
      limit_(INVALID_LIMIT),
      offset_(0),
      hasOrderBy_(false),
      hasLimit_(false),
      hasPrefixKey_(false),
      hasInKeys_(false),
      orderByCounts_(0)
{
    GetAttrFromQueryObjNodes();
}

QueryObject::~QueryObject()
{}

int QueryObject::Init()
{
    if (initialized_) {
        return E_OK;
    }

    int errCode = Parse();
    if (errCode != E_OK) {
        LOGE("Parse query object err[%d]!", errCode);
        return errCode;
    }

    initialized_ = true;
    return errCode;
}

SqliteQueryHelper QueryObject::GetQueryHelper(int &errCode)
{
    errCode = Init();
    if (errCode != E_OK) {
        return SqliteQueryHelper(QueryObjInfo{});
    }
    QueryObjInfo info {schema_, queryObjNodes_, prefixKey_, suggestIndex_, keys_,
        orderByCounts_, isValid_, hasOrderBy_, hasLimit_, hasPrefixKey_, tableName_, isTableNameSpecified_, sortType_};
    return SqliteQueryHelper {info}; // compiler RVO by default, and RVO is generally required after C++17
}

bool QueryObject::IsValid()
{
    if (!initialized_) {
        (void)Init();
    }
    return isValid_;
}

bool QueryObject::HasLimit() const
{
    return hasLimit_;
}

void QueryObject::GetLimitVal(int &limit, int &offset) const
{
    limit = limit_;
    offset = offset_;
}

void QueryObject::SetSchema(const SchemaObject &schema)
{
    schema_ = schema;
}

bool QueryObject::IsCountValid() const
{
    if (hasLimit_ || hasOrderBy_) {
        LOGI("It is invalid for limit and orderby!");
        return false;
    }
    return true;
}

const std::vector<uint8_t> &QueryObject::GetPrefixKey() const
{
    return prefixKey_;
}

void QueryObject::ClearNodesFlag()
{
    limit_ = INVALID_LIMIT;
    offset_ = 0;
    isValid_ = true;
    hasOrderBy_ = false;
    hasLimit_ = false;
    hasPrefixKey_ = false;
    hasInKeys_ = false;
    orderByCounts_ = 0;
}

int QueryObject::Parse()
{
    if (!isValid_) {
        LOGE("Invalid query object!");
        return -E_INVALID_QUERY_FORMAT;
    }
    int errCode = ParseQueryObjNodes();
    if (errCode != E_OK) {
        LOGE("Check query object illegal!");
        isValid_ = false;
    }
    return errCode;
}

int QueryObject::ParseQueryObjNodes()
{
    ClearNodesFlag();

    auto iter = queryObjNodes_.begin();
    int errCode = E_OK;
    while (iter != queryObjNodes_.end()) {
        errCode = ParseNode(iter);
        if (errCode != E_OK) {
            return errCode;
        }
        iter++;
    }
    return errCode;
}

int QueryObject::ParseNode(const std::list<QueryObjNode>::iterator &iter)
{
    // The object is newly instantiated in the connection, and there is no reentrancy problem.
    if (!iter->IsValid()) {
        return -E_INVALID_QUERY_FORMAT;
    }

    switch (SqliteQueryHelper::GetSymbolType(iter->operFlag)) {
        case COMPARE_SYMBOL:
        case RELATIONAL_SYMBOL:
        case RANGE_SYMBOL:
            return CheckEqualFormat(iter);
        case LINK_SYMBOL:
            return CheckLinkerFormat(iter);
        case PREFIXKEY_SYMBOL: {
            if (hasPrefixKey_) {
                LOGE("Only filter by prefix key once!!");
                return -E_INVALID_QUERY_FORMAT;
            }
            hasPrefixKey_ = true;
            if (prefixKey_.size() > DBConstant::MAX_KEY_SIZE) {
                return -E_INVALID_ARGS;
            }
            return E_OK;
        }
        case SUGGEST_INDEX_SYMBOL:
            return CheckSuggestIndexFormat(iter);
        case IN_KEYS_SYMBOL: {
            if (hasInKeys_) {
                LOGE("Only filter by keys in once!!");
                return -E_INVALID_QUERY_FORMAT;
            }
            int errCode = CheckInKeys();
            if (errCode != E_OK) {
                return errCode;
            }
            hasInKeys_ = true;
            return E_OK;
        }
        default:
            return ParseNodeByOperFlag(iter);
    }
    return E_OK;
}

int QueryObject::ParseNodeByOperFlag(const std::list<QueryObjNode>::iterator &iter)
{
    switch (iter->operFlag) {
        case QueryObjType::LIMIT:
            hasLimit_ = true;
            if (iter->fieldValue.size() == LIMIT_FIELD_VALUE_SIZE) {
                limit_ = iter->fieldValue[0].integerValue;
                offset_ = iter->fieldValue[1].integerValue;
            }
            return CheckLimitFormat(iter);
        case QueryObjType::ORDERBY:
            return CheckOrderByFormat(iter);
        default:
            return E_OK;
    }
    return E_OK;
}

int QueryObject::CheckLinkerBefore(const std::list<QueryObjNode>::iterator &iter) const
{
    auto preIter = std::prev(iter, 1);
    SymbolType symbolType = SqliteQueryHelper::GetSymbolType(preIter->operFlag);
    if (symbolType != COMPARE_SYMBOL && symbolType != RELATIONAL_SYMBOL && symbolType != LOGIC_SYMBOL &&
        symbolType != RANGE_SYMBOL && symbolType != PREFIXKEY_SYMBOL && symbolType != IN_KEYS_SYMBOL) {
        LOGE("Must be a comparison operation before the connective! operFlag = %s", VNAME(preIter->operFlag));
        return -E_INVALID_QUERY_FORMAT;
    }
    return E_OK;
}

bool QueryObject::IsRelationalQuery() const
{
    return isTableNameSpecified_;
}

int QueryObject::CheckEqualFormat(const std::list<QueryObjNode>::iterator &iter) const
{
    if (!schema_.IsSchemaValid()) {
        LOGE("Schema is invalid!");
        return -E_NOT_SUPPORT;
    }

    FieldPath fieldPath;
    int errCode = SchemaUtils::ParseAndCheckFieldPath(iter->fieldName, fieldPath);
    if (errCode != E_OK) {
        return -E_INVALID_QUERY_FIELD;
    }

    FieldType schemaFieldType = FieldType::LEAF_FIELD_BOOL;
    errCode = schema_.CheckQueryableAndGetFieldType(fieldPath, schemaFieldType);
    if (errCode != E_OK) {
        LOGE("Get field type fail when check compare format! errCode = %d, fieldType = %u",
            errCode, static_cast<unsigned>(schemaFieldType));
        return -E_INVALID_QUERY_FIELD;
    }

    if (schemaFieldType == FieldType::LEAF_FIELD_BOOL &&
        SqliteQueryHelper::GetSymbolType(iter->operFlag) == COMPARE_SYMBOL &&
        iter->operFlag != QueryObjType::EQUALTO && iter->operFlag != QueryObjType::NOT_EQUALTO) { // bool can == or !=
        LOGE("Bool forbid compare!!!");
        return -E_INVALID_QUERY_FORMAT;
    }
    auto nextIter = std::next(iter, 1);
    if (nextIter != queryObjNodes_.end()) {
        SymbolType symbolType = SqliteQueryHelper::GetSymbolType(nextIter->operFlag);
        if (symbolType == RELATIONAL_SYMBOL || symbolType == COMPARE_SYMBOL || symbolType == RANGE_SYMBOL) {
            LOGE("After Compare you need, You need the conjunction like and or for connecting!");
            return -E_INVALID_QUERY_FORMAT;
        }
    }
    return E_OK;
}

int QueryObject::CheckLinkerFormat(const std::list<QueryObjNode>::iterator &iter) const
{
    auto itPre = iter;
    for (; itPre != queryObjNodes_.begin(); itPre = std::prev(itPre, 1)) {
        SymbolType symbolType = SqliteQueryHelper::GetSymbolType(std::prev(itPre, 1)->operFlag);
        if (symbolType != PREFIXKEY_SYMBOL && symbolType != IN_KEYS_SYMBOL) {
            break;
        }
    }
    if (itPre == queryObjNodes_.begin()) {
        LOGE("Connectives are not allowed in the first place!");
        return -E_INVALID_QUERY_FORMAT;
    }
    auto nextIter = std::next(iter, 1);
    if (nextIter == queryObjNodes_.end()) {
        LOGE("Connectives are not allowed in the last place!");
        return -E_INVALID_QUERY_FORMAT;
    }
    SymbolType symbolType = SqliteQueryHelper::GetSymbolType(nextIter->operFlag);
    if (symbolType == INVALID_SYMBOL || symbolType == LINK_SYMBOL || symbolType == SPECIAL_SYMBOL) {
        LOGE("Must be followed by comparison operation! operflag[%u], symbolType[%u]",
            static_cast<unsigned>(nextIter->operFlag), static_cast<unsigned>(symbolType));
        return -E_INVALID_QUERY_FORMAT;
    }
    return CheckLinkerBefore(iter);
}

int QueryObject::CheckSuggestIndexFormat(const std::list<QueryObjNode>::iterator &iter) const
{
    auto next = std::next(iter, 1);
    if (next != queryObjNodes_.end()) {
        LOGE("SuggestIndex only allowed once, and must appear at the end!");
        return -E_INVALID_QUERY_FORMAT;
    }
    return E_OK;
}

int QueryObject::CheckOrderByFormat(const std::list<QueryObjNode>::iterator &iter)
{
    if (!schema_.IsSchemaValid()) {
        return -E_NOT_SUPPORT;
    }

    FieldType schemaFieldType;
    FieldPath fieldPath;

    int errCode = SchemaUtils::ParseAndCheckFieldPath(iter->fieldName, fieldPath);
    if (errCode != E_OK) {
        return -E_INVALID_QUERY_FIELD;
    }
    errCode = schema_.CheckQueryableAndGetFieldType(fieldPath, schemaFieldType);
    if (errCode != E_OK) {
        return -E_INVALID_QUERY_FIELD;
    }
    if (schemaFieldType == FieldType::LEAF_FIELD_BOOL) {
        return -E_INVALID_QUERY_FORMAT;
    }
    hasOrderBy_ = true;
    ++orderByCounts_;
    LOGD("Need order by %d filed value!", orderByCounts_);
    return E_OK;
}

int QueryObject::CheckLimitFormat(const std::list<QueryObjNode>::iterator &iter) const
{
    auto next = std::next(iter, 1);
    if (next != queryObjNodes_.end() && SqliteQueryHelper::GetSymbolType(next->operFlag) != SUGGEST_INDEX_SYMBOL) {
        LOGE("Limit should be last node or just before suggest-index node!");
        return -E_INVALID_QUERY_FORMAT;
    }
    return E_OK;
}

bool QueryObject::IsQueryOnlyByKey() const
{
    return std::none_of(queryObjNodes_.begin(), queryObjNodes_.end(), [&](const QueryObjNode &node) {
        return node.operFlag != QueryObjType::LIMIT && node.operFlag != QueryObjType::QUERY_BY_KEY_PREFIX &&
            node.operFlag != QueryObjType::IN_KEYS;
    });
}

bool QueryObject::IsQueryForRelationalDB() const
{
    return isTableNameSpecified_ &&
        std::none_of(queryObjNodes_.begin(), queryObjNodes_.end(), [&](const QueryObjNode &node) {
        return node.operFlag != QueryObjType::EQUALTO && node.operFlag != QueryObjType::NOT_EQUALTO &&
            node.operFlag != QueryObjType::AND && node.operFlag != QueryObjType::OR &&
            node.operFlag != QueryObjType::ORDERBY && node.operFlag != QueryObjType::LIMIT;
    });
}

bool QueryObject::HasOrderBy() const
{
    return hasOrderBy_;
}

bool QueryObject::Empty() const
{
    return queryObjNodes_.empty();
}

int QueryObject::CheckInKeys() const
{
    if (keys_.empty()) {
        LOGE("Inkeys cannot be empty.");
        return -E_INVALID_ARGS;
    }
    if (keys_.size() > DBConstant::MAX_INKEYS_SIZE) {
        LOGE("Inkeys cannot be over 128.");
        return -E_MAX_LIMITS;
    }
    for (const auto &key : keys_) {
        if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE) {
            LOGE("The key in Inkeys cannot be empty or overlong, size:%zu.", key.size());
            return -E_INVALID_ARGS;
        }
    }
    return E_OK;
}

#ifdef RELATIONAL_STORE
int QueryObject::SetSchema(const RelationalSchemaObject &schemaObj)
{
    if (!isTableNameSpecified_) {
        return -E_INVALID_ARGS;
    }
    const auto &tableInfo = schemaObj.GetTable(tableName_);
    SchemaObject schema(tableInfo);
    schema_ = schema;
    return E_OK;
}
#endif
}

