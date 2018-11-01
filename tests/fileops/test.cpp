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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <syscalls.h>
#include <sys/collect.h>

#define TEST_FILE "/system/test.txt"
#define TEST_MESSAGE "I am going to leave a message here\n"

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    private:
        const char* testRead(const char* expected) {
            FILE* fd = fopen(TEST_FILE, "r");
            CHECK_NOT_EQ(fd, nullptr);
            uint8_t *buffer = (uint8_t *)malloc(3999);
            bzero(buffer, 3999);
            CHECK_NOT_EQ(fread(buffer, 1, 3999, fd), 0);

            CHECK_EQ(strcmp((const char*)buffer, expected), 0);

            return (const char*)buffer;
        }

    protected:
        void run() override {
            size_t fd = fopen_syscall(TEST_FILE, FILE_OPEN_NEW | FILE_OPEN_WRITE);
            CHECK_EQ(0, (fd & 1));
            fd >>= 1;
            exec_fileop_t fileops[3] = {
                exec_fileop_t{
                    .op = exec_fileop_t::operation::CLOSE_CHILD_FD,
                    .param1 = STDOUT_FILENO,
                    .param2 = 0,
                    .param3 = nullptr,
                },
                exec_fileop_t{
                    .op = exec_fileop_t::operation::DUP_PARENT_FD,
                    .param1 = fd,
                    .param2 = 0,
                    .param3 = nullptr,
                },
                exec_fileop_t{
                    .op = exec_fileop_t::operation::END_OF_LIST,
                    .param1 = 0,
                    .param2 = 0,
                    .param3 = nullptr,
                },
            };

            char* argv[] = {
                (char*)"/system/tests/fileops",
                (char*)"--write-text",
                nullptr
            };

            int pid = exec_syscall("/system/tests/fileops", argv, nullptr, 0, &fileops[0]);
            CHECK_EQ(0, (pid & 1));
            pid >>= 1;

            fclose_syscall(fd);
            collect(pid);

            CHECK_EQ(fileops[1].param2, STDOUT_FILENO);

            testRead(TEST_MESSAGE);
        }

        void teardown() override {
            unlink(TEST_FILE);
        }
};

int main(int argc, char**) {
    if (argc < 2) {
        Test* test = new TheTest();
        test->test();
    } else {
        printf(TEST_MESSAGE);
    }

    return 0;
}
