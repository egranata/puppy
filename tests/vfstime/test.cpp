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
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#define TEST_CONTENT "just a string"

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
            auto fd = fopen(getTempFile("data"), "w");
            if (fd == nullptr) return false;
            printf("file opened\n");
            auto c = writeString(fd, TEST_CONTENT);
            printf("written %d bytes\n", c);
            fclose(fd);
            return true;
        }

        void run() override {
            sleep(2); // remove the inherent race if the test proceeds quickly enough
            struct stat s;
            CHECK_EQ(0, stat(getTempFile("data"), &s));
            auto now = time(nullptr);
            auto then = s.st_atime;
            CHECK_TRUE(now > then); // this file should have been created in the past...
            auto delta = now - then;
            CHECK_TRUE(delta < 60); // the whole thing should take less than a minute...
            printf("test took %llu seconds\n", delta);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
