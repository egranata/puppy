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

import subprocess
import sys
import os

def shell(command, shell=True, stdin=None, printout=False, onerrignore=False, curdir=None):
    if printout: print("$ %s" % command)
    try:
        stdout = subprocess.check_output(command, stdin=stdin, stderr=subprocess.STDOUT, shell=shell, cwd=curdir)
        o = stdout.decode('utf-8')
        if printout: print(o)
        return o
    except subprocess.CalledProcessError as e:
        print("$ %s" % command)        
        print(e.output.decode('utf-8'))
        if onerrignore:
            print("shell command failed")
        else:
            error("shell command failed")

def copy(src, dst):
    cmdline = 'cp "%s" "%s"' % (src, dst)
    shell(cmdline)
    return dst

INFILES = sys.argv[2:]
OUTFILE = "out/libnewlib.a"
INLIB = "newlib/lib/libc.a"

copy(INLIB, OUTFILE)

print("INFILES = %s" % (INFILES))

shell("i686-elf-ar qs %s %s" % (OUTFILE, ' '.join(INFILES)))
