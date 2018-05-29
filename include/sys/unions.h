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

#ifndef SYS_UNIONS
#define SYS_UNIONS

#include <sys/stdint.h>

union sixteen {
    uint16_t word;
    uint8_t byte[2];
};

union thirtytwo {
    uint32_t dword;
    uint16_t word[2];
    uint8_t byte[4];
};

union sixtyfour {
    uint64_t qword;
    uint32_t dword[2];
    uint16_t word[4];
    uint8_t byte[8];
};

template<unsigned int Nbytes>
union buffer {
    static_assert(Nbytes >= 4);

    uint32_t dword[Nbytes / 4];
    uint16_t word[Nbytes / 2];
    uint8_t byte[Nbytes];
};

template<typename T>
union wrapper {
    uint8_t bytes[sizeof(T)];
    T thing;

    static_assert(sizeof(wrapper<T>) == sizeof(T));
};

#endif
