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

 #ifndef SYNCH_WAITQUEUE
 #define SYNCH_WAITQUEUE

#include <kernel/sys/nocopy.h>
#include <kernel/libc/dynqueue.h>

class process_t;

class WaitQueue : NOCOPY {
    public:
        WaitQueue() = default;

        // enter the WaitQueue, mark this process as WAITING, and return to scheduler
        void yield(process_t*, uint32_t);

        // enter the WaitQueue, mark the process as WAITING, and then return control to the process
        void wait(process_t*);

        void remove(process_t*);

        process_t* wakeone();
        void wakeall();
        process_t *peek();
        bool empty();
    private:
        struct queue_entry_t {
            uint64_t token;
            process_t *process;
        };

        static bool is_same_process(const queue_entry_t& q, process_t* t) {
            return q.process == t;
        }

        void pushToQueue(process_t*);

        bool wake(const queue_entry_t&);

        dynqueue<queue_entry_t> mProcesses;
};

#endif
