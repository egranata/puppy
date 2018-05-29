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

#include <mm/memmgr.h>
#include <log/log.h>

static constexpr uintptr_t gKernelInitial = 0xC0000000;
static constexpr uintptr_t gKernelFinal =   0xFFFFFFFF;

MemoryManager::MemoryManager(process_t* process) : mProcess(process), mRegions() {
    // do not allow the zero page to be mapped
    mRegions.add({0x0, 0xFFF});

    // the entire kernel region is off limits, never allow it
    mRegions.add({gKernelInitial,gKernelFinal});
}

// align this to being a multiple of page size
// this (+ the initial mappings being page-aligned) are sufficient
// to guarantee that - despite IntervalList's best efforts - we will never
// try to map anything outside of page-sized regions
static constexpr size_t toPage(size_t size) {
    auto m = (size % VirtualPageManager::gPageSize);
    if (m == 0) return size;
    return size - m + VirtualPageManager::gPageSize;
}

bool MemoryManager::isWithinRegion(uintptr_t addr) {
    return mRegions.contains(addr);
}

bool MemoryManager::findRegionImpl(size_t size, MemoryManager::region_t& region) {
    size = toPage(size);
    if (mRegions.findFree(size, region)) {
        LOG_DEBUG("free region found [%x - %x]", region.from, region.to);
        return true;
    } else {
        LOG_DEBUG("failed to find a region, returning false");
        return false;
    }
}

MemoryManager::region_t MemoryManager::findRegion(size_t size) {
    region_t region;
    if (findRegionImpl(size, region)) {
        return region;
    } else {
        return {0,0};
    }
}

MemoryManager::region_t MemoryManager::findAndMapRegion(size_t size, const VirtualPageManager::map_options_t& opts) {
    region_t region;
    if (findRegionImpl(size, region)) {
        LOG_DEBUG("mapping all pages from %p to %p", region.from, region.to);
        auto&& vmm(VirtualPageManager::get());
        for(auto base = region.from; base < region.to; base += VirtualPageManager::gPageSize) {
            vmm.newmap(base, opts);
        }
        return addMapping(region);
    } else {
        return {0,0};
    }
}

MemoryManager::region_t MemoryManager::findAndZeroPageRegion(size_t size) {
    region_t region;
    if (findRegionImpl(size, region)) {
        LOG_DEBUG("zeropage mapping all pages from %p to %p", region.from, region.to);        
        auto&& vmm(VirtualPageManager::get());
        for(auto base = region.from; base < region.to; base += VirtualPageManager::gPageSize) {
            vmm.mapZeroPage(base);
        }
        return addMapping(region);
    } else {
        return {0,0};
    }
}

MemoryManager::region_t MemoryManager::findAndAllocateRegion(size_t size) {
    region_t region;
    if (findRegionImpl(size, region)) {
        LOG_DEBUG("adding a known region from %p to %p", region.from, region.to);
        return addMapping(region);
    } else {
        return {0,0};
    }        
}

MemoryManager::region_t MemoryManager::addMapping(uintptr_t from, uintptr_t to) {
    return addMapping({from, to});
}

MemoryManager::region_t MemoryManager::addMapping(const MemoryManager::region_t& region) {
    LOG_DEBUG("adding a mapping region [%x - %x]", region.from, region.to);
    return mRegions.add(region), region;
}
