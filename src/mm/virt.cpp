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

#include <sys/globals.h>
#include <i386/primitives.h>
#include <mm/virt.h>
#include <libc/string.h>
#include <libc/pair.h>
#include <panic/panic.h>
#include <libc/memory.h>

#define LOG_NODEBUG
#include <log/log.h>

static constexpr uintptr_t gBootVirtualOffset = 0xC0000000;

static constexpr uint32_t gPresentBit = 0x1;
static constexpr uint32_t gWritableBit = 0x2;
static constexpr uint32_t gUserBit = 0x4;
static constexpr uint32_t gNewPhysicalPageBit = 0x200;

VirtualPageManager::DirectoryEntry* VirtualPageManager::gPageDirectory = (DirectoryEntry*)gPageDirectoryAddress;

VirtualPageManager::map_options_t::map_options_t() : _rw(true), _user(false), _clear(false), _frompmm(false) {}

VirtualPageManager::map_options_t::map_options_t(bool rw, bool user, bool clear, bool frompmm) : _rw(rw), _user(user), _clear(clear), _frompmm(frompmm) {}

#define DEFINE_OPTION(type, name) \
type VirtualPageManager::map_options_t:: name () const { return _ ## name; } \
VirtualPageManager::map_options_t& VirtualPageManager::map_options_t:: name ( type val ) { _ ## name = val; return *this; }

DEFINE_OPTION(bool, rw);
DEFINE_OPTION(bool, user);
DEFINE_OPTION(bool, clear);
DEFINE_OPTION(bool, frompmm);

#undef DEFINE_OPTION

const VirtualPageManager::map_options_t& VirtualPageManager::map_options_t::kernel() {
	static map_options_t gOptions(true, false, false, false);
	return gOptions;
}

const VirtualPageManager::map_options_t& VirtualPageManager::map_options_t::userspace() {
	static map_options_t gOptions(true, true, false, false);
	return gOptions;
}

#define DEFINE_BOOL_FIELD(type, name, offset) bool type :: name () { \
	return mValue & (1 << offset); \
} \
void type :: name ( bool b ) { \
	if (b) { \
		mValue |= (1 << offset); \
	} else { \
		mValue &= ~(1 << offset); \
	} \
}

VirtualPageManager::DirectoryEntry::DirectoryEntry() : mValue(0) {}

uintptr_t VirtualPageManager::DirectoryEntry::table() {
	return mValue & 0xFFFFF000;
}

void VirtualPageManager::DirectoryEntry::table(uintptr_t value) {
    mValue &= 0xFFF;
	mValue |= (value & 0xFFFFF000);
}

DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, present, 0);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, rw, 1);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, user, 2);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, writethrough, 3);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, cacheoff, 4);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, accessed, 5);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, fourmb, 7);

VirtualPageManager::TableEntry::TableEntry() : mValue(0) {}

DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, present, 0);
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, rw, 1);
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, user, 2);
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, writethrough, 3);
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, cacheoff, 4);
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, accessed, 5);
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, dirty, 6);
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, global, 8);
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, frompmm, 9);
// leave bits 10 and 11 unused for now - they are a precious and scarce resource
// DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, bit10, 10)
// DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, bit11, 11)

uintptr_t VirtualPageManager::TableEntry::page() {
	return mValue & 0xFFFFF000;
}

void VirtualPageManager::TableEntry::page(uintptr_t value) {
    mValue &= 0xFFF;
	mValue |= (value & 0xFFFFF000);
}

#undef DEFINE_BOOL_FIELD

VirtualPageManager::TableEntry* VirtualPageManager::gPageTable(size_t table) {
	auto tbl = (TableEntry*)(gPageTableAddress + PhysicalPageManager::gPageSize * table);
	// LOG_DEBUG("WOMAN gPageTableAddress = %p, table = %u, tbl = %p", gPageTableAddress, table, tbl);
	return tbl;
}

VirtualPageManager::TableEntry& VirtualPageManager::gPageTable(size_t dir, size_t tbl) {
	return gPageTable(dir)[tbl];
}

