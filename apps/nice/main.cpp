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

#include <newlib/stdlib.h>
#include <newlib/stdio.h>
#include <newlib/syscalls.h>
#include <newlib/unistd.h>
#include <newlib/getopt.h>

void usage(int ec = 0) {
    printf("nice -p <pid> [-q <quantum>] [-s <sched>]\n");
    printf("edits (or displays) the priority of a process\n");
    printf("quantum controls the amount of time that a process will run once scheduled;\n");
    printf("sched controls the likelyhood of a process to be scheduled at each opportunity;\n");
    exit(ec);
}

int main(int argc, char* const* argv) {
    kpid_t pid = 0;
    uint8_t qt = 0;
    uint64_t sk = 0;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "q:p:s:")) != -1) {
        switch (c) {
            case 'q': {
                qt = atoi(optarg);
                break;
            }
            case 'p': {
                pid = atoi(optarg);
                break;
            }
            case 's': {
                sk = atoi(optarg);
                break;
            }
            default: {
                usage(1);
                break;
            }
        }
    }

    exec_priority_t prio_in; prio_in.quantum = qt; prio_in.scheduling = sk;
    exec_priority_t prio_out; prio_out.quantum = 0; prio_out.scheduling = 0;

    int ok = prioritize_syscall(pid, prioritize_target::PRIORITIZE_SET_CURRENT, &prio_in, &prio_out);
    if (ok == 0) {
        printf("for pid %u, quantum is %u, scheduling is %llu\n", pid, prio_out.quantum, prio_out.scheduling);
    } else {
        usage(1);
    }

    return 0;
}
