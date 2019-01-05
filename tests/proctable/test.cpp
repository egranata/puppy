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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <syscalls.h>
#include <kernel/syscalls/types.h>
#include "common.h"
#include <vector>
#include <functional>

static bool validateProcessList(const std::vector<process_info_t>& plist,
                                std::function<bool(const process_info_t&)> filter,
                                std::function<bool(const process_info_t&)> validator) {
    for (const auto& proc : plist) {
        if (filter(proc)) {
            if (!validator(proc)) return false;
        }
    }

    return true;
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    private:
        std::vector<process_info_t> processes() {
            auto sz = proctable_syscall(nullptr, 0);
            sz >>= 1;
            process_info_t* ptable = new process_info_t[sz];
            proctable_syscall(ptable, sz);
            std::vector<process_info_t> pdata;
            for(auto i = 0u; i < sz; ++i) {
                pdata.push_back(ptable[i]);
            }
            delete[] ptable;
            return pdata;
        }

    protected:
        void run() override {
            FILE* fsema1 = fopen(SEMA1_NAME, "r");
            CHECK_NOT_EQ(fsema1, nullptr);
            int fdsema1 = fileno(fsema1);

            FILE* fsema2 = fopen(SEMA2_NAME, "r");
            CHECK_NOT_EQ(fsema2, nullptr);
            int fdsema2 = fileno(fsema2);

            const char* argv[] = {"/system/tests/proctable_helper", nullptr};

            auto cpid = execve(argv[0], (char* const*)argv, environ);
            CHECK_NOT_EQ(cpid, 0);
            printf("helper spawned: pid %u\n", cpid);

            ioctl(fdsema1, semaphore_ioctl_t::IOCTL_SEMAPHORE_WAIT, 0);
            printf("helper signaled\n");

            auto plist = processes();
            ioctl(fdsema2, semaphore_ioctl_t::IOCTL_SEMAPHORE_SIGNAL, 0);
            printf("test signaled\n");

            CHECK_TRUE(
                validateProcessList(plist,
                [cpid] (const process_info_t& proc) -> bool {
                    return proc.pid == cpid;
                },
                [] (const process_info_t& proc) -> bool {
                    if (proc.ppid != getpid()) return false;
                    return true;
                })
            );
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
