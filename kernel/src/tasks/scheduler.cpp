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
#include <kernel/libc/function.h>
#include <kernel/libc/rand.h>
#include <kernel/i386/primitives.h>
#include <kernel/panic/panic.h>

#define LOG_LEVEL 2
#include <kernel/log/log.h>

LOG_TAG(LOTTERY, 0);

namespace tasks::scheduler {
    namespace lottery {
        process_t *next() {
            auto& ready = ProcessManager::gReadyQueue();
            auto&& pmm(ProcessManager::get());

            Rng rng(readtsc());

            process_t *next_task = nullptr;

            uint64_t totalTickets = 0;
            ready.foreach([&totalTickets, &pmm] (const process_t* process) -> bool {
                totalTickets += process->priority.scheduling.current;
                return true;
            });

retry:
            uint64_t currentWinner = (uint64_t)rng.next() | (((uint64_t)rng.next()) << 32);
            currentWinner %= totalTickets;

            TAG_DEBUG(LOTTERY, "lottery has total %llu tickets - winner is %llu", totalTickets, currentWinner);
            uint64_t countedTickets = 0;
            ready.foreach([&countedTickets, currentWinner, &next_task] (process_t* process) -> bool {
                countedTickets += process->priority.scheduling.current;
                if (countedTickets >= currentWinner) {
                    TAG_DEBUG(LOTTERY, "process %u has %llu tickets - this brings count up to %llu which is >= %llu",
                        process->pid, process->priority.scheduling.current, countedTickets, currentWinner);
                    next_task = process;
                    return false;
                } else return true;
            });

            if (next_task == nullptr) {
                PANIC("lottery did not select a winner - scheduler aborted");
            }
            switch(next_task->state) {
                case process_t::State::AVAILABLE:
                    TAG_DEBUG(LOTTERY, "process %u wins this lottery", next_task->pid);
                    return next_task;
                case process_t::State::EXITED:
                    totalTickets -= next_task->priority.scheduling.current;
                    ready.erase(next_task);
                    pmm.enqueueForDeath(next_task);
                    /* fallthrough */
                default:
                    TAG_DEBUG(LOTTERY, "lottery did not select a runnable winner - try again");
                    goto retry;
            }
        }
    }
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
            auto next_task = lottery::next();
            next_task->flags.due_for_reschedule = false;
            ProcessManager::ctxswitch(next_task);
        }
    }
}
