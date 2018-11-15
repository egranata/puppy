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

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            FILE *f = fopen("/tmp/wb", "wb+");
            CHECK_EQ(0, ferror(f));
            int ok = fwrite("hello", 1, 5, f);
            printf("ok = %d\n", ok);
            CHECK_NOT_EQ(0, ok);
            CHECK_EQ(0, ferror(f));
            fclose(f);
            f = fopen("/tmp/wb", "r");
            char data[6] = {0};
            ok = fread(data, 1, 5, f);
            CHECK_EQ(0, strcmp("hello", data));
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
