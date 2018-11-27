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

#include <libshell/system.h>

#include <stdio.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            FILE *f = fopen("/tmp/redir.in", "w");
            CHECK_NOT_EQ(f, nullptr);
            fprintf(f, "echo \"Test file\" > /tmp/redir.out\n");
            fclose(f);

            f = fopen("/tmp/redir.sh", "w");
            CHECK_NOT_EQ(f, nullptr);
            fprintf(f, "shell < /tmp/redir.in\n");
            fclose(f);

            libShellSupport::system("shell /tmp/redir.sh");

            f = fopen("/tmp/redir.out", "r");
            CHECK_NOT_EQ(f, nullptr);
            CHECK_EQ(fgetc(f), 'T');
            CHECK_EQ(fgetc(f), 'e');
            CHECK_EQ(fgetc(f), 's');
            CHECK_EQ(fgetc(f), 't');
            CHECK_EQ(fgetc(f), ' ');
            fclose(f);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}