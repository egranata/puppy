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

#ifndef CHECKUP_TESTPLAN
#define CHECKUP_TESTPLAN

#include <vector>
#include <shared_ptr>
#include <string>

class Test;

class TestPlan {
    private:
        using tests_t = std::vector<std::shared_ptr<Test>>;

        tests_t mTests;
        std::string mName;
    public:
        static TestPlan& defaultPlan(const char* name);

        TestPlan(const char* name);

        const char* name();

        TestPlan& add(std::shared_ptr<Test> test);

        template<typename T, class... Args>
        TestPlan& add(Args&&... args) {
            return add(std::make_shared<T>(args...));
        }

        void test();
};

#endif
