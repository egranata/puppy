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

#include <tasks/awaker.h>
#include <process/manager.h>
#include <process/current.h>
#include <drivers/pit/pit.h>

#define LOG_NODEBUG
#include <log/log.h>

namespace tasks::awaker {    
    void task() {
        auto& sq(ProcessManager::gSleepQueue());
        auto& pmm(ProcessManager::get());
        while(true) {
            if (sq.empty()) {
                pmm.yield();
            } else {
                auto now = PIT::getUptime();
                auto top = sq.top();
                if (top->state == process_t::State::EXITED) {
                    pmm.enqueueForDeath(sq.pop());
                    continue;
                }
                if (top->sleeptill <= now) {
                    LOG_DEBUG("awakening process %u - it asked to sleep till %u", top->pid, top->sleeptill);
                    top = sq.pop();
                    pmm.ready(top);
                } else {
                    pmm.yield();
                }
            }
        }
    }
}
