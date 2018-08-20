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

#include <newlib/sys/errno.h>
#undef errno
extern "C" int  errno;

#include <newlib/sys/kill.h>
#include <newlib/sys/stat.h>
#include <newlib/sys/types.h>
#include <newlib/sys/fcntl.h>
#include <newlib/sys/times.h>
#include <newlib/sys/time.h>
#include <newlib/stdio.h>
#include <newlib/syscalls.h>
#include <newlib/stdlib.h>
#include <newlib/malloc.h>

#include <kernel/syscalls/types.h>

#define NEWLIB_IMPL_REQUIREMENT extern "C" 

NEWLIB_IMPL_REQUIREMENT void _exit(int code) {
    exit_syscall(code);
}

NEWLIB_IMPL_REQUIREMENT int close(int file) {
    return fclose_syscall(file);
}

NEWLIB_IMPL_REQUIREMENT int execve(char* /*name*/, char** /*argv*/, char** /*env*/) {
  errno = ENOMEM;
  return -1;    
}

NEWLIB_IMPL_REQUIREMENT int fork() {
  errno = EAGAIN;
  return -1;
}

NEWLIB_IMPL_REQUIREMENT int fstat(int /*file*/, struct stat* /*st*/) { return 0; }

NEWLIB_IMPL_REQUIREMENT int getpid() {
    return getpid_syscall() >> 1;
}

NEWLIB_IMPL_REQUIREMENT int getppid() {
    return getppid_syscall() >> 1;
}

NEWLIB_IMPL_REQUIREMENT int isatty(int file) {
    switch (file) {
        case 0:
        case 1:
        case 2:
            return 1;
        default:
            return 0;
    }
}

NEWLIB_IMPL_REQUIREMENT int link(char*, char*) {
    errno = EMLINK;
    return -1;
}

NEWLIB_IMPL_REQUIREMENT int lseek(int /*file*/, int /*ptr*/, int /*dir*/) { return 0; }

#define FLAG_TEST(flag) (flag == (flags & flag))

NEWLIB_IMPL_REQUIREMENT int open(const char *name, int flags, ... /*mode: file permissions not supported - ignore*/) {
    uint32_t puppy_flags = 0;
    if (FLAG_TEST(O_RDONLY)) {
        puppy_flags = FILE_OPEN_READ;
    } else if (FLAG_TEST(O_WRONLY)) {
        puppy_flags = FILE_OPEN_WRITE;
    } else if (FLAG_TEST(O_RDWR)) {
        puppy_flags = FILE_OPEN_READ | FILE_OPEN_WRITE;
    }

    if (FLAG_TEST(O_APPEND)) {
        puppy_flags |= FILE_OPEN_APPEND;
    }
    if (FLAG_TEST(O_TRUNC)) {
        puppy_flags |= FILE_OPEN_NEW;
    }

    auto fd = fopen_syscall(name, puppy_flags);
    if (fd & 1) return -1;
    return fd >> 1;
}

#undef FLAG_TEST

NEWLIB_IMPL_REQUIREMENT int read(int file, char* ptr, int len) {
    // special case non-blocking stdin
    if (file == 0) {
        int i = 0;
        len = 1;
        while (i < len) {
            char buf[2] = {0, 0};
            auto r = fread_syscall(file, 1, (uint32_t)&buf[0]);
            if (r != 0) ptr[i++] = buf[0]; // TODO: echo character
        }
        return len;
    } else {
        auto ro = fread_syscall(file, len, (uint32_t)ptr);
        if (ro & 1) return -1;
        return ro >> 1;
    }
}

uint8_t *gSbrkPointer = nullptr;

NEWLIB_IMPL_REQUIREMENT caddr_t sbrk(int incr) {
    if (gSbrkPointer == nullptr) {
        gSbrkPointer = (uint8_t*)mapregion_syscall(128*1024*1024, 1);
    }
    auto ptr = gSbrkPointer;
    gSbrkPointer += incr;
    return (caddr_t)ptr;
}

NEWLIB_IMPL_REQUIREMENT int stat(const char *file, struct stat *st);

NEWLIB_IMPL_REQUIREMENT clock_t times(struct tms* /*buf*/) {
    return -1;
}

NEWLIB_IMPL_REQUIREMENT int unlink(char *name) {
    auto r = fdel_syscall(name);
    if (r == 0) return 0;
    errno = ENOENT;
    return -1;
}

NEWLIB_IMPL_REQUIREMENT int wait(int *stat_loc) {
    uint16_t pid;
    process_exit_status_t status(0);
    if (0 != collectany_syscall(&pid, &status)) {
        return -1;
    }
    switch (status.reason) {
        case process_exit_status_t::reason_t::cleanExit:
            *stat_loc = status.status << 8;
            break;
        case process_exit_status_t::reason_t::exception:
        case process_exit_status_t::reason_t::kernelError:
            *stat_loc = 0x80;
            break;
        case process_exit_status_t::reason_t::killed:
            *stat_loc = 0x901; // signal 9
            break;
        case process_exit_status_t::reason_t::alive:
            return -1;
    }
    return pid;
}

NEWLIB_IMPL_REQUIREMENT int write(int file, char *ptr, int len) {
    auto wo = fwrite_syscall(file, len, (uint32_t)ptr);
    if (wo & 1) return -1;
    return wo >> 1;
}

NEWLIB_IMPL_REQUIREMENT int gettimeofday (struct timeval *__restrict __p, void *__restrict /**__tz: no timezone support */) {
    char* buf = nullptr;
    size_t n = 0;
    FILE *f = fopen("/devices/rtc/now", "r");
    if (f == nullptr) return -1;
    __getline(&buf, &n, f);
    fclose(f);
    __p->tv_sec = atoll(buf);
    free(buf);
    return 0;
}

NEWLIB_IMPL_REQUIREMENT int mkdir(const char *path, mode_t /**mode: no mode support*/) {
    auto mo = mkdir_syscall(path);
    if (mo == 0) return 0;
    errno = ENOENT;
    return -1;
}

NEWLIB_IMPL_REQUIREMENT int kill (pid_t p, int sig) {
    if (sig != SIGKILL) {
        errno = EPERM;
        return -1;
    }
    if (0 == kill_syscall(p)) {
        return 0;
    }
    return -1;
}

NEWLIB_IMPL_REQUIREMENT int ioctl(int fd, int a, int b) {
    auto io = fioctl_syscall(fd, a, b);
    if (io & 1) return -1;
    return io >> 1;
}

NEWLIB_IMPL_REQUIREMENT char **environ;
