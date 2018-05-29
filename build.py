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
import glob
import os
import os.path
import shutil
import subprocess
import sys
import time

BUILD_START = time.time()

BASIC_CFLAGS = "-O2 -fno-omit-frame-pointer -march=i686 -masm=intel -m32 -nostdlib -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Wno-main -Werror -funsigned-char -fno-exceptions -fdiagnostics-color=always -c"
BASIC_CPPFLAGS = " -std=c++14 -fno-exceptions -fno-rtti "

FATFS_CFLAGS = BASIC_CFLAGS + " -Ithird_party/fatfs/include"
FATFS_LDFLAGS = "-ffreestanding -nostdlib"

MUZZLE_CFLAGS = BASIC_CFLAGS + " -Ithird_party/muzzle/include"
MUZZLE_CPPFLAGS = BASIC_CPPFLAGS + " -Ithird_party/muzzle/include"
MUZZLE_LDFLAGS = "-ffreestanding -nostdlib"
MUZZLE_ASFLAGS = "-nostartfiles -nodefaultlibs -Wall -Wextra -fdiagnostics-color=always -nostdlib -c"

CFLAGS = BASIC_CFLAGS + " -mgeneral-regs-only -Iinclude -Ithird_party/muzzle -Ithird_party/muzzle/libmuzzle -Ithird_party"
CPPFLAGS = BASIC_CPPFLAGS + CFLAGS
ASFLAGS = "-f elf"
LDFLAGS = "-T build/linker.ld -ffreestanding -nostdlib"

LIBUSERSPACE_CFLAGS = BASIC_CFLAGS + " -Ilibuserspace/include -Iinclude -Ithird_party/muzzle -Ithird_party/muzzle/include"
LIBUSERSPACE_ASFLAGS = "-f binary"

APP_CFLAGS = BASIC_CFLAGS + " -Ilibuserspace/include -Iinclude -Ithird_party/muzzle -Ithird_party/muzzle/include"
APP_ASFLAGS = "-f binary"
APP_LDFLAGS = "-T build/app.ld -ffreestanding -nostdlib -e__app_entry"

def error(msg):
    print("error: %s" % msg)
    sys.exit(1)

def findSubdirectories(dir):
    for subdir in os.listdir(dir):
        candidate = os.path.join(dir, subdir)
        if os.path.isdir(candidate):
            yield candidate

def find(dir, extension=None):
    if extension:
        if extension[0] != '.':
            extension = '.%s' % extension
    for root, dirs, files in os.walk(dir):
        for file in files:
            if extension is None or file.endswith(extension):
                yield os.path.join(root, file)

def shell(command, shell=True):
    # print("$ %s" % command)
    try:
        stdout = subprocess.check_output(command, stderr=subprocess.STDOUT, shell=shell)
        # print(stdout.decode('utf-8'))
        return stdout
    except subprocess.CalledProcessError as e:
        print("$ %s" % command)        
        print(e.output.decode('utf-8'))
        error("shell command failed")

def findAll(base, extension):
    L = []
    for subdir in findSubdirectories(base):
        for f in find(subdir, extension):
            L.append(f)
    return L

def makeOutputFilename(src, extension, prefix=None):
    if extension[0] != '.':
        extension = '.%s' % extension
    outextension = "_%s.o" % extension[1:]
    if prefix is not None: src = "%s_%s" % (prefix, src)
    return os.path.join("out", src.replace("src/","").replace("/","_").replace(extension,outextension))

def copy(src, dst):
    cmdline = 'cp "%s" "%s"' % (src, dst)
    shell(cmdline)
    return dst

def read(src):
    with open(src, 'r') as f:
        return f.read()

def write(dst, txt):
    with open(dst, 'w') as f:
        f.write(txt)

def buildAsm(inp, out, flags=ASFLAGS, assembler='nasm'):
    cmdline = '%s %s %s -o %s' % (assembler, flags, inp, out)
    shell(cmdline)
    return out

def buildCpp(inp, out, flags=CPPFLAGS, compiler="i686-elf-gcc"):
    cmdline = "%s %s %s -o %s" % (compiler, flags, inp, out)
    shell(cmdline)
    return out

def buildC(inp, out, flags=CFLAGS, compiler="i686-elf-gcc"):
    cmdline = "%s %s %s -o %s" % (compiler, flags, inp, out)
    shell(cmdline)
    return out

def linkGcc(files, out, flags=LDFLAGS, linker="i686-elf-gcc"):
    CMDLINE = "%s %s %s -o %s -lgcc" % (linker, flags, ' '.join(files), out)
    shell(CMDLINE)
    return out

def clearDir(path):
    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)

clearDir("out")

print("Building FatFS")

FATFS_C_FILES = findAll("third_party/fatfs", "c")

FATFS_OUT_FILES = []

for inp in FATFS_C_FILES:
    out = makeOutputFilename(inp, "c")
    FATFS_OUT_FILES.append(buildC(inp, out, flags=FATFS_CFLAGS))

