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

#include <syscalls/handlers.h>
#include <i386/reboot.h>
#include <drivers/rtc/rtc.h>
#include <drivers/pit/pit.h>
#include <mm/phys.h>

HANDLER0(reboot) {
    reboot();
    return OK; // we should never return from here
}

HANDLER1(uptime,dst) {
    uint64_t *dest = (uint64_t*)dst;
    *dest = PIT::getUptime();

    return OK;
}

HANDLER1(now,dst) {
    uint64_t *dest = (uint64_t*)dst;
    *dest = RTC::get().timestamp();

    return OK;
}

HANDLER1(getmeminfo,dst) {
    auto&& pm(PhysicalPageManager::get());

    uint64_t *dest = (uint64_t*)dst;
    *dest = ((uint64_t)pm.gettotalmem() << 32) | (pm.getfreemem());

    return OK;
}
