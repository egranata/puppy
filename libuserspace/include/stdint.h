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

#ifndef LIBUSERSPACE_STDINT
#define LIBUSERSPACE_STDINT

static_assert(sizeof(unsigned char) == 1, "unsigned char != uint8_t");
typedef unsigned char uint8_t;

static_assert(sizeof(unsigned short) == 2, "unsigned short != uint16_t");
typedef unsigned short uint16_t;

static_assert(sizeof(unsigned int) == 4, "unsigned int != uint32_t");
typedef unsigned int uint32_t;

static_assert(sizeof(unsigned long long) == 8, "unsigned long != uint64_t");
typedef unsigned long long uint64_t;

typedef long unsigned int size_t;

static_assert(sizeof(void*) == sizeof(uint32_t), "void* != uint32_t");
typedef uint32_t uintptr_t;

#endif
