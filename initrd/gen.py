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

import argparse
import sys
import io
import struct
import os
import os.path

def writeHeader(stream):
    header = struct.pack('<B3sBBBB', 0x80, 'IRD', 1, 0, 0, 0)
    stream.write(header)

def writeFile(stream, f):
    name = os.path.basename(f.path)
    entry = struct.pack('<64sII', name, f.size, f.offset)
    stream.write(entry)

def writeNullFile(stream):
    entry = struct.pack('<64sII', '', 0, 0)
    stream.write(entry)

def writeTable(stream, files):
    count = len(files)
    entry = struct.pack('<I', count)
    stream.write(entry)
    i = 0
    for f in files:
        writeFile(stream, f)
        i += 1
    while i < 64:
        writeNullFile(stream)
        i += 1

def writeFiles(stream, files):
    for f in files:
        with io.FileIO(f.path, mode='r') as f:
            buffer = f.readall()
            stream.write(buffer)

class InputFile(object):
    def __init__(self, path, offset):
        self.path = os.path.abspath(path)
        self.size = os.stat(path).st_size
        self.offset = offset

def genInputFiles(files):
    offset = 4620
    l = []
    for f in files:
        f = InputFile(f, offset)
        offset += f.size
        l.append(f)
    return l

parser = argparse.ArgumentParser(description='Generate an initrd image for Puppy')
parser.add_argument('--dest', help='the file where the image will be written to', default=None, required=True)
parser.add_argument('--file', help='a file to add to the image', action='append', default=[], required=True)
parser.add_argument('--version', action='version', version='%(prog)s 1.0')

args = parser.parse_args()

if len(args.file) > 64:
    print('error: cannot write more than 64 files to initrd image')
    sys.exit(1)

stream = io.FileIO(args.dest, mode='w')

files = genInputFiles(args.file)
for f in files:
    if f.size == 0:
        print("error: cannot insert empty file in initrd image")
        sys.exit(1)
    print("Adding file '%s' to initrd image.. size: %u, offset: %u" % (f.path, f.size, f.offset))

writeHeader(stream)
writeTable(stream, files)
writeFiles(stream, files)

stream.close()
