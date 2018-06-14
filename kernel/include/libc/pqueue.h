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

#ifndef LIBC_PQUEUE
#define LIBC_PQUEUE

#include <kernel/libc/vec.h>
#include <kernel/libc/swap.h>

template<typename ArgType, typename Comparator>
class pqueue {
private:
    vector<ArgType> mData;

    using idx_t = typename decltype(mData)::size_type;
    
    ArgType& at(idx_t idx) {
        return mData[idx];
    }
    
    idx_t parentIdx(idx_t idx) {
        return (idx - 1) / 2;
    }
    idx_t leftIdx(idx_t idx) {
        return (2 * idx) + 1;
    }
    idx_t rightIdx(idx_t idx) {
        return (2 * idx) + 2;
    }
    
    void down(idx_t idx) {
        const auto& left = leftIdx(idx);
        const auto& right = rightIdx(idx);
        
        auto largest = idx;
        
        if (left < size() && Comparator::compare(at(left), at(idx)) > 0) {
            largest = left;
        }
        if (right < size() && Comparator::compare(at(right), at(largest)) > 0) {
            largest = right;
        }
        
        if (largest != idx) {
            swap(at(idx), at(largest));
            down(largest);
        }
    }
    
    void up(idx_t idx) {
        const auto& parent = parentIdx(idx);
        
        if (idx && Comparator::compare(at(parent), at(idx)) < 0) {
            swap(at(idx), at(parent));
            up(parent);
        }
    }
    public:
    idx_t size() {
        return mData.size();
    }
    bool empty() {
        return size() == 0;
    }
    
    void insert(ArgType a) {
        mData.push_back(a);
        up(size() - 1);
    }
    
    ArgType& top() {
        return at(0);
    }
    
    ArgType pop() {
        auto t = top();
        at(0) = mData.back();
        mData.pop_back();
        down(0);
        return t;
    }
};

#endif
