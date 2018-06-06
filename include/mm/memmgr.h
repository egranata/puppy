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
#include <libc/interval.h>
#include <mm/virt.h>

struct process_t;

class MemoryManager : NOCOPY {
    public:
        struct region_t : public interval_t {
            using permission_t = VirtualPageManager::map_options_t;

            permission_t permission;

            region_t (uintptr_t = 0, uintptr_t = 0, permission_t = permission_t::kernel());

            bool operator==(const region_t&) const;
        };

        MemoryManager(process_t*);
        // finds a region and returns it
        region_t findRegion(size_t size);

        // is the given address part of a region?
        bool isWithinRegion(uintptr_t, region_t*);

        // finds a region and maps all pages of it in the address space
        region_t findAndMapRegion(size_t size, const VirtualPageManager::map_options_t&);

        // finds a region and records it, all pages of the region are mapped to the zero page
        region_t findAndZeroPageRegion(size_t size, const VirtualPageManager::map_options_t&);

        // here lie dragons - these calls always assume that the mapping is sane and do no checking
        region_t addMappedRegion(uintptr_t from, uintptr_t to);
        region_t addUnmappedRegion(uintptr_t from, uintptr_t to);

        // remove this region - and unmap all pages of it
        void removeRegion(region_t region);

        uintptr_t getTotalRegionsSize() const;
    private:
        bool findRegionImpl(size_t size, region_t&);
        region_t addRegion(const region_t&);

        process_t* mProcess;
        IntervalList<region_t> mRegions;

        uintptr_t mAllRegionsSize; /** size of all allocated regions (excludes the kernel) */
    public:
};

#endif
