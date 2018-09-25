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

#include <kernel/tasks/scheduler.h>
#include <kernel/process/manager.h>
#include <kernel/process/current.h>

#define LOG_LEVEL 2
#include <kernel/log/log.h>

namespace tasks::scheduler {    
    void task() {
        auto&& pmm(ProcessManager::get());
        auto&& ready = ProcessManager::gReadyQueue();

        while(true) {
            auto cur_task = gCurrentProcess;
            
            auto next_task = ready.front();
            LOG_DEBUG("front of ready queue is %p", next_task);
            ready.erase(ready.begin());
            LOG_DEBUG("erased front of ready queue");

            switch(next_task->state) {
                case process_t::State::AVAILABLE:
                    LOG_DEBUG("next_task = %p %u is available, scheduling", next_task, next_task->pid);
                    break;
                case process_t::State::EXITED:
                    pmm.enqueueForDeath(next_task);
                    /* fallthrough */
                default:
                    LOG_DEBUG("next_task = %p %u is not available, keep going", next_task, next_task->pid);
                    continue;
            }

            ready.push_back(next_task);

            LOG_DEBUG("cur_task = %p %u - next_task = %p %u", cur_task, cur_task->pid, next_task, next_task->pid);

            if (next_task == cur_task) {
                LOG_DEBUG("task %u yielding to itself, so nothing to see here", cur_task->pid);        
            } else {
                LOG_DEBUG("task %u yielding to task %u", cur_task->pid, next_task->pid);
                ProcessManager::ctxswitch(next_task);
                LOG_DEBUG("back from yielding");
            }
        }
    }
}
