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
#include <libuserspace/file.h>
#include <libuserspace/printf.h>
#include <libuserspace/yield.h>
#include <libuserspace/stdio.h>
#include <stdint.h>
#include <libuserspace/exec.h>
#include <libuserspace/getpid.h>
#include <libuserspace/string.h>
#include <kernel/sys/osinfo.h>
#include <libuserspace/collect.h>
#include <libuserspace/syscalls.h>
#include <libuserspace/memory.h>
#include <muzzle/stdlib.h>
#include <muzzle/string.h>
#include <libuserspace/shell.h>

template<typename T>
T* zero(T* thing, size_t size) {
    uint8_t *buf = (uint8_t*)thing;
    size *= sizeof(T);
    while(size) {
        *buf = 0;
        ++buf, --size;
    }
    return thing;
}

void handleExitStatus(uint16_t pid, process_exit_status_t es) {
    switch (es.reason) {
        case process_exit_status_t::reason_t::cleanExit:
            // do not write anything for a clean exit with status 0
            if(es.status) {
                printf("[child %u] exited - status %u\n", pid, es.status);
            }
            break;
        case process_exit_status_t::reason_t::exception:
            printf("[child %u] killed due to exception %u\n", pid, es.status);
            break;
        case process_exit_status_t::reason_t::kernelError:
            printf("[child %u] terminated due to kernel error %u\n", pid, es.status);
            break;
        case process_exit_status_t::reason_t::killed:
            printf("[child %u] killed\n", pid);
            break;
        case process_exit_status_t::reason_t::alive:
        default:
            printf("[child %u] termination reason unknown %x %x\n", pid, es.reason, es.status);
            break;
    }
}

void tryCollect() {
    uint16_t pid;
    process_exit_status_t status(0);

    if (collectany(&pid, &status)) {
        handleExitStatus(pid, status);
    }
}

static const char* trim(const char* s) {
    while(s && *s == ' ') ++s;
    return s;
}

static char* bufgetline(char* src, char* dest, size_t maxlen) {
    while(true) {
        if (maxlen == 0) return src;
        if (*src == 0) return src;
        if (*src == '\n') return ++src;
        *dest = *src;
        ++dest;
        ++src;
        --maxlen;
    }
}

bool runInitScript() {
    uint32_t fd = open("/system/config/init", gModeRead);
    if (fd == gInvalidFd) {
        printf("[init] warning: no /system/config/init script found\n");
        return true;
    }
    uint32_t size = 0;
    if (!fsize(fd, size)) {
        close(fd);
        return false;
    }
    if (size == 0) {
        close(fd);
        return true;
    }
    uint8_t* initfile = (uint8_t*)calloc(size, 1);
    if (size != read(fd, size, initfile)) {
        close(fd);
        return false;
    }

    char* src = (char*)initfile;
    while(true) {
        char buffer[512] = {0};
        src = bufgetline(src, &buffer[0], 511);
        if (buffer[0] == 0) break;
        printf("[init] %s\n", &buffer[0]);
        auto result = shell(&buffer[0]);
        if (result.reason != process_exit_status_t::reason_t::cleanExit) {
            printf("[init] non-clean exit in init script; exiting\n");
            close(fd);
            return false;
        }
        if (result.status != 0) {
            printf("[init] non zero exit in init script; exiting\n");
            close(fd);
            return false;
        }
        if (*src == 0) break;
    }

    close(fd);
    return true;
}

int main(int, const char**) {
    static char buffer[512] = {0};
    
    printf("This is the init program for " OSNAME ".\nEventually this program will do great things.\n");
    klog_syscall("init is up and running");

    if (!runInitScript()) {
        klog_syscall("init could not run config - will exit");
        exit(1);
    }

    while(true) {
        bzero(&buffer[0], 512);
        printf("init %u> ", getpid());
        getline(&buffer[0], 511);
        const char* program = trim(&buffer[0]);
        if (program == 0 || *program == 0) continue;
        bool letgo = ('&' == *program);
        if (letgo) ++program;
        char* args = (char*)strchr(program, ' ');
        if (args != nullptr) {
            *args = 0;
            ++args;
        }
        auto chld = exec(program, args, !letgo);
        if (!letgo) {
            auto exitcode = collect(chld);
            handleExitStatus(chld, exitcode);
        }
        tryCollect();
    }
}
