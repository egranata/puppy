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

#include <kernel/sys/globals.h>
#include <kernel/i386/primitives.h>
#include <kernel/mm/virt.h>
#include <kernel/libc/string.h>
#include <kernel/libc/pair.h>
#include <kernel/panic/panic.h>
#include <kernel/libc/memory.h>
#include <kernel/process/current.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/bitmask.h>

#define LOG_LEVEL 2
#include <kernel/log/log.h>

LOG_TAG(MEMLEAK, 2);

static constexpr uint32_t gPresentBit = 0x1;
static constexpr uint32_t gWritableBit = 0x2;
static constexpr uint32_t gUserBit = 0x4;
static constexpr uint32_t gGlobalBit = 0x100;
static constexpr uint32_t gNewPhysicalPageBit = 0x200;

static constexpr uint32_t gFlagsBitmask = bitmask<0,11>();
static_assert(gFlagsBitmask == 0xFFF);

VirtualPageManager::DirectoryEntry* VirtualPageManager::gPageDirectory = (DirectoryEntry*)gPageDirectoryAddress;

VirtualPageManager::map_options_t::map_options_t() : _rw(true), _user(false), _clear(false), _frompmm(false), _cow(false), _cached(true), _global(false) {}

VirtualPageManager::map_options_t::map_options_t(bool rw, bool user, bool clear, bool frompmm) : _rw(rw), _user(user), _clear(clear), _frompmm(frompmm), _cow(false), _cached(true), _global(false) {}

#define DEFINE_OPTION(type, name) \
type VirtualPageManager::map_options_t:: name () const { return _ ## name; } \
VirtualPageManager::map_options_t& VirtualPageManager::map_options_t:: name ( type val ) { _ ## name = val; return *this; }

DEFINE_OPTION(bool, rw);
DEFINE_OPTION(bool, user);
DEFINE_OPTION(bool, clear);
DEFINE_OPTION(bool, frompmm);
DEFINE_OPTION(bool, cow);
DEFINE_OPTION(bool, cached);
DEFINE_OPTION(bool, global);

#undef DEFINE_OPTION

VirtualPageManager::map_options_t VirtualPageManager::map_options_t::kernel() {
	return map_options_t(true, false, false, false);
}

VirtualPageManager::map_options_t VirtualPageManager::map_options_t::userspace() {
	return map_options_t(true, true, false, false);
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
	if (value & gFlagsBitmask) {
		LOG_ERROR("value=0x%p is not page aligned", value);
		PANIC("trying to set page directory entry to non-page aligned");
	}
    mValue &= gFlagsBitmask;
	mValue |= value;
}

DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, present, 0);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, rw, 1);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, user, 2);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, writethrough, 3);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, cacheoff, 4);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, accessed, 5);
DEFINE_BOOL_FIELD(VirtualPageManager::DirectoryEntry, fourmb, 7);

VirtualPageManager::TableEntry::TableEntry() : mValue(0) {}

VirtualPageManager::TableEntry::operator uint32_t() const {
	return mValue;
}

DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, present, 0); /** 1 == this page is present and valid */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, rw, 1); /** 1 == this page can be written to */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, user, 2); /** 1 == CPL3 code can access this page */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, writethrough, 3); /** 1 == write-through caching */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, cacheoff, 4); /** 1 == do not cache this page */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, accessed, 5); /** 1 = this page has been used */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, dirty, 6); /** 1 == this page has been written to */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, global, 8); /** 1 == global translation if CR4.PGE is 1 */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, frompmm, 9); /** 1 == the PhysicalPageManager provided this page */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, cow, 10); /** 1 == create a copy of this page on a "write" page fault (present == 1) */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, zpmap, 10); /** 1 == this is a zero page mapped (present == 0) */
// bit 11 is available for us to define as needed
// DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, bit11, 11)

uintptr_t VirtualPageManager::TableEntry::page() {
	return mValue & 0xFFFFF000;
}

void VirtualPageManager::TableEntry::page(uintptr_t value) {
	if (value & gFlagsBitmask) {
		LOG_ERROR("value=0x%p is not page aligned", value);
		PANIC("trying to set page table entry to non-page aligned");
	}
    mValue &= gFlagsBitmask;
	mValue |= value;
}

#undef DEFINE_BOOL_FIELD

