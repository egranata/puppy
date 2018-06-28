/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <checkup/test.h>
#include <checkup/assert.h>
#include <libuserspace/mutex.h>
#include <libuserspace/exit.h>
#include <libuserspace/clone.h>
#include <libuserspace/sleep.h>
#include <libuserspace/collect.h>

#define MUTEX_NAME "/tests/mutex/mtx"

void child1() {
    sleep(800);
    Mutex mtx(MUTEX_NAME);
    mtx.lock();
    sleep(800);
    mtx.unlock();
    exit(1);
}

void child2() {
    sleep(400);
    Mutex mtx(MUTEX_NAME);
    mtx.lock();
    sleep(500);
    mtx.unlock();
    exit(2);
}

void child3() {
    sleep(350);
    Mutex mtx(MUTEX_NAME);
    mtx.lock();
    sleep(800);
    mtx.unlock();
    exit(3);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            Mutex mtx(MUTEX_NAME);

            CHECK_TRUE(mtx.trylock());

            auto c1 = clone(child1);
            auto c2 = clone(child2);
            auto c3 = clone(child3);

            sleep(500);
            mtx.unlock();

            auto s1 = collect(c1);
            auto s2 = collect(c2);
            auto s3 = collect(c3);

            CHECK_EQ(s1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s2.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s3.reason, process_exit_status_t::reason_t::cleanExit);

            CHECK_TRUE(mtx.trylock());
            mtx.unlock();
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
