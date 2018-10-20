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

static const char* state2String(process_state_t state) {
    switch (state) {
        case process_state_t::NEW:        return "New       ";
        case process_state_t::AVAILABLE:  return "Available ";
        case process_state_t::WAITING:    return "Waiting   ";
        case process_state_t::SLEEPING:   return "Sleeping  ";
        case process_state_t::EXITED:     return "Exited    ";
        case process_state_t::COLLECTED:  return "Collected ";
        case process_state_t::COLLECTING: return "Collecting";
        default:                          return "Unknown   ";
    }
}

int main() {
    auto sz = proctable_syscall(nullptr, 0);
    sz >>= 1;
    process_info_t* ptable = new process_info_t[sz];
    proctable_syscall(ptable, sz);

    printf("%-6s", "PID");
    printf("%-6s", "PPID");
    printf("%-11s", "State");
    printf("%-30s", "Path");
    printf("%-15s", "Runtime (ms)");
    printf("%-11s", "VirtMem");
    printf("%-11s", "PhysMem");
    printf("%-6s",  "Flags");

    for (auto i = 0u; i < sz; ++i) {
        printf("\n");
        const auto& process = ptable[i];

        printf("%-6d", process.pid);
        printf("%-6d", process.ppid);
        printf("%-11s", state2String(process.state));
        printf("%-30.29s", process.path);
        printf("%-15lld", process.runtime);
        printf("%-11.10lu", process.vmspace);
        printf("%-11.10lu", process.pmspace);
        printf("%5s", process.flags.system ? "S" : "-");
    }

    return (void)printf("\n"), 0;
}
