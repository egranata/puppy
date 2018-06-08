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

#include <syscalls/handlers.h>
#include <process/manager.h>
#include <synch/message.h>
#include <synch/semaphore.h>
#include <drivers/pit/pit.h>
#include <process/process.h>

HANDLER3(msgsend, destpid, msg1, msg2) {
    auto&& pmm(ProcessManager::get());
    auto dest = pmm.getprocess(destpid);
    if (dest == nullptr) {
        return ERR(NO_SUCH_PROCESS);
    }

    auto msg = message_t{PIT::getUptime(), gCurrentProcess->pid, msg1, msg2};

    dest->msg.q.push_back(msg);
    if (dest->state == process_t::State::WAITMSG) {
        pmm.ready(dest);
    }

    return OK;
}

HANDLER2(msgrecv,msgptr,wait) {
    auto&& pmm(ProcessManager::get());

    while (gCurrentProcess->msg.q.empty()) {
        if (wait) {
            pmm.deschedule(gCurrentProcess, process_t::State::WAITMSG);
            pmm.yield();
        } else {
            return ERR(MSG_QUEUE_EMPTY);
        }
    }
    
    message_t *dest = (message_t*)msgptr;
    *dest = gCurrentProcess->msg.q.back();
    gCurrentProcess->msg.q.pop_back();
    return OK;
}
