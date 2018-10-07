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

#ifndef LIBC_KEYEDSTORE
#define LIBC_KEYEDSTORE

#include <kernel/libc/hash.h>
#include <kernel/libc/pair.h>
#include <kernel/libc/string.h>
#include <kernel/libc/forward.h>

template<typename T, size_t N = 1024>
class KeyedStore {
    private:
        struct helper {
            static size_t index(const char* s) {
                if (s == nullptr) return 0;
                if (*s == 0) return 0;
                size_t i = 0;
                for(; *s; ++s) {
                    i = (31 * i) + *s;
                }
                return i;
            }
            
            static bool eq(const char* s1, const char* s2) {
                return 0 == strcmp(s1, s2);
            }
        };
    
    using val_t = pair<T*, uint32_t>;

    hash<const char*, val_t*, helper, helper, N> mHash;

    protected:
        KeyedStore() : mHash() {}

        template<typename ...Args>
        T* getOrMake(const char* key, Args&&... args) {
            makeOrNull(key, args...); // create if not existing; do nothing otherwise
            return getOrNull(key); // return if it exists (if it didn't before, it sure does now)
        }

        template<typename ...Args>
        T* makeOrNull(const char* key, Args&&... args) {
            val_t* pointer = nullptr;
            if (mHash.find(key, &pointer)) return nullptr;

            pointer = new val_t(new T(key, forward<Args>(args)...), 1);
            mHash.insert(key, pointer);
            return pointer->first;
        }

        T* getOrNull(const char* key) {
            val_t* pointer = nullptr;
            if (mHash.find(key, &pointer)) {
                return ++pointer->second, pointer->first;
            }
            
            return nullptr;
        }

        bool release(const char* key) {
            val_t* pointer = nullptr;
            if (mHash.find(key, &pointer)) {
                if (--pointer->second == 0) {
                    mHash.erase(key);
                    delete pointer->first;
                    delete pointer;
                    return true;
                }
            }
            return false;
        }
};

#endif
