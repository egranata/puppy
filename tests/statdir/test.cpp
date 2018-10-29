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
#include <newlib/stdlib.h>
#include <newlib/stdio.h>
#include <newlib/sys/stat.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            struct stat statbuf;
            bzero(&statbuf, sizeof(statbuf));
            int ok = stat("/system/apps", &statbuf);
            CHECK_EQ(0, ok);
            CHECK_TRUE(S_ISDIR(statbuf.st_mode));
            CHECK_FALSE(S_ISREG(statbuf.st_mode));

            bzero(&statbuf, sizeof(statbuf));
            ok = stat("/system/apps/ls", &statbuf);
            CHECK_EQ(0, ok);
            CHECK_FALSE(S_ISDIR(statbuf.st_mode));
            CHECK_TRUE(S_ISREG(statbuf.st_mode));

            bzero(&statbuf, sizeof(statbuf));
            ok = stat("/system/not/a/real/path", &statbuf);
            CHECK_NOT_EQ(0, ok);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
