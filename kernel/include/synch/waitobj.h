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

#ifndef SYNCH_WAITOBJ
#define SYNCH_WAITOBJ

#include <kernel/synch/waitqueue.h>

struct process_t;

class WaitableObject {
    public:
        enum class Kind {
            Kind_Event,
            Kind_Mutex,
            Kind_Semaphore
        };
        Kind getKind() const;

        WaitQueue* waitqueue();
        virtual bool wait(uint32_t timeout) = 0;
        virtual bool myWake(process_t*);
    protected:
        WaitableObject(Kind);
        virtual ~WaitableObject();
    private:
        Kind mKind;
        WaitQueue mWQ;
};

#endif
