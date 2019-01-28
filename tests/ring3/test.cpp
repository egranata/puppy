/*
 * Copyright 2019 Google LLC
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

#include <sys/collect.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscalls.h>

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

static void runMemoryRead() {
    int* ptr = (int*)0xC0112233;
    printf("*ptr = %d\n", *ptr);
}

static void runMemoryWrite() {
    int* ptr = (int*)0xC03344AA;
    printf("*ptr = %d\n", (*ptr = 22));
}

static void runInstruction() {
    __asm__ volatile("hlt");
}

class TestMemoryRead : public Test {
    public:
        TestMemoryRead() : Test(TEST_NAME ".TestMemoryRead") {}

    protected:
        void run() override {
            auto child = clone(runMemoryRead);
            CHECK_NOT_EQ(child, 0);
            auto status = collect(child);
            CHECK_EQ(status.reason, process_exit_status_t::reason_t::exception);
        }
};

class TestMemoryWrite : public Test {
    public:
        TestMemoryWrite() : Test(TEST_NAME ".TestMemoryWrite") {}

    protected:
        void run() override {
            auto child = clone(runMemoryWrite);
            CHECK_NOT_EQ(child, 0);
            auto status = collect(child);
            CHECK_EQ(status.reason, process_exit_status_t::reason_t::exception);
        }
};

class TestPrivilegedInstruction : public Test {
    public:
        TestPrivilegedInstruction() : Test(TEST_NAME ".TestPrivilegedInstruction") {}

    protected:
        void run() override {
            auto child = clone(runInstruction);
            CHECK_NOT_EQ(child, 0);
            auto status = collect(child);
            CHECK_EQ(status.reason, process_exit_status_t::reason_t::exception);
        }
};

int main() {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<TestMemoryRead>()
            .add<TestMemoryWrite>()
            .add<TestPrivilegedInstruction>();

    testPlan.test();
    return 0;
}
