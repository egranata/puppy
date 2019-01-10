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

#ifndef MM_VIRT
#define MM_VIRT

#include <kernel/sys/stdint.h>
#include <kernel/mm/phys.h>
#include <kernel/libc/bytesizes.h>
#include <kernel/mm/heap.h>
#include <kernel/sys/nocopy.h>
#include <kernel/libc/intervals.h>

class PhysicalPageManager;
struct process_t;

#define DECLARE_FIELD(name, type) type name (); void name ( type )

class VirtualPageManager : NOCOPY {
public:
	static constexpr uintptr_t gKernelBase = 0xC0000000;

	struct map_options_t {
		#define DECLARE_OPTION(type, name) type name () const; map_options_t& name ( type ); type _ ## name

		DECLARE_OPTION(bool, rw);
		DECLARE_OPTION(bool, user);
		DECLARE_OPTION(bool, clear);
		DECLARE_OPTION(bool, frompmm);
		DECLARE_OPTION(bool, cow);
		DECLARE_OPTION(bool, cached);
		DECLARE_OPTION(bool, global);

		#undef DECLARE_OPTION

		map_options_t();

		static map_options_t kernel();
		static map_options_t userspace();

	private:
		map_options_t(bool rw, bool user, bool clear, bool frompmm);
	};

	struct DirectoryEntry {
		uint32_t mValue;
		
		DirectoryEntry();
		
		DECLARE_FIELD(present, bool);
		DECLARE_FIELD(rw, bool);
		DECLARE_FIELD(user, bool);
		DECLARE_FIELD(writethrough, bool);
		DECLARE_FIELD(cacheoff, bool);
		DECLARE_FIELD(accessed, bool);
		DECLARE_FIELD(fourmb, bool);
		DECLARE_FIELD(table, uintptr_t);
	};
	struct TableEntry {
		uint32_t mValue;
		
		TableEntry();

		explicit operator uint32_t() const;
		
		DECLARE_FIELD(present, bool);
		DECLARE_FIELD(rw, bool);
		DECLARE_FIELD(user, bool);
		DECLARE_FIELD(writethrough, bool);
		DECLARE_FIELD(cacheoff, bool);
		DECLARE_FIELD(accessed, bool);
		DECLARE_FIELD(dirty, bool);
		DECLARE_FIELD(global, bool);
		DECLARE_FIELD(frompmm, bool);
		DECLARE_FIELD(cow, bool);
		DECLARE_FIELD(zpmap, bool);
		DECLARE_FIELD(page, uintptr_t);
	};
	
#undef DECLARE_FIELD
	
	static constexpr size_t gKernelHeapSize = 128_MB;
	static constexpr size_t gScratchPagesSize = 8_MB + 4_KB;

	static constexpr uintptr_t gZeroPagePhysical = 0x0;

	static constexpr uintptr_t gPageDirectoryAddress = 0xffbff000;
	static constexpr uintptr_t gPageTableAddress = gPageDirectoryAddress + PhysicalPageManager::gPageSize;
	static DirectoryEntry* gPageDirectory;
	static TableEntry* gPageTable(size_t table);
	static TableEntry& gPageTable(size_t dir, size_t tbl);

	static uintptr_t page(uintptr_t addr);
	static uintptr_t offset(uintptr_t addr);

	static bool iskernel(uintptr_t addr);

	static constexpr size_t gPageSize = PhysicalPageManager::gPageSize;
	
	static VirtualPageManager& get();

	uintptr_t mapZeroPage(uintptr_t virt, const map_options_t&);
	uintptr_t mapZeroPage(uintptr_t from, uintptr_t to, const map_options_t&);
	bool isZeroPageAccess(uintptr_t virt);

	bool isCOWAccess(unsigned int errcode, uintptr_t virt);

	// returns the new *physical* address
	uintptr_t clonePage(uintptr_t virt, const map_options_t&);

	uintptr_t map(uintptr_t phys, uintptr_t virt, const map_options_t&);

	uintptr_t mapAnyPhysicalPage(uintptr_t virt, const map_options_t& = map_options_t());

	uintptr_t maprange(uintptr_t physlow, uintptr_t physhigh, uintptr_t virt, const map_options_t& = map_options_t());

	uintptr_t mapPageWithinRange(uintptr_t low, uintptr_t high, const map_options_t& = map_options_t());
	uintptr_t findPageWithinRange(uintptr_t low, uintptr_t high);

	void unmap(uintptr_t virt);

	void unmaprange(uintptr_t low, uintptr_t high);

	uintptr_t mapping(uintptr_t virt);

	bool mapped(uintptr_t virt, map_options_t* = nullptr);
	bool zeroPageMapped(uintptr_t virt, map_options_t* = nullptr);

	uintptr_t newoptions(uintptr_t virt, const map_options_t&);

	uintptr_t markCOW(uintptr_t virt);

	uintptr_t ksbrk(size_t amount);
	
	uintptr_t createAddressSpace();
	void cleanAddressSpace();

	uintptr_t cloneAddressSpace();

	void addKernelRegion(uintptr_t low, uintptr_t high);
	bool findKernelRegion(size_t size, interval_t& rgn);
	void delKernelRegion(interval_t rgn);
	bool isWithinKernelRegion(uintptr_t address, interval_t *rgn);

	void setheap(uintptr_t heap, size_t size);
	uintptr_t getheap() const;
	uintptr_t getheapbegin() const;
	uintptr_t getheapend() const;

	/**
	 * A scratch page is a short-term storage area - the idea is for the kernel to grab one, do something with it
	 * and then let go ASAP for the next guy to come over and grab it. Unlike the heap, scratch pages only come
	 * in page-size, so they are fairly suitable for things that naturally map to a page (e.g. peeking at another
	 * process's VM space)
	 */
	class scratch_page_t {
		public:
			scratch_page_t(uintptr_t, bool);
			scratch_page_t(scratch_page_t&&);
			scratch_page_t(const scratch_page_t&) = delete;

			explicit operator bool();
			explicit operator uintptr_t();

			uintptr_t reset();

			template<typename T>
			T* get() {
				return (T*)this->operator uintptr_t();
			}

			~scratch_page_t(); // unmaps the page
		private:
			uintptr_t address;	
			bool owned;
	};

	scratch_page_t getScratchPage(const map_options_t& = map_options_t());
	scratch_page_t getScratchPage(uintptr_t phys, const map_options_t& = map_options_t());

	// maps a page of memory "stolen" from another process into the current process' address space
	uintptr_t mapOtherProcessPage(process_t* other, uintptr_t otherVirt, uintptr_t selfVirt, const map_options_t& = map_options_t());
private:
	class KernelHeap : public Heap {
		public:
			KernelHeap(uintptr_t, uintptr_t);
			virtual ~KernelHeap() = default;
		protected:
			uintptr_t oneblock() override;

		private:
			VirtualPageManager& mVm;
			PhysicalPageManager& mPhys;
	};

	VirtualPageManager();
	
	Heap *mKernelHeap;
	// chicken and egg problem - we don't have a heap, but we need somewhere to store the heap information
	// so make that a static allocation
	uint8_t mKernelHeapMemory[sizeof(KernelHeap)];

	// the kernel exists in a 1GB region from 0xC0000000 to 0xFFFFFFFF
	IntervalList<interval_t, 1_GB> mKernelRegions;

	struct {
		uintptr_t low;
		uintptr_t high;
	} mScratchPageInfo;
};

#endif
