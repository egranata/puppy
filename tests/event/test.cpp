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
#include <checkup/testplan.h>
#include <checkup/assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <syscalls.h>
#include <sys/collect.h>
#include <sys/ioctl.h>
#include <sys/process.h>

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

#define EVENT_PATH "/events/testevent"

void child1() {
    FILE *fevt = fopen(EVENT_PATH, "r");
    int fdevt = fileno(fevt);
    int ok = wait1_syscall(fdevt, 0);
    exit(ok);
}

void child2() {
    FILE *fevt = fopen(EVENT_PATH, "r");
    int fdevt = fileno(fevt);
    int ok = wait1_syscall(fdevt, 0);
    exit(ok);
}

void child3() {
    FILE *fevt = fopen(EVENT_PATH, "r");
    int fdevt = fileno(fevt);
    int ok = wait1_syscall(fdevt, 3000);
    exit(ok);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            FILE *fevt = fopen(EVENT_PATH, "r");
            CHECK_NOT_EQ(fevt, nullptr);
            int fdevt = fileno(fevt);

            auto c1 = clone(child1);
            CHECK_NOT_EQ(c1, 0);
            sleep(2);
            ioctl(fdevt, event_ioctl_t::IOCTL_EVENT_RAISE, 0);
            auto s1 = collect(c1);
            CHECK_EQ(s1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s1.status, 0);

            auto c2 = clone(child2);
            CHECK_NOT_EQ(c2, 0);
            auto s2 = collect(c2);
            CHECK_EQ(s2.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s2.status, 0);

            ioctl(fdevt, event_ioctl_t::IOCTL_EVENT_LOWER, 0);
            auto c3 = clone(child3);
            CHECK_NOT_EQ(c3, 0);
            auto s3 = collect(c3);
            CHECK_EQ(s3.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_NOT_EQ(s3.status, 0);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