uintptr_t VirtualPageManager::page(uintptr_t addr) {
    return addr & ~(gPageSize - 1);
}
uintptr_t VirtualPageManager::offset(uintptr_t addr) {
    return addr & (gPageSize - 1);
}

bool VirtualPageManager::iskernel(uintptr_t addr) {
	return addr >= 0xC0000000;
}

VirtualPageManager& VirtualPageManager::get() {
	static VirtualPageManager gVM;
	
	return gVM;
}

// assumes physical is page aligned
// only works until proper page tables are in place
template<typename T>
T* scratchMap(uintptr_t physical, uint16_t page) {
	uintptr_t gLogicalMapping = 0xFFC00000 + 4096*page;
	uintptr_t newmapping = 0x3 | physical;
	uintptr_t oldmapping = bootpagetable<uintptr_t*>()[page];
	// idempotent and with no unnecessary slowdowns
	if (newmapping != oldmapping) {
		bootpagetable<uintptr_t*>()[page] = newmapping;
		invtlb(gLogicalMapping);
	}
	return (T*)gLogicalMapping;
}

struct PagingIndices {
	uint32_t dir;
	uint32_t tbl;
	
	PagingIndices(uint32_t dir, uint32_t tbl) : dir(dir), tbl(tbl) {}
	
	PagingIndices(uintptr_t virt) {
		dir = virt >> 22;
		tbl = (virt >> 12) & 0x03FF;
	}
	
	template<typename T>
	PagingIndices(T* virt) : PagingIndices((uintptr_t)virt) {}
	
	PagingIndices& operator++() {
		if (++tbl == 1024) {
			tbl = 0;
			if(++dir == 1024) {
				dir = 0;
			}
		}
		return *this;
	}

	bool operator==(const PagingIndices& indices) const {
		return (dir == indices.dir) && (tbl == indices.tbl);
	}

	bool operator!=(const PagingIndices& indices) const {
		return (dir != indices.dir) || (tbl != indices.tbl);
	}

	bool operator<(const PagingIndices& indices) const {
		return (dir < indices.dir) || (tbl < indices.tbl);
	}

	uintptr_t address() const {
		// 4MB per directory, 4KB per table
		return 4_MB * dir + 4_KB * tbl;
	}
	
	VirtualPageManager::TableEntry& table() {
		return VirtualPageManager::gPageTable(dir, tbl);
	}
};

uintptr_t VirtualPageManager::ksbrk(size_t amount) {
	if (mKernelHeap == nullptr) {
		PANIC("kernel heap never initialized. can't allocate.")
	}

	auto ptr = mKernelHeap->sbrk(amount);
	if (ptr == 0) {
		PANIC("kernel heap allocation failed");
	}

	return ptr;
}

uintptr_t VirtualPageManager::gZeroPageVirtual() {
	return zeropage<>();
}

uintptr_t VirtualPageManager::gZeroPagePhysical() {
	return mZeroPagePhysical;
}

uintptr_t VirtualPageManager::mapZeroPage(uintptr_t virt) {
	return map(mZeroPagePhysical, virt, map_options_t(map_options_t::kernel()).rw(false));
}

uintptr_t VirtualPageManager::map(uintptr_t phys, uintptr_t virt, const map_options_t& options) {
	PagingIndices indices(virt);
	
	LOG_DEBUG("asked to map phys %p to virt %p; that will be page dir entry %u, and page table entry %u", phys, virt, indices.dir, indices.tbl);

	if (options.user() & !options.clear()) {
		LOG_WARNING("page mapping virt=%p phys=%p shall be visible to userspace, but is not being zeroed out", virt, phys);
	}

	TableEntry &tbl(indices.table());
	tbl.present(true);
	tbl.rw(options.rw());
	tbl.user(options.user());
	tbl.frompmm(options.frompmm());
	tbl.page(phys);
	invtlb(virt);

	if (options.clear()) {
		auto pageptr = (uint8_t*)virt;
		bzero(pageptr, gPageSize);
	}

	return virt;
}

