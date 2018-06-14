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

#ifndef LIBC_INTERVALS
#define LIBC_INTERVALS

#include <kernel/sys/stdint.h>
#include <kernel/libc/slist.h>
#include <kernel/libc/interval.h>

template <typename T /** : interval_t */, uint32_t max = 0xFFFFFFFF>
class IntervalList {
public:
    void add(const T& i);
    bool contains(uint32_t addr, T* where);
    bool findFree(uint32_t size, T& range);
    bool add(uint32_t size, T& range);
    bool del(const T& i);
private:
    slist<T> mIntervals;
};

template<typename T, uint32_t max>
void IntervalList<T, max>::add(const T& i) {
    if (mIntervals.empty()) {
        mIntervals.add(i);
        return;
    }
    if (mIntervals.count() == 1) {
        auto&& other = mIntervals.top();
        if (other.thisBefore(i)) {
            mIntervals.add(i);
        } else {
            mIntervals.add_head(i);
        }
        return;
    }
    auto b = mIntervals.begin(), e = mIntervals.end();
    while(b != e) {
        auto& other = *b;
        if (other.thisBefore(i)) {
            ++b; continue;
        }
        mIntervals.insert(b, i);
        return;
    }
    mIntervals.add(i);
}

template<typename T, uint32_t max>
bool IntervalList<T, max>::contains(uint32_t addr, T* where) {
    for (auto&& ival : mIntervals) {
        if (ival.from > addr) return false;
        if (ival.contains(addr)) {
            if (where != nullptr) *where = ival;
            return true;
        }
    }
    return false;
}

template<typename T, uint32_t max>
bool IntervalList<T, max>::findFree(uint32_t size, T& range) {
    if (size == 0) return false;

    if (mIntervals.empty()) {
        range = {0,size-1};
        return true;
    }
    
    if (mIntervals.count() == 1) {
        auto&& ival = mIntervals.top();
        if (ival.from >= size) {
            range = {0,size-1};
            return true;
        }
        if (max - ival.to >= size) {
            range = {ival.to + 1, size + ival.to};
            return true;
        }

        return false;
    }
    
    auto b = mIntervals.begin();
    auto e = mIntervals.end();
    
    while(true) {
        if (b == e) break;
        auto& i0 = *b;
        ++b;
        if (b == e) break;
        auto& i1 = *b;
        if (i1.from > (size + i0.to)) {
            range = {i0.to + 1, size + i0.to};
            return true;
        }
    }
    
    auto&& ival = mIntervals.back();
    if (max - ival.to >= size) {
        range = {ival.to + 1, size + ival.to};
        return true;
    }
    
    return false;
}

template<typename T, uint32_t max>
bool IntervalList<T, max>::add(uint32_t size, T& range) {
    if (findFree(size, range)) {
        add(range);
        return true;
    }
    return false;
}

template<typename T, uint32_t max>
bool IntervalList<T, max>::del(const T& i) {
    return mIntervals.remove(i);
}

#endif
