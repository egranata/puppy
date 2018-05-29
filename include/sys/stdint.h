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

#ifndef SYS_STDINT
#define SYS_STDINT

#define HAVE_STDINT_ALREADY

#include <sys/sanity.h>

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

static_assert(sizeof(signed char) == 1, "signed char != int8_t");
typedef signed char int8_t;

static_assert(sizeof(signed short) == 2, "signed short != int16_t");
typedef signed short int16_t;

static_assert(sizeof(signed int) == 4, "signed int != int32_t");
typedef signed int int32_t;

static_assert(sizeof(signed long long) == 8, "signed long != int64_t");
typedef signed long long int64_t;

typedef int32_t ptrdiff_t;

static constexpr uint8_t UINT8_MAX = -1;
static constexpr uint16_t UINT16_MAX = -1;
static constexpr uint32_t UINT32_MAX = -1;
static constexpr uint64_t UINT64_MAX = -1;

#endif
