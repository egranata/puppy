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
extern _kmain                            ; _main is defined elsewhere
global __bootpagedir
global __bootpagetbl
global __gdt
global __gdtinfo
global __numsysgdtentries

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
 
section .data
align 0x1000
__bootpagetbl:
	times 1024 dd 0
__bootpagedir:
    ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB.
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.
    ; This entry must be here -- otherwise the kernel will crash immediately after paging is
    ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0                 ; Pages before kernel space.
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 2) dd 0
	dd __bootpagedir - KERNEL_VIRTUAL_BASE ; we will replace this mapping in _loader

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
	times 2000 dd 0

__gdtinfo:
	dw __gdtinfo - __gdt - 1
	dd __gdt
 
STACKSIZE equ 0x10000
 
loader:
    ; NOTE: Until paging is set up, the code must be position-independent and use physical
    ; addresses, not virtual ones!
	mov ecx, (__bootpagetbl - KERNEL_VIRTUAL_BASE)
	or ecx, 3
	mov [__bootpagedir - KERNEL_VIRTUAL_BASE + 4092], ecx
    mov ecx, (__bootpagedir - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx                                        ; Load Page Directory Base Register.
 
    mov ecx, cr4
    or ecx,  0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
    ; or ecx,  0x00000100                          ; Allow RDPMC at CPL=3
    ; or ecx,  0x00000080                          ; Allow global page mappings
    ; or ecx,  0x00000800                          ; Disable SGDT, SIDT, SLDT, SMSW and STR at CPL=3
    ; and ecx, 0xFFFFFFFB                          ; Allow RDTSC at CPL=3
    mov cr4, ecx
 
    mov ecx, cr0
    or ecx, 0x80000000                          ; Set PG bit in CR0 to enable paging.
    or ecx, 0x20                                ; Enable native floating point exceptions.
    mov cr0, ecx
 
    ; Start fetching instructions in kernel space.
    ; Since eip at this point holds the physical address of this command (approximately 0x00100000)
    ; we need to do a long jump to the correct virtual address of StartInHigherHalf which is
    ; approximately 0xC0100000.
    lea ecx, [StartInHigherHalf]
    jmp ecx                                                     ; NOTE: Must be absolute jump!
 
StartInHigherHalf:
    ; Unmap the identity-mapped first 4MB of physical address space. It should not be needed
    ; anymore.
    mov dword [__bootpagedir], 0
    invlpg [0]

	lgdt [__gdtinfo]
	jmp 0x8:StartSegmentation
StartSegmentation:
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
    call _kmain
kmainReturn:
        hlt
       jmp kmainReturn

section .bss
align 32
    resb STACKSIZE
stack:
