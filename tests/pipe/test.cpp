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
#include <newlib/sys/collect.h>
#include <newlib/stdio.h>
#include <newlib/stdlib.h>
#include <newlib/unistd.h>
#include <newlib/syscalls.h>

#define TEST_MESSAGE "I am going to write some text to my stdout and then exit\n"

static void writeTask() {
    printf(TEST_MESSAGE);
    exit(0);
}

static uint16_t clone(void (*func)(), exec_fileop_t* fops) {
    auto ok = clone_syscall( (uintptr_t)func, fops );
    if (ok & 1) return 0;
    return ok >> 1;
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            int pipefd[2] = {0,0};
            CHECK_EQ(0, pipe(pipefd));

            exec_fileop_t fops[] = {
                exec_fileop_t{
                    .op = exec_fileop_t::operation::CLOSE_CHILD_FD,
                    .param1 = STDOUT_FILENO,
                    .param2 = 0,
                    .param3 = nullptr
                },
                exec_fileop_t{
                    .op = exec_fileop_t::operation::DUP_PARENT_FD,
                    .param1 = (size_t)pipefd[1],
                    .param2 = 0,
                    .param3 = nullptr,
                },
                exec_fileop_t{
                    .op = exec_fileop_t::operation::END_OF_LIST,
                    .param1 = 0,
                    .param2 = 0,
                    .param3 = nullptr
                },
            };

            FILE* fpipe = fdopen(pipefd[0], "r");

            auto wrPid = clone(writeTask, fops);
            fclose_syscall(pipefd[1]);

            char buf[1024] = {0};
            size_t count = fread(buf, 1, 1024, fpipe);
            CHECK_NOT_EQ(0, count);
            CHECK_EQ(0, strcmp(&buf[0], TEST_MESSAGE));
            count = fread(buf, 1, 1024, fpipe);
            CHECK_EQ(0, count);
            CHECK_TRUE(feof(fpipe));

            auto s = collect(wrPid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
