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

#ifndef SYNCH_EVENT
#define SYNCH_EVENT

#include <stdint.h>
#include <kernel/sys/nocopy.h>
#include <kernel/synch/waitobj.h>
#include <kernel/syscalls/types.h>

class Event : public WaitableObject {
    public:
        Event(const char*);
        void raise();
        void lower();

        bool raised() const;

        bool wait(uint32_t) override;

        ~Event();

        const char* key();

    private:
        const char* mKey;
        bool mRaised;
};

#endif
