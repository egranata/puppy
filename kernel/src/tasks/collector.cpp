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

#include <kernel/tasks/collector.h>
#include <kernel/process/manager.h>
#include <kernel/process/current.h>
#include <kernel/drivers/pit/pit.h>

#define LOG_LEVEL 2
#include <kernel/log/log.h>

LOG_TAG(COLLECTOR, 0);

namespace tasks::collector {    
    void task() {
        auto&& pmm(ProcessManager::get());
        while(true) {
            auto&& children = gCurrentProcess->children;
            if (!children.empty()) {
                TAG_DEBUG(COLLECTOR, "child list for pid %u is not empty", gCurrentProcess->pid);
                auto child = children.top();
                TAG_DEBUG(COLLECTOR, "child is %p %u", child, child->pid);
                pmm.collect(child->pid);
                TAG_DEBUG(COLLECTOR, "collection done");
            }
            pmm.yield();
        }
    }
}
