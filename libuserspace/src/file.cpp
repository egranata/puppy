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

#include <libuserspace/file.h>
#include <libuserspace/syscalls.h>

extern "C"
uint32_t open(const char* path, uint32_t mode) {
    auto o = fopen_syscall(path, mode);
    if (o & 1) return -1;
    return o >> 1;
}

extern "C"
bool del(const char* path) {
    auto o = fdel_syscall(path);
    return (o & 1) ? false : true;
}

extern "C"
void close(uint32_t fid) {
    fclose_syscall(fid);
}

extern "C"
size_t read(uint32_t fid, uint32_t size, unsigned char* buffer) {
    if (fid == gInvalidFd) return false;
    if (buffer == nullptr) return false;
    auto ro = fread_syscall(fid,size,(uint32_t)buffer);
    if (ro & 1) return 0;
    return ro >> 1;
}

extern "C"
size_t write(uint32_t fid, uint32_t size, unsigned char* buffer) {
    if (fid == gInvalidFd) return false;
    if (buffer == nullptr) return false;
    auto wo = fwrite_syscall(fid,size,(uint32_t)buffer);
    if (wo & 1) return 0;
    return wo >> 1;
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
    file_kind_t kind;
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

extern "C"
bool mkdir(const char* path) {
    auto o = mkdir_syscall(path);
    return (o & 1) ? false : true;
}
