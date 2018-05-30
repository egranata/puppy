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

#include <sys/stdint.h>
#include <i386/tss.h>
#include <synch/semaphore.h>
#include <synch/mutex.h>
#include <synch/message.h>
#include <tty/tty.h>
#include <tty/file.h>
#include <fs/handletable.h>
#include <fs/filesystem.h>
#include <mm/heap.h>
#include <fs/vfs.h>
#include <libc/vec.h>
#include <libc/slist.h>
#include <mm/memmgr.h>
#include <syscalls/types.h>

struct process_t {
    enum class State : uint8_t {
        NEW,
        AVAILABLE,
        WAITSYNC, /** waiting for a synchronization object */
        WAITMSG,  /** waiting for a message */
        SLEEPING,
        EXITED,
        COLLECTED,
        COLLECTING,
    };
    typedef uint16_t pid_t;

    TaskStateSegment tss;
    size_t gdtidx;
    uintptr_t cr0;
    pid_t pid;
    pid_t ppid;
    const char* path;
    const char* args;
    State state;
    uint64_t sleeptill;
    struct {
        vector<message_t> q;
    } msg;
    MemoryManager mmap;
    struct ttyinfo_t {
        TTY* tty;
        TTYFile* ttyfile;

        ttyinfo_t();
        ttyinfo_t(TTY*);
        ~ttyinfo_t();
    } ttyinfo;
    struct {
        uint8_t prio0; // initial priority at creation (and, incidentally, maximum priority)
        uint8_t prio; // current priority
    } priority;
    uint8_t usedticks;
    uint8_t fpstate[108];
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
    } flags;

    struct memstats_t {
        uint32_t allocated; /** size of all memory allocated by this process */
        uint32_t pagefaults; /** number of page faults triggered by this process */
    } memstats;

    struct runtimestats_t {
        uint64_t runbegin; /** tick value at which this process started running */
        uint64_t runtime; /** total time that this process has been running */
    } runtimestats;

    static_assert(sizeof(flags_t) == sizeof(uint16_t), "process_t::flags_t must fit in 2 bytes");

    process_t();

    MemoryManager* getMemoryManager();
};

static_assert(sizeof(process_t) <= 4096, "process_t does not fit in a page!");

#endif