uintptr_t VirtualPageManager::maprange(uintptr_t physlow, uintptr_t physhigh, uintptr_t virt, const map_options_t& options) {
	// round low end low to previous page and high end to next page
	physlow = page(physlow);
	if (physhigh > page(physhigh)) {
		physhigh = page(physhigh) + gPageSize;
	}
	auto phys = physlow;
	for(;phys < physhigh; phys += gPageSize, virt += gPageSize) {
		map(phys, virt, options);
	}

	// return last page mapped
	return virt;
}

uintptr_t VirtualPageManager::newmap(uintptr_t virt, const map_options_t& options) {
	if (mapped(virt)) {
		LOG_DEBUG("virtual address %p already mapped", virt);
	} else {
		auto&& physall(PhysicalPageManager::get());
		auto phys = physall.alloc();
		auto opts = map_options_t(options).frompmm(true);
		map(phys, virt, opts);
	}

	return virt;
}

void VirtualPageManager::unmap(uintptr_t virt) {
	PagingIndices indices(virt);
	
	LOG_DEBUG("asked to unmap virt %p; that will be page dir entry %u, and page table entry %u", virt, indices.dir, indices.tbl);

	TableEntry &tbl(indices.table());
	tbl.present(false);
	invtlb(virt);

	if (tbl.frompmm()) {
		auto& pmm(PhysicalPageManager::get());
		pmm.dealloc(tbl.page());
	}
	tbl.page(0);

}

void VirtualPageManager::unmaprange(uintptr_t low, uintptr_t high) {
	for (; low < high; low += gPageSize) {
		unmap(low);
	}
}

uintptr_t VirtualPageManager::mapping(uintptr_t virt) {
	uintptr_t b = page(virt);
	uintptr_t o = offset(virt);
	PagingIndices indices(b);
	TableEntry &tbl(indices.table());
	if (tbl.present()) {
		return tbl.page()+o;
	} else {
		return 0;
	}
}

bool VirtualPageManager::mapped(uintptr_t virt, map_options_t* opts) {
	uintptr_t b = page(virt);
	PagingIndices indices(b);
	TableEntry &tbl(indices.table());

	if (tbl.present()) {
		if (opts) {
			opts->rw(tbl.rw());
			opts->user(tbl.user());
			opts->frompmm(tbl.frompmm());
		}
		return true;
	}

	return false;
}

uintptr_t VirtualPageManager::findpage(uintptr_t low, uintptr_t high, const map_options_t& options) {
	auto ilow = PagingIndices(low);
	auto ihigh = PagingIndices(high);
	while (ilow < ihigh) {
		if(mapped(ilow.address())) {
			++ilow;
			continue;
		} else {
			return newmap(ilow.address(), options);
		}
	}

	// return something non-page-aligned if we failed
	return (gPageSize - 1);
}

