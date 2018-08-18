#include <newlib/sys/errno.h>
#undef errno
extern "C" int  errno;

#include <newlib/sys/stat.h>
#include <newlib/sys/types.h>
#include <newlib/sys/fcntl.h>
#include <newlib/sys/times.h>
#include <newlib/sys/time.h>
#include <newlib/stdio.h>
#include <newlib/syscalls.h>

#define NEWLIB_IMPL_REQUIREMENT extern "C" 

NEWLIB_IMPL_REQUIREMENT void _exit() {
    exit_syscall(12);
}

NEWLIB_IMPL_REQUIREMENT int close(int /*file*/) { return 0; }

NEWLIB_IMPL_REQUIREMENT int execve(char *name, char **argv, char **env);

NEWLIB_IMPL_REQUIREMENT int fork();

NEWLIB_IMPL_REQUIREMENT int fstat(int /*file*/, struct stat* /*st*/) { return 0; }

NEWLIB_IMPL_REQUIREMENT int getpid();

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

NEWLIB_IMPL_REQUIREMENT int kill(int pid, int sig);

NEWLIB_IMPL_REQUIREMENT int link(char *oldname, char *newname);

NEWLIB_IMPL_REQUIREMENT int lseek(int /*file*/, int /*ptr*/, int /*dir*/) { return 0; }

NEWLIB_IMPL_REQUIREMENT int open(const char *name, int flags, ...);

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

NEWLIB_IMPL_REQUIREMENT clock_t times(struct tms *buf);

NEWLIB_IMPL_REQUIREMENT int unlink(char *name);

NEWLIB_IMPL_REQUIREMENT int wait(int *status);

NEWLIB_IMPL_REQUIREMENT int write(int file, char *ptr, int len) {
    auto wo = fwrite_syscall(file, len, (uint32_t)ptr);
    if (wo & 1) return -1;
    return wo >> 1;
}

NEWLIB_IMPL_REQUIREMENT int gettimeofday (struct timeval *__restrict __p, void *__restrict __tz);

NEWLIB_IMPL_REQUIREMENT char **environ;