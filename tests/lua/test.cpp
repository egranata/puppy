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
#include <string>
#include <libshell/system.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            const char* script = getTempFile("script");
            const char* dest = getTempFile("dest");

            FILE *f = fopen(script, "w");
            fprintf(f, "file = io.open(\"%s\", \"w\")\n", dest);
            fprintf(f, "file:write(\"test\\n\")\n");
            fprintf(f, "file:close()\n");
            fclose(f);

            std::string cmdline;
            cmdline.append_sprintf("lua %s", script);
            libShellSupport::system(cmdline.c_str());

            f = fopen(dest, "r");
            CHECK_EQ(fgetc(f), 't');
            CHECK_EQ(fgetc(f), 'e');
            CHECK_EQ(fgetc(f), 's');
            CHECK_EQ(fgetc(f), 't');
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
