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
#include <unistd.h>
#include <time.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            time_t at = 0;

            setenv("TZ", "America/New_York", 1);
            struct tm* lt = localtime(&at);
            printf("localtime in New York @ the epoch is %d:%d:%d %d/%d/%d\n",
                lt->tm_hour, lt->tm_min, lt->tm_sec,
                lt->tm_mday, lt->tm_mon, lt->tm_year);
            CHECK_EQ(69, lt->tm_year);
            CHECK_EQ(11, lt->tm_mon);
            CHECK_EQ(31, lt->tm_mday);
            CHECK_EQ(19, lt->tm_hour);

            setenv("TZ", "Europe/Rome", 1);
            lt = localtime(&at);
            printf("localtime in Rome @ the epoch is %d:%d:%d %d/%d/%d\n",
                lt->tm_hour, lt->tm_min, lt->tm_sec,
                lt->tm_mday, lt->tm_mon, lt->tm_year);
            CHECK_EQ(70, lt->tm_year);
            CHECK_EQ(0, lt->tm_mon);
            CHECK_EQ(1, lt->tm_mday);
            CHECK_EQ(1, lt->tm_hour);

            CHECK_NOT_EQ(nullptr, ctime(&at));
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
