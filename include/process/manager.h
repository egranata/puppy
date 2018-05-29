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

#include <sys/stdint.h>
#include <i386/tss.h>
#include <synch/message.h>
#include <synch/semaphore.h>
#include <libc/queue.h>
#include <process/process.h>
#include <sys/nocopy.h>

class ProcessManager : NOCOPY {
    public:
        // this is tied to the size of the GDT table prepared in loader.s
        static constexpr size_t gNumProcesses = 2000;

        static ProcessManager& get();

        uintptr_t setprocesspagerange(uintptr_t);

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
        void exit(uint32_t exitcode);
        void kill(pid_t);

        pid_t initpid();
        pid_t schedulerpid();

        process_t* getcurprocess();
        process_t* getprocess(pid_t pid);

        uint32_t collect(pid_t pid);

        void ready(process_t*);
        void deschedule(process_t*, process_t::State);

    private:
        ProcessManager();

        void switchtoscheduler();
        void exit(process_t*, uint32_t exitcode);
        
        uintptr_t mProcessPagesLow;
        uintptr_t mProcessPagesHigh;
};

#endif
