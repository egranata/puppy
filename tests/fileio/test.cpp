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
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define TEST_FILE "/system/test.txt"

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
        TheTest() : Test(TEST_NAME) {}

    private:
        size_t writeString(FILE* fd, const char* s) {
            auto ls = strlen(s);
            return fwrite(s, 1, ls, fd);
        }

        const char* testRead(const char* expected) {
            FILE* fd = fopen(TEST_FILE, "r");
            CHECK_NOT_EQ(fd, nullptr);
            uint8_t *buffer = (uint8_t *)malloc(3999);
            bzero(buffer, 3999);
            CHECK_NOT_EQ(fread(buffer, 1, 3999, fd), 0);

            CHECK_EQ(strcmp((const char*)buffer, expected), 0);

            return (const char*)buffer;
        }

        void testAppend(const char* stuff) {
            auto fd = fopen(TEST_FILE, "a");
            CHECK_NOT_EQ(fd, nullptr);
            CHECK_TRUE(writeString(fd, stuff));
            fclose(fd);
        }

        void testCleanSlate(const char* stuff) {
            FILE* fd = fopen(TEST_FILE, "w");
            CHECK_NOT_EQ(fd, nullptr);
            CHECK_TRUE(writeString(fd, stuff));
            fclose(fd);
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
            unlink(TEST_FILE);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
