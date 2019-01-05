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
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#define FILE_PATH "/devices/time/uptime"

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    private:
        uint64_t readUptime() {
            FILE* f = fopen(FILE_PATH, "r");
            CHECK_NOT_NULL(f);
            char c[24] = {0};
            CHECK_NOT_EQ(0, fread(c, 1, sizeof(c), f));
            fclose(f);
            printf("uptime is %s\n", c);
            return strtoull(c, nullptr, 0);
        }

    protected:
        void run() override {
            auto uptime0 = readUptime();
            CHECK_NOT_EQ(0, uptime0); // system should have booted already
            sleep(2);
            auto uptime1 = readUptime();
            CHECK_TRUE(uptime1 > uptime0); // system should have slept and uptime gone up
            auto uptime2 = readUptime();
            CHECK_TRUE(uptime2 >= uptime1); // no going back in time
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
