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

global rettokiller
rettokiller:
    sti ; allow interrupts, we may take a while here

    push eax

    xor eax, eax
    mov ax, 0x10
    mov ds, eax ; set data segments to be 0x10
    mov es, eax
    mov fs, eax
    mov gs, eax
    
    pop eax
    ret
