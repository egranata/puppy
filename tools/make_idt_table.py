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

# see Intel Manual 3A and http://wiki.osdev.org/Exceptions for source data for this script

reserved = set([15, 21, 22, 23, 24, 25, 26, 27, 28, 29, 31])
errorcode = set([8, 10, 11, 12, 13, 14, 17, 30])
userspace = set([128])
interruptible = set([128])

import sys

mode = sys.argv[1]

if mode == 'idt.s':
    for i in range(0, 256):
        if i in reserved: continue
        print("%serror_code_interrupt_handler %d" % ("" if i in errorcode else "no_", i))
if mode == 'idt.cpp':
    for i in range(0, 256):
        print('extern uintptr_t interrupt_handler_%d;' % i)
    
    print("\nInterrupts::Interrupts() {")
    print("    mCliCount = (enabled() ? 1 : 0);")
    print("    bzero((uint8_t*)&mHandlers[0], sizeof(mHandlers));")
    for i in range(0, 256):
        if i in reserved: continue
        print("    mEntries[%d] = Entry((uintptr_t)&interrupt_handler_%d, %s, %s);" % (i, i,
            "true" if i in userspace else "false",
            "false" if i in interruptible else "true"))            
    print("}")


