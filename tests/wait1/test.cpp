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
#include <libcheckup/testplan.h>
#include <libcheckup/assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <syscalls.h>
#include <sys/collect.h>
#include <sys/ioctl.h>
#include <sys/process.h>

#define NOTIMEOUT_SEMPATH "/semaphores/wait1/notimeout"

#define TIMEOUT_SEMPATH "/semaphores/wait1/notimeout"

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

static void test_noTimeout_child1() {
    sleep(2);
    FILE *fsema = fopen(NOTIMEOUT_SEMPATH, "r");
    int fdsema = fileno(fsema);
    int ok = wait1_syscall(fdsema, 0);
    exit(ok);
}

static void test_timeout_child1() {
    sleep(2);
    FILE *fsema = fopen(TIMEOUT_SEMPATH, "r");
    int fdsema = fileno(fsema);
    int ok = wait1_syscall(fdsema, 2500);
    exit(ok);
}

static void test_timeout_child2() {
    sleep(2);
    FILE *fsema = fopen(TIMEOUT_SEMPATH, "r");
    int fdsema = fileno(fsema);
    int ok = wait1_syscall(fdsema, 2500);
    exit(ok);
}

class TestNoTimeout : public Test {
    public:
        TestNoTimeout() : Test("wait1.TestNoTimeout") {}
    protected:
        void run() override {
            FILE *fsema = fopen(NOTIMEOUT_SEMPATH, "r");
            CHECK_NOT_EQ(fsema, nullptr);
            int fdsema = fileno(fsema);

            auto c1 = clone(test_noTimeout_child1);
            CHECK_NOT_EQ(c1, 0);

            sleep(3);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL, 0);

            auto s1 = collect(c1);
            CHECK_EQ(s1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s1.status, 0);
        }
};

class TestTimeout : public Test {
    public:
        TestTimeout() : Test("wait1.TestTimeout") {}
    protected:
        void run() override {
            FILE *fsema = fopen(TIMEOUT_SEMPATH, "r");
            CHECK_NOT_EQ(fsema, nullptr);
            int fdsema = fileno(fsema);

            auto c1 = clone(test_timeout_child1);
            CHECK_NOT_EQ(c1, 0);
            auto s1 = collect(c1);
            CHECK_EQ(s1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_NOT_EQ(s1.status, 0); // semaphore was not signaled - process should be exiting due to timeout

            auto c2 = clone(test_timeout_child2);
            CHECK_NOT_EQ(c2, 0);
            ioctl(fdsema, semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL, 0);
            auto s2 = collect(c2);
            CHECK_EQ(s2.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(s2.status, 0); // semaphore was signaled
        }
};


int main(int, char**) {
    TestPlan& plan(TestPlan::defaultPlan(TEST_NAME));
    plan.add<TestNoTimeout>()
        .add<TestTimeout>();
    plan.test();
    return 0;
}