// returns the physical address of the page directory suitable for
// launching a new process. The kernel is mapped in, sharing mapping with
// the process that called newmap(). No user space memory is available.
uintptr_t VirtualPageManager::newmap() {
	PhysicalPageManager &phys(PhysicalPageManager::get());

	auto new_table_options = map_options_t().rw(true).user(false).clear(true);
	auto table_options = map_options_t().rw(true).user(false);

	// allocate page directory & page tables & clear out page tables
	uintptr_t pPageDir = phys.alloc();
	LOG_DEBUG("new page tables will be at physical %p", pPageDir);
	uint32_t *pageDir = (uint32_t*)map(pPageDir, 0x40000000, new_table_options);
	uint32_t *pageTbl = nullptr;
	uint32_t *kPageDir = (uint32_t*)0xffbff000;
	for (auto i = 0u; i < 768u; ++i) {
		auto pPageTbl = phys.alloc();
		LOG_DEBUG("new page table %u will be at physical %p", i, pPageTbl);
		pageTbl = (uint32_t*)map(pPageTbl, 0x40001000, new_table_options);
		LOG_DEBUG("[S0] pageTbl is physical %p and logical %p mapping is %p", pPageTbl, pageTbl, mapping(0x40001000));
		pageDir[i] = pPageTbl | (gPresentBit | gWritableBit | gUserBit | gNewPhysicalPageBit);
		unmap(0x40001000);
	}
	// kernel memory just points to our original allocation
	for (auto i = 768u; i < 1022u; ++i) {
		LOG_DEBUG("new page table %u will be at physical %p", i, kPageDir[i]);
		pageDir[i] = kPageDir[i];
	}
	// but allocate for pages 1022 and 1023 (i.e. page tables)
	for (auto i = 1022u; i < 1024u; ++i) {
		auto pPageTbl = phys.alloc();
		LOG_DEBUG("new page table %u will be at physical %p", i, pPageTbl);
		pageTbl = (uint32_t*)map(pPageTbl, 0x40001000, new_table_options);
		LOG_DEBUG("[S0] pageTbl is physical %p and logical %p mapping is %p", pPageTbl, pageTbl, mapping(0x40001000));
		pageDir[i] = pPageTbl | (gPresentBit | gWritableBit | gUserBit | gNewPhysicalPageBit);
		unmap(0x40001000);		
	}

	// map kernel memory, i.e. anything from 0xC0000000 to the start of the page tables mapping
	PagingIndices kernelidx(0xC0000000);
	LOG_DEBUG("kernel mapping starts at dir %u page %u", kernelidx.dir, kernelidx.tbl);
	while(kernelidx.dir < 1022) {
		auto curmap = mapping(kernelidx.address());
		if (0 != curmap) {
			LOG_DEBUG("required to remap dir %u page %u to physical %p", kernelidx.dir, kernelidx.tbl, curmap);
			pageTbl = (uint32_t*)map(pageDir[kernelidx.dir] & ~0x7, 0x40001000, table_options);
			LOG_DEBUG("[S2] pageTbl is physical %p and logical %p mapping is %p", pageDir[kernelidx.dir] & ~0x7, pageTbl, mapping(0x40001000));
			pageTbl[kernelidx.tbl] = curmap | (gPresentBit | gWritableBit | gNewPhysicalPageBit);
			unmap(0x40001000);
		}
		++kernelidx;
	}
	// now fill in page directory entry
	pageTbl = (uint32_t*)map(pageDir[1022] & ~0x7, 0x40001000, table_options);
	pageTbl[1023] = pPageDir | (gPresentBit | gWritableBit | gUserBit);
	pageTbl = (uint32_t*)map(pageDir[1023] & ~0x7, 0x40001000, table_options);
	// and all of the page table pointers:
	// 0 thru 767 must point to the pages we just allocated;
	// 768 thru 1021 must point to the kernel pages;
	// 1022 and 1023 must point to what we allocated
	for (auto i = 0u; i < 768u; ++i) {
		pageTbl[i] = pageDir[i] & ~(gUserBit | gNewPhysicalPageBit);
	}
	for (auto i = 768u; i < 1022u; ++i) {
		pageTbl[i] = kPageDir[i];
	}
	for (auto i = 1022u; i < 1024u; ++i) {
		pageTbl[i] = pageDir[i] & ~(gUserBit | gNewPhysicalPageBit);
	}
	unmap(0x40001000);
	unmap(0x40000000);
	return pPageDir;	
}

// deallocate all memory that this process ever acquired
// this leaves kernel memory in place, so that the rest of the exit()
// can occur, and also because kernel memory is shared anyway
void VirtualPageManager::release() {
	PhysicalPageManager &phys(PhysicalPageManager::get());

	auto freepages = phys.getfreepages();

	for (auto i = 0u; i < 1024; ++i) {
		if ((i >= 768) && (i <= 1021)) continue;
		auto tbl = gPageTable(i);
		LOG_DEBUG("page table %u is at %p", i, tbl);
		for (auto j = 0u; j < 1024u; ++j) {
			if (tbl[j].present() && tbl[j].frompmm()) {
				auto ptr = tbl[j].page();
				LOG_DEBUG("freeing page %u at %p", j, ptr);
				phys.dealloc(ptr);
			}
		}

		auto m = mapping((uintptr_t)tbl);
		LOG_DEBUG("and now freeing entire table at %p", m);
		phys.dealloc(m);
	}

	freepages = phys.getfreepages() - freepages;
	LOG_INFO("freed %u pages, worth %u bytes", freepages, PhysicalPageManager::gPageSize * freepages);
}

