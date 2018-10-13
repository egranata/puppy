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
    namespace rr {
        process_t *next() {
            auto&& ready = ProcessManager::gReadyQueue();
            auto&& pmm(ProcessManager::get());

            process_t *next_task = nullptr;

            // in theory this could cause an endless loop, but in practice at least the "dummy process" will always be ready to run
            while(true) {
                next_task = ready.front();
                ready.erase(ready.begin());

                switch(next_task->state) {
                    case process_t::State::AVAILABLE:
                        ready.push_back(next_task);
                        return next_task;
                    case process_t::State::EXITED:
                        pmm.enqueueForDeath(next_task);
                        /* fallthrough */
                    default:
                        continue;
                }

            }
        }
    }

    void task() {
        while(true) {
            auto next_task = rr::next();
            next_task->flags.due_for_reschedule = false;
            ProcessManager::ctxswitch(next_task);
        }
    }
}
