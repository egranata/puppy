#!/usr/bin/python3
#
# Copyright 2018 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# see Intel Manual 3A and http://wiki.osdev.org/Exceptions for useful configuration data

import json, sys

class IRQ(object):
    def __init__(self, num):
        self.num = num
        self.reserved = False
        self.pushes_error = False
        self.userspace = False
        self.interruptible  = False

if len(sys.argv) != 4:
    print("error: invoke with .s or .cpp and a destination file name")
    print("make_idt_table <json> <.s | .cpp> <dest>")
    sys.exit(1)

source = sys.argv[1]
mode = sys.argv[2]
destination = sys.argv[3]

IRQ_Table = {}
for i in range(0, 256):
    IRQ_Table[i] = IRQ(i)

with open(source, "r") as f:
    table = json.load(f)
    for num in table['irqs']:
        irq_data = table['irqs'][num]
        IRQ_Table[int(num)].reserved = irq_data.get('reserved', False)
        IRQ_Table[int(num)].pushes_error = irq_data.get('pushes_error', False)
        IRQ_Table[int(num)].userspace = irq_data.get('userspace', False)
        IRQ_Table[int(num)].interruptible = irq_data.get('interruptible', False)

def doGenerateAssembly():
    with open(destination, "w") as f:
        print("; Copyright 2018 Google LLC", file=f)
        print(";", file=f)
        print("; Licensed under the Apache License, Version 2.0 (the \"License\");", file=f)
        print("; you may not use this file except in compliance with the License.", file=f)
        print("; You may obtain a copy of the License at", file=f)
        print(";", file=f)
        print(";      http://www.apache.org/licenses/LICENSE-2.0", file=f)
        print(";", file=f)
        print("; Unless required by applicable law or agreed to in writing, software", file=f)
        print("; distributed under the License is distributed on an \"AS IS\" BASIS,", file=f)
        print("; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.", file=f)
        print("; See the License for the specific language governing permissions and", file=f)
        print("; limitations under the License.", file=f)
        print("", file=f)
        print("extern interrupt_handler", file=f)
        print("", file=f)
        print("%macro common_interrupt_handler 1", file=f)
        print("	push esp", file=f)
        print("	push edi", file=f)
        print("	push esi", file=f)
        print("	push ebp", file=f)
        print("	push edx", file=f)
        print("	push ecx", file=f)
        print("	push ebx", file=f)
        print("	push eax", file=f)
        print("	mov eax, cr4", file=f)
        print("	push eax", file=f)
        print("	mov eax, cr3", file=f)
        print("	push eax", file=f)
        print("	mov eax, cr2", file=f)
        print("	push eax", file=f)
        print("	mov eax, cr0", file=f)
        print("	push eax", file=f)
        print("", file=f)
        print("    call interrupt_handler", file=f)
        print("", file=f)
        print("	pop eax; # mov cr0, eax", file=f)
        print("	pop eax; # mov cr2, eax", file=f)
        print("	pop eax; # mov cr3, eax", file=f)
        print("	pop eax; # mov cr4, eax", file=f)
        print("	pop eax", file=f)
        print("	pop ebx", file=f)
        print("	pop ecx", file=f)
        print("	pop edx", file=f)
        print("	pop ebp", file=f)
        print("	pop esi", file=f)
        print("	pop edi", file=f)
        print("    pop esp", file=f)
        print("", file=f)
        print("	add esp, %1", file=f)
        print("	iret", file=f)
        print("%endmacro", file=f)
        print("", file=f)
        print("%macro no_error_code_interrupt_handler 1", file=f)
        print("    global interrupt_handler_%1", file=f)
        print("    interrupt_handler_%1:", file=f)
        print("        push dword 0", file=f)
        print("		push dword %1", file=f)
        print("    	common_interrupt_handler 8", file=f)
        print("%endmacro", file=f)
        print("", file=f)
        print("%macro error_code_interrupt_handler 1", file=f)
        print("	global interrupt_handler_%1", file=f)
        print("	interrupt_handler_%1:", file=f)
        print("		push dword %1", file=f)
        print("    	common_interrupt_handler 8", file=f)
        print("%endmacro", file=f)
        for irq in IRQ_Table.values():
            if irq.reserved: continue
            print("%serror_code_interrupt_handler %d" % ("" if irq.pushes_error else "no_", irq.num), file=f)

def doGenerateCpp():
        with open(destination, "w") as f:
            print("// Copyright 2018 Google LLC", file=f)
            print("//", file=f)
            print("// Licensed under the Apache License, Version 2.0 (the \"License\");", file=f)
            print("// you may not use this file except in compliance with the License.", file=f)
            print("// You may obtain a copy of the License at", file=f)
            print("//", file=f)
            print("//      http://www.apache.org/licenses/LICENSE-2.0", file=f)
            print("//", file=f)
            print("// Unless required by applicable law or agreed to in writing, software", file=f)
            print("// distributed under the License is distributed on an \"AS IS\" BASIS,", file=f)
            print("// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.", file=f)
            print("// See the License for the specific language governing permissions and", file=f)
            print("// limitations under the License.", file=f)
            print("", file=f)
            print("#include <kernel/i386/idt.h>", file=f)
            print("#include <kernel/libc/string.h>", file=f)
            print("#include <muzzle/string.h>", file=f)
            for irq in IRQ_Table.keys():
                print('extern uintptr_t interrupt_handler_%d;' % irq, file=f)
            print("\nInterrupts::Interrupts() {", file=f)
            print("    mCliCount = (enabled() ? 1 : 0);", file=f)
            print("    bzero((uint8_t*)&mHandlers[0], sizeof(mHandlers));", file=f)
            for irq in IRQ_Table.values():
                if irq.reserved: continue
                print("    mEntries[%d] = Entry((uintptr_t)&interrupt_handler_%d, %s, %s);" % (irq.num, irq.num,
                    "true" if irq.userspace else "false",
                    "false" if irq.interruptible else "true"), file=f)
            print("}", file=f)

if mode == ".s":
    doGenerateAssembly()
elif mode == ".cpp":
    doGenerateCpp()
else:
    print("error: invoke with .s or .cpp and a destination file name")
    print("make_idt_table <json> <.s | .cpp> <dest>")
    sys.exit(1)
