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

#include <test.h>
#include <assert.h>
#include <memory.h>
#include <exit.h>
#include <clone.h>
#include <collect.h>

static constexpr int gExitCode = 96;

struct foo {
    int value;
} gFoo;

uint32_t doStuff(int a, int b) {
    uint32_t *ptr = (uint32_t*)malloc(4);
    *ptr = a + b;
    return *ptr - gFoo.value;
}

static void newmain() {
    gFoo.value = 1;
    exit(gExitCode + doStuff(3,-2));
}

class TheTest : public Test {
    public:
        TheTest() : Test(__FILE__) {}
    
    protected:
        void run() override {
            gFoo.value = 0; 

            uint16_t pid = clone(&newmain);

            CHECK_NOT_EQ(pid, 0);

            auto es = collect(pid);

            CHECK_EQ(es.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(es.status, gExitCode);
            CHECK_EQ(gFoo.value, 0);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
