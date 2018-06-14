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

#include <libuserspace/syscalls.h>
#include <libuserspace/sysinfo.h>

sysinfo_t sysinfo(bool global, bool local) {
    sysinfo_t si;

    if (0 == sysinfo_syscall(
        &si,
        (global ? INCLUDE_GLOBAL_INFO : 0) |
        (local ? INCLUDE_LOCAL_INFO : 0)
    )) {
        return si;
    } else {
        return sysinfo_t();
    }
}
