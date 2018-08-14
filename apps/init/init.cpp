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
        const char* cmdline = trim(&buffer[0]);
        printf("[init] %s\n", cmdline);
        auto result = shell(cmdline);
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

bool runShell() {
    auto pid = exec("/system/apps/shell", nullptr, true);
    return pid != 0;
}

int main(int, const char**) {
    printf("This is the init program for " OSNAME ".\nEventually this program will do great things.\n");
    klog_syscall("init is up and running");

    if (!runInitScript()) {
        klog_syscall("init could not run config - will exit");
        exit(1);
    }

    if (!runShell()) {
        klog_syscall("init could not run shell - will exit");
        exit(1);
    }

    while(true) {
        // TODO: init could be receiving messages from the rest of the system
        // and execute system-y operations on behalf of the rest of the system
        yield();
    }
}
