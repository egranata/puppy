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
#include <kernel/libc/str.h>

template<typename T, size_t N = 1024>
class KeyedStore {
    private:
        struct helper {
            static size_t index(const string& st) {
                if (st.c_str() == nullptr || st.size() == 0) return 0;
                const char* s = st.c_str();
                if (s == nullptr) return 0;
                if (*s == 0) return 0;
                size_t i = 0;
                for(; *s; ++s) {
                    i = (31 * i) + *s;
                }
                return i;
            }
            
            static bool eq(const string& s1, const string& s2) {
                return (s1 == s2);
            }
        };
    
    using val_t = pair<T*, uint32_t>;

    hash<string, val_t*, helper, helper, N> mHash;

    protected:
#define SKEY (string(key))
        KeyedStore() : mHash() {}

        template<typename ...Args>
        T* getOrMake(const char* key, Args&&... args) {
            T* object = makeOrNull(key, args...); // create if not existing; do nothing otherwise
            return object ? object : getOrNull(key); // return if it exists (if it didn't before, it sure does now)
        }

        template<typename ...Args>
        T* makeOrNull(const char* key, Args&&... args) {
            val_t* pointer = nullptr;
            if (mHash.find(SKEY, &pointer)) return nullptr;

            pointer = new val_t(new T(key, forward<Args>(args)...), 1);
            mHash.insert(SKEY, pointer);
            return pointer->first;
        }

        T* getOrNull(const char* key) {
            val_t* pointer = nullptr;
            if (mHash.find(SKEY, &pointer)) {
                return ++pointer->second, pointer->first;
            }
            
            return nullptr;
        }

        bool release(const char* key) {
            val_t* pointer = nullptr;
            if (mHash.find(SKEY, &pointer)) {
                if (--pointer->second == 0) {
                    mHash.erase(SKEY);
                    delete pointer->first;
                    delete pointer;
                    return true;
                }
            }
            return false;
        }
#undef SKEY
};

#endif
