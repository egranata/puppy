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

#ifndef SYNCH_SEMAPHORE
#define SYNCH_SEMAPHORE

#include <kernel/sys/stdint.h>
#include <kernel/synch/waitqueue.h>
#include <kernel/synch/refcount.h>
#include <kernel/sys/nocopy.h>

struct process_t;

class Semaphore : public refcounted<Semaphore> {
    public:
        Semaphore(const char* name, uint32_t initial, uint32_t max);
        void wait();
        void signal();

        ~Semaphore();

        const char* key();

    private:
        char* mKey;
        uint32_t mValue;
        uint32_t mMaxValue;
        WaitQueue mWQ;

        friend class SemaphoreManager;
};


#endif
