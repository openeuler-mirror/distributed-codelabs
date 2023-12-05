/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef DATASECURITYLEVEL_TEST_H
#define DATASECURITYLEVEL_TEST_H

int TestOnstart(void);
int TestGetHighestSecLevel001(void);
int TestGetHighestSecLevel002(void);
int TestGetHighestSecLevel003(void);
int TestGetHighestSecLevel004(void);

int TestGetHighestSecLevelAsync001(void);
int TestGetHighestSecLevelAsync002(void);
int TestGetHighestSecLevelAsync003(void);
int TestGetHighestSecLevelAsync004(void);

int TestGetHighestSecLevelExcept001(void);
int TestGetHighestSecLevelExcept002(void);

#endif