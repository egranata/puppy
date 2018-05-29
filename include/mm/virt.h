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

#include <sys/stdint.h>
#include <mm/phys.h>
#include <libc/bytesizes.h>
#include <mm/heap.h>
#include <sys/nocopy.h>

class PhysicalPageManager;

#define DECLARE_FIELD(name, type) type name (); void name ( type )

class VirtualPageManager : NOCOPY {
public:
	struct map_options_t {
		#define DECLARE_OPTION(type, name) type name () const; map_options_t& name ( type ); type _ ## name

		DECLARE_OPTION(bool, rw);
		DECLARE_OPTION(bool, user);
		DECLARE_OPTION(bool, clear);
		DECLARE_OPTION(bool, frompmm);

		#undef DECLARE_OPTION

		map_options_t();

		static const map_options_t& kernel();
		static const map_options_t& userspace();

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
		
		DECLARE_FIELD(present, bool);
		DECLARE_FIELD(rw, bool);
		DECLARE_FIELD(user, bool);
		DECLARE_FIELD(writethrough, bool);
		DECLARE_FIELD(cacheoff, bool);
		DECLARE_FIELD(accessed, bool);
		DECLARE_FIELD(dirty, bool);
		DECLARE_FIELD(global, bool);
		DECLARE_FIELD(frompmm, bool);
		DECLARE_FIELD(page, uintptr_t);
	};
	
#undef DECLARE_FIELD
	
	static constexpr size_t gKernelHeapSize = 128_MB;

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

	uintptr_t gZeroPageVirtual();
	uintptr_t gZeroPagePhysical();

	uintptr_t mapZeroPage(uintptr_t virt);

	uintptr_t map(uintptr_t phys, uintptr_t virt, const map_options_t&);

	uintptr_t newmap(uintptr_t virt, const map_options_t& = map_options_t());

	uintptr_t maprange(uintptr_t physlow, uintptr_t physhigh, uintptr_t virt, const map_options_t& = map_options_t());

	uintptr_t findpage(uintptr_t low, uintptr_t high, const map_options_t& = map_options_t());

	void unmap(uintptr_t virt);

	void unmaprange(uintptr_t low, uintptr_t high);

	uintptr_t mapping(uintptr_t virt);
	bool mapped(uintptr_t virt, map_options_t* = nullptr);

	uintptr_t ksbrk(size_t amount);
	
	uintptr_t newmap();
	void release();

	void setheap(uintptr_t heap, size_t size);
	uintptr_t getheap() const;
	uintptr_t getheapbegin() const;
	uintptr_t getheapend() const;
private:
	class KernelHeap : public Heap {
		public:
			KernelHeap(uintptr_t, uintptr_t);
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

	uintptr_t mZeroPagePhysical;
};

#endif