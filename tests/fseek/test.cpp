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
#include <string.h>
#include <stdio.h>

#define SOURCE_TEXT "This is a long string to write into a file in order to test seek behavior on Puppy filesystem implementation."

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            FILE* f = fopen("/tmp/seek.txt", "w");
            fwrite(SOURCE_TEXT, 1, strlen(SOURCE_TEXT), f);
            fclose(f);

            f = fopen("/tmp/seek.txt", "r");

            CHECK_EQ(ftell(f), 0);
            CHECK_EQ(fgetc(f), 'T');
            CHECK_EQ(ftell(f), 1);

            CHECK_EQ(0, fseek(f, 10, SEEK_SET));
            CHECK_EQ(ftell(f), 10);
            CHECK_EQ(fgetc(f), 'l');
            CHECK_EQ(fgetc(f), 'o');
            CHECK_EQ(ftell(f), 12);

            CHECK_EQ(0, fseek(f, -1, SEEK_CUR));
            CHECK_EQ(ftell(f), 11);
            CHECK_EQ(fgetc(f), 'o');

            CHECK_EQ(0, fseek(f, -2, SEEK_END));
            CHECK_EQ(fgetc(f), 'n');
            CHECK_EQ(fgetc(f), '.');
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
