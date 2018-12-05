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
#include <checkup/testplan.h>
#include <checkup/assert.h>

#include <libshell/system.h>

#include <stdio.h>

class IoRedirectionTest : public Test {
    public:
        IoRedirectionTest() : Test("shell.IoRedirectionTest") {}
    
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

class SuccessConcatTest : public Test {
    public:
        SuccessConcatTest() : Test("shell.SuccessConcatTest") {}
    
    protected:
        void run() override {
            const char* script = getTempFile("sct.script.sh");
            const char* dest = getTempFile("sct.script.txt");
            FILE* f = fopen(script, "w");
            CHECK_NOT_NULL(f);
            fprintf(f, "#!/system/apps/shell\n");
            fprintf(f, "touch %s && echo 1 > %s && cat %s && echo 234 > %s\n", dest, dest, dest, dest);
            fclose(f);
            int ok = libShellSupport::system(script);
            CHECK_EQ(ok, 0);
            f = fopen(dest, "r");
            CHECK_EQ('2', fgetc(f));
            CHECK_EQ('3', fgetc(f));
            CHECK_EQ('4', fgetc(f));
        }
};


int main() {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<IoRedirectionTest>()
            .add<SuccessConcatTest>();

    testPlan.test();
    return 0;
}
