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
#include <libuserspace/memory.h>
#include <libuserspace/exit.h>
#include <libuserspace/clone.h>
#include <libuserspace/collect.h>
#include <libuserspace/sleep.h>

static struct {
    volatile uint32_t value;
} gFoo;

static void child1() {
    sleep(1000);
    exit(1);
}

static void child2() {
    sleep(1000);
    exit(2);
}

static void child3() {
    gFoo.value = 234;
    sleep(1000);
    exit(3);
}

static void child4() {
    sleep(1000);
    __asm__ volatile ("hlt");
    exit(4);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            gFoo.value = 123;

            auto c1 = clone(child1);
            auto c2 = clone(child2);
            auto c3 = clone(child3);
            auto c4 = clone(child4);

            auto s1 = collect(c1);
            auto s2 = collect(c2);
            auto s3 = collect(c3);
            auto s4 = collect(c4);

            CHECK_EQ(s1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s2.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s3.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s4.reason, process_exit_status_t::reason_t::exception);

            CHECK_EQ(s1.status, 1);
            CHECK_EQ(s2.status, 2);
            CHECK_EQ(s3.status, 3);

            CHECK_EQ(gFoo.value, 123);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
