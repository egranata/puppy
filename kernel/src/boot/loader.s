; Copyright 2018 Google LLC
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;      http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.

global loader                          ; Make entry point visible to linker.
extern _earlyBoot                      ; Entry point for C++ code
global __bootpagedir
global __bootpagetbl
global __gdt
global __gdtinfo
global __numsysgdtentries
global MultiBootHeader

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0             ; align loaded modules on page boundaries
MEMINFO     equ  1<<1             ; provide memory map
GRAPHICS    equ  1<<2             ; setup screen mode
FLAGS       equ  MODULEALIGN | MEMINFO | GRAPHICS
MAGIC       equ    0x1BADB002     ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)  ; checksum required

section .grub
align 4 
MultiBootHeader:
     dd MAGIC
     dd FLAGS
     dd CHECKSUM
     dd 0 ; header_addr
     dd 0 ; load_addr
     dd 0 ; load_end_addr
     dd 0 ; bss_end_addr
     dd 0 ; entry_addr
     dd 0 ; mode_type (0 == graphics, 1 == text)
     dd 1024 ; width
     dd 768 ; height
     dd 32 ; depth ; 0 == no preference


; This is the virtual base address of kernel space. It must be used to convert virtual
; addresses into physical addresses until paging is enabled. Note that this is not
; the virtual address where the kernel image itself is loaded -- just the amount that must
; be subtracted from a virtual address to get a physical address.
KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)  ; Page directory index of kernel's 4MB PTE. 
KERNEL_NUM_PAGES equ 2

section .data
align 0x1000
__bootpagetbl:
	times 1024 dd 0
__bootpagedir:
    ; This is the initial page table setup - it is used to bootstrap the kernel into early boot
    ; and from there map all memory and into _kmain and onwards. It provides an initial mapping
    ; at the start of the memory space & in the higher half. The mapping is done as follows:
    ; - 4MB pages; read/write; present (user/supervisor doesn't matter much as userspace only
    ;                                   enters the picture at the very end of the boot process)
    ; These initial entries (and the higher half below) will be unmapped later on by the VMM layer
    dd 0x00000083
    dd 0x00400083
    times (KERNEL_PAGE_NUMBER - KERNEL_NUM_PAGES) dd 0                 ; Pages before kernel space.
    ; Map the kernel again in higher half
    dd 0x00000083
    dd 0x00400083
    times (1024 - KERNEL_PAGE_NUMBER - KERNEL_NUM_PAGES) dd 0 ; loader will overwrite the last entry here

; change this value here if system entries are added to the GDT
__numsysgdtentries:
    dd 6
align 0x1000
__gdt: ; refer to tools/make_gdt_descriptor.cpp
    ; null descriptor [0]
	dd 0x0
	dd 0x0

    ; kernel code 0x8 [1]
	dd 0xFFFF
	dd 0xCF9A00

    ; kernel data 0x10 [2]
	dd 0xFFFF
	dd 0xCF9200

    ; user code 0x18 [3]
	dd 0xFFFF
	dd 0xCFFA00

    ; user data 0x20 [4]
	dd 0xFFFF
	dd 0xCFF200

    ; task switching gate 0x28 [5]
	dd 0x280000
	dd 0x8500

    ; task segments [6...]
	times 2000 dq 0

__gdtinfo:
	dw __gdtinfo - __gdt - 1
	dd __gdt
 
STACKSIZE equ 0x10000
 
loader:
    ; map the last entry to the page tables - this allows the VMM setup phase
    ; to see the provisional page tables and setup scratch mappings as needed
    mov ecx, (__bootpagetbl - KERNEL_VIRTUAL_BASE)
    or ecx, 3
    mov [__bootpagedir - KERNEL_VIRTUAL_BASE + 4092], ecx
    
    mov ecx, (__bootpagedir - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx
 
    mov ecx, cr4
    or ecx,  0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx
 
    mov ecx, cr0
    or ecx, 0x80000000                          ; Set PG bit in CR0 to enable paging.
    or ecx, 0x20                                ; Enable native floating point exceptions.
    or ecx, 0x10000                             ; Don't let the kernel write to readonly pages
    mov cr0, ecx

    mov ecx, cr4
    or ecx,  0x00000080                          ; Allow global page mappings (must be done with PG=1)
    mov cr4, ecx
 
    ; Start fetching instructions in kernel space.
    ; Since eip at this point holds the physical address of this command (approximately 0x00100000)
    ; we need to do a long jump to the correct virtual address of StartInHigherHalf which is
    ; approximately 0xC0100000.
    lea ecx, [StartInHigherHalf]
    jmp ecx                                                     ; NOTE: Must be absolute jump!
 
StartInHigherHalf:
    mov dword [__bootpagedir], 0 ; remove the page 0 mapping
    invlpg [0]

StartSegmentation:
	lgdt [__gdtinfo]
	jmp 0x8:SegmentsOn
SegmentsOn:
	mov ecx,0x10
	mov ds,ecx
	mov es,ecx
	mov fs,ecx
	mov gs,ecx
	mov ss,ecx	
 
    ; NOTE: From now on, paging should be enabled. The first 4MB of physical address space is
    ; mapped starting at KERNEL_VIRTUAL_BASE. Everything is linked to this address, so no more
    ; position-independent code or funny business with virtual-to-physical address translation
    ; should be necessary. We now have a higher-half kernel.
    mov esp, stack
    push eax 
    push ebx

    xor ebp, ebp
    call _earlyBoot ; hand off to the rest of the kernel
WhyHere: ; TODO: could _earlyBoot and/or _kmain return a code to tell us what happened, and what to do?
    cli ; can't assume much about the state.. try to hang and hope for the best
    hlt
    jmp WhyHere

section .kstack
align 32
    times STACKSIZE db 0
stack:
