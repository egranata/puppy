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

#include <kernel/mm/memmgr.h>
#include <kernel/log/log.h>
#include <kernel/process/process.h>
#include <kernel/process/current.h>

static constexpr uintptr_t gKernelInitial = 0xC0000000;
static constexpr uintptr_t gKernelFinal =   0xFFFFFFFF;

LOG_TAG(PROTECT, 2);

MemoryManager::region_t::region_t (uintptr_t f, uintptr_t t, permission_t p) {
    from = f;
    to = t;
    permission = p;
    mmap_data.fhandle.object = nullptr;
    mmap_data.size = 0;
}

bool MemoryManager::region_t::operator==(const region_t& other) const {
    return (from == other.from) && (to == other.to);
}

bool MemoryManager::region_t::isMmapRegion() const {
    return (bool)mmap_data.fhandle && mmap_data.size > 0;
}

MemoryManager::MemoryManager(process_t* process) : mProcess(process), mRegions(), mAllRegionsSize(0) {
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

bool MemoryManager::protectRegionAtAddress(uintptr_t address, const VirtualPageManager::map_options_t& new_opts) {
    auto&& vmm(VirtualPageManager::get());

    region_t rgn;
    if (!isWithinRegion(address, &rgn)) return false;

    mRegions.foreach([rgn, new_opts] (region_t& R) -> bool {
        if (R.from == rgn.from && R.to == rgn.to) {
            TAG_DEBUG(PROTECT, "found a matching region; changing permissions");
            R.permission = new_opts; // (I)
            return false;
        }
        return true;
    });

    // (II)
    for (auto pp = rgn.from; pp <= rgn.to; pp += VirtualPageManager::gPageSize) {
        VirtualPageManager::map_options_t page_opts;
        bool mapped = false;
        bool zpmapped = false;
        mapped = vmm.mapped(pp, &page_opts);
        if (!mapped) zpmapped = vmm.zeroPageMapped(pp, &page_opts);
        if (mapped || zpmapped) {
            // do not allow this, as it would prevent the process from getting its own
            // copy of the page upon a COW fault; the copy will be properly marked
            // when it is made
            if (mapped && new_opts.rw() && page_opts.cow()) {
                TAG_WARNING(PROTECT, "page 0x%p is a COW page - cannot mark writable; skipping edit", pp);
                continue;
            } else {
                // TODO: any other bit that must be edited?
                page_opts.rw(new_opts.rw());
                vmm.newoptions(pp, page_opts);
            }
        }
    }

    return true;
}

bool MemoryManager::findRegionImpl(size_t size, MemoryManager::region_t& region) {
    size = toPage(size);
    if (mRegions.findFree(size, region)) {
        LOG_DEBUG("free region found [0x%x - 0x%x]", region.from, region.to);
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
        LOG_DEBUG("mapping all pages from 0x%p to 0x%p", region.from, region.to);
        auto&& vmm(VirtualPageManager::get());
        for(auto base = region.from; base < region.to; base += VirtualPageManager::gPageSize) {
            vmm.mapAnyPhysicalPage(base, opts);
        }
        return addRegion(region);
    } else {
        return {0,0};
    }
}

MemoryManager::region_t MemoryManager::findAndZeroPageRegion(size_t size, const VirtualPageManager::map_options_t& opts) {
    region_t region;
    if (findRegionImpl(size, region)) {
        region.permission = opts;
        LOG_DEBUG("zeropage mapping all pages from 0x%p to 0x%p", region.from, region.to);
        auto&& vmm(VirtualPageManager::get());
        vmm.mapZeroPage(region.from, region.to, opts);
        return addRegion(region);
    } else {
        return {0,0};
    }
}

MemoryManager::region_t MemoryManager::findAndFileMapRegion(VFS::filehandle_t fhandle, size_t size) {
    region_t region;
    if (fhandle && findRegionImpl(size, region)) {
        auto opts = VirtualPageManager::map_options_t()
            .rw(true)
            .user(true);
        auto&& vmm(VirtualPageManager::get());
        vmm.mapZeroPage(region.from, region.to, opts);
        fhandle.region = (void*)region.from;
        fhandle.object->incref(); // hold an extra reference to this file for the mmap
                                  // the file descriptor can now be closed by userspace
                                  // as the memory map will keep the underlying handle alive
        region.mmap_data.fhandle = fhandle;
        region.mmap_data.size = size;
        region.permission = opts;
        return addRegion(region);
    } else {
        return {0,0};
    }
}

MemoryManager::region_t MemoryManager::addMappedRegion(uintptr_t f, uintptr_t t) {
    return addRegion({f, t});
}

MemoryManager::region_t MemoryManager::addUnmappedRegion(uintptr_t f, uintptr_t t) {
    return addRegion({f, t});
}

void MemoryManager::removeRegion(region_t region) {
    auto&& vmm(VirtualPageManager::get());

    if (mRegions.del(region)) {
        LOG_DEBUG("region [0x%p - 0x%p] deleted, unmapping all pages", region.from, region.to);
        mAllRegionsSize -= region.size();
        gCurrentProcess->memstats.available -= region.size();
        for (auto base = region.from; base < region.to; base += VirtualPageManager::gPageSize) {
            vmm.unmap(base);
        }
        if (region.isMmapRegion()) region.mmap_data.fhandle.close();
    } else {
        LOG_DEBUG("attempted to remove region [0x%p - 0x%p] but was not found", region.from, region.to);
    }
}

MemoryManager::region_t MemoryManager::addRegion(const MemoryManager::region_t& region) {
    LOG_DEBUG("adding a memory region [0x%x - 0x%x]", region.from, region.to);
    mAllRegionsSize += region.size();
    gCurrentProcess->memstats.available += region.size();
    return mRegions.add(region), region;
}

uintptr_t MemoryManager::getTotalRegionsSize() const {
    return mAllRegionsSize;
}

void MemoryManager::clone(MemoryManager* ret) const {
    ret->mRegions = mRegions;
    ret->mAllRegionsSize = mAllRegionsSize;
}

void MemoryManager::cleanupAllRegions() {
    mRegions.foreach([this] (region_t rgn) -> bool {
        if (rgn.isMmapRegion() && rgn.mmap_data.fhandle) {
            LOG_DEBUG("clearing memory mapping for region [0x%p - 0x%p], fhandle=0x%p,0x%p",
                rgn.from, rgn.to,
                rgn.mmap_data.fhandle.filesystem, rgn.mmap_data.fhandle.object);
            rgn.mmap_data.fhandle.close();
        }
        return true;
    });
}
