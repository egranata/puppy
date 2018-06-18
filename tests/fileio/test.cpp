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
#include <libuserspace/file.h>
#include <muzzle/string.h>
#include <libuserspace/memory.h>
#include <libuserspace/printf.h>
#include <libuserspace/exit.h>

#define TEST_FILE "/mainfs/test.txt"

#define VERY_LONG_STRING \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" \
"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"

class TheTest : public Test {
    public:
        TheTest() : Test(__FILE__) {}

    private:
        size_t writeString(int fd, const char* s) {
            auto ls = strlen(s);
            return ::write(fd, ls, (uint8_t*)s);
        }

        const char* testRead(const char* expected) {
            auto fd = open(TEST_FILE, FILE_NO_CREATE | FILE_OPEN_READ);
            CHECK_NOT_EQ(fd, gInvalidFd);
            uint8_t *buffer = (uint8_t *)malloc(3999);
            bzero(buffer, 3999);
            CHECK_NOT_EQ(read(fd, 3998, buffer), 0);

            CHECK_EQ(strcmp((const char*)buffer, expected), 0);

            return (const char*)buffer;
        }

        void testAppend(const char* stuff) {
            auto fd = open(TEST_FILE, FILE_OPEN_WRITE | FILE_OPEN_APPEND | FILE_NO_CREATE);
            CHECK_NOT_EQ(fd, gInvalidFd);
            CHECK_TRUE(writeString(fd, stuff));
            close(fd);
        }

        void testCleanSlate(const char* stuff) {
            auto fd = open(TEST_FILE, FILE_OPEN_WRITE | FILE_OPEN_NEW);
            CHECK_NOT_EQ(fd, gInvalidFd);
            CHECK_TRUE(writeString(fd, stuff));
            close(fd);
        }

    protected:
        bool setup() override {
            auto fd = open(TEST_FILE, FILE_OPEN_NEW | FILE_OPEN_WRITE);
            if (fd == gInvalidFd) return false;
            printf("file opened\n");
            auto c = writeString(fd, "hello world!");
            printf("written %d bytes\n", c);

            close(fd);

            return true;
        }

        void run() override {
            printf("first check: %s\n", testRead("hello world!"));
            testAppend("just some text..");
            printf("second pass: %s\n", testRead("hello world!just some text.."));
            testCleanSlate("entirely new content");
            printf("one more test: %s\n", testRead("entirely new content"));
            testCleanSlate(VERY_LONG_STRING);
            printf("a long string: %c\n", *testRead(VERY_LONG_STRING));
            testAppend(VERY_LONG_STRING);
            printf("and an even longer one: %c\n", *testRead(VERY_LONG_STRING VERY_LONG_STRING));
        }

        void teardown() override {
            del(TEST_FILE);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
