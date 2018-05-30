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

#include <libc/interval.h>

uint32_t interval_t::size() const {
    return to - from + 1;
}

bool interval_t::thisBefore(const interval_t& other) const {
    return (other.from >= to);
}

bool interval_t::operator==(const interval_t& other) const {
    return (from == other.from) && (to == other.to);
}

bool interval_t::contains(uint32_t num) const {
    return (num >= from) && (num <= to);
}

bool interval_t::contains(const interval_t& other) const {
    return contains(other.from) && contains(other.to);
}

bool interval_t::containsAny(const interval_t& other) const {
    return contains(other.from) || contains(other.to);
}

bool interval_t::intersects(const interval_t& other) const {
    return this->containsAny(other) || other.containsAny(*this);
}
