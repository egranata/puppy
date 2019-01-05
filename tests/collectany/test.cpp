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

#include <libcheckup/test.h>
#include <libcheckup/assert.h>
#include <sys/collect.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscalls.h>

static void child1() {
    sleep(5);
    exit(0);
}

static void child2() {
    sleep(5);
    exit(0);
}

static void child3() {
    sleep(5);
    exit(0);
}

static void child4() {
    sleep(10);
    exit(0);
}

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

#define IS_SHORT_CHILD(c) ((c == c1) || (c == c2) || (c == c3))
#define IS_LONG_CHILD(c)  ((c == c4))

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            auto c1 = clone(child1);
            auto c2 = clone(child2);
            auto c3 = clone(child3);
            auto c4 = clone(child4);
            CHECK_NOT_EQ(c1, 0);
            CHECK_NOT_EQ(c2, 0);
            CHECK_NOT_EQ(c3, 0);
            CHECK_NOT_EQ(c4, 0);

            printf("spawned %u %u %u %u\n", c1, c2, c3, c4);

            kpid_t pid;
            process_exit_status_t exs(0);

            CHECK_FALSE(collectany(false, &pid, &exs));

            CHECK_TRUE(collectany(true, &pid, &exs));
            printf("collected %u\n", pid);
            CHECK_TRUE(IS_SHORT_CHILD(pid));

            CHECK_TRUE(collectany(true, &pid, &exs));
            printf("collected %u\n", pid);
            CHECK_TRUE(IS_SHORT_CHILD(pid));

            CHECK_TRUE(collectany(true, &pid, &exs));
            printf("collected %u\n", pid);
            CHECK_TRUE(IS_SHORT_CHILD(pid));

            CHECK_FALSE(collectany(false, &pid, &exs));

            CHECK_TRUE(collectany(true, &pid, &exs));
            printf("collected %u\n", pid);
            CHECK_TRUE(IS_LONG_CHILD(pid));
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
