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
#include <file.h>

extern "C"
uint32_t open(const char* path, uint32_t mode) {
    auto o = fopen_syscall((uint32_t)path, mode);
    if (o & 1) return -1;
    return o >> 1;
}

extern "C"
void close(uint32_t fid) {
    fclose_syscall(fid);
}

extern "C"
bool read(uint32_t fid, uint32_t size, unsigned char* buffer) {
    if (fid == gInvalidFd) return false;
    if (buffer == nullptr) return false;
    return 0 == fread_syscall(fid,size,(uint32_t)buffer);
}

extern "C"
bool write(uint32_t fid, uint32_t size, unsigned char* buffer) {
    if (fid == gInvalidFd) return false;
    if (buffer == nullptr) return false;
    return 0 == fwrite_syscall(fid,size,(uint32_t)buffer);
}

extern "C"
uintptr_t ioctl(uint32_t fid, uintptr_t a1, uintptr_t a2) {
    if (fid == gInvalidFd) return 0;
    auto ret = fioctl_syscall(fid,a1,a2);
    if (ret & 1) return 0;
    return (ret >> 1);    
}

// must be binary compatible with stat_t in filesystem
struct stat_t {
    uint32_t size;
};

extern "C"
bool fsize(uint32_t fid, uint32_t& sz) {
    stat_t stat;
    if (0 == fstat_syscall(fid, (uint32_t)&stat)) {
        sz = stat.size;
        return true;
    }
    return false;
}

