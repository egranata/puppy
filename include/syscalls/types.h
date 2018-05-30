/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This file should be includable from both kernel and userspace - it is meant to contain common macros and types that enable
// libuserspace and the kernel's system call interface to safely agree on the definition of data structures
#ifndef SYSCALLS_TYPES
#define SYSCALLS_TYPES

#include <sys/stdint.h>
using syscall_response_t = uint32_t;

struct sysinfo_t {
    struct {
        uint64_t uptime; /** uptime of the system */
        uint32_t totalmem; /** total amount of RAM */
        uint32_t freemem; /** amount of free RAM */
    } global;
    struct {
        uint64_t runtime; /** time this process has been running */
        uint32_t allocated; /** amount of memory allocated to this process */
        uint32_t committed; /** amount of actual RAM given to this process */
        uint32_t pagefaults; /** number of page faults that this process caused */
    } local;
};

enum {
    INCLUDE_GLOBAL_INFO = 1,
    INCLUDE_LOCAL_INFO = 2,
};

enum class filemode_t {
    read = 0,
    write = 1
};

enum {
    REGION_ALLOW_WRITE = 1
};

struct process_exit_status_t {
    enum class reason_t : uint8_t {
        alive = 0, /** this process is still alive */
        cleanExit = 1, /** this process chose to exit of its own free will */
        exception = 2, /** this process caused a fatal exception */
        killed = 3, /** this process was killed */
        kernelError = 0xFF /** the kernel encountered an issue and terminated the process */
    } reason;
    uint8_t status; /** exit code if clean-exit, error code if exception, reserved otherwise */

    uint32_t toWord() {
        return status | (((uint8_t)reason) << 24);
    }

    process_exit_status_t(reason_t r, uint8_t s) : reason(r), status(s) {}

    explicit process_exit_status_t(uint32_t word) {
        status = word & 0xFF;
        reason = (reason_t)(word >> 24);
    }
};

#endif
