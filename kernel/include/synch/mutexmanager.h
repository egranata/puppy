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

#ifndef SYNCH_MUTEXMANAGER
#define SYNCH_MUTEXMANAGER

#include <kernel/synch/mutex.h>
#include <kernel/synch/manager.h>
#include <kernel/synch/store.h>

class MutexStore : public SynchStore<Mutex> {
    public:
        Mutex* getOrMake(const char* key);
};

class MutexManager : public SynchManager<Mutex, MutexStore> {};

#endif
