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

#ifndef LIBC_MAPPING
#define LIBC_MAPPING

#include <sys/stdint.h>
#include <sys/nocopy.h>

/**
 * Establishes an identity mapping between a region of virtual and physical memory, e.g.
 * given 8KB at 0x5000
 * virtual(0x5000) -> physical(0x5000)
 * virtual(0x6000) -> physical(0x6000)
 * 
 * RAII - lets go of the mapping on destruction
 */ 
class Mapping : NOCOPY {
    public:
        Mapping(uintptr_t, size_t);
        ~Mapping();

        template<typename T>
        T* get() {
            return (T*)mBase;
        }
    private:
        uintptr_t mBase;
        uintptr_t mPageFirst;
        uintptr_t mPageLast;
};

#endif