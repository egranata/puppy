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

#include <stdint.h>
#include <checkup/test.h>
#include <checkup/assert.h>
#include <newlib/sys/collect.h>
#include <newlib/syscalls.h>
#include <newlib/unistd.h>
#include <newlib/stdio.h>
#include <newlib/stdlib.h>

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

void child() {
    while(1);
}

static bool operator==(const exec_priority_t& p1, const exec_priority_t& p2) {
    return (p1.quantum == p2.quantum) && (p1.scheduling == p2.scheduling);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            auto c = clone(child);
            CHECK_NOT_EQ(c, 0);

            exec_priority_t max_prio{0,0};

            prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_MAXIMUM, nullptr, &max_prio);
            CHECK_NOT_EQ(max_prio.quantum, 0);
            CHECK_NOT_EQ(max_prio.scheduling, 0);

            exec_priority_t in = max_prio;
            CHECK_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_CURRENT, &in, nullptr));

            in.scheduling = in.scheduling - 1;
            CHECK_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_CURRENT, &in, nullptr));

            in.scheduling = in.scheduling + 2;
            CHECK_NOT_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_CURRENT, &in, nullptr));
            CHECK_NOT_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_MAXIMUM, &in, nullptr));

            in.scheduling = in.scheduling - 1;
            in.quantum = in.quantum - 1;
            CHECK_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_CURRENT, &in, nullptr));

            in.quantum = in.quantum + 2;
            CHECK_NOT_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_CURRENT, &in, nullptr));
            CHECK_NOT_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_MAXIMUM, &in, nullptr));

            in.quantum = in.scheduling = 1;
            CHECK_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_MAXIMUM, &in, &max_prio));
            CHECK_EQ(max_prio, in);
            CHECK_EQ(0,
                prioritize_syscall(c, prioritize_target::PRIORITIZE_SET_CURRENT, nullptr, &in));
            CHECK_EQ(max_prio, in);

            kill_syscall(c);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
