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

global reboot8042
reboot8042:
    mov ecx, 1000
    in al, 0x64
    and al, 0x02
    cmp al, 0
    call hold
doit:
    mov al, 0xFE
    out 0x64, al
halt: cli
      hlt
      jmp halt
hold:
    sub ecx, 1
    cmp ecx, 0
    je doit
    mov al, 0
    mov dh, 100
holdloop:
    out 0x80, al
    sub dh, 1
    cmp dh, 0
    jne holdloop
    ret
