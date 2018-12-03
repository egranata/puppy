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

#ifndef NEWLIB_MMAN
#define NEWLIB_MMAN

#include <newlib/stdint.h>
#include <newlib/sys/types.h>

#define PROT_EXEC 1
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_NONE 2

#define MAP_DENYWRITE 0
#define MAP_EXECUTABLE 0
#define MAP_FILE 0
#define MAP_NORESERVE 0

#define MAP_PRIVATE 1
#define MAP_SHARED 2
#define MAP_32BIT 2
#define MAP_FIXED 2
#define MAP_FIXED_NOREPLACE MAP_FIXED
#define MAP_GROWSDOWN 2
#define MAP_HUGETLB 2
#define MAP_HUGE_2MB MAP_HUGETLB
#define MAP_HUGE_1GB MAP_HUGETLB
#define MAP_LOCKED 2
#define MAP_NONBLOCK 2
#define MAP_STACK 2
#define MAP_SYNC 2
#define MAP_UNINITIALIZED 2

#define MAP_ANONYMOUS 4
#define MAP_ANON MAP_ANONYMOUS
#define MAP_POPULATE 8

#ifdef __cplusplus
extern "C" {
#endif

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);

#ifdef __cplusplus
}
#endif

#endif
