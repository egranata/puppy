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

#include <newlib/sys/collect.h>
#include <newlib/syscalls.h>

NEWLIB_IMPL_REQUIREMENT process_exit_status_t collect(uint16_t pid) {
    process_exit_status_t result(0);
    if (0 == collect_syscall(pid, &result)) {
        return result;
    }
    return result;
}

NEWLIB_IMPL_REQUIREMENT bool collectany(bool wait, uint16_t* pid, process_exit_status_t* status) {
    return 0 == collectany_syscall(wait, pid, status);
}
