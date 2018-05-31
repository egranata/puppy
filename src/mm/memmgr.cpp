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
#include <process/process.h>

static constexpr uintptr_t gKernelInitial = 0xC0000000;
static constexpr uintptr_t gKernelFinal =   0xFFFFFFFF;

MemoryManager::region_t::region_t (uintptr_t f, uintptr_t t, permission_t p) {
    from = f;
    to = t;
    permission = p;
}

bool MemoryManager::region_t::operator==(const region_t& other) const {
    return (from == other.from) && (to == other.to);
}

MemoryManager::MemoryManager(process_t* process) : mProcess(process), mRegions(), mAllRegionsSize(0), mMappedRegionsSize(0) {
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

bool MemoryManager::isWithinRegion(uintptr_t addr, region_t *rgn) {
    // hacky but effective - page 0 is never permitted to be mapped
    if (VirtualPageManager::page(addr) == 0) return false;
    return mRegions.contains(addr, rgn);
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
        region.permission = opts;
        LOG_DEBUG("mapping all pages from %p to %p", region.from, region.to);
        auto&& vmm(VirtualPageManager::get());
        for(auto base = region.from; base < region.to; base += VirtualPageManager::gPageSize) {
            vmm.mapAnyPhysicalPage(base, opts);
        }
        mMappedRegionsSize += region.size();
        return addRegion(region);
    } else {
        return {0,0};
    }
}

MemoryManager::region_t MemoryManager::findAndZeroPageRegion(size_t size, const VirtualPageManager::map_options_t& opts) {
    region_t region;
    if (findRegionImpl(size, region)) {
        region.permission = opts;
        LOG_DEBUG("zeropage mapping all pages from %p to %p", region.from, region.to);        
        auto&& vmm(VirtualPageManager::get());
        vmm.mapZeroPage(region.from, region.to);
        return addRegion(region);
    } else {
        return {0,0};
    }
}

MemoryManager::region_t MemoryManager::addMappedRegion(uintptr_t f, uintptr_t t) {
    auto rgn = addRegion({f, t});
    mMappedRegionsSize += rgn.size();
    return rgn;
}

MemoryManager::region_t MemoryManager::addUnmappedRegion(uintptr_t f, uintptr_t t) {
    return addRegion({f, t});
}

void MemoryManager::removeRegion(region_t region) {
    auto&& vmm(VirtualPageManager::get());

    if (mRegions.del(region)) {
        LOG_DEBUG("region [%p - %p] deleted, unmapping all pages", region.from, region.to);
        mAllRegionsSize -= region.size();
        mMappedRegionsSize -= region.size();
        for (auto base = region.from; base < region.to; base += VirtualPageManager::gPageSize) {
            vmm.unmap(base);
        }
    } else {
        LOG_DEBUG("attempted to remove region [%p - %p] but was not found", region.from, region.to);
    }
}

MemoryManager::region_t MemoryManager::addRegion(const MemoryManager::region_t& region) {
    LOG_DEBUG("adding a memory region [%x - %x]", region.from, region.to);
    mProcess->memstats.allocated += region.size();
    mAllRegionsSize += region.size();
    return mRegions.add(region), region;
}

void MemoryManager::mapOneMorePage() {
    mMappedRegionsSize += VirtualPageManager::gPageSize;
}

uintptr_t MemoryManager::getTotalRegionsSize() const {
    return mAllRegionsSize;
}

uintptr_t MemoryManager::getMappedSize() const {
    return mMappedRegionsSize;
}
