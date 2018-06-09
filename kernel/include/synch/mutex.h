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

#include <sys/stdint.h>
#include <synch/refcount.h>
#include <libc/dynqueue.h>
#include <sys/nocopy.h>

struct process_t;

class Mutex : public refcounted<Mutex> {
    public:
        Mutex(const char*);
        void lock();
        bool trylock();
        void unlock();

        ~Mutex();

        const char* key();

    private:
        char* mKey;
        bool mLocked;
        uint16_t mPid;
        dynqueue<process_t*> mWaiters;

        friend class MutexManager;
};

#endif