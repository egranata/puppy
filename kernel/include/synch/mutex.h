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

#ifndef SYNCH_MUTEX
#define SYNCH_MUTEX

#include <stdint.h>
#include <kernel/synch/refcount.h>
#include <kernel/sys/nocopy.h>
#include <kernel/synch/waitobj.h>
#include <kernel/syscalls/types.h>

struct process_t;

class Mutex : public WaitableObject {
    public:
        Mutex(const char*);
        void lock();
        bool trylock();
        void unlock();

        bool wait(uint32_t) override;

        ~Mutex();

        const char* key();

        static bool classof(const WaitableObject*);

    private:
        bool dolock(process_t*);

        char* mKey;
        bool mLocked;
        kpid_t mPid;

        friend class MutexManager;
};

#endif
