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

# writes an MBR partition table to a disk image file

import json, os, os.path, struct, sys

class Partition(object):
    FORMAT_STRING = "B 3B B 3B I I"
    def __init__(self):
        self.bootable = False
        self.type = 0
        self.lba = 0
        self.size = 0
    
    def setBootable(self, b):
        self.bootable = b
        return self
    def setType(self, t):
        self.type = t
        return self
    def setLBA(self, l):
        self.lba = l
        return self
    def setSize(self, s):
        self.size = s
        return self
    def toBytes(self):
        ba = bytearray(struct.calcsize(Partition.FORMAT_STRING))
        self.writeToBuffer(ba, 0)
        return ba
    def writeToBuffer(self, buffer, offset):
        return struct.pack_into(Partition.FORMAT_STRING, buffer, offset,
            0x80 if self.bootable else 0,
            0xFF, 0xFF, 0xFF,
            self.type,
            0xFF, 0xFF, 0xFF,
            self.lba,
            self.size)
    def __repr__(self):
        return self.__str__()
    def __str__(self):
        return "<Partition Type:0x%x, Extent:[base=%d size=%d sectors], %sbootable>" % \
            (self.type, self.lba, self.size, "" if self.bootable else "not ")

class PartitionTable(object):
    def __init__(self):
        self.partitions = []
    def addPartition(self, partition):
        if len(self.partitions) == 4:
            raise ValueError("only 4 partitions supported")
        else:
            self.partitions.append(partition)
    def toBytes(self):
        ba = bytearray(4 * struct.calcsize(Partition.FORMAT_STRING))
        return self.writeToBuffer(ba, 0)
    def writeToBuffer(self, buffer, offset):
        i = 0
        for p in self.partitions:
            p.writeToBuffer(buffer, offset + struct.calcsize(Partition.FORMAT_STRING) * i)
            i = i + 1
        return buffer
    def __repr__(self):
        return self.__str__()
    def __str__(self):
        return "<Partition Table: " + [', '.join(self.partitions)] + ">"

class BootRecord(object):
    def __init__(self):
        self.table = PartitionTable()
        self.volid = os.urandom(4)
    def addPartition(self, partition):
        self.table.addPartition(partition)
    def toBytes(self):
        ba = bytearray(512)
        ba[510] = 0x55
        ba[511] = 0xAA
        ba[440] = self.volid[0]
        ba[441] = self.volid[1]
        ba[442] = self.volid[2]
        ba[443] = self.volid[3]
        self.table.writeToBuffer(ba, 446)
        return ba
    def __repr__(self):
        return self.__str__()
    def __str__(self):
        return self.table.__str__()
    def fromFile(self, path):
        previousLastSector = 0
        if not os.path.exists(path):
            raise ValueError("file not found")
        with open(path, "r") as f:
            jsonData = json.load(f)
            for jsonEntry in jsonData:
                newPartition = Partition()
                newPartition.setBootable(jsonEntry.get('bootable', 'no') == 'yes')
                newPartition.setType(jsonEntry.get('type', 12))
                newPartition.setLBA(jsonEntry.get('lba', previousLastSector))
                newPartition.setSize( int(jsonEntry.get('size', 1024*1024) / 512) )
                lastSector = newPartition.lba + newPartition.size
                previousLastSector = max(previousLastSector, lastSector)
                self.addPartition(newPartition)
                print("Adding partition %s" % (newPartition))

if len(sys.argv) != 3:
    print("syntax: %s <source file> <dest file>" % sys.argv[0])
    print("<source file> is a JSON file that contains the description of a partition table")
    print("<dest file> is the MBR for the disk described in source")
else:
    mbr = BootRecord()
    mbr.fromFile(sys.argv[1])
    buffer = mbr.toBytes()
    with open(sys.argv[2], "wb") as f:
        f.write(buffer)

