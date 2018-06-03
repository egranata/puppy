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
#include <process/current.h>

#define LOG_NODEBUG
#include <log/log.h>

static constexpr uintptr_t gBootVirtualOffset = 0xC0000000;

static constexpr uint32_t gPresentBit = 0x1;
static constexpr uint32_t gWritableBit = 0x2;
static constexpr uint32_t gUserBit = 0x4;
static constexpr uint32_t gGlobalBit = 0x100;
static constexpr uint32_t gNewPhysicalPageBit = 0x200;

VirtualPageManager::DirectoryEntry* VirtualPageManager::gPageDirectory = (DirectoryEntry*)gPageDirectoryAddress;

VirtualPageManager::map_options_t::map_options_t() : _rw(true), _user(false), _clear(false), _frompmm(false), _cached(true) {}

VirtualPageManager::map_options_t::map_options_t(bool rw, bool user, bool clear, bool frompmm) : _rw(rw), _user(user), _clear(clear), _frompmm(frompmm), _cached(true), _global(false) {}

#define DEFINE_OPTION(type, name) \
type VirtualPageManager::map_options_t:: name () const { return _ ## name; } \
VirtualPageManager::map_options_t& VirtualPageManager::map_options_t:: name ( type val ) { _ ## name = val; return *this; }

DEFINE_OPTION(bool, rw);
DEFINE_OPTION(bool, user);
DEFINE_OPTION(bool, clear);
DEFINE_OPTION(bool, frompmm);
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

DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, present, 0); /** 1 == this page is present and valid */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, rw, 1); /** 1 == this page can be written to */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, user, 2); /** 1 == CPL3 code can access this page */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, writethrough, 3); /** 1 == write-through caching */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, cacheoff, 4); /** 1 == do not cache this page */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, accessed, 5); /** 1 = this page has been used */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, dirty, 6); /** 1 == this page has been written to */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, global, 8); /** 1 == global translation if CR4.PGE is 1 */
DEFINE_BOOL_FIELD(VirtualPageManager::TableEntry, frompmm, 9); /** 1 == the PhysicalPageManager provided this page */
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

uintptr_t VirtualPageManager::mapZeroPage(uintptr_t virt) {
	PagingIndices indices(virt);

	LOG_DEBUG("asked to map %p as the zero page", virt);

	TableEntry &tbl(indices.table());
	tbl.present(false);
	tbl.rw(false);
	tbl.user(false);
	tbl.page(gZeroPagePhysical);
	invtlb(virt);

	return virt;
}

uintptr_t VirtualPageManager::mapZeroPage(uintptr_t from, uintptr_t to) {
	for(auto i = from; i <= to; i += gPageSize) {
		mapZeroPage(i);
	}

	return from;
}

