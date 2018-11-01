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
#include <stdlib.h>
#include <stdio.h>
#include <syscalls.h>
#include <sys/collect.h>
#include <sys/process.h>
#include <unistd.h>

static uint16_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

static bool do_check(const char* n, const char* value) {
    const char* true_value = getenv(n);
    printf("[%u] environ[%s] = %p %s (wanted is %s)\n", getpid(), n, true_value, true_value, value);
    if (value == nullptr) {
        return true_value == nullptr;
    } else {
        return 0 == strcmp(true_value, value);
    }
}

static void child_main() {
    if (do_check("PARENT", "CHILD")) {
        setenv("CHILD", "WHATEVER", 1);
        if (do_check("CHILD", "WHATEVER")) exit(0);
    }
    exit(1);
}

class EnvironTest : public Test {
    protected:
        EnvironTest(const char* name) : Test(name) {}
        void check(const char* n, const char* v) {
            CHECK_TRUE(do_check(n, v));
        }
};

class TestSameProcess : public EnvironTest {
    public:
        TestSameProcess() : EnvironTest("environ.TestSameProcess") {}
    protected:
        void run() override {
            int w = setenv("NAME", "VALUE", 1);

            CHECK_EQ(w, 0);
            check("NAME", "VALUE");

            w = setenv("NAME", "SOMETHINGELSE", 0);
            CHECK_EQ(w, 0);
            check("NAME", "VALUE");

            w = setenv("NAME", "SOMETHINGELSE", 1);
            CHECK_EQ(w, 0);
            check("NAME", "SOMETHINGELSE");

            int c = unsetenv("NAME");
            CHECK_EQ(c, 0);
            check("NAME", nullptr);
        }
};

class TestClone : public EnvironTest {
    public:
        TestClone() : EnvironTest("environ.TestClone") {}
    protected:
        void run() override {
            setenv("PARENT", "CHILD", 1);
            check("PARENT", "CHILD");

            auto cpid = clone(child_main);
            CHECK_NOT_EQ(cpid, 0);

            auto status = collect(cpid);
            CHECK_EQ(status.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(status.status, 0);

            CHECK_TRUE(do_check("CHILD", nullptr));
        }
};

class TestSpawn : public EnvironTest {
    public:
        TestSpawn() : EnvironTest("environ.TestSpawn") {}
    protected:
        void run() override {
            setenv("PARENT", "CHILD", 1);
            check("PARENT", "CHILD");

            char* argv[] = {
                (char*)"/system/tests/environ",
                (char*)"check",
                nullptr
            };

            auto cpid = spawn("/system/tests/environ", argv, SPAWN_FOREGROUND, nullptr);
            CHECK_NOT_EQ(cpid, 0);

            auto status = collect(cpid);
            CHECK_EQ(status.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(status.status, 0);

            CHECK_TRUE(do_check("CHILD", nullptr));
        }
};

int main(int argc, char**) {
    if (argc == 1) {
        TestPlan& plan(TestPlan::defaultPlan(TEST_NAME));
        plan.add<TestSameProcess>()
            .add<TestClone>()
            .add<TestSpawn>();
        plan.test();
        return 0;
    } else {
        if (do_check("PARENT", "CHILD")) {
            setenv("CHILD", "WHATEVER", 1);
            if (do_check("CHILD", "WHATEVER")) exit(0);
        }
        exit(1);
    }
}
