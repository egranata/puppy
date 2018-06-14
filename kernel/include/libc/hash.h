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

#ifndef LIBC_HASH
#define LIBC_HASH

#include <kernel/libc/slist.h>
#include <kernel/libc/pair.h>

// static hashf::index(Key) -> size_t
// static comparef::eq(Key, Key) -> bool
template<typename Key, typename Value, typename hashf, typename comparef, size_t N>
class hash {
private:
    using value_type = pair<Key, Value>;
    using entry_type = slist<value_type>;
    entry_type mData[N];

public:
    hash() {}
    
    void insert(const Key& key, const Value& value) {
        auto idx = hashf::index(key) % N;
        auto& entry = mData[idx];
        if (!entry.empty()) {
            auto b = entry.begin(), e = entry.end();
            for(;b != e; ++b) {
                const auto& candidate = (*b);
                if (comparef::eq(candidate.first, key)) {
                    entry.remove(b);
                    break;
                }
            }
        }
        entry.add({key, value});
    }
    
    bool find(const Key& key, Value* value = nullptr) {
        auto idx = hashf::index(key) % N;
        auto& entry = mData[idx];
        for (const auto& candidate : entry) {
            if (comparef::eq(candidate.first, key)) {
                if (value) {
                    *value = candidate.second;
                }
                return true;
            }
        }
        
        return false;
    }
    
    bool erase(const Key& key) {
        auto idx = hashf::index(key) % N;
        auto& entry = mData[idx];
        auto b = entry.begin(), e = entry.end();
        for (; b != e; ++b) {
            const auto& candidate = (*b);
            if (comparef::eq(candidate.first, key)) {
                entry.remove(b);
                return true;
            }
        }
        
        return false;
    }
    
    void clear() {
        for(size_t i = 0; i < N; ++i) {
            mData[i].clear();
        }
    }
};

#endif
