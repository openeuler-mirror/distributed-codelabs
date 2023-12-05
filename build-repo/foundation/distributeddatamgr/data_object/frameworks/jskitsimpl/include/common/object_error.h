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

#ifndef OBJECT_ERROR_H
#define OBJECT_ERROR_H

#include "js_utils.h"

namespace OHOS {
namespace ObjectStore {
static const int EXCEPTION_DEVICE_NOT_SUPPORT = 801;
static const int EXCEPTION_PARAMETER_CHECK = 401;
static const int EXCEPTION_NO_PERMISSION = 201;
static const int EXCEPTION_DB_EXIST = 15400001;
static const int EXCEPTION_INNER = 0;

class Error {
public:
    virtual ~Error(){};
    virtual std::string GetMessage() = 0;
    virtual int GetCode() = 0;
};

class ParametersType : public Error {
public:
    ParametersType(const std::string &name, const std::string &wantType) : name(name), wantType(wantType){};
    std::string GetMessage() override;
    int GetCode() override;

private:
    std::string name;
    std::string wantType;
};

class ParametersNum : public Error {
public:
    ParametersNum(const std::string &wantNum) : wantNum(wantNum){};
    std::string GetMessage() override;
    int GetCode() override;

private:
    std::string wantNum;
};

class PermissionError : public Error {
public:
    PermissionError() = default;
    std::string GetMessage() override;
    int GetCode() override;
};

class DatabaseError : public Error {
public:
    DatabaseError() = default;
    std::string GetMessage() override;
    int GetCode() override;
};

class InnerError : public Error {
public:
    InnerError() = default;
    std::string GetMessage() override;
    int GetCode() override;
};

class DeviceNotSupportedError : public Error {
public:
    DeviceNotSupportedError() = default;
    std::string GetMessage() override;
    int GetCode() override;
};
} // namespace ObjectStore
} // namespace OHOS

#endif // OBJECT_ERROR_H