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

LOG_TAG(LOTTERY, 2);

namespace tasks::scheduler {
    namespace lottery {
        process_t *next() {
            auto& ready = ProcessManager::gReadyQueue();
            auto&& pmm(ProcessManager::get());

            Rng rng(readtsc());

            process_t *next_task = nullptr;
            uint64_t totalTickets;

loop_all:
            totalTickets = 0;
            for(size_t i = 0; i < ready.size(); ++i) {
                process_t *p = ready.at(i);
                switch(p->state) {
                    case process_t::State::EXITED:
                        pmm.enqueueForDeath(p);
                        ready.erase(p);
                        goto loop_all;
                        break;
                    default:
                        break;
                    case process_t::State::AVAILABLE:
                        totalTickets += p->priority.scheduling.current;
                        break;
                }
            }

            uint64_t currentWinner = (uint64_t)rng.next() | (((uint64_t)rng.next()) << 32);
            currentWinner %= totalTickets;

            TAG_DEBUG(LOTTERY, "lottery has total %llu tickets - winner is %llu", totalTickets, currentWinner);

            uint64_t countedTickets = 0;
            for(size_t i = 0; i < ready.size(); ++i) {
                process_t* p = ready.at(i);
                countedTickets += p->priority.scheduling.current;
                if ((countedTickets >= currentWinner) && (p->state == process_t::State::AVAILABLE)) {
                    next_task = p;
                    break;
                }
            }

            if (next_task == nullptr) {
                PANIC("lottery did not select a winner - scheduler aborted");
            } else return next_task;
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
