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

; uint32_t syscall0(uint8_t n);
global syscall0:
syscall0:
    mov eax, [esp + 4]
    int 0x80
    ret

; uint32_t syscall1(uint8_t n, uint32_t arg0);
global syscall1
syscall1:
    push ebx
    xor eax, eax
    mov ebx, [esp + 12]
    mov al, [esp + 8]
    int 0x80
    pop ebx
    ret

; uint32_t syscall2(uint8_t n, uint32_t arg0, uint32_t arg1);
global syscall2
syscall2:
    push ebx
    push ecx
    xor eax, eax
    mov ecx, [esp + 20]
    mov ebx, [esp + 16]
    mov al, [esp + 12]
    int 0x80
    pop ecx
    pop ebx
    ret

; uint32_t syscall3(uint8_t n, uint32_t arg0, uint32_t arg1, uint32_t arg2);
global syscall3
syscall3:
    push ebx
    push ecx
    push edx
    xor eax, eax
    mov edx, [esp + 28]
    mov ecx, [esp + 24]
    mov ebx, [esp + 20]
    mov al, [esp + 16]
    int 0x80
    pop edx
    pop ecx
    pop ebx
    ret

; uint32_t syscall4(uint8_t n, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
global syscall4
syscall4:
    push ebx
    push ecx
    push edx
    push edi
    xor eax, eax
    mov edi, [esp + 36]
    mov edx, [esp + 32]
    mov ecx, [esp + 28]
    mov ebx, [esp + 24]
    mov al, [esp + 20]
    int 0x80
    pop edi
    pop edx
    pop ecx
    pop ebx
    ret

; uint32_t syscall5(uint8_t n, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
global syscall5
syscall5:
    push ebx
    push ecx
    push edx
    push edi
    push esi
    xor eax, eax
    mov esi, [esp + 44]
    mov edi, [esp + 40]
    mov edx, [esp + 36]
    mov ecx, [esp + 32]
    mov ebx, [esp + 28]
    mov al, [esp + 24]
    int 0x80
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    ret
