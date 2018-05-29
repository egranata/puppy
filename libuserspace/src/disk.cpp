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

#include <syscalls.h>
#include <disk.h>

controller getcontroller(uint8_t n) {
    controller result = 0;
    getcontroller_syscall(n, (uint32_t)&result);
    return result;
}

diskinfo getdisk(controller ctrl, uint8_t channel, uint8_t bus) {
    disk d;
    d.channel = channel;
    d.bus = bus;
    d.present = false;
    if (0 == discoverdisk_syscall(ctrl, (uint32_t)&d)) {
        return diskinfo{getcontroller(ctrl),d};
    }
    return diskinfo{0,{0, 0, false, "", 0, 0}};
}

// must be binary compatible with the struct of the same name in the syscalls implementation
struct disksyscalls_disk_descriptor {
    uint32_t ctrl;
    uint32_t dsk;
};

bool readsector(const diskinfo& di, uint32_t sec0, uint8_t count, unsigned char* buffer) {
    disksyscalls_disk_descriptor descriptor{di.ctrl, (uint32_t)&di.d};
    return 0 == readsector_syscall((uint32_t)&descriptor, sec0, count, (uint32_t)buffer);
}
bool writesector(const diskinfo& di, uint32_t sec0, uint8_t count, unsigned char* buffer) {
    disksyscalls_disk_descriptor descriptor{di.ctrl, (uint32_t)&di.d};
    return 0 == writesector_syscall((uint32_t)&descriptor, sec0, count, (uint32_t)buffer);
}
