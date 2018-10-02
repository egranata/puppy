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

static void writeTask() {
    printf("I am going to write some text to my stdout and then exit\n");
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
            size_t pipe_fd = pipe_syscall();
            CHECK_EQ(0, (pipe_fd & 1));
            pipe_fd >>= 1;
            exec_fileop_t fops[] = {
                exec_fileop_t{
                    .op = exec_fileop_t::operation::CLOSE_CHILD_FD,
                    .param1 = STDOUT_FILENO,
                    .param2 = 0,
                    .param3 = nullptr
                },
                exec_fileop_t{
                    .op = exec_fileop_t::operation::DUP_PARENT_FD,
                    .param1 = pipe_fd,
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

            FILE* fpipe = fdopen(pipe_fd, "r");

            auto wrPid = clone(writeTask, fops);

            // TODO: check the sleep/wake behavior
            CHECK_EQ(fgetc(fpipe), 'I');
            CHECK_EQ(fgetc(fpipe), ' ');
            CHECK_EQ(fgetc(fpipe), 'a');
            CHECK_EQ(fgetc(fpipe), 'm');
            CHECK_EQ(fgetc(fpipe), ' ');
            CHECK_EQ(fgetc(fpipe), 'g');
            CHECK_EQ(fgetc(fpipe), 'o');
            CHECK_EQ(fgetc(fpipe), 'i');
            CHECK_EQ(fgetc(fpipe), 'n');
            CHECK_EQ(fgetc(fpipe), 'g');
            CHECK_EQ(fgetc(fpipe), ' ');

            auto s = collect(wrPid);
            CHECK_EQ(s.reason, process_exit_status_t::reason_t::cleanExit);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
