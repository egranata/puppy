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

#include <kernel/tasks/awaker.h>
#include <kernel/process/manager.h>
#include <kernel/process/current.h>
#include <kernel/time/manager.h>

#define LOG_LEVEL 2
#include <kernel/log/log.h>

KERNEL_TASK_NAMESPACE_OPEN(awaker) {
    WaitQueue& queue() {
        static WaitQueue gQueue;

        return gQueue;
    }

    void task() {
        auto& sq(ProcessManager::gSleepQueue());
        auto& pmm(ProcessManager::get());
        gCurrentProcess->priority.scheduling.current = 5;
        while(true) {
            /* if there's a process waiting to be woken up, yield and continue
               until that has happened - and only then go back to waiting;
               if one sets a process to sleep for - say - an hour then this
               is highly suboptimal, but if no processes are sleeping or all sleeps
               are short and sporadic, then this is almost good..
            */
            while(!sq.empty()) {
                auto now = TimeManager::get().millisUptime();
                auto top = sq.top();
                if (top.process->state == process_t::State::EXITED) {
                    pmm.enqueueForDeath(sq.pop().process);
                    continue;
                }
                if (top.process->sleeptill <= now) {
                    if (top.process->waitToken == top.token) {
                        LOG_DEBUG("awakening process %u - it asked to sleep till %u", top.process->pid, top.process->sleeptill);
                        top = sq.pop();
                        pmm.wake(top.process);
                    } else {
                        LOG_ERROR("process %u in sleep queue; queue token is %llu, but process has token %llu; ignoring wake",
                            top.process->pid, top.token, top.process->waitToken);
                    }
                } else {
                    pmm.yield();
                }
            }
            queue().yield(gCurrentProcess, 0);
        }
    }
}
