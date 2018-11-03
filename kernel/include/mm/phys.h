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

#ifndef MM_PHYS
#define MM_PHYS

#include <kernel/sys/stdint.h>
#include <kernel/libc/atomic.h>
#include <kernel/libc/bytesizes.h>
#include <kernel/sys/nocopy.h>
#include <kernel/libc/error.h>

class PhysicalPageManager : NOCOPY {
public:
	static constexpr size_t gPageSize = 4_KB;
	static constexpr size_t gNumPages = 4_GB / gPageSize;
	static PhysicalPageManager& get();
	
	void addpage(uintptr_t base);
	void addpages(uintptr_t base, size_t len);
	
	void reserve(uintptr_t base);
	void reserve(uintptr_t begin, uintptr_t end);

	ResultOrError<uintptr_t> alloc();
	uintptr_t alloc(uintptr_t base);
	void dealloc(uintptr_t base);

	// allocate "num" contiguous pages of physical memory; return true if found and
	// *base == initial allocated byte (so it's [base, base+num*gPageSize-1])
	// if false, nothing was allocated
	bool allocContiguousPages(size_t num, uintptr_t *base);

	size_t gettotalpages() const;
	size_t getfreepages() const;
	
	size_t gettotalmem() const;
	size_t getfreemem() const;
	
private:
	struct phys_page_t {
		typedef uint8_t ref;
		bool usable;
		atomic<ref> refcnt;

		bool free();
		ref incref();
		ref decref();
	};
	PhysicalPageManager();
	phys_page_t mPages[gNumPages];
	size_t mLastChecked;
	size_t mTotalPages;
	size_t mLowestPage; // lowest page index that exists
	size_t mHighestPage; // highest page index that exists
	atomic<size_t> mFreePages;
};

#endif
