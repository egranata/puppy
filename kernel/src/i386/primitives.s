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

global outb
outb:
	mov al, [esp + 8]
	mov dx, [esp + 4]
	out dx, al
	ret

global inb
inb:
	mov dx, [esp + 4]
	in  al, dx
	ret

global outl
outl:
	mov eax, [esp + 8]
	mov dx, [esp + 4]
	out dx, eax
	ret

global outw
outw:
	mov ax, [esp + 8]
	mov dx, [esp + 4]
	out dx, ax
	ret

global inl
inl:
	mov dx, [esp + 4]
	in eax, dx
	ret

global inw
inw:
	mov dx, [esp + 4]
	xor eax, eax
	in ax, dx
	ret

global invtlb
invtlb:
	mov eax, [esp + 4]
	invlpg [eax]
	ret
	
global loadidt
loadidt:
	mov eax, [esp + 4]
	lidt [eax]
	ret

global enableirq
enableirq:
	sti
	ret

global disableirq
disableirq:
	cli
	ret

global loadpagedir
loadpagedir:
	mov eax, [esp + 4]
	mov cr3, eax
	ret

global readflags
readflags:
	pushf
	pop eax
	ret

global readcr0
readcr0:
	mov eax, cr0
	ret

global writecr0
writecr0:
	mov eax, [esp + 4]
	mov cr0, eax
	ret

global readcr3
readcr3:
	mov eax, cr3
	ret

global writecr3
writecr3:
	mov eax, [esp + 4]
	mov cr3, eax
	ret

global readcr4
readcr4:
	mov eax, cr4
	ret

global writecr4
writecr4:
	mov eax, [esp + 4]
	mov cr4, eax
	ret

global readeip
readeip:
	mov eax, [esp]
	ret

global readesp
readesp:
	mov eax, esp
	ret

global readebp
readebp:
	mov eax, ebp
	ret

global readcs:
readcs:
	xor eax, eax
	mov ax, cs
	ret

global readds:
readds:
	xor eax, eax
	mov ax, ds
	ret

global reades:
reades:
	xor eax, eax
	mov ax, es
	ret

global readfs:
readfs:
	xor eax, eax
	mov ax, fs
	ret

global readgs:
readgs:
	xor eax, eax
	mov ax, gs
	ret

global readss:
readss:
	xor eax, eax
	mov ax, ss
	ret

global writetaskreg
writetaskreg:
	mov ax, [esp + 4]
	ltr ax
	ret

global readtaskreg
readtaskreg:
	xor eax, eax
	str ax
	ret

global ctxswitch
ctxswitch:
	jmp 0x28:0
	ret

global iowait
iowait:
	xor al, al
	out 0x80, al
	ret

global toring3
toring3:
	mov eax, [esp + 8] ; esp
	mov ebx, [esp + 4] ; eip
	mov ecx, 0x23
	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx
	push 0x23
	push eax
	push 512 ; enable IRQ
	push 0x1b
	push ebx
	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	iret

global readtsc
readtsc:
	rdtsc
	ret

global insd
insd:
	mov ax, [esp + 4]
	push edx
	mov dx, ds
	push es
	mov es, dx
	xor edx, edx	
	push edi
	xor edi, edi
	lea edi, [_insd_data]
	mov dx, ax
	insd
	mov eax, _insd_data
	mov eax, [eax]
	pop edi
	pop es
	pop edx
	ret
_insd_data:
	dd 0

; uint32_t cpuinfo(uint32_t eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
;                            +4	  		    +8			   +12			  +16
global cpuinfo
cpuinfo:
	push ebx
	push ecx
	push edx
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	mov eax, [esp + 16]
	cpuid
	push eax
	mov eax, [esp + 24]
	cmp eax, 0
	je _cpuinfo_skipebx
	mov [eax], ebx
_cpuinfo_skipebx:
	mov eax, [esp + 28]
	cmp eax, 0
	je _cpuinfo_skipecx	
	mov [eax], ecx
_cpuinfo_skipecx:
	mov eax, [esp + 32]
	cmp eax, 0
	je _cpuinfo_skipedx	
	mov [eax], edx
_cpuinfo_skipedx:
	pop eax
	pop edx
	pop ecx
	pop ebx
	ret

global fpinit
fpinit:
	finit
	fldcw [_fpinit_cw]
	xor eax, eax
	ret
_fpinit_cw:
	dw 0x37A

global fpsave
fpsave:
	mov eax, [esp + 4]
	fnsave [eax]
	xor eax, eax
	ret

global fprestore
fprestore:
	mov eax, [esp + 4]
	frstor [eax]
	xor eax, eax
	ret

global cleartaskswitchflag
cleartaskswitchflag:
	clts
	xor eax, eax
	ret

global undefined
undefined:
	enter 0, 0
	ud2
	leave
	ret ; should never be executed

global readmsr
readmsr:
	mov ecx, [esp + 4]
	rdmsr
	ret

global writemsr
writemsr:
	mov edx, [esp + 12]
	mov eax, [esp + 8]
	mov ecx, [esp + 4]
	wrmsr
	ret

global haltforever
haltforever:
	hlt
	jmp haltforever
