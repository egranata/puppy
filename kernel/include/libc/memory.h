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

#ifndef LIBC_MEMORY
#define LIBC_MEMORY

#include <kernel/sys/stdint.h>

extern "C"
void* memcopy(uint8_t* src, uint8_t* dst, size_t len);

extern "C"
void free(void* ptr);

extern "C"
void *malloc(size_t size);

extern "C"
void* calloc(size_t num, size_t len);

extern "C"
void* realloc(void *ptr, size_t new_size);

template <typename T>
void* operator new(size_t, T* ptr) { return ptr; }

template <typename T, int N>
void* operator new(size_t s, T ptr[N]) { return ptr; }

template<typename T = uint8_t, size_t S = sizeof(T)>
T* allocate(size_t num = 1) {
    return (T*)malloc(S * num);
}

#endif
