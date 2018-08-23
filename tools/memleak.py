#!/usr/bin/env python3
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

import sys
import re

log = sys.argv[1]

# print("Will look for MEMLEAK information in %s " % log)

PAGES = set()
PIDS = dict()
VIRT = dict()

PM_INFO = re.compile(r"\(PMLEAK\) (?P<op>[A-Z]+) (?P<addr>0x[0-9A-Fa-f]+)")
ML_INFO = re.compile(r"MEMLEAK: process (?P<pid>\d+) allocated page virt=(?P<virt>0x[0-9A-Fa-f]+) phys=(?P<phys>0x[0-9A-Fa-f]+)")
FIL = open(log, "r")
for line in FIL:
    match = PM_INFO.search(line)
    if match:
        operation = match.group("op")
        addr = match.group("addr")
        if operation == "ALLOC":
            PAGES.add(addr)
        elif operation == "DEALLOC":
            if addr not in PAGES:
                print("Page %s being freed which was not allocated" % addr)
            else:
                PAGES.remove(addr)
        else:
            print("Unknown operation: %s" % operation)
    match = ML_INFO.search(line)
    if match:
        pid = match.group("pid")
        phys = match.group("phys")
        virt = match.group("virt")
        PIDS[phys] = pid
        VIRT[phys] = virt

if len(PAGES) > 0:
    for pg in PAGES:
        owner = PIDS.get(pg)
        virt = VIRT.get(pg)
        if owner is None: owner = "unknown"
        if virt is None: virt = "unknown"
        print("Leaked page: %s (owner=%s, virtual=%s)" % (pg, owner, virt))

