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
#include <kernel/i386/reboot.h>
#include <kernel/drivers/rtc/rtc.h>
#include <kernel/drivers/pit/pit.h>
#include <kernel/mm/phys.h>
#include <kernel/process/current.h>
#include <kernel/syscalls/types.h>

HANDLER0(reboot) {
    reboot();
    return OK; // we should never return from here
}

syscall_response_t now_syscall_handler(uint64_t *dest) {
    *dest = RTC::get().timestamp();

    return OK;
}

syscall_response_t sysinfo_syscall_handler(sysinfo_t* dest, uint32_t fill) {
    if (fill & INCLUDE_GLOBAL_INFO) {
        dest->global.uptime = PIT::getUptime();
        dest->global.totalmem = PhysicalPageManager::get().gettotalmem();
        dest->global.freemem = PhysicalPageManager::get().getfreemem();
    }

    if (fill & INCLUDE_LOCAL_INFO) {
        dest->local.runtime = gCurrentProcess->runtimestats.runtime;
        dest->local.committed = gCurrentProcess->memstats.allocated;
        dest->local.pagefaults = gCurrentProcess->memstats.pagefaults;
        dest->local.allocated = gCurrentProcess->getMemoryManager()->getTotalRegionsSize();
    }
    
    return OK;
}
