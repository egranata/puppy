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

#include <string.h>

extern "C"
const char* strchr(const char* s, char c) {
    for(;s && *s;++s) {
        if (*s == c) return s;
    }
    return nullptr;
}

extern "C"
uint64_t atoi(const char* s) {
    uint64_t r = 0;
    while(s && *s) {
        auto v = *s - '0';
        if ((v >= 0) && (v <= 9)) {
            r = 10 * r + v;
            ++s;
        } else break;
    }

    return r;
}