VirtualPageManager::KernelHeap::KernelHeap(uintptr_t low, uintptr_t high) : Heap(low, high), mVm(VirtualPageManager::get()), mPhys(PhysicalPageManager::get()) {}

uintptr_t VirtualPageManager::KernelHeap::oneblock() {
	auto opts = VirtualPageManager::map_options_t().rw(true).user(false).clear(true);
	mVm.map(mPhys.alloc(), current(), opts);
	return current();
}

void VirtualPageManager::setheap(uintptr_t heap, size_t size) {
	mKernelHeap = new (mKernelHeapMemory) KernelHeap(heap, heap + size);
	LOG_INFO("kernel heap will begin at %p and span to %p", mKernelHeap->low(), mKernelHeap->high());
}

uintptr_t VirtualPageManager::getheap() const {
	return mKernelHeap->current();
}

uintptr_t VirtualPageManager::getheapbegin() const {
	return mKernelHeap->low();
}

uintptr_t VirtualPageManager::getheapend() const {
	return mKernelHeap->high();
}

VirtualPageManager::VirtualPageManager() : mKernelHeap(nullptr) {
	PhysicalPageManager &phys(PhysicalPageManager::get());
	
	bzero(&mKernelHeapMemory[0], sizeof(mKernelHeapMemory));

	LOG_INFO("setting up sane page directories");
	uintptr_t pPde = phys.alloc();
	uint32_t *pde = scratchMap<uint32_t>(pPde, 0);
	LOG_DEBUG("pde physical address %p, virrtual address %p", pPde, pde);
	
	PagingIndices kernelidx(kernel_start());
	LOG_DEBUG("kernel mapping starts at dir %u page %u", kernelidx.dir, kernelidx.tbl);
	
	uint8_t *pq = nullptr;
	for (auto i = 0u; i < 1024; ++i) {
		pde[i] = phys.alloc() | 0x7;
		pq = scratchMap<uint8_t>(pde[i] & ~0x7, 4);
		bzero(pq, PhysicalPageManager::gPageSize);
	}
	// now start allocating memory for the kernel
	uintptr_t cur = kernel_start() - gBootVirtualOffset;
	uintptr_t end = kernel_end() - gBootVirtualOffset;
	uint32_t *pt = nullptr;
	while(cur <= end) {
		pt = scratchMap<uint32_t>(pde[kernelidx.dir] & ~0x7, 1);
		LOG_DEBUG("mapping phys %p to dir %u page %u aka virt %p", cur, kernelidx.dir, kernelidx.tbl, kernelidx.address());
		pt[kernelidx.tbl] = cur | 0x3;
		cur += 4096;
		++kernelidx;
	}
	// now map the PDE at 0xffbff000
	PagingIndices pagingidx(0xffbff000);
	LOG_DEBUG("mapping phys %p to dir %u page %u aka virt %p", pPde, pagingidx.dir, pagingidx.tbl, pagingidx.address());
	pt = scratchMap<uint32_t>(pde[pagingidx.dir] & ~0x7, 2);
	pt[pagingidx.tbl] = pPde | 0x7;
	// and map each of the (present) page table entries at 0xFFC00000 onwards
	++pagingidx;
	uintptr_t logical = pagingidx.address();
	for(auto i = 0u; i < 1024; logical += 4096, ++i, ++pagingidx) {
		pt = scratchMap<uint32_t>(pde[pagingidx.dir] & ~0x7, 3);
		if (pde[i] == 0) { continue; }
		LOG_DEBUG("mapping phys %p to dir %u page %u aka virt %p", pde[i] & ~0x7, pagingidx.dir, pagingidx.tbl, pagingidx.address());
		pt[pagingidx.tbl] = pde[i] & ~0x4;
	}
	LOG_DEBUG("about to reload cr3");
	loadpagedir(pPde);
	LOG_INFO("new page tables loaded");
	mZeroPagePhysical = mapping(zeropage<>());
	LOG_INFO("zero page is loaded at virt=%p, phys=%p", zeropage<>(), mZeroPagePhysical);
}
