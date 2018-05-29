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

#include <sys/stdint.h>
#include <libc/slist.h>

class IntervalList {
public:
    struct interval_t {
        uint32_t from;
        uint32_t to;
        
        uint32_t size() const;
        bool thisBefore(const interval_t& other) const;
        bool operator==(const interval_t& other) const;
        bool contains(uint32_t num) const;
        bool contains(const interval_t& other) const;
        bool containsAny(const interval_t& other) const;
        bool intersects(const interval_t& other) const;
    };
    
    void add(const interval_t& i);
    bool contains(uint32_t addr);
    bool findFree(uint32_t size, interval_t& range);
    bool add(uint32_t size, interval_t& range);
private:
    slist<interval_t> mIntervals;
};

#endif
