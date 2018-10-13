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

#ifndef PROCESS_PROCESS
#define PROCESS_PROCESS

#include <kernel/libc/bytesizes.h>
#include <kernel/sys/stdint.h>
#include <kernel/i386/tss.h>
#include <kernel/synch/messages.h>
#include <kernel/tty/tty.h>
#include <kernel/tty/file.h>
#include <kernel/fs/handletable.h>
#include <kernel/fs/filesystem.h>
#include <kernel/mm/heap.h>
#include <kernel/fs/vfs.h>
#include <kernel/libc/vec.h>
#include <kernel/libc/slist.h>
#include <kernel/mm/memmgr.h>
#include <kernel/syscalls/types.h>
#include <kernel/synch/waitqueue.h>

class Semaphore;
class Mutex;

struct process_t {
    static constexpr size_t gDefaultStackSize = 4_MB;
    using State = process_state_t;

    TaskStateSegment tss;
    size_t gdtidx;
    uintptr_t cr0;
    kpid_t pid;
    kpid_t ppid;
    const char* path;
    const char* args;
    const char* cwd;
    char** environ;
    State state;
    uint64_t sleeptill;
    messages_t msg;
    MemoryManager mmap;
    struct ttyinfo_t {
        TTY* tty;
        TTYFile ttyfile;

        ttyinfo_t();
        ttyinfo_t(TTY*);
        ttyinfo_t(const ttyinfo_t&) = delete;
        ttyinfo_t& operator=(const ttyinfo_t&);
        ~ttyinfo_t();
    } ttyinfo;
    struct {
        // this controls how many time quanta a process gets to run for once it has been scheduled
        struct {
            uint8_t max;
            uint8_t current;
        } quantum;
        // this controls how likely a process it to be scheduled at each possible decision point
        struct {
            uint64_t max;
            uint64_t current;
        } scheduling;
    } priority;
    uint8_t usedticks;
    uint8_t fpstate[512] __attribute__((aligned(16))); // TODO: FPU state takes 108 bytes - could we dynamically shrink this?
    Handletable<VFS::filehandle_t, 32> fds;
    Handletable<Semaphore*, 32> semas;
    Handletable<Mutex*, 32> mutexes;

    // initial values for esp0 and esp that were setup by the kernel
    // at initialization time - we need to free them when we're tearing down
    uintptr_t esp0start;
    uintptr_t espstart;

    process_exit_status_t exitstatus;

    slist<process_t*> children;

    union flags_t {
        uint16_t flags;
        bool system : 1;
        bool due_for_reschedule : 1;
    } flags;

    struct memstats_t {
        uint32_t available; /** size of all regions mapped by this process */
        uint32_t allocated; /** size of all memory allocated by this process */
        uint32_t pagefaults; /** number of page faults triggered by this process */
    } memstats;

    struct iostats_t {
        uint64_t read; /** total size of data read from storage by this process */
        uint64_t written; /** total size of data written from storage by this process */
    } iostats;

    struct runtimestats_t {
        uint64_t runtime; /** time that this process has been running */
    } runtimestats;

    static_assert(sizeof(flags_t) == sizeof(uint16_t), "process_t::flags_t must fit in 2 bytes");

    process_t();

    void clone(process_t*);

    MemoryManager* getMemoryManager();
};

static_assert(sizeof(process_t) <= 4096, "process_t does not fit in a page!");

#endif
