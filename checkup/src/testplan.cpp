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

#include <checkup/testplan.h>
#include <checkup/test.h>
#include <checkup/success.h>

TestPlan::TestPlan(const char* name) : mName(name ? name : "TestPlan") {}

const char* TestPlan::name() {
    return mName.c_str();
}

TestPlan& TestPlan::add(std::shared_ptr<Test> test) {
    mTests.push_back(test);
    return *this;
}

void TestPlan::test() {
    for (auto& t : mTests) {
        if (t) t->test();
    }
    __success(name());
}

TestPlan& TestPlan::defaultPlan(const char* name) {
    static TestPlan gPlan(name);

    return gPlan;
}
