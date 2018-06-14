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
#include <kernel/synch/message.h>
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

namespace boot::task {
    uint32_t init();
}

#define PM_GLOBAL(Type, Name) static Type& Name();

class ProcessManager : NOCOPY {
    public:
        // this is tied to the size of the GDT table prepared in loader.s
        static constexpr size_t gNumProcesses = 2000;

        static ProcessManager& get();

        // NB: implementation of this is in process/exceptions.h
        void installexceptionhandlers();

        using pid_t = process_t::pid_t;

        struct spawninfo_t {
            uintptr_t cr3;
            uintptr_t eip;
            uint8_t priority;
            uintptr_t argument;
            bool schedulable;
            bool system;
        };

        process_t *spawn(const spawninfo_t&);
        process_t *kspawn(const spawninfo_t&);

        process_t *setup(const char* path, const char* args, uint8_t prio = 2, uintptr_t argp = 0);
        void tick();

        void ctxswitch(pid_t task);
        static void ctxswitch(process_t* task);

        void resumeat(process_t* task, uintptr_t eip, uintptr_t esp, uint16_t cs, uint16_t ss);

        pid_t getpid();
        void yield();
        void sleep(uint32_t durationMs);
        void exit(process_exit_status_t);
        void kill(pid_t);

        pid_t initpid();
        pid_t schedulerpid();

        process_t* getprocess(pid_t pid);

        process_exit_status_t collect(pid_t pid);
        bool collectany(pid_t*, process_exit_status_t*);

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

        process_t* cloneProcess();

    private:
        ProcessManager();

        void switchtoscheduler();
        void exit(process_t*, process_exit_status_t);
        
        uintptr_t mProcessPagesLow;
        uintptr_t mProcessPagesHigh;

        friend uint32_t boot::task::init();
};

#undef PM_GLOBAL

#endif
