// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <kernel/syscalls/handlers.h>
#include <kernel/process/manager.h>
#include <kernel/syscalls/types.h>
#include <kernel/synch/semaphore.h>
#include <kernel/drivers/pit/pit.h>
#include <kernel/process/process.h>

syscall_response_t msgsend_syscall_handler(uint32_t pid, uint32_t a1, uint32_t a2) {
    auto&& pmm(ProcessManager::get());
    auto dest = pmm.getprocess(pid);
    if (dest == nullptr) {
        return ERR(NO_SUCH_PROCESS);
    }

    auto msg = message_t{PIT::getUptime(), gCurrentProcess->pid, a1, a2};

    dest->msg.deliver(msg);

    return OK;
}

syscall_response_t msgrecv_syscall_handler(message_t *msg, bool wait) {
    if (wait) {
        *msg = gCurrentProcess->msg.receive();
        return OK;
    } else {
        bool any = gCurrentProcess->msg.receive(msg);
        return any ? OK : ERR(MSG_QUEUE_EMPTY);
    }
}
