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
#include <gtest/gtest.h>
#include <thread>
#include <string>

#include "rwlock.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {

class UtilsRWLockTest : public testing::Test {};

// This class is designed for test RWLock. "buf" is protected by "rwLock".
class TestRWLock {
public:
    TestRWLock():rwLock(), buf() {}

    explicit TestRWLock(bool writeFirst):rwLock(writeFirst), buf() {}

    void WriteStr(const string& str)
    {
        rwLock.LockWrite();
        for (auto it = str.begin(); it != str.end(); it++) {
            buf.push_back(*it);
            this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        rwLock.UnLockWrite();
        return;
    }

    void ReadStr(string& str)
    {
        rwLock.LockRead();
        for (auto it = buf.begin(); it != buf.end(); it++) {
            str.push_back(*it);
            this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        rwLock.UnLockRead();
        return;
    }
private:
    Utils::RWLock rwLock;
    string buf;
};

const string WRITE_IN_1("write1");
const string WRITE_IN_2("write2");

/*
 * @tc.name: testRWLock001
 * @tc.desc: RWLock here is under write-first mode. If there are some writing operation waiting,
 * reading will never happen. Reading operations will happen at the same time, when all writing operations
 * have finished.
 */
HWTEST_F(UtilsRWLockTest, testRWLock001, TestSize.Level1)
{
    TestRWLock test;

    thread first(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_1)));
    this_thread::sleep_for(chrono::milliseconds(1));

    string readOut1("");
    thread second(bind(&TestRWLock::ReadStr, ref(test), ref(readOut1)));
    this_thread::sleep_for(chrono::milliseconds(1));

    thread third(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_2)));
    this_thread::sleep_for(chrono::milliseconds(1));

    string readOut2("");
    thread fourth(bind(&TestRWLock::ReadStr, ref(test), ref(readOut2)));

    first.join();
    second.join();
    third.join();
    fourth.join();

    EXPECT_EQ(readOut1, WRITE_IN_1 + WRITE_IN_2);
    EXPECT_EQ(readOut2, WRITE_IN_1 + WRITE_IN_2);
}

/*
 * @tc.name: testRWLock002
 * @tc.desc: RWLock here is not under write-first mode. So if there are writing and reading operations in queue
 * with a writing mission running, they will compete when the writing mission completing. But what we can ensure
 * is that reading operations in queue will happen at the same time.
 */
HWTEST_F(UtilsRWLockTest, testRWLock002, TestSize.Level1)
{
    TestRWLock test(false);

    thread first(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_1)));
    this_thread::sleep_for(chrono::milliseconds(1));

    string readOut1("");
    thread second(bind(&TestRWLock::ReadStr, ref(test), ref(readOut1)));
    this_thread::sleep_for(chrono::milliseconds(1));

    thread third(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_2)));
    this_thread::sleep_for(chrono::milliseconds(1));

    string readOut2("");
    thread fourth(bind(&TestRWLock::ReadStr, ref(test), ref(readOut2)));

    first.join();
    second.join();
    third.join();
    fourth.join();

    EXPECT_EQ(readOut1, readOut2);
}
}  // namespace
}  // namespace OHOS