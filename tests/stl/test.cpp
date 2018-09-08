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
#include <EASTL/vector.h>
#include <newlib/stdio.h>
#include <EASTL/map.h>
#include <EASTL/string.h>

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    private:
        void testvector() {
            eastl::vector<int> vint;
            CHECK_EQ(vint.size(), 0);
            CHECK_TRUE(vint.empty());

            vint.push_back(123);
            vint.push_back(456);

            CHECK_EQ(vint.size(), 2);
            CHECK_FALSE(vint.empty());

            CHECK_EQ(123, vint[0]);
            CHECK_EQ(456, vint[1]);
        }

        void testmap() {
            eastl::map<int, int> mint;
            CHECK_TRUE(mint.empty());

            mint.emplace(123, 456);
            
            CHECK_EQ(456, mint[123]);
            CHECK_EQ(mint.end(), mint.find(234));
        }

        void teststring() {
            eastl::string s = "Hello world";
            CHECK_TRUE(s == "Hello world");
            CHECK_EQ(s[0], 'H');
            s += ". This is a test";
            CHECK_TRUE(s == "Hello world. This is a test");
        }

    protected:
        void run() override {
            testvector();
            testmap();
            teststring();
        }
};

int main(int, char**) {
    Test* test = new TheTest();
    test->test();
    return 0;
}
