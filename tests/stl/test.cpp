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
#include <libcheckup/testplan.h>

#include <stdio.h>

#include <map>
#include <string>
#include <vector>
#include <list>
#include <functional>

class TestVector : public Test {
    public:
        TestVector() : Test(TEST_NAME ".vector") {}

    protected:
        void run() override {
            std::vector<int> vint;
            CHECK_EQ(vint.size(), 0);
            CHECK_TRUE(vint.empty());

            vint.push_back(123);
            vint.push_back(456);

            CHECK_EQ(vint.size(), 2);
            CHECK_FALSE(vint.empty());

            CHECK_EQ(123, vint[0]);
            CHECK_EQ(456, vint[1]);
        }
};

class TestMap : public Test {
    public:
        TestMap() : Test(TEST_NAME ".map") {}

    protected:
        void run() override {
            std::map<int, int> mint;
            CHECK_TRUE(mint.empty());

            mint.emplace(123, 456);
            
            CHECK_EQ(456, mint[123]);
            CHECK_EQ(mint.end(), mint.find(234));
        }
};

class TestList : public Test {
    public:
        TestList() : Test(TEST_NAME ".list") {}

    protected:
        void run() override {
            std::list<int> l;
            CHECK_TRUE(l.empty());

            l.push_back(123);
            l.push_back(456);
            l.push_back(789);

            CHECK_FALSE(l.empty());
            CHECK_EQ(3, l.size());

            CHECK_EQ(123, l.front());
            CHECK_EQ(789, l.back());

            l.pop_front();
            l.pop_back();

            CHECK_EQ(l.front(), l.back());
            CHECK_EQ(456, l.back());
        }
};

class TestString : public Test {
    public:
        TestString() : Test(TEST_NAME ".string") {}

    protected:
        void run() override {
            std::string s = "Hello world";
            CHECK_TRUE(s == "Hello world");
            CHECK_EQ(s[0], 'H');
            s += ". This is a test";
            CHECK_TRUE(s == "Hello world. This is a test");
        }
};

class TestFunction : public Test {
    public:
        TestFunction() : Test(TEST_NAME ".function") {}

    protected:
        void run() override {
            std::function<int(int)> f;
            f = [](int x) -> int { return x + 1; };
            CHECK_EQ(4, f(3));

            int n = 3;
            f = [n](int x) -> int { return n + x; };
            CHECK_EQ(6, f(3));
            n = 4;
            CHECK_EQ(6, f(3));

            f = [&n](int x) -> int { return n + x; };
            CHECK_EQ(7, f(3));
            n = 3;
            CHECK_EQ(6, f(3));
        }
};

int main(int, char**) {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<TestVector>()
            .add<TestMap>()
            .add<TestString>()
            .add<TestList>()
            .add<TestFunction>();

    testPlan.test();
    return 0;
}
