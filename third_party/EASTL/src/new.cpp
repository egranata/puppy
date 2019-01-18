/*
 * Copyright 2019 Google LLC
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

#include <EASTL/new>
#include <newlib/stdio.h>
#include <libcxxsup/memory.h>
#include <newlib/stdlib.h>

void* operator new[](size_t size, const char*, int, unsigned, const char*, int) {
    void* p = malloc(size);
    return p;
}

void* operator new[](size_t size, size_t, size_t, const char*, int, unsigned, const char*, int) {
    void* p = malloc(size);
    return p;
}