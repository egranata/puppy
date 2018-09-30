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

#include <kernel/boot/earlyboot.h>
#include <kernel/mm/phys.h>
#include <kernel/log/log.h>
#include <kernel/sys/globals.h>
#include <kernel/boot/bootinfo.h>
#include <kernel/mm/virt.h>
#include <kernel/drivers/framebuffer/fb.h>
#include <kernel/i386/primitives.h>
#include <kernel/libc/memory.h>
#include <kernel/i386/idt.h>
#include <kernel/panic/panic.h>
#include <kernel/boot/kmain.h>

static constexpr uint32_t gMultibootValid = 0x2BADB002;

// find which 4MB page index corresponds to the multiboot data
static constexpr size_t fourMBPageForMultiboot(uintptr_t multiboot_data) {
	return multiboot_data / (4_MB);
}
static constexpr size_t multibootOffset(uintptr_t multiboot_data) {
	return multiboot_data % (4_MB);
}

static void early_opcodehandler(GPR& gpr, InterruptStack& stack, void*) {
	LOG_ERROR("invalid instruction [eip = %p]", stack.eip);
	PANICFORWARD("invalid instruction. see log for details.", gpr, stack);
}

static void early_pagefaulthandler(GPR& gpr, InterruptStack& stack, void*) {
	LOG_ERROR("page fault due to adress %p [eip = %p]", gpr.cr2, stack.eip);
	PANICFORWARD("page fault. see log for details.", gpr, stack);
}

static void early_gpfhandler(GPR& gpr, InterruptStack& stack, void*) {
	LOG_ERROR("GPF error code %u", stack.error);
	PANICFORWARD("general protection fault. see log for details.", gpr, stack);
}

static void early_divby0handler(GPR& gpr, InterruptStack& stack, void*) {
	LOG_ERROR("division by zero occurred [eip = %p]", stack.eip);
	PANICFORWARD("division by zero. see log for details.", gpr, stack);
}

static void early_doublefaulthandler(GPR& gpr, InterruptStack& stack, void*) {
	LOG_ERROR("double fault [eip = %p]", stack.eip);
	PANICFORWARD("double fault. see log for details.", gpr, stack);
}

void setupEarlyIRQs() {
    Interrupts &idt(Interrupts::get());
	idt.install();
	idt.sethandler(0x0, "div0", early_divby0handler);
	idt.sethandler(0x6, "opcode", early_opcodehandler);
	idt.sethandler(0x8, "dblflt", early_doublefaulthandler);
	idt.sethandler(0xD, "gpf", early_gpfhandler);
	idt.sethandler(0xE, "pagefllt", early_pagefaulthandler);
}

static void setupPhysicalMemory(multiboot_info* multiboot) {
	PhysicalPageManager &phys(PhysicalPageManager::get());
	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)multiboot->mmap_addr;
	while((uintptr_t)mmap < (multiboot->mmap_addr + multiboot->mmap_length)) {
		LOG_INFO("memory block type = %u addr = %p len = %u", mmap->type, (void*)mmap->addr, (uint32_t)mmap->len);
		if (mmap->type == 1) {
			// ignore the low MB
			const auto threshold = 1048576;
			auto start = mmap->addr;
			auto len = mmap->len;
			if (start < threshold) {
				if (len >= threshold) {
					len = (len - (start - threshold));
					start = threshold;
				}
			}
			if (start >= threshold) {
				auto astart = (uint8_t*)mmap->addr;
				auto aend = start + (uint32_t)len;
				LOG_DEBUG("found a valid block of RAM - range [%p - %p]", astart, aend);
				phys.addpages(start, len);
			}
		}
		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(mmap->size) );
	}
	
	LOG_INFO("RAM size is %u pages, aka %u bytes", phys.gettotalpages(), phys.gettotalmem());
}

static void reserveMemory(multiboot_info* multiboot) {
	PhysicalPageManager &phys(PhysicalPageManager::get());	

	phys.reserve(addr_kernel_start() - gBootVirtualOffset, addr_kernel_end() - gBootVirtualOffset);

	grub_modules_info_t* bootmodules = bootmodinfo();
	if (multiboot->mods_count > grub_modules_info_t::gMaxModules) {
		bootmodules->count = grub_modules_info_t::gMaxModules;
	} else {
		bootmodules->count = (uint8_t)multiboot->mods_count;
	}

	LOG_DEBUG("%d modules are loaded starting at %p", bootmodules->count, multiboot->mods_addr);
	for (auto mod_id = 0u; mod_id < bootmodules->count; ++mod_id) {
		multiboot_mod_list &module = ((multiboot_mod_list*)multiboot->mods_addr)[mod_id];
		auto&& bootmodule = bootmodules->info[mod_id];
		bootmodule.start = module.mod_start;
		bootmodule.end = module.mod_end;
		bootmodule.vmstart = 0x0;
		const char* cmdline = (const char*)module.cmdline;
		for (auto i = 0; i < 256; ++i) {
			bootmodule.args[i] = *cmdline;
			if (*cmdline++ == 0) break;			
		}
		LOG_DEBUG("module %u ranges from %p to %p, command string is %p %s", mod_id, module.mod_start, module.mod_end, module.cmdline, &bootmodule.args[0]);		
		phys.reserve(module.mod_start, module.mod_end);
	}
}

