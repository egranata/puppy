/*
 * Copyright 2019 Google LLC
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

#include <set>
#include <string>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}

    private:
        std::set<int> mDescriptors;

        bool add(FILE* f) {
            if (f == nullptr) return false;

            int fd = fileno(f);
            if (mDescriptors.count(fd)) return false;

            mDescriptors.emplace(fd);
            return true;
        }

    protected:
        void run() override {
            std::string file_path = "/tmp/fileno.test.A";

            CHECK_TRUE(add(stdin));
            CHECK_TRUE(add(stdout));
            CHECK_TRUE(add(stderr));

            for (unsigned char i = 'A'; i < 'Z'; ++i) {
                file_path.back() = i;
                CHECK_TRUE(add(fopen(file_path.c_str(), "w")));
            }

            for (unsigned char i = 'a'; i < 'z'; ++i) {
                file_path.back() = i;
                CHECK_TRUE(add(fopen(file_path.c_str(), "w")));
            }

            for (unsigned char i = '0'; i < '9'; ++i) {
                file_path.back() = i;
                CHECK_TRUE(add(fopen(file_path.c_str(), "w")));
            }
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
