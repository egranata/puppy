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

#include <libuserspace/exit.h>
#include <libuserspace/nice.h>
#include <libuserspace/printf.h>
#include <libuserspace/string.h>

#include <muzzle/stdlib.h>

int getnice(const char *spid) {
    auto pid = (uint16_t)atoi(spid);
    auto prio = (uint8_t)nice(pid);

    printf("Priority of process %u is %u\n", pid, prio);
    return 0;
}

int setnice(const char* spid, const char* sprio) {
    auto pid = (uint16_t)atoi(spid);
    auto prio = (uint8_t)atoi(sprio);

    prio = renice(pid, prio);
    printf("Priority of process %u set to %u (asked %s)\n", pid, prio, sprio);
    return 0;
}

int main(int argc, const char** argv) {
    switch (argc) {
        case 2:
            return getnice(argv[1]);
        case 3:
            return setnice(argv[1], argv[2]);
        default:
            printf("nice <pid> [<prio>]\n");
            exit(1);
    }
}