static multiboot_info* getMultiboot(uintptr_t multiboot_data, uint32_t multiboot_magic) {
	if (gMultibootValid != multiboot_magic) {
		LOG_ERROR("invalid multiboot_magic, got %x but expected %x. hanging.", multiboot_magic, gMultibootValid);
		while(true);
	}

	auto mbpage = fourMBPageForMultiboot(multiboot_data);
	LOG_DEBUG("mapping multiboot at %p", mbpage);
	addr_bootpagedirectory<uintptr_t*>()[mbpage] = 0x83 | (multiboot_data & 0xFFC00000);
	invtlb(mbpage);
	auto multiboot_hdr = (multiboot_info*)(mbpage+multibootOffset(multiboot_data));

	if (0x1041 != (multiboot_hdr->flags & 0x1041)) {
		LOG_ERROR("multiboot flags value %x implies no memory map. hanging.", multiboot_hdr->flags);
		while(true);
	}

	return multiboot_hdr;
}

static void setupKernelHeap() {
	auto&& vmm(VirtualPageManager::get());

	auto kernel_high = addr_kernel_end();
	// round up to the end of page - this is >= the actual end of the kernel image
	kernel_high = VirtualPageManager::page(kernel_high) + VirtualPageManager::gPageSize - 1;

	// and one page later, start the heap
	auto heap_start = kernel_high + 1 + VirtualPageManager::gPageSize;
	vmm.setheap(heap_start, VirtualPageManager::gKernelHeapSize);
}

static Framebuffer* setupFramebuffer(const framebuf_info_t& framebufferinfo) {
	VirtualPageManager &vm(VirtualPageManager::get());
	auto fb = Framebuffer::init(
		framebufferinfo.width,
		framebufferinfo.height,
		framebufferinfo.pitch,
		framebufferinfo.bpp,
		framebufferinfo.videoram
	);

	interval_t fb_region;
	if (false == vm.findKernelRegion(fb->size(), fb_region)) {
		LOG_ERROR("failed to obtain a memory region to map the framebuffer!");
		PANIC("unable to map framebuffer in memory");
	} else {
		LOG_DEBUG("framebuffer region is [%p - %p]", fb_region.from, fb_region.to);
		fb->map(fb_region.from);
		vm.addKernelRegion(fb_region.from, fb_region.to);
	}

	return fb;
}

static void loadModules() {
	VirtualPageManager &vm(VirtualPageManager::get());

	grub_modules_info_t* bootmodules = bootmodinfo();

	LOG_DEBUG("%d modules being initialized", bootmodules->count);
	for (auto mod_id = 0u; mod_id < bootmodules->count; ++mod_id) {
		auto&& module = bootmodules->info[mod_id];

		interval_t module_rgn;
		if (false == vm.findKernelRegion(module.end - module.start, module_rgn)) {
			PANIC("unable to find memory for module mapping");
		}
		vm.addKernelRegion(module_rgn.from, module_rgn.to);
		vm.maprange(module.start, module.end, module_rgn.from, VirtualPageManager::map_options_t::kernel());
		module.vmstart = module_rgn.from;
		LOG_DEBUG("module %u has ranges: phys=[%p - %p], virt=[%p - %p]",
			mod_id, module.start, module.end, module_rgn.from, module_rgn.to);
	}
}

static void loadCommandline(multiboot_info* multiboot) {
	auto bcmd = bootcmdline();
	auto copied = bcmd->fill(multiboot->cmdline);
	if (copied) {
		LOG_DEBUG("copied %d bytes of kernel command line - value is %s", copied, bcmd->cmdline);
	} else {
		LOG_DEBUG("kernel command line found empty!");
	}
}

extern "C"
void _earlyBoot(uintptr_t multiboot_data, uint32_t multiboot_magic) {
	setupEarlyIRQs();
	auto multiboot = getMultiboot(multiboot_data, multiboot_magic);
	loadCommandline(multiboot);
	framebuf_info_t framebuf(*multiboot);
	setupPhysicalMemory(multiboot);
	reserveMemory(multiboot);
	setupKernelHeap();
	setupFramebuffer(framebuf);
	loadModules();

	{
		PhysicalPageManager &phys(PhysicalPageManager::get());	
		LOG_INFO("free memory is %u pages, aka %u bytes", phys.getfreepages(), phys.getfreemem());
	}

	{
		LOG_DEBUG("multiboot magic is valid; multiboot_data at physical address %p", multiboot_data);
		_kmain();
	}
}
