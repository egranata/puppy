// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <libc/intervals.h>

uint32_t IntervalList::interval_t::size() const {
    return to - from + 1;
}

bool IntervalList::interval_t::thisBefore(const interval_t& other) const {
    return (other.from >= to);
}

bool IntervalList::interval_t::operator==(const interval_t& other) const {
    return (from == other.from) && (to == other.to);
}

bool IntervalList::interval_t::contains(uint32_t num) const {
    return (num >= from) && (num <= to);
}

bool IntervalList::interval_t::contains(const interval_t& other) const {
    return contains(other.from) && contains(other.to);
}

bool IntervalList::interval_t::containsAny(const interval_t& other) const {
    return contains(other.from) || contains(other.to);
}

bool IntervalList::interval_t::intersects(const interval_t& other) const {
    return this->containsAny(other) || other.containsAny(*this);
}

void IntervalList::add(const interval_t& i) {
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

bool IntervalList::contains(uint32_t addr) {
    for (auto&& ival : mIntervals) {
        if (ival.from > addr) return false;
        if (ival.contains(addr)) return true;
    }
    return false;
}

bool IntervalList::findFree(uint32_t size, interval_t& range) {
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
        if (UINT32_MAX - ival.to >= size) {
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
        auto& i1 = *b;
        if (i1.from > (size + i0.to)) {
            range = {i0.to + 1, size + i0.to};
            return true;
        }
    }
    
    auto&& ival = mIntervals.back();
    if (UINT32_MAX - ival.to >= size) {
        range = {ival.to + 1, size + ival.to};
        return true;
    }
    
    return false;
}

bool IntervalList::add(uint32_t size, interval_t& range) {
    if (findFree(size, range)) {
        add(range);
        return true;
    }
    return false;
}
