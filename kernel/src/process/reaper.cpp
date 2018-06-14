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

#include <kernel/process/manager.h>
#include <kernel/mm/virt.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>

extern "C"
void reaper(uint32_t exitword) {
    LOG_DEBUG("in reaper - releasing VM space");
    VirtualPageManager::get().cleanAddressSpace();

    LOG_DEBUG("in reaper - exiting process");
    ProcessManager::get().exit(process_exit_status_t(exitword));
}
