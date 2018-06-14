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

#ifndef SYNCH_SEMAPHOREMANAGER
#define SYNCH_SEMAPHOREMANAGER

#include <kernel/synch/semaphore.h>
#include <kernel/synch/manager.h>
#include <kernel/synch/store.h>

class SemaphoreStore : public SynchStore<Semaphore> {
    public:
        Semaphore* getOrMake(const char* key);
};

class SemaphoreManager : public SynchManager<Semaphore, SemaphoreStore> {};

#endif
