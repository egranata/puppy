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

#ifndef PROCESS_MANAGER
#define PROCESS_MANAGER

#include <kernel/sys/stdint.h>
#include <kernel/i386/tss.h>
#include <kernel/syscalls/types.h>
#include <kernel/synch/semaphore.h>
#include <kernel/libc/slist.h>
#include <kernel/libc/queue.h>
#include <kernel/process/current.h>
#include <kernel/process/process.h>
#include <kernel/sys/nocopy.h>
#include <kernel/process/table.h>
#include <kernel/process/bitmap.h>
#include <kernel/libc/pqueue.h>
#include <kernel/libc/pair.h>
#include <kernel/libc/function.h>

namespace boot::task {
    uint32_t init();
}

#define PM_GLOBAL(Type, Name) static Type& Name();

extern "C" process_t *gParentProcess();

class ProcessManager : NOCOPY {
    public:
        // this is tied to the size of the GDT table prepared in loader.s
        static constexpr size_t gNumProcesses = 2000;

        static constexpr uint8_t gDefaultBasePriority = 10;

        static ProcessManager& get();

        // returns true if this is an EIP address that the scheduler
        // is allowed to interrupt
        static bool isinterruptible(uintptr_t);

        // NB: implementation of this is in process/exceptions.h
        void installexceptionhandlers();

        struct spawninfo_t {
            uintptr_t cr3; /** the root of the address space */
            uintptr_t eip; /** what address to start running from */
            exec_priority_t priority;
            uintptr_t argument; /** a pointer to be pushed onto the stack for the entry point to consume */

            const char** environment; /** a pointer to a list of strings that are to be passed to the process as its environment */
            const char* name;
            const char* cwd;

            exec_fileop_t* fileops; /** nullptr - or a pointer to a sequence of operations, terminated by END_OF_LIST */

            bool schedulable; /** should this process be scheduled */
            bool system; /** is this a system process */
            bool clone; /** is this process a clone of its parent */
        };

        process_t *spawn(const spawninfo_t&);
        process_t *kspawn(const spawninfo_t&);

        process_t *exec(const char* path, const char* args, const char** env, uint32_t flags, uint8_t prio = gDefaultBasePriority, uintptr_t argp = 0, exec_fileop_t* fileops = nullptr);

        void tick(bool can_yield);

        static void ctxswitch(process_t* task);

        void resumeat(process_t* task, uintptr_t eip, uintptr_t esp, uint16_t cs, uint16_t ss);

        kpid_t getpid();
        void yield(bool bytimer=false);
        void sleep(uint32_t durationMs);
        void exit(process_exit_status_t);
        void kill(kpid_t);

        kpid_t initpid();
        kpid_t schedulerpid();

        process_t* getprocess(kpid_t pid);

        process_exit_status_t collect(kpid_t pid);
        bool collectany(kpid_t*, process_exit_status_t*);

        void ready(process_t*);
        void deschedule(process_t*, process_t::State);
        void enqueueForDeath(process_t*);

        PM_GLOBAL(ProcessBitmap<ProcessManager::gNumProcesses>, gPidBitmap);
        PM_GLOBAL(ProcessBitmap<ProcessManager::gNumProcesses>, gGDTBitmap);
        PM_GLOBAL(ProcessTable<ProcessManager::gNumProcesses>, gProcessTable);
        PM_GLOBAL(ProcessTable<ProcessManager::gNumProcesses>, gExitedProcesses);
        PM_GLOBAL(slist<process_t*>, gCollectedProcessList);
        PM_GLOBAL(vector<process_t*>, gReadyQueue);

        class processcomparator {
            public:
                static int compare(process_t* p1, process_t* p2);
        };

        // cannot be defined via PM_GLOBAL because of macro expansion limitations (hint: would look like a 3-arg macro)
        static pqueue<process_t*, processcomparator>& gSleepQueue();

        process_t* cloneProcess(uintptr_t eip, exec_fileop_t* fileops);

        size_t numProcesses();
        void foreach(function<bool(const process_t*)>);

    private:
        ProcessManager();

        uint64_t fillGDT(process_t*);
        void forwardTTY(process_t*);
        void execFileops(process_t* parent, process_t *child, exec_fileop_t *fops);

        void switchtoscheduler();
        void exit(process_t*, process_exit_status_t);
        
        uintptr_t mProcessPagesLow;
        uintptr_t mProcessPagesHigh;

        friend uint32_t boot::task::init();
};

#undef PM_GLOBAL

#endif
