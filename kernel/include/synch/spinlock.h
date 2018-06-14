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

#ifndef SYNCH_SPINLOCK
#define SYNCH_SPINLOCK

#include <kernel/sys/stdint.h>

typedef class spinlock {
    public:
        class unlock_t {
            private:
                unlock_t(spinlock& lock);
                ~unlock_t();
                spinlock& mLock;

                friend class spinlock;
        };
        spinlock();

        unlock_t lock();
        bool tryLock();
        void unlock();

    private:
        volatile uint32_t mLock = 0;
        spinlock(const spinlock&) = delete;
        spinlock(spinlock&&) = delete;
        spinlock& operator=(const spinlock&) = delete;

        friend class lock;
} spinlock_t;

#endif
