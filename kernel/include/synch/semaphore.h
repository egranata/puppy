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
#include <kernel/synch/waitobj.h>
#include <kernel/synch/refcount.h>
#include <kernel/sys/nocopy.h>

struct process_t;

class Semaphore : public WaitableObject {
    public:
        Semaphore(const char* name, uint32_t initial, uint32_t max);
        bool wait(uint32_t) override;
        void signal();

        ~Semaphore();

        const char* key();

        uint32_t value() const;
        uint32_t max() const;
        void max(uint32_t);

    private:
        char* mKey;
        uint32_t mValue;
        uint32_t mMaxValue;

        friend class SemaphoreManager;
};


#endif
