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
#include <newlib/sys/ioctl.h>

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

static void MessageOrderTest_sender() {
    FILE* f = fopen("/queues/newmessage/MessageOrder", "w");
    if (f == nullptr) exit(1);
    setvbuf(f, nullptr, _IOFBF, 4096);
    uint32_t msg = 0xBeefF00d;
    fwrite(&msg, sizeof(msg), 1, f);
    fflush(f);
    msg = 0xA11DF00d;
    fwrite(&msg, sizeof(msg), 1, f);
    fflush(f);
    exit(0);
}
class MessageOrderTest : public Test {
    public:
        MessageOrderTest() : Test("newmessage.MessageOrder") {}
    
    protected:
        void run() override {
            FILE* f = fopen("/queues/newmessage/MessageOrder", "r");
            CHECK_NOT_EQ(f, nullptr);
            setvbuf(f, nullptr, _IOFBF, 4096);
            kpid_t spid = clone(MessageOrderTest_sender);
            CHECK_NOT_EQ(spid, 0);
            auto sexit = collect(spid);
            CHECK_EQ(sexit.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(sexit.status, 0);

            new_message_t msg1;
            CHECK_EQ(sizeof(msg1), fread(&msg1, 1, sizeof(msg1), f));
            CHECK_EQ(msg1.header.sender, spid);
            CHECK_EQ(msg1.header.payload_size, sizeof(pid_t));
            uint32_t *val = (uint32_t*)&msg1.payload[0];
            CHECK_NOT_EQ(val, nullptr);
            CHECK_EQ(*val, 0xBeefF00d);

            new_message_t msg2;
            CHECK_EQ(sizeof(msg2), fread(&msg2, 1, sizeof(msg2), f));
            CHECK_EQ(msg2.header.sender, spid);
            CHECK_EQ(msg2.header.payload_size, sizeof(pid_t));
            val = (uint32_t*)&msg2.payload[0];
            CHECK_NOT_EQ(val, nullptr);
            CHECK_EQ(*val, 0xA11DF00d);

            fclose(f);
        }
};

static void TwoSendersTest_sender1() {
    FILE* f = fopen("/queues/newmessage/TwoSenders", "w");
    if (f == nullptr) exit(1);
    setvbuf(f, nullptr, _IOFBF, 4096);
    uint32_t msg = 0xAA55BB44;
    sleep(3);
    fwrite(&msg, sizeof(msg), 1, f);
    fflush(f);
    sleep(1);
    exit(0);
}
static void TwoSendersTest_sender2() {
    FILE* f = fopen("/queues/newmessage/TwoSenders", "w");
    if (f == nullptr) exit(1);
    setvbuf(f, nullptr, _IOFBF, 4096);
    uint32_t msg = 0xCC00AA88;
    sleep(1);
    fwrite(&msg, sizeof(msg), 1, f);
    fflush(f);
    sleep(1);
    exit(0);
}
class TwoSendersTest : public Test {
    public:
        TwoSendersTest() : Test("newmessage.TwoSenders") {}
    
    protected:
        void run() override {
            FILE* f = fopen("/queues/newmessage/TwoSenders", "r");
            CHECK_NOT_EQ(f, nullptr);
            setvbuf(f, nullptr, _IOFBF, 4096);
            kpid_t spid1 = clone(TwoSendersTest_sender1);
            CHECK_NOT_EQ(spid1, 0);
            kpid_t spid2 = clone(TwoSendersTest_sender2);
            CHECK_NOT_EQ(spid2, 0);
            auto sexit1 = collect(spid1);
            CHECK_EQ(sexit1.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(sexit1.status, 0);
            auto sexit2 = collect(spid2);
            CHECK_EQ(sexit2.reason, process_exit_status_t::reason_t::cleanExit);
            CHECK_EQ(sexit2.status, 0);

            new_message_t msg1;
            CHECK_EQ(sizeof(msg1), fread(&msg1, 1, sizeof(msg1), f));
            new_message_t msg2;
            CHECK_EQ(sizeof(msg2), fread(&msg2, 1, sizeof(msg2), f));

            CHECK_EQ(sizeof(uint32_t), msg1.header.payload_size);
            CHECK_EQ(sizeof(uint32_t), msg2.header.payload_size);

            uint32_t *msg1_payload = (uint32_t*)&msg1.payload[0];
            uint32_t *msg2_payload = (uint32_t*)&msg2.payload[0];
            CHECK_NOT_EQ(msg1_payload, nullptr);
            CHECK_NOT_EQ(msg2_payload, nullptr);

            if (msg1.header.sender == spid1) {
                CHECK_EQ(*msg1_payload, 0xAA55BB44);
                CHECK_EQ(msg2.header.sender, spid2);
            } else if (msg1.header.sender == spid2) {
                CHECK_EQ(*msg1_payload, 0xCC00AA88);
                CHECK_EQ(msg2.header.sender, spid1);
            } else {
                CHECK_TRUE(false);
            }

            if (msg2.header.sender == spid1) {
                CHECK_EQ(*msg2_payload, 0xAA55BB44);
                CHECK_EQ(msg1.header.sender, spid2);
            } else if (msg2.header.sender == spid2) {
                CHECK_EQ(*msg2_payload, 0xCC00AA88);
                CHECK_EQ(msg1.header.sender, spid1);
            } else {
                CHECK_TRUE(false);
            }

            fclose(f);
        }
};

class NonBlockingReadTest : public Test {
    public:
        NonBlockingReadTest() : Test("newmessage.NonBlockingRead") {}
    
    protected:
        void run() override {
            FILE* f = fopen("/queues/newmessage/NonBlockingRead", "r");
            CHECK_NOT_EQ(f, nullptr);
            setvbuf(f, nullptr, _IOFBF, 4096);
            ioctl(fileno(f), IOCTL_BLOCK_ON_EMPTY, false);
            new_message_t msg;
            CHECK_EQ(0, fread(&msg, 1, sizeof(msg), f));
            fclose(f);
        }
};

int main() {
    auto& testPlan = TestPlan::defaultPlan(TEST_NAME);

    testPlan.add<ReceiveMessageTest>()
            .add<MessageOrderTest>()
            .add<TwoSendersTest>()
            .add<NonBlockingReadTest>();

    testPlan.test();
    return 0;
}
