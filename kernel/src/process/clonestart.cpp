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

#include <kernel/process/clonestart.h>
#include <kernel/log/log.h>
#include <kernel/process/current.h>
#include <kernel/mm/memmgr.h>
#include <kernel/i386/primitives.h>

extern "C"
void clone_start(uintptr_t eip) {
    LOG_INFO("process %u is starting at 0x%p", gCurrentProcess->pid, eip);
    auto&& memmgr(gCurrentProcess->getMemoryManager());

    auto stackpermission = VirtualPageManager::map_options_t::userspace().clear(true);
    auto stackregion = memmgr->findAndZeroPageRegion(process_t::gDefaultStackSize, stackpermission);
    LOG_INFO("stack is begin = 0x%p, end = 0x%p", stackregion.to, stackregion.from);

    // gcc tends to expect ESP+4 to be available; and an 8 byte aligned stack
    // is a good thing for other reasons - so just leave 8 bytes and be done with it
    toring3(eip, stackregion.to - 8);
}
