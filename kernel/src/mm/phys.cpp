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

#include <kernel/mm/phys.h>
#include <kernel/libc/string.h>
#include <kernel/libc/pair.h>
#include <kernel/panic/panic.h>

#define LOG_LEVEL 2
#include <kernel/log/log.h>

LOG_TAG(PMLEAK, 2);

PhysicalPageManager& PhysicalPageManager::get() {
	static PhysicalPageManager gAllocator;
	
	return gAllocator;
}

PhysicalPageManager::PhysicalPageManager() : mLastChecked(0), mTotalPages(0), mLowestPage(0), mHighestPage(0), mFreePages(0) {
	bzero((uint8_t*)&mPages[0],sizeof(mPages));
}

static constexpr uintptr_t offalignment(uintptr_t addr) {
	return addr & (PhysicalPageManager::gPageSize - 1);
}

bool PhysicalPageManager::phys_page_t::free() {
	return refcnt == 0;
}

uint8_t PhysicalPageManager::phys_page_t::incref() {
	return (++refcnt).load();
}

uint8_t PhysicalPageManager::phys_page_t::decref() {
	auto ov = refcnt.load();
	if (ov == 0) return 0;
	auto nv = ov - 1;
	while(!refcnt.cmpxchg(ov, nv)) {
		if (ov == 0) return 0;
		nv = ov - 1;
	}
	return nv;
}

static size_t index(uintptr_t addr) {
	return addr / PhysicalPageManager::gPageSize;
}

void PhysicalPageManager::addpage(uintptr_t base) {
	auto i = index(base);
	if (i < mLowestPage) mLowestPage = i;
	if (i > mHighestPage) mHighestPage = i;
	mPages[i].usable = true;
	++mTotalPages;
	++mFreePages;
	LOG_DEBUG("added a new physical page, base = 0x%p, index = %u (lowest page = %u, highest page = %u)", base, i, mLowestPage, mHighestPage);
}

void PhysicalPageManager::addpages(uintptr_t base, size_t len) {
	auto offset = offalignment(base);
	if (offset > 0) {
		// align to the next full page
		base = base - offset + gPageSize;
		len -= gPageSize - offset;
	}
	while(len >= gPageSize) {
		addpage(base);
		base += gPageSize;
		len -= gPageSize;
	}
}

void PhysicalPageManager::reserve(uintptr_t base) {
	auto idx = index(base);
	LOG_DEBUG("reserving memory page 0x%p at index %u", base, idx);

	if (mPages[idx].usable == false) {
		PANIC("trying to reserve a non-usable page");		
	}

	mPages[idx].usable = false;

	--mFreePages;
}

void PhysicalPageManager::reserve(uintptr_t begin, uintptr_t end) {
	auto offset = offalignment(begin);
	if (offset > 0) {
		// reserve the entire page
		begin -= offset;
	}
	LOG_DEBUG("asked to reserve memory in range 0x%p to 0x%p", begin, end);
	for(;begin < end;begin += gPageSize) {
		reserve(begin);
	}
}

ResultOrError<uintptr_t> PhysicalPageManager::alloc() {
	for (size_t n = 0u, idx = mLastChecked; n < gNumPages; ++n, ++idx) {
		if (idx == gNumPages) idx = 0;
		if (!mPages[idx].usable) continue;
		if (mPages[idx].free()) {
			auto rc = mPages[idx].incref();
			mLastChecked = idx;
			// this if condition is going to basically always be true - but having it there
			// allows us to leave rc in the code, even if LOG_NODEBUG is defined
			if (rc > 0) --mFreePages;
			auto base = gPageSize * idx;
			TAG_DEBUG(PMLEAK, "ALLOC 0x%p", base);
			LOG_DEBUG("physical allocator returned page at 0x%p (idx = %u) - rc = %u", base, idx, rc);
			return ResultOrError<uintptr_t>::result(base);
		}
	}

	return ResultOrError<uintptr_t>::error("out of physical pages");
}

uintptr_t PhysicalPageManager::alloc(uintptr_t base) {
	auto idx = index(base);
	if (!mPages[idx].usable) {
		PANIC("trying to allocate a non-usable page");
	}
	auto rc = mPages[idx].incref();
	if (1 == rc) --mFreePages;
	LOG_DEBUG("allocated page at 0x%p (idx = %u) - rc = %u", base, idx, rc);

	return base;
}

void PhysicalPageManager::dealloc(uintptr_t base) {
	auto idx = index(base);
	if (mPages[idx].free() || !mPages[idx].usable) {
		LOG_ERROR("trying to free physical page 0x%p (idx = %u free = %u usable = %u)", base, idx, mPages[idx].free(), mPages[idx].usable);
		PANIC("trying to deallocate a free/unusable page");
	}
	auto rc = mPages[idx].decref();
	if (0 == rc) {
		++mFreePages;
		TAG_DEBUG(PMLEAK, "DEALLOC 0x%p", base);
	}
	LOG_DEBUG("deallocated page at 0x%p (idx = %u) - rc = %u", base, idx, rc);
}

bool PhysicalPageManager::allocContiguousPages(size_t num, uintptr_t *base) {
	auto page_idx = mLowestPage;
	while(true) {
		bool all_free = true;
		for (auto u = 0u; u < num; ++u) {
			const bool usable = mPages[page_idx + u].usable;
			const bool free = mPages[page_idx + u].free();
			if (!usable || !free) {
				// start again right after the unavailable page
				page_idx = page_idx + u + 1;
				all_free = false;
				break;
			}
		}
		if (all_free) {
			for (auto u = 0u; u < num; ++u) {
				alloc( (page_idx + u) * gPageSize );
			}
			*base = page_idx * gPageSize;
			return true;
		}
		if (page_idx > mHighestPage) {
			*base = 0;
			return false;
		}
	}
}

size_t PhysicalPageManager::gettotalpages() const {
	return mTotalPages;
}
size_t PhysicalPageManager::getfreepages() const {
	return mFreePages.load();
}

size_t PhysicalPageManager::gettotalmem() const {
	return mTotalPages * gPageSize;
}
size_t PhysicalPageManager::getfreemem() const {
	return mFreePages.load() * gPageSize;
}