bool VirtualPageManager::isZeroPageAccess(uintptr_t virt) {
	auto pg = page(virt);

	PagingIndices indices(pg);
	TableEntry &tbl(indices.table());

	return (tbl.page() == gZeroPagePhysical && tbl.present() == false);
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
	tbl.cacheoff(!options.cached());
	tbl.global(options.global());
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

uintptr_t VirtualPageManager::mapAnyPhysicalPage(uintptr_t virt, const map_options_t& options) {
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
			opts->cached(!tbl.cacheoff());
			opts->global(tbl.global());
		}
		return true;
	}

	return false;
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
	LOG_DEBUG("attempting to find a page in virtual range %p - %p", low, high);
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
// the process that called createAddressSpace(). No user space memory is available.
uintptr_t VirtualPageManager::createAddressSpace() {
	LOG_DEBUG("attempting to create a new address space");
	PhysicalPageManager &phys(PhysicalPageManager::get());

	auto new_table_options = map_options_t().rw(true).user(false).clear(true);
	auto table_options = map_options_t().rw(true).user(false);

	static constexpr uint32_t newPageDirBits = gPresentBit | gWritableBit | gUserBit | gNewPhysicalPageBit;

	// allocate page directory & page tables & clear out page tables
	uintptr_t pPageDir = phys.alloc();
	auto pageDirScratch = getScratchPage(pPageDir, new_table_options);
	uint32_t *pageDir = pageDirScratch.get<uint32_t>();
	LOG_DEBUG("new page tables will be at physical %p virtual = %p", pPageDir, pageDir);
	uint32_t *kPageDir = (uint32_t*)0xffbff000;
	for (auto i = 0u; i < 768u; ++i) {
		auto pPageTbl = phys.alloc();
		LOG_DEBUG("new page table %u will be at physical %p", i, pPageTbl);
		pageDir[i] = pPageTbl | newPageDirBits;
		auto pageTblScratch = getScratchPage(pPageTbl, new_table_options);
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
	if (mKernelHeap != nullptr) {
		PANIC("cannot change kernel heap - size is non-zero");
	}
	if (size == 0) {
		PANIC("cannot set an empty kernel heap");
	}

	mKernelHeap = new (mKernelHeapMemory) KernelHeap(heap, heap + size - 1);
	LOG_INFO("kernel heap will begin at %p and span to %p", mKernelHeap->low(), mKernelHeap->high());

	// now that we have a heap - it's a good time to setup kernel regions!
	// make one mega region for the kernel image + the kernel heap (we could split them
	// but it wouldn't be very useful..)
	auto ks = kernel_start();
	auto he = mKernelHeap->high();
	addKernelRegion(ks, he);

	interval_t spr;
	if (false == findKernelRegion(gScratchPagesSize, spr)) {
		PANIC("failed to find memory for scratch pages!");
	} else {
		LOG_INFO("scratch page area will span [%p - %p]", spr.from, spr.to);
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
	LOG_DEBUG("scratch page at address %p will map physical page %p", vaddr, phys);
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

VirtualPageManager::scratch_page_t::~scratch_page_t() {
	if (operator bool()) {
		VirtualPageManager::get().unmap(address);
	}
	address = 0;
}

void VirtualPageManager::addKernelRegion(uintptr_t low, uintptr_t high) {
	LOG_INFO("adding a kernel region [%p - %p]", low, high);
	low -= gBootVirtualOffset;
	high -= gBootVirtualOffset;
	mKernelRegions.add({low, high});
}

bool VirtualPageManager::findKernelRegion(size_t size, interval_t& rgn) {
	if (offset(size) > 0) {
		size = size - offset(size) + gPageSize;
	}

	if (mKernelRegions.findFree(size, rgn)) {
		rgn.from += gBootVirtualOffset;
		rgn.to += gBootVirtualOffset;
		return true;
	}

	return false;
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
	uintptr_t pPde = phys.alloc();
	uint32_t *pde = scratchMap<uint32_t>(pPde, 0);
	LOG_DEBUG("pde physical address %p, virrtual address %p", pPde, pde);
	
	PagingIndices kernelidx(kernel_start());
	LOG_DEBUG("kernel mapping starts at dir %u page %u", kernelidx.dir, kernelidx.tbl);

	// map all page directories as present / writable / user - we will enforce actual permissions at the page table level	
	static constexpr uint32_t pagedirAttrib = gPresentBit | gWritableBit | gUserBit;

	// map the kernel as global
	static constexpr uint32_t kerneltblAttrib = gPresentBit | gWritableBit | gGlobalBit;

	uint8_t *pq = nullptr;
	for (auto i = 0u; i < 1024; ++i) {
		auto pa = phys.alloc();
		pde[i] = pa | pagedirAttrib;
		pq = scratchMap<uint8_t>(pa, 4);
		bzero(pq, PhysicalPageManager::gPageSize);
	}
	// all kernel memory is shared
	uintptr_t cur = kernel_start() - gBootVirtualOffset;
	uintptr_t end = kernel_end() - gBootVirtualOffset;
	uint32_t *pt = nullptr;
	while(cur < end) {
		pt = scratchMap<uint32_t>(pde[kernelidx.dir] & ~pagedirAttrib, 1);
		LOG_DEBUG("mapping phys %p to dir %u page %u aka virt %p", cur, kernelidx.dir, kernelidx.tbl, kernelidx.address());
		pt[kernelidx.tbl] = cur | kerneltblAttrib;
		cur += gPageSize;
		++kernelidx;
	}
	// now map the PDE at 0xffbff000
	PagingIndices pagingidx(gPageDirectoryAddress);
	LOG_DEBUG("mapping phys %p to dir %u page %u aka virt %p", pPde, pagingidx.dir, pagingidx.tbl, pagingidx.address());
	pt = scratchMap<uint32_t>(pde[pagingidx.dir] & ~pagedirAttrib, 2);
	pt[pagingidx.tbl] = pPde | pagedirAttrib;
	// and map each of the (present) page table entries at 0xFFC00000 onwards
	++pagingidx;
	uintptr_t logical = pagingidx.address();
	for(auto i = 0u; i < 1024; logical += gPageSize, ++i, ++pagingidx) {
		pt = scratchMap<uint32_t>(pde[pagingidx.dir] & ~pagedirAttrib, 3);
		if (pde[i] == 0) { continue; }
		LOG_DEBUG("mapping phys %p to dir %u page %u aka virt %p", pde[i] & ~pagedirAttrib, pagingidx.dir, pagingidx.tbl, pagingidx.address());
		pt[pagingidx.tbl] = pde[i] & ~gUserBit;
	}
	LOG_DEBUG("about to reload cr3");
	loadpagedir(pPde);
	LOG_INFO("new page tables loaded");
}
