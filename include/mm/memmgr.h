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

#ifndef MM_MEMMGR
#define MM_MEMMGR

#include <sys/stdint.h>
#include <sys/nocopy.h>
#include <libc/intervals.h>
#include <mm/virt.h>

struct process_t;

class MemoryManager : NOCOPY {
    public:
        using region_t = IntervalList::interval_t;

        MemoryManager(process_t*);
        // finds a region and returns it
        region_t findRegion(size_t size);

        // is the given address part of a region?
        bool isWithinRegion(uintptr_t);

        // finds a region and maps all pages of it in the address space
        region_t findAndMapRegion(size_t size, const VirtualPageManager::map_options_t&);

        // finds a region and records it, but adds no mapping to the address space
        region_t findAndAllocateRegion(size_t size);

        // finds a region and records it, all pages of the region are mapped to the zero page
        region_t findAndZeroPageRegion(size_t size);

        // this is a dangerous method - it does little to no checking that adding this mapping is sane (TODO: it should)
        region_t addMapping(uintptr_t from, uintptr_t to);
    private:
        bool findRegionImpl(size_t size, region_t&);
        region_t addMapping(const region_t&);

        process_t* mProcess;
        IntervalList mRegions;
    public:
};

#endif
