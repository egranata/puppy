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

#ifndef LIBC_INTERVAL
#define LIBC_INTERVAL

#include <kernel/sys/stdint.h>

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

#endif
