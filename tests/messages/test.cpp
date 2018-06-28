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
#include <libuserspace/message.h>
#include <libuserspace/exit.h>
#include <libuserspace/getpid.h>
#include <libuserspace/clone.h>
#include <libuserspace/collect.h>

void sender() {
    auto parent = getppid();
    message::send(parent, 0xf00d, 0x1234);

    exit(0);
}

void forwarder() {
    auto msg = message::receive();
    message::send(msg.sender, msg.arg1, msg.arg2);

    exit(0);
}

void nowait() {
    message msg;
    bool anything = message::receive(&msg);
    exit(anything ? 0 : 1);
}

class TheTest : public Test {
    public:
        TheTest() : Test(TEST_NAME) {}
    
    protected:
        void run() override {
            auto csender = clone(sender);
            auto cforwarder = clone(forwarder);
            auto cnowait = clone(nowait);

            auto msg1 = message::receive();
            CHECK_EQ(msg1.sender, csender);
            CHECK_EQ(msg1.arg1, 0xf00d);
            CHECK_EQ(msg1.arg2, 0x1234);

            message::send(cforwarder, 0xabcd, 0xaa55);

            auto msg2 = message::receive();
            CHECK_EQ(msg2.sender, cforwarder);
            CHECK_EQ(msg2.arg1, 0xabcd);
            CHECK_EQ(msg2.arg2, 0xaa55);

            auto snowait = collect(cnowait);
            CHECK_EQ(snowait.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(snowait.status, 1);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
