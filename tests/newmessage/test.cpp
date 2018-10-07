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
#include <newlib/stdlib.h>
#include <newlib/stdio.h>
#include <newlib/syscalls.h>
#include <newlib/sys/collect.h>
#include <kernel/syscalls/types.h>
#include <newlib/unistd.h>

static kpid_t clone(void (*func)()) {
    auto ok = clone_syscall( (uintptr_t)func, nullptr );
    if (ok & 1) return 0;
    return ok >> 1;
}

static void ReceiveMessageTest_sender() {
    FILE* f = fopen("/queues/newmessage/ReceiveMessage", "w");
    if (f == nullptr) exit(1);
    setvbuf(f, nullptr, _IOFBF, 4096);
    auto my_pid = getpid();
    fwrite(&my_pid, sizeof(my_pid), 1, f);
    fflush(f);
    fclose(f);
    exit(0);
}
class ReceiveMessageTest : public Test {
    public:
        ReceiveMessageTest() : Test("newmessage.ReceiveMessage") {}
    
    protected:
        void run() override {
            FILE* f = fopen("/queues/newmessage/ReceiveMessage", "r");
            CHECK_NOT_EQ(f, nullptr);
            setvbuf(f, nullptr, _IOFBF, 4096);
            kpid_t spid = clone(ReceiveMessageTest_sender);
            CHECK_NOT_EQ(spid, 0);
            auto sexit = collect(spid);
            CHECK_EQ(sexit.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(sexit.status, 0);
            new_message_t msg;
            CHECK_EQ(sizeof(msg), fread(&msg, 1, sizeof(msg), f));
            CHECK_EQ(msg.header.sender, spid);
            CHECK_EQ(msg.header.payload_size, sizeof(pid_t));
            pid_t *mpid = (pid_t*)&msg.payload[0];
            CHECK_NOT_EQ(mpid, nullptr);
            CHECK_EQ(*mpid, spid);
            fclose(f);
        }
};

int main() {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<ReceiveMessageTest>();

    testPlan.test();
    return 0;
}
