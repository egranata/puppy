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

#include <kernel/tasks/deleter.h>
#include <kernel/process/manager.h>
#include <kernel/process/current.h>
#include <kernel/drivers/pit/pit.h>
#include <kernel/mm/phys.h>
#include <kernel/mm/virt.h>
#include <kernel/sys/globals.h>

#include <kernel/log/log.h>

namespace tasks::deleter {
    void task() {
        auto&& pmm(PhysicalPageManager::get());
        auto&& vmm(VirtualPageManager::get());
        auto& pm(ProcessManager::get());
        auto& col(ProcessManager::gCollectedProcessList());
        auto& pidBitmap(ProcessManager::gPidBitmap());
        auto& gdtBitmap(ProcessManager::gGDTBitmap());
        auto& procTable(ProcessManager::gProcessTable());
        while(true) {
            if (!col.empty()) {
                auto proc = col.pop();
                LOG_DEBUG("deleting process object for %u", proc->pid);
                if (proc->path) free((void*)proc->path);
                if (proc->args) free((void*)proc->args);

                auto dtbl = addr_gdt<uint64_t*>();
                dtbl[proc->pid + val_numsysgdtentries<uint32_t>()] = 0;

                pidBitmap.free(proc->pid);
                gdtBitmap.free(proc->gdtidx);
                procTable.free(proc);

                pmm.dealloc((uintptr_t)proc->tss.cr3);
                if (proc->esp0start) vmm.unmap(proc->esp0start);
                if (proc->espstart) vmm.unmap(proc->espstart);
                proc->~process_t();
                vmm.unmap((uintptr_t)proc);
            }
            
            pm.yield();            
        }        
    }
}
