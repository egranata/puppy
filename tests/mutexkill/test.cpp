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
#include <libuserspace/collect.h>
#include <libuserspace/mutex.h>
#include <libuserspace/clone.h>
#include <libuserspace/getpid.h>
#include <libuserspace/kill.h>
#include <libuserspace/sleep.h>

void lockMutex() {
    Mutex mutex("/testmutexkill/mutex");

    mutex.lock();
}

void child() {
    lockMutex();
}

class TheTest : public Test {
    public:
        TheTest() : Test(__FILE__) {}
    
    protected:
        void run() override {
            lockMutex();
            auto cpid = clone(child);

            sleep(2000); // HACK: race condition - hopefully the child has enough time to hang
            kill(cpid);

            auto status = collect(cpid);
            CHECK_EQ(status.reason, process_exit_status_t::reason_t::killed);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
