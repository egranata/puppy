#!/system/apps/micropython
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

def nibble(b):
    return (((b & 0xF0) >> 4), (b & 0x0F))

def useNibbles(nibbles, N):
    while N > 0:
        print( ("%x" % nibbles[0]), end='')
        if len(nibbles) > 1: nibbles = nibbles[1:]
        N = N - 1
    return nibbles

with open("/devices/prng/value", "r") as f:
    data = f.read(16)
    nibbles = []
    for byte in data:
        nb = nibble(ord(byte))
        nibbles.append(nb[0])
        nibbles.append(nb[1])
    nibbles = useNibbles(nibbles, 8); print("-", end='')
    nibbles = useNibbles(nibbles, 4); print("-", end='')
    nibbles = useNibbles(nibbles, 4); print("-", end='')
    nibbles = useNibbles(nibbles, 4); print("-", end='')
    nibbles = useNibbles(nibbles, 12); print("")
