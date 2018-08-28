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
#include <newlib/string.h>
#include <newlib/strings.h>
#include <stdarg.h>

#include <kernel/syscalls/types.h>

static __unused void klog(const char* fmt, ...) {
    char buf[1024] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    klog_syscall(buf);
    va_end(ap);
}

#define NEWLIB_IMPL_REQUIREMENT extern "C"

NEWLIB_IMPL_REQUIREMENT char* getcwd (char* buf = nullptr, size_t sz = 0) {
    if (buf == nullptr) sz = 0;
    auto ok = getcurdir_syscall(buf, &sz);
    if (ok == 0) return buf;
    if (buf == nullptr) {
        buf = (char*)calloc(1, sz + 1);
        ok = getcurdir_syscall(buf, &sz);
        if (ok == 0) return buf;
    }
    errno = ERANGE;
    return nullptr;
}

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

static char* concatPaths(const char* p1, const char* p2) {
    auto lp1 = strlen(p1);
    auto lp2 = strlen(p2);
    if (p1 == nullptr || lp1 == 0) return strdup(p2);
    if (p2 == nullptr || lp2 == 0) return strdup(p1);
    char* dest = (char*)calloc(1, lp1 + lp2 + 2);
    memcpy(dest, p1, lp1);
    if (p1[lp1 - 1] == '/' && p2[0] == '/') ++p2;
    else if (p1[lp1 - 1] != '/' && p2[0] != '/') dest[lp1++] = '/';
    memcpy(dest + lp1, p2, lp2);
    return dest;
}

#define FLAG_TEST(flag) (flag == (flags & flag))

#define FLAG_MATCH(c, p) if (FLAG_TEST(c)) puppy_flags |= p;

static bool isAbsolutePath(const char* path) {
    if (path[0] == '/') {
        if (nullptr == strstr(path, "/.")) {
            return true;
        }
    }

    return false;
}

NEWLIB_IMPL_REQUIREMENT int open(const char *name, int flags, ... /*mode: file permissions not supported - ignore*/) {
    uint32_t puppy_flags = 0;

    char* rp;

    if (name == nullptr || name[0] == 0) {
        errno = ENOENT;
        return -1;
    }

    if (!isAbsolutePath(name)) {
        char* cwd = getcwd();
        char* concat = concatPaths(cwd, name);
        klog("path concat '%s' + '%s' : '%s'", cwd, name, concat);
        rp = realpath(concat, nullptr);
        klog("real path of '%s' is '%s'", concat, rp);
        free(concat);
    } else {
        rp = strdup(name);
    }

    FLAG_MATCH(O_RDONLY, FILE_OPEN_READ);
    FLAG_MATCH(O_WRONLY, FILE_OPEN_WRITE);
    FLAG_MATCH(O_APPEND, FILE_OPEN_APPEND);
    FLAG_MATCH(O_TRUNC, FILE_OPEN_NEW);

    auto fd = fopen_syscall(rp, puppy_flags);
    free(rp);
    if (fd & 1) return -1;
    return fd >> 1;
}

#undef FLAG_MATCH
#undef FLAG_TEST

NEWLIB_IMPL_REQUIREMENT int read(int file, char* ptr, int len) {
    auto ro = fread_syscall(file, len, (uint32_t)ptr);
    if (ro & 1) return -1;
    return ro >> 1;
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

NEWLIB_IMPL_REQUIREMENT int fstat(int fd, struct stat* st) {
    file_stat_t fs;
    bzero(st, sizeof(struct stat));

    bool ok = (0 == fstat_syscall(fd, (uint32_t)&fs));

    if (ok) {
        st->st_size = fs.size;
        return 0;
    } else {
        errno = EIO;
        return -1;
    }
}

NEWLIB_IMPL_REQUIREMENT int stat(const char *file, struct stat *st) {
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        errno = EACCES;
        return -1;
    }

    int ok = fstat(fd, st);
    close(fd);
    return ok;
}

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
            *stat_loc = 0x0909; // signal 9
            break;
        case process_exit_status_t::reason_t::alive:
            return -1;
    }
    return pid;
}

NEWLIB_IMPL_REQUIREMENT pid_t waitpid(pid_t pid, int *stat_loc, int /*options: implement WNOHANG*/) {
    if ((pid == -1) || (pid == 0)) {
        return wait(stat_loc);
    }
    process_exit_status_t status(0);
    if (0 != collect_syscall(pid, &status)) {
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

NEWLIB_IMPL_REQUIREMENT int spawn(const char* path, const char* args, int flags) {
    auto eo = exec_syscall((uint32_t)path, (uint32_t)args, flags);
    if (eo & 1) return -1;
    return eo >> 1;
}

NEWLIB_IMPL_REQUIREMENT unsigned int sleep(unsigned int seconds) {
    sleep_syscall(1000 * seconds);
    return 0;
}

NEWLIB_IMPL_REQUIREMENT int chdir(const char *path) {
    auto ok = setcurdir_syscall(path);
    if (ok == 0) return 0;
    errno = EFAULT;
    return -1;
}

NEWLIB_IMPL_REQUIREMENT char **environ;
