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

#ifndef SYNCH_MANAGER
#define SYNCH_MANAGER

#include <sys/nocopy.h>
#include <log/log.h>

template<typename Object, typename Store>
class SynchManager : NOCOPY {
    protected:
        using self_t = SynchManager<Object, Store>;
        using store_t = Store;

        static store_t& getStore() {
            static store_t gStore;
            return gStore;
        }
    public:
        static self_t& get() {
            static self_t gManager;
            return gManager;
        }        

        Object* getOrMake(const char* key) {
            auto obj = getStore().getOrMake(key);
            LOG_DEBUG("for key %s, returning object %p", key, obj);
            return obj;
        }
        Object* getOrNull(const char* key) {
            auto obj = getStore().getOrNull(key);
            LOG_DEBUG("for key %s, returning object %p", key, obj);
            return obj;
        }
        void release(Object* obj) {
            auto deleted = getStore().release(obj);
            LOG_DEBUG("object %p was %s deleted", obj, deleted ? "" : "n't");
        }
};

#endif
