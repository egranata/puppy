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
#include <newlib/sys/unistd.h>
#include <newlib/unistd.h>
#include <newlib/string.h>
#include <newlib/stdio.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            const char* msg = "Test message";
            CHECK_EQ(0, chdir("/tmp"));
            char* cwd = getcwd(nullptr, 0);
            CHECK_EQ(0, strcmp("/tmp", cwd));
            writeFile(msg);
            readFile("/tmp/file.txt", msg);
            readFile("file.txt", msg);
            readFile("./file.txt", msg);
            readFile("../tmp/file.txt", msg);
            readFile("./../tmp/file.txt", msg);
            readFile("../../system/config/../../../..////tmp/file.txt", msg);
            CHECK_EQ(0, chdir("../system"));
            cwd = getcwd(nullptr, 0);
            CHECK_EQ(0, strcmp("/system", cwd));
            readFile("../tmp/file.txt", msg);
            readFile("/tmp/file.txt", msg);
            CHECK_EQ(-1, chdir("/noSuchPath"));
        }

    private:
        void readFile(const char* path, const char* msg) {
            printf("Trying to read from %s\n", path);
            FILE *fd = fopen(path, "r");
            CHECK_NOT_NULL(fd);
            char buffer[512] = {0};
            CHECK_NOT_EQ(fread(&buffer[0], 1, 511, fd), 0);
            CHECK_EQ(0, strcmp(&buffer[0], msg));
            fclose(fd);
        }

        void writeFile(const char* msg) {
            FILE* fd = fopen("/tmp/file.txt", "w");
            CHECK_NOT_NULL(fd);
            CHECK_NOT_EQ(fwrite(msg, 1, strlen(msg), fd), 0);
            fclose(fd);
        }
};

int main(int, char**) {
    Test* test = new TheTest();
    test->test();
    return 0;
}
