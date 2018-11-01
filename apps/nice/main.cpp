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

#include <stdlib.h>
#include <stdio.h>
#include <syscalls.h>
#include <unistd.h>
#include <getopt.h>

void usage(int ec = 0) {
    printf("nice -p <pid> [-q <quantum>] [-s <sched>] [-Mc]\n");
    printf("edits (or displays) the priority of a process\n");
    printf("quantum controls the amount of time that a process will run once scheduled;\n");
    printf("sched controls the likelyhood of a process to be scheduled at each opportunity;\n");
    printf("-M will change / display the max priority; -c will change / display the current priority;\n");
    exit(ec);
}

int main(int argc, char* const* argv) {
    kpid_t pid = 0;
    uint8_t qt = 0;
    uint64_t sk = 0;
    int c;
    prioritize_target tgt = prioritize_target::PRIORITIZE_SET_CURRENT;
    const char* display_tgt = "current";
    bool is_setting = false;

    opterr = 0;

    while ((c = getopt (argc, argv, "Mcq:p:s:")) != -1) {
        switch (c) {
            case 'M':
                tgt = prioritize_target::PRIORITIZE_SET_MAXIMUM;
                display_tgt = "max";
                break;
            case 'c':
                tgt = prioritize_target::PRIORITIZE_SET_CURRENT;
                display_tgt = "current";
                break;
            case 'q': {
                is_setting = true;
                qt = atoi(optarg);
                break;
            }
            case 'p': {
                pid = atoi(optarg);
                break;
            }
            case 's': {
                is_setting = true;
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

    int ok = prioritize_syscall(pid, tgt, is_setting ? &prio_in : nullptr, &prio_out);
    if (ok == 0) {
        printf("for pid %u, %s priority is (quantum = %u, scheduling = %llu)\n", pid, display_tgt, prio_out.quantum, prio_out.scheduling);
    } else {
        usage(1);
    }

    return 0;
}
