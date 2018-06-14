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

#ifndef LIBC_BITMASK
#define LIBC_BITMASK

#include <kernel/sys/stdint.h>
#include <kernel/libc/enableif.h>

template<int N, typename T = uint32_t>
static inline constexpr T bit() {
    static_assert(N < 8 * sizeof(T));
    return (1u << N);
}

template<int L, int H, typename T = uint32_t>
inline constexpr typename enable_if<L == H, T>::type bitmask() {
    return bit<L, T>();
}

template<int L, int H, typename T = uint32_t>
inline constexpr typename enable_if<L < H, T>::type bitmask() {
    return bit<L, T>() | bitmask<L+1, H, T>();
}

#endif
