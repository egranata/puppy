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
#include <kernel/mm/phys.h>
#include <kernel/process/current.h>
#include <kernel/syscalls/types.h>
#include <kernel/libc/sprint.h>
#include <kernel/log/log.h>
#include <kernel/time/manager.h>
#include <kernel/process/manager.h>
#include <kernel/drivers/acpi/acpica/acpica.h>

HANDLER0(reboot) {
    reboot();
    return OK; // we should never return from here
}

HANDLER0(halt) {
    AcpiEnterSleepStatePrep(ACPI_STATE_S5);
    AcpiEnterSleepState(ACPI_STATE_S5);
    return OK; // we should never return from here
}

syscall_response_t sysinfo_syscall_handler(sysinfo_t* dest, uint32_t fill) {
    if (fill & INCLUDE_GLOBAL_INFO) {
        dest->global.uptime = TimeManager::get().millisUptime();
        dest->global.totalmem = PhysicalPageManager::get().gettotalmem();
        dest->global.freemem = PhysicalPageManager::get().getfreemem();
        dest->global.ctxswitches = ProcessManager::numContextSwitches();
    }

    if (fill & INCLUDE_LOCAL_INFO) {
        dest->local.runtime = gCurrentProcess->runtimestats.runtime;
        dest->local.committed = gCurrentProcess->memstats.allocated;
        dest->local.pagefaults = gCurrentProcess->memstats.pagefaults;
        dest->local.allocated = gCurrentProcess->getMemoryManager()->getTotalRegionsSize();
        dest->local.ctxswitches = gCurrentProcess->runtimestats.ctxswitches;
    }
    
    return OK;
}
