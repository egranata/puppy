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

#include <newlib/syscalls.h>
#include <newlib/stdlib.h>
#include <newlib/stdio.h>

int main() {
    auto sz = proctable_syscall(nullptr, 0);
    sz >>= 1;
    process_info_t* ptable = new process_info_t[sz];
    proctable_syscall(ptable, sz);

    printf("PID    PPID       NAME                    VM            PM               RT\n");
    for (auto i = 0u; i < sz; ++i) {
        auto& p = ptable[i];
        printf("%-7u%-7u    %-20.20s    %-.10u    %-.10u       %-4u\n",
               p.pid, p.ppid, p.path, p.vmspace, p.pmspace, p.runtime);
    }

    return 0;
}
