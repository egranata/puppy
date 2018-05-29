#!/usr/bin/python
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

#!/usr/bin/env python3

def printMapping(physical, virtual, flags=0x3):
    directoryIndex = virtual >> 22
    pageIndex = (virtual >> 12) & 0x03FF
    mapping = physical | (flags & 0xFFF) | 0x1
    print("To map phys 0x%x to virt 0x%x, directoryIndex will be %d, pageIndex will be %d and the mapping will be 0x%x" % (physical, virtual, directoryIndex, pageIndex, mapping))

def directoryEntry(virtual):
    directoryIndex = virtual >> 22
    pageIndex = (virtual >> 12) & 0x03FF
    print("To map virtual 0x%x, directory entry %d page entry %d must be filled" % (virtual, directoryIndex, pageIndex))

printMapping(physical=0xB8000, virtual=0xC00B8000)
printMapping(physical=0x100000, virtual=0xC0100000)
printMapping(physical=0x13A000, virtual=0xFFFFF000)
printMapping(physical=0x13A000, virtual=0xFFFFE000)
printMapping(physical=0x13B000, virtual=0xffbff000)
directoryEntry(virtual=3*1024*1024*1024)
directoryEntry(virtual=0xffbff000)
directoryEntry(virtual=0x2000000)

directoryEntry(virtual=0xF0000)
directoryEntry(virtual=0xFFFFF)

directoryEntry(virtual=0xE0000)
