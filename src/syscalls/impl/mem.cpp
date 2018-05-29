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
#include <process/manager.h>
#include <process/process.h>
#include <mm/memmgr.h>
#include <mm/virt.h>

HANDLER1(mapregion,size) {
    // only regions that are multiple of the page size can be mapped
    if (VirtualPageManager::offset(size)) {
        return ERR(UNIMPLEMENTED);
    }

    auto&& pmm(ProcessManager::get());
    auto self = pmm.getcurprocess();
    auto&& memmgr = self->getMemoryManager();

    VirtualPageManager::map_options_t opts;
    auto rgn = memmgr->findAndZeroPageRegion(size);
    if (rgn.from != 0) {
        return OK | rgn.from;
    }
    return ERR(OUT_OF_MEMORY);
}