VirtualPageManager::TableEntry* VirtualPageManager::gPageTable(size_t table) {
	auto tbl = (TableEntry*)(gPageTableAddress + PhysicalPageManager::gPageSize * table);
	// LOG_DEBUG("WOMAN gPageTableAddress = 0x%p, table = %u, tbl = 0x%p", gPageTableAddress, table, tbl);
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
	return addr >= gKernelBase;
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
	uintptr_t oldmapping = addr_bootpagetable<uintptr_t*>()[page];
	// idempotent and with no unnecessary slowdowns
	if (newmapping != oldmapping) {
		addr_bootpagetable<uintptr_t*>()[page] = newmapping;
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

uintptr_t VirtualPageManager::mapZeroPage(uintptr_t virt, const map_options_t& opts) {
	PagingIndices indices(virt);

	LOG_DEBUG("asked to map 0x%p as the zero page", virt);

	TableEntry &tbl(indices.table());
	tbl.present(false);
	tbl.rw(opts.rw());
	tbl.user(opts.user());
	tbl.page(gZeroPagePhysical);
	tbl.zpmap(true);
	tbl.frompmm(false); // do not assume physical memory backing this page
	invtlb(virt);

	return virt;
}

uintptr_t VirtualPageManager::mapZeroPage(uintptr_t from, uintptr_t to, const map_options_t& opts) {
	for(auto i = from; i <= to; i += gPageSize) {
		mapZeroPage(i, opts);
	}

	return from;
}

bool VirtualPageManager::isZeroPageAccess(uintptr_t virt) {
	auto pg = page(virt);

	PagingIndices indices(pg);
	TableEntry &tbl(indices.table());

	return (tbl.page() == gZeroPagePhysical && tbl.present() == false);
}

bool VirtualPageManager::isCOWAccess(unsigned int errcode, uintptr_t virt) {
	auto pg = page(virt);

	PagingIndices indices(pg);
	TableEntry &tbl(indices.table());

	if (tbl.present() == true && tbl.rw() == false && tbl.cow() == true) {
		// 0x3 == "protection violation & write access"
		if (0x3 == (errcode & 0x3)) {
			return true;
		}
	}

	return false;
}

kernel_result_t<uintptr_t> VirtualPageManager::clonePage(uintptr_t virt, const map_options_t& options) {
	auto pg = page(virt);

	auto&& physall(PhysicalPageManager::get());
	auto phys_result = physall.alloc();
	uintptr_t phys;
	if (phys_result.result(&phys)) {
		scratch_page_t sp = getScratchPage(phys);
		memcopy((uint8_t*)pg, sp.get<uint8_t>(), gPageSize);
	} else {
		return kernel_failure<uintptr_t>(kernel_status_t::OUT_OF_MEMORY);
	}

	// new page is from PMM (we just allocated it above...)
	// it is not COW (because it was COW before and now it's its own copy)
	// and it's clearly writable (or else it wouldn't have been COW)
	// all other options, leave them alone
	auto opts = map_options_t(options).frompmm(true).cow(false).rw(true);

	unmap(pg);
	map(phys, pg, opts);

	return kernel_success(phys);
}

uintptr_t VirtualPageManager::map(uintptr_t phys, uintptr_t virt, const map_options_t& options) {
	PagingIndices indices(virt);
	
	LOG_DEBUG("asked to map phys 0x%p to virt 0x%p; that will be page dir entry %u, and page table entry %u", phys, virt, indices.dir, indices.tbl);

	if (options.user() & !options.clear()) {
		LOG_WARNING("page mapping virt=0x%p phys=0x%p shall be visible to userspace, but is not being zeroed out", virt, phys);
	}

	if (options.cow() && options.rw()) {
		LOG_WARNING("page mapping virt=0x%p phys=0x%p is COW but writable - will never be copied!", virt, phys);
	}

	const bool isuserspace = !iskernel(virt);

	// if we are asked to clear the page, first map it R/W, so the kernel can clear it
	// then clear it, then protect it readonly
	bool mustprotect = options.clear() && !options.rw();

	TableEntry &tbl(indices.table());
	tbl.present(true);
	if (mustprotect) {
		tbl.rw(true);
	} else {
		tbl.rw(options.rw());
	}
	tbl.user(options.user());
	tbl.cacheoff(!options.cached());
	tbl.global(options.global());
	tbl.frompmm(options.frompmm());
	tbl.cow(options.cow());
	tbl.page(phys);
	invtlb(virt);

	if (options.clear()) {
		auto pageptr = (uint8_t*)virt;
		bzero(pageptr, gPageSize);

		if (mustprotect) {
			tbl.rw(false);
			invtlb(virt);
		}
	}

	if (gCurrentProcess && isuserspace) {
		gCurrentProcess->memstats.allocated += gPageSize;
	}

	if (gCurrentProcess) TAG_DEBUG(MEMLEAK, "MEMLEAK: process %u allocated page virt=0x%x phys=0x%x", gCurrentProcess->pid, virt, phys);

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

uintptr_t VirtualPageManager::mapAnyPhysicalPage(uintptr_t virt, const map_options_t& options) {
	if (mapped(virt)) {
		LOG_DEBUG("virtual address 0x%p already mapped", virt);
	} else {
		auto&& physall(PhysicalPageManager::get());
		auto phys_result = physall.alloc();
		auto phys = phys_result.result();
		auto opts = map_options_t(options).frompmm(true);
		map(phys, virt, opts);
	}

	return virt;
}

void VirtualPageManager::unmap(uintptr_t virt) {
	PagingIndices indices(virt);
	
	LOG_DEBUG("asked to unmap virt 0x%p; that will be page dir entry %u, and page table entry %u", virt, indices.dir, indices.tbl);

	TableEntry &tbl(indices.table());

	const bool wasthere = tbl.present();
	const bool isuserspace = !iskernel(virt);
	const bool isfrompmm = tbl.frompmm();

	if (isfrompmm && !wasthere) {
		LOG_ERROR("virtual page 0x%p is marked not present but frompmm", virt);
		PANIC("non-present page cannot be backed by physical memory");
	}

	tbl.present(false);
	tbl.zpmap(false); // make sure we don't think this is a zeropage mapping
	tbl.frompmm(false); // do not assume this page is bound to any physical storage
	invtlb(virt);

	uintptr_t phys = 0;

	if (isfrompmm) {
		auto& pmm(PhysicalPageManager::get());
		phys = tbl.page();
		pmm.dealloc(tbl.page());
	}
	tbl.page(0);

	if (gCurrentProcess && wasthere&& isuserspace) {
		gCurrentProcess->memstats.allocated -= gPageSize;
	}

	if (gCurrentProcess) TAG_DEBUG(MEMLEAK, "MEMLEAK: process %u freed page virt=0x%x phys=0x%x", gCurrentProcess->pid, virt, phys);
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
			opts->cached(!tbl.cacheoff());
			opts->global(tbl.global());
			opts->cow(tbl.cow());
		}
		return true;
	}

	return false;
}

bool VirtualPageManager::zeroPageMapped(uintptr_t virt, map_options_t* opts) {
	uintptr_t b = page(virt);
	PagingIndices indices(b);
	TableEntry &tbl(indices.table());

	if (!tbl.present() && tbl.zpmap()) {
		if (opts) {
			opts->rw(tbl.rw());
			opts->user(tbl.user());
			opts->frompmm(tbl.frompmm());
			opts->cached(!tbl.cacheoff());
			opts->global(tbl.global());
			opts->cow(tbl.cow());
		}
		return true;
	}

	return false;
}


uintptr_t VirtualPageManager::newoptions(uintptr_t virt, const map_options_t& options) {
	auto pg = page(virt);

	PagingIndices indices(pg);
	TableEntry &tbl(indices.table());

	if (tbl.present() || tbl.zpmap()) {
		tbl.rw(options.rw());
		tbl.user(options.user());
		tbl.cacheoff(!options.cached());
		tbl.global(options.global());
		tbl.frompmm(options.frompmm());
		tbl.cow(options.cow());
		invtlb(pg);
		return virt;
	}

	return gPageSize - 1;
}

uintptr_t VirtualPageManager::markCOW(uintptr_t virt) {
	map_options_t opts;
	if (mapped(virt, &opts)) {
		opts.rw(false);
		opts.cow(true);
		return newoptions(virt, opts);
	}

	return gPageSize - 1;
}

uintptr_t VirtualPageManager::mapPageWithinRange(uintptr_t low, uintptr_t high, const map_options_t& options) {
	auto pg = findPageWithinRange(low, high);
	if (pg & 1) {
		return pg;
	} else {
		return mapAnyPhysicalPage(pg, options);
	}
}

uintptr_t VirtualPageManager::findPageWithinRange(uintptr_t low, uintptr_t high) {
	LOG_DEBUG("attempting to find a page in virtual range 0x%p - 0x%p", low, high);
	auto ilow = PagingIndices(low);
	auto ihigh = PagingIndices(high);
	while (ilow < ihigh) {
		if(mapped(ilow.address())) {
			++ilow;
			continue;
		} else {
			return ilow.address();
		}
	}

	// return something non-page-aligned if we failed
	return (gPageSize - 1);
}

// returns the physical address of the page directory suitable for
// launching a new process. The kernel is mapped in, sharing mapping with
// the process that called createAddressSpace(). The RW portions of memory of
// this process are COW-marked in both this and the other process.
uintptr_t VirtualPageManager::cloneAddressSpace() {
	LOG_DEBUG("attempting to clone this address space");

	PhysicalPageManager &phys(PhysicalPageManager::get());

	auto dir_options = map_options_t().rw(true).user(false);
	auto table_options = map_options_t().rw(true).user(false).clear(true);

	uintptr_t newCR3 = createAddressSpace();
	auto pageDirScratch = getScratchPage(newCR3, dir_options);
	uint32_t *pageDir = pageDirScratch.get<uint32_t>();

	PagingIndices indices(0x0);
	for (auto i = 0u; i < 768u; ++i) {
		auto pPageTbl = pageDir[i] & ~gFlagsBitmask;
		auto pageTblScratch = getScratchPage(pPageTbl, table_options);
		uint32_t *pageTbl = pageTblScratch.get<uint32_t>();

		for (auto j = 0u; j < 1024u; ++j, ++indices) {
			TableEntry tbl(indices.table()); // NB: this is making a *copy* of the original TableEntry
			if (!tbl.present()) continue;
			if (tbl.rw()) {
				// only mark RW pages as COW
				markCOW(indices.address());
				tbl.cow(true);
				tbl.rw(false);
			}
			LOG_INFO("cloning indices 0x%p (%u, %u) into dir = %u, tbl = %u - writing 0x%x", indices.address(), indices.dir, indices.tbl, i, j, (uint32_t)tbl);
			pageTbl[j] = (uint32_t)tbl;
			if (tbl.frompmm()) {
				// add a reference to the page so we don't let go of it
				phys.alloc(tbl.page());
			}
		}
	}

	return newCR3;
}

// returns the physical address of the page directory suitable for
// launching a new process. The kernel is mapped in, sharing mapping with
// the process that called createAddressSpace(). No user space memory is available.
uintptr_t VirtualPageManager::createAddressSpace() {
	LOG_DEBUG("attempting to create a new address space");
	PhysicalPageManager &phys(PhysicalPageManager::get());

	auto new_table_options = map_options_t().rw(true).user(false).clear(true);
	auto table_options = map_options_t().rw(true).user(false);

	static constexpr uint32_t newPageDirBits = gPresentBit | gWritableBit | gUserBit | gNewPhysicalPageBit;

	// allocate page directory & page tables & clear out page tables
	auto pPageDir_result = phys.alloc();
	uintptr_t pPageDir = pPageDir_result.result();
	auto pageDirScratch = getScratchPage(pPageDir, new_table_options);
	uint32_t *pageDir = pageDirScratch.get<uint32_t>();
	LOG_DEBUG("new page tables will be at physical 0x%p virtual = 0x%p", pPageDir, pageDir);
	uint32_t *kPageDir = (uint32_t*)gPageDirectoryAddress;
	for (auto i = 0u; i < 768u; ++i) {
		auto pPageTbl = phys.alloc().result();
		LOG_DEBUG("new page table %u will be at physical 0x%p", i, pPageTbl);
		pageDir[i] = pPageTbl | newPageDirBits;
		auto pageTblScratch = getScratchPage(pPageTbl, new_table_options);
	}

	// kernel memory just points to our original allocation
	for (auto i = 768u; i < 1022u; ++i) {
		LOG_DEBUG("new page table %u will be at physical 0x%p", i, kPageDir[i]);
		pageDir[i] = kPageDir[i];
	}

	// but allocate for pages 1022 and 1023 (i.e. page tables)
	for (auto i = 1022u; i < 1024u; ++i) {
		auto pPageTbl = phys.alloc().result();;
		LOG_DEBUG("new page table %u will be at physical 0x%p", i, pPageTbl);
		pageDir[i] = pPageTbl | newPageDirBits;
		auto pageTblScratch = getScratchPage(pPageTbl, new_table_options);		
	}

	{
		// now fill in page directory entry
		auto pageDir1022 = pageDir[1022] & ~newPageDirBits;
		auto pageTblScratch = getScratchPage(pageDir1022, table_options);
		auto pageTbl = pageTblScratch.get<uint32_t>();
		pageTbl[1023] = pPageDir | gPresentBit | gWritableBit | gUserBit;
	}

	{
		// and all of the page table pointers:
		// 0 thru 767 must point to the pages we just allocated;
		// 768 thru 1021 must point to the kernel pages;
		// 1022 and 1023 must point to what we allocated	
		auto pageDir1023 = pageDir[1023] & ~newPageDirBits;
		auto pageTblScratch = getScratchPage(pageDir1023, table_options);
		auto pageTbl = pageTblScratch.get<uint32_t>();		

		for (auto i = 0u; i < 768u; ++i) {
			pageTbl[i] = pageDir[i] & ~(gUserBit | gNewPhysicalPageBit);
		}
		for (auto i = 768u; i < 1022u; ++i) {
			pageTbl[i] = kPageDir[i];
		}
		for (auto i = 1022u; i < 1024u; ++i) {
			pageTbl[i] = pageDir[i] & ~(gUserBit | gNewPhysicalPageBit);
		}
	}

	return pPageDir;
}

// deallocate all memory that this process ever acquired
// this leaves kernel memory in place, so that the rest of the exit()
// can occur, and also because kernel memory is shared anyway
void VirtualPageManager::cleanAddressSpace() {
	PhysicalPageManager &phys(PhysicalPageManager::get());

	auto freepages = phys.getfreepages();

	for (auto i = 0u; i < 1024; ++i) {
		if ((i >= 768) && (i <= 1021)) continue;
		auto tbl = gPageTable(i);
		LOG_DEBUG("page table %u is at 0x%p", i, tbl);
		for (auto j = 0u; j < 1024u; ++j) {
			if (tbl[j].present() && tbl[j].frompmm()) {
				auto ptr = tbl[j].page();
				LOG_DEBUG("freeing page %u at 0x%p", j, ptr);
				LOG_WARNING("vm page tbl[%u] page[%u] at 0x%p being unmapped outside of regions", i, j, ptr);
				phys.dealloc(ptr);
				TAG_DEBUG(MEMLEAK, "MEMLEAK: process %u freed page virt=0x%x phys=0x%x", gCurrentProcess->pid, 0x0, ptr);
			}
		}

		auto m = mapping((uintptr_t)tbl);
		LOG_DEBUG("and now freeing entire table at 0x%p", m);
		phys.dealloc(m);
	}

	freepages = phys.getfreepages() - freepages;
	LOG_INFO("freed %u pages, worth %u bytes", freepages, PhysicalPageManager::gPageSize * freepages);
}

VirtualPageManager::KernelHeap::KernelHeap(uintptr_t low, uintptr_t high) : Heap(low, high), mVm(VirtualPageManager::get()), mPhys(PhysicalPageManager::get()) {}

uintptr_t VirtualPageManager::KernelHeap::oneblock() {
	auto opts = VirtualPageManager::map_options_t().rw(true).user(false).clear(true);
	auto physalloc = mPhys.alloc();
	auto block = physalloc.result();
	mVm.map(block, current(), opts);
	return current();
}

void VirtualPageManager::setheap(uintptr_t heap, size_t size) {
	if (mKernelHeap != nullptr) {
		PANIC("cannot change kernel heap - size is non-zero");
	}
	if (size == 0) {
		PANIC("cannot set an empty kernel heap");
	}

	mKernelHeap = new (mKernelHeapMemory) KernelHeap(heap, heap + size - 1);
	LOG_INFO("kernel heap will begin at 0x%p and span to 0x%p", mKernelHeap->low(), mKernelHeap->high());

	// now that we have a heap - it's a good time to setup kernel regions!
	// make one mega region for the kernel image + the kernel heap (we could split them
	// but it wouldn't be very useful..)
	auto ks = addr_kernel_start();
	auto he = mKernelHeap->high();
	addKernelRegion(ks, he);

	interval_t spr;
	if (false == findKernelRegion(gScratchPagesSize, spr)) {
		PANIC("failed to find memory for scratch pages!");
	} else {
		LOG_INFO("scratch page area will span [0x%p - 0x%p]", spr.from, spr.to);
		addKernelRegion(mScratchPageInfo.low = spr.from, mScratchPageInfo.high = spr.to);
		// mapZeroPage(spr.from, spr.to);
	}
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

VirtualPageManager::scratch_page_t VirtualPageManager::getScratchPage(const map_options_t& op) {
	auto options = map_options_t(op).clear(true); // always clear scratch pages

	return scratch_page_t(mapPageWithinRange(mScratchPageInfo.low, mScratchPageInfo.high, options), true);
}

VirtualPageManager::scratch_page_t VirtualPageManager::getScratchPage(uintptr_t phys, const map_options_t& op) {
	auto vaddr = findPageWithinRange(mScratchPageInfo.low, mScratchPageInfo.high);
	LOG_DEBUG("scratch page at address 0x%p will map physical page 0x%p", vaddr, phys);
	map(phys, vaddr, op);
	return scratch_page_t(vaddr, false);
}

VirtualPageManager::scratch_page_t::scratch_page_t(uintptr_t a, bool o) : address(a), owned(o) {}

VirtualPageManager::scratch_page_t::scratch_page_t(scratch_page_t&& p) : address(p.address), owned(p.owned) {
	p.address = 0;
}

VirtualPageManager::scratch_page_t::operator bool() {
	return (address != 0) && (address & 1) == 0; // is it page aligned and non-zero?
}

VirtualPageManager::scratch_page_t::operator uintptr_t() {
	return address;
}

uintptr_t VirtualPageManager::scratch_page_t::reset() {
	auto a = address;
	return (address = 0), a;
}

VirtualPageManager::scratch_page_t::~scratch_page_t() {
	if (operator bool()) {
		VirtualPageManager::get().unmap(address);
	}
	address = 0;
}

void VirtualPageManager::addKernelRegion(uintptr_t low, uintptr_t high) {
	LOG_INFO("adding a kernel region [0x%p - 0x%p]", low, high);
	low -= gKernelBase;
	high -= gKernelBase;
	mKernelRegions.add({low, high});
}

bool VirtualPageManager::findKernelRegion(size_t size, interval_t& rgn) {
	if (offset(size) > 0) {
		size = size - offset(size) + gPageSize;
	}

	if (mKernelRegions.findFree(size, rgn)) {
		rgn.from += gKernelBase;
		rgn.to += gKernelBase;
		return true;
	}

	return false;
}

void VirtualPageManager::delKernelRegion(interval_t rgn) {
	rgn.from -= gKernelBase;
	rgn.to -= gKernelBase;

	mKernelRegions.del(rgn);
}

bool VirtualPageManager::isWithinKernelRegion(uintptr_t address, interval_t *rgn) {
	address -= gKernelBase;
	return mKernelRegions.contains(address, rgn);
}

uintptr_t VirtualPageManager::mapOtherProcessPage(process_t* other, uintptr_t otherVirt, uintptr_t selfVirt, const map_options_t& op) {
	auto opts = map_options_t(op);
	PagingIndices indices(otherVirt);

	// get a scratch page - unmap the physical memory it points to, and remap it to the other process' CR3
	scratch_page_t other_pd = getScratchPage(map_options_t::kernel());
	unmap(other_pd.operator uintptr_t());
	map(other->tss.cr3, other_pd.operator uintptr_t(), map_options_t::kernel());

	uint32_t *otherPageDir = other_pd.get<uint32_t>();
	auto otherPageDirEntry = otherPageDir[indices.dir];
	if (0 == (otherPageDirEntry & 1)) {
		// this was not mapped in the other process - return
		return 0x0;
	}
	otherPageDirEntry = page(otherPageDirEntry);

	// unmap the directory - map the table
	unmap(other_pd.operator uintptr_t());
	map(otherPageDirEntry, other_pd.operator uintptr_t(), map_options_t::kernel());

	uint32_t *otherPageTbl = other_pd.get<uint32_t>();
	auto otherPageTblEntry = otherPageTbl[indices.tbl];
	if (0 == (otherPageTblEntry & 1)) {
		// this was not mapped in the other process - return
		return 0x0;
	}
	const bool isFromPMM = otherPageTblEntry & 0x200;
	otherPageTblEntry = page(otherPageTblEntry);
	if (isFromPMM) {
		PhysicalPageManager::get().alloc(otherPageTblEntry);
		opts.frompmm(true);
	} else {
		opts.frompmm(false);
	}
	map(otherPageTblEntry, selfVirt, opts);
	return selfVirt;
}

VirtualPageManager::VirtualPageManager() : mKernelHeap(nullptr), mScratchPageInfo({low : 0, high : 0}) {
	PhysicalPageManager &phys(PhysicalPageManager::get());
	
	bzero(&mKernelHeapMemory[0], sizeof(mKernelHeapMemory));

	LOG_INFO("setting up sane page directories");
	uintptr_t pPde = phys.alloc().result();
	uint32_t *pde = scratchMap<uint32_t>(pPde, 0);
	LOG_DEBUG("pde physical address 0x%p, virrtual address 0x%p", pPde, pde);
	
	PagingIndices kernelidx(addr_kernel_start());
	LOG_DEBUG("kernel mapping starts at dir %u page %u", kernelidx.dir, kernelidx.tbl);

	// map all page directories as present / writable / user - we will enforce actual permissions at the page table level	
	static constexpr uint32_t pagedirAttrib = gPresentBit | gWritableBit | gUserBit;

	// map the kernel as global
	static constexpr uint32_t kerneltblAttrib = gPresentBit | gWritableBit | gGlobalBit;

	uint8_t *pq = nullptr;
	for (auto i = 0u; i < 1024; ++i) {
		auto pa = phys.alloc().result();
		pde[i] = pa | pagedirAttrib;
		pq = scratchMap<uint8_t>(pa, 4);
		bzero(pq, PhysicalPageManager::gPageSize);
	}
	// all kernel memory is shared
	uintptr_t cur = addr_kernel_start() - gKernelBase;
	uintptr_t end = addr_kernel_end() - gKernelBase;
	uint32_t *pt = nullptr;
	while(cur < end) {
		pt = scratchMap<uint32_t>(pde[kernelidx.dir] & ~pagedirAttrib, 1);
		LOG_DEBUG("mapping phys 0x%p to dir %u page %u aka virt 0x%p", cur, kernelidx.dir, kernelidx.tbl, kernelidx.address());
		pt[kernelidx.tbl] = cur | kerneltblAttrib;
		cur += gPageSize;
		++kernelidx;
	}
	// now map the PDE at 0xffbff000
	PagingIndices pagingidx(gPageDirectoryAddress);
	LOG_DEBUG("mapping phys 0x%p to dir %u page %u aka virt 0x%p", pPde, pagingidx.dir, pagingidx.tbl, pagingidx.address());
	pt = scratchMap<uint32_t>(pde[pagingidx.dir] & ~pagedirAttrib, 2);
	pt[pagingidx.tbl] = pPde | pagedirAttrib;
	// and map each of the (present) page table entries at 0xFFC00000 onwards
	++pagingidx;
	uintptr_t logical = pagingidx.address();
	for(auto i = 0u; i < 1024; logical += gPageSize, ++i, ++pagingidx) {
		pt = scratchMap<uint32_t>(pde[pagingidx.dir] & ~pagedirAttrib, 3);
		if (pde[i] == 0) { continue; }
		LOG_DEBUG("mapping phys 0x%p to dir %u page %u aka virt 0x%p", pde[i] & ~pagedirAttrib, pagingidx.dir, pagingidx.tbl, pagingidx.address());
		pt[pagingidx.tbl] = pde[i] & ~gUserBit;
	}
	LOG_DEBUG("about to reload cr3");
	loadpagedir(pPde);
	LOG_INFO("new page tables loaded");
}