cmdline = "i686-elf-ar rcs out/libfatfs.a %s" % (' '.join(FATFS_OUT_FILES))
shell(cmdline)

print("Building muzzle - a port of MUSL libc")

MUZZLE_CPP_FILES = findAll("third_party/muzzle/src", "cpp")
MUZZLE_C_FILES = findAll("third_party/muzzle/src", "c")
MUZZLE_ASM_FILES = findAll("third_party/muzzle/src", "s")

MUZZLE_OUT_FILES = []

for inp in MUZZLE_CPP_FILES:
    out = makeOutputFilename(inp, "cpp")
    MUZZLE_OUT_FILES.append(buildCpp(inp, out, flags=MUZZLE_CPPFLAGS))

for inp in MUZZLE_C_FILES:
    out = makeOutputFilename(inp, "c")
    MUZZLE_OUT_FILES.append(buildC(inp, out, flags=MUZZLE_CFLAGS))

for inp in MUZZLE_ASM_FILES:
    out = makeOutputFilename(inp, "s")
    MUZZLE_OUT_FILES.append(buildAsm(inp, out, assembler="i686-elf-gcc", flags=MUZZLE_ASFLAGS))

cmdline = "i686-elf-ar rcs out/libmuzzle.a %s" % (' '.join(MUZZLE_OUT_FILES))
shell(cmdline)

print("Building kernel.elf")

CPP_FILES = findAll("src", "cpp")
C_FILES = findAll("src", "c")
ASM_FILES = findAll("src", "s")

OUT_FILES = []

for inp in CPP_FILES:
    out = makeOutputFilename(inp, "cpp")
    OUT_FILES.append(buildCpp(inp, out))

for inp in C_FILES:
    out = makeOutputFilename(inp, "c")
    OUT_FILES.append(buildC(inp, out))

for inp in ASM_FILES:
    out = makeOutputFilename(inp, "s")
    OUT_FILES.append(buildAsm(inp, out))

OUT_FILES.append("out/libmuzzle.a")
OUT_FILES.append("out/libfatfs.a")

linkGcc(files=OUT_FILES, out="out/kernel.elf")

print("Size of kernel binary: %d bytes" % os.stat("out/kernel.elf").st_size)

print("Building libuserspace")

clearDir("out/modules")

LIBUSERSPACE_OUT_FILES = []
LIBUSERSPACE_CPP_FILES = find("libuserspace/src", "cpp")
LIBUSERSPACE_ASM_FILES = find("libuserspace/src", "s")

for inp in LIBUSERSPACE_CPP_FILES:
    out = makeOutputFilename(inp, "cpp")
    LIBUSERSPACE_OUT_FILES.append(buildCpp(inp, out, flags=LIBUSERSPACE_CFLAGS))

for inp in LIBUSERSPACE_ASM_FILES:
    out = makeOutputFilename(inp, "s")
    LIBUSERSPACE_OUT_FILES.append(buildAsm(inp, out))

LIBUSERSPACE_OUT_FILES.append("out/libmuzzle.a")

cmdline = "i686-elf-ar rcs out/libuserspace.a %s" % (' '.join(LIBUSERSPACE_OUT_FILES))
shell(cmdline)

print("Building userspace apps")

clearDir("out/apps")
clearDir("out/iso/boot/grub")

APP_DIRS = findSubdirectories("apps")
APPS = []
APP_REFS = []
for app in APP_DIRS:
    name = os.path.basename(app)
    cpp = find(app, "cpp")
    outs = []
    for inp in cpp:
        out = makeOutputFilename(inp, "cpp")
        outs.append(buildCpp(inp, out, flags=APP_CFLAGS))
    outs.append("out/libuserspace.a")
    outs.append("out/libmuzzle.a")
    APPS.append(linkGcc(outs, "out/apps/%s" % name, APP_LDFLAGS))
    APP_REFS.append("--file out/apps/%s" % name)

shell("initrd/gen.py --dest out/iso/boot/initrd.img %s" % ' '.join(APP_REFS))
MENU_MODULE_REFS = ["module /boot/initrd.img /initrd"] # add kernel modules here, should any exist

print("Size of initrd image: %d bytes" % os.stat("out/iso/boot/initrd.img").st_size)

menulst = read('build/grub.cfg')
menulst = menulst.replace("${MODULES}", '\n'.join(MENU_MODULE_REFS))
write("out/iso/boot/grub/grub.cfg", menulst)

copy("out/kernel.elf", "out/iso/boot/puppy")

print("Generating os.iso")

CMDLINE = "genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -boot-info-table -o out/os.iso out/iso"
CMDLINE = "grub-mkrescue -o out/os.iso out/iso"
shell(CMDLINE)

print("Size of OS iso image: %d bytes" % os.stat("out/os.iso").st_size)

print("Generating kernel symbol table")

CMDLINE = "nm out/kernel.elf | grep -e ' [BbDdGgSsTtRr] ' | awk '{ print $1 \" \" $3 }' > out/kernel.sym"
shell(CMDLINE)

BUILD_END = time.time()
print("Build took %s seconds" % int(BUILD_END - BUILD_START))
