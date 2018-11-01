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

#define TEST_FILE "/system/test.txt"

#define READ_BYTE(fid, expected) {\
    int ok = fread_syscall(fid, 1, (uint32_t)&data); \
    CHECK_EQ(0, (ok & 1)); \
    CHECK_EQ(data, expected); \
    printf("from handle %u, I read %c\n", fid, expected); \
}

#define FAIL_READ_BYTE(fid) {\
    int ok = fread_syscall(fid, 1, (uint32_t)&data); \
    CHECK_EQ(1, (ok & 1)); \
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    private:
        size_t writeString(FILE* fd, const char* s) {
            auto ls = strlen(s);
            return fwrite(s, 1, ls, fd);
        }

    protected:
        bool setup() override {
            auto fd = fopen(TEST_FILE, "w");
            if (fd == nullptr) return false;
            printf("file opened\n");
            auto c = writeString(fd, "hello world!");
            printf("written %d bytes\n", c);

            fclose(fd);

            return true;
        }

        void run() override {
            int fd = fopen_syscall(TEST_FILE, FILE_OPEN_READ);
            printf("fd = %u\n", fd);
            CHECK_EQ(0, (fd & 1));
            fd >>= 1;
            int fd2 = fdup_syscall(fd);
            printf("fd2 = %u\n", fd2);
            CHECK_EQ(0, (fd2 & 1));
            fd2 >>= 1;
            int fd3 = fdup_syscall(fd2);
            printf("fd3 = %u\n", fd3);
            CHECK_EQ(0, (fd3 & 1));
            fd3 >>= 1;

            uint8_t data = 0;
            READ_BYTE(fd,  'h');
            READ_BYTE(fd2, 'e');
            READ_BYTE(fd2, 'l');
            READ_BYTE(fd, 'l');

            fclose_syscall(fd);
            FAIL_READ_BYTE(fd);

            READ_BYTE(fd2, 'o');
            READ_BYTE(fd2, ' ');

            fclose_syscall(fd2);
            FAIL_READ_BYTE(fd2);

            READ_BYTE(fd3, 'w');
            READ_BYTE(fd3, 'o');

            fclose_syscall(fd3);
            FAIL_READ_BYTE(fd3);
        }

        void teardown() override {
            unlink(TEST_FILE);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
