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

#include <stdint.h>
#include "ptable.h"
#include <libuserspace/printf.h>
#include <libuserspace/file.h>
#include <libuserspace/exit.h>
#include <libuserspace/memory.h>

void dump(uint8_t n, const partition& p) {
    if (p.sysid == 0) return;
    printf("Partition %u is of kind %u", n, p.sysid);
    printf(", it contains %u sectors starting at LBA %u", p.num_sectors, p.start_lba);
    printf(", it is%sbootable\n", p.bootable ? " " : "n't ");
}

void dump(const ptable& pt) {
    dump(0, pt.p0);
    dump(1, pt.p1);
    dump(2, pt.p2);
    dump(3, pt.p3);
}

void dump(const char* path) {
    printf("Partition Table Information for %s\n", path);
    printf("==============================================================================\n");
    auto fd = open(path, gModeRead);
    if (fd == gInvalidFd) {
        printf("could not open %s - exiting\n", path);
        exit(1);
    }

    ptable pt;
    if (false == read(fd, sizeof(pt), (uint8_t*)&pt)) {
        printf("could not read from %s - exiting\n", path);
        exit(1);
    }

    dump(pt);
    printf("==============================================================================\n");    
    close(fd);
}

int main(int argc, const char** argv) {
    for (auto i = 1; i < argc; ++i) {
        dump(argv[i]);
    }

    return 0;
}
