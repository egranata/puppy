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
#include <sys/collect.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#define DIR_PATH "/tmp/mysubsdir"

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            int ok = mkdir(DIR_PATH, 0777);
            CHECK_EQ(0, ok);
            struct stat st;
            ok = stat(DIR_PATH, &st);
            CHECK_EQ(0, ok);
            CHECK_TRUE(S_ISDIR(st.st_mode));
            ok = rmdir(DIR_PATH);
            CHECK_EQ(0, ok);
            ok = stat(DIR_PATH, &st);
            CHECK_NOT_EQ(0, ok);
            ok = rmdir(DIR_PATH);
            CHECK_NOT_EQ(0, ok);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
