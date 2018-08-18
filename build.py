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

from datetime import datetime
import glob
import json
from multiprocessing import Pool
import os
import os.path
import shutil
import subprocess
import sys
import time

MYPATH = os.path.abspath(os.getcwd())

print("Building OS image from %s" % MYPATH)

BUILD_START = time.time()

BASIC_CFLAGS = [
    '-c',
    '-fdiagnostics-color=always',
    '-ffreestanding',
    '-fno-builtin',
    '-fno-exceptions',
    '-fno-omit-frame-pointer',
    '-fno-stack-protector',
    '-funsigned-char',
    '-m32',
    '-march=i686',
    '-masm=intel',
    '-nodefaultlibs',
    '-nostartfiles',
    '-nostdlib',
    '-O2',
    '-Wall',
    '-Werror',
    '-Wextra',
    '-Wno-error=format',
    '-Wno-main',
    '-Wno-missing-field-initializers']
BASIC_CPPFLAGS = [
    '-fno-exceptions',
    '-fno-rtti',
    '-std=c++14']
BASIC_ASFLAGS = ["-f elf"]
BASIC_LDFLAGS = ["-ffreestanding", "-nostdlib"]

def findSubdirectories(dir, self=True):
    if self:
        yield dir
    for subdir in os.listdir(dir):
        candidate = os.path.join(dir, subdir)
        if os.path.isdir(candidate):
            yield candidate

def error(msg):
    print("error: %s" % msg)
    shell("umount out/mnt", onerrignore=True)
    raise SystemError # force the subprocesses to exit as brutally as possible

def shell(command, shell=True, stdin=None, printout=False, onerrignore=False):
    if printout: print("$ %s" % command)
    try:
        stdout = subprocess.check_output(command, stdin=stdin, stderr=subprocess.STDOUT, shell=shell)
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

def findAll(base, extension):
    def _find(dir, extension=None):
        if extension:
            if extension[0] != '.':
                extension = '.%s' % extension
        for root, dirs, files in os.walk(dir):
            for file in files:
                if extension is None or file.endswith(extension):
                    yield os.path.join(root, file)

    L = set()
    for subdir in findSubdirectories(base):
        for f in _find(subdir, extension):
            L.add(f)
    return L

def makeOutputFilename(src, prefix=None):
    if prefix is not None: src = "%s_%s" % (prefix, src)
    return os.path.join("out", src.replace("src/","").replace("/","_").replace(".","_")) + ".o"

def copy(src, dst):
    cmdline = 'cp "%s" "%s"' % (src, dst)
    shell(cmdline)
    return dst

def rcopy(src, dst):
    cmdline = 'cp -r "%s" "%s"' % (src, dst)
    shell(cmdline)
    return dst

def read(src):
    with open(src, 'r') as f:
        return f.read()

def write(dst, txt):
    with open(dst, 'w') as f:
        f.write(txt)

def buildAsm(inp, flags=BASIC_ASFLAGS, assembler='nasm'):
    out = makeOutputFilename(inp)
    cmdline = '%s %s %s -o %s' % (assembler, flags, inp, out)
    shell(cmdline)
    return out

def buildCpp(inp, flags=BASIC_CPPFLAGS, compiler="i686-elf-gcc"):
    out = makeOutputFilename(inp)
    cmdline = "%s %s %s -o %s" % (compiler, flags, inp, out)
    shell(cmdline)
    return out

def buildC(inp, flags=BASIC_CFLAGS, compiler="i686-elf-gcc"):
    out = makeOutputFilename(inp)
    cmdline = "%s %s %s -o %s" % (compiler, flags, inp, out)
    shell(cmdline)
    return out

def linkGcc(files, out, flags=BASIC_LDFLAGS, linker="i686-elf-gcc"):
    CMDLINE = "%s %s %s -o %s -lgcc" % (linker, flags, ' '.join(files), out)
    shell(CMDLINE)
    return out

def clearDir(path):
    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)

def makeDir(path):
    if not os.path.isdir(path):
        cmdline = 'mkdir "%s"' % path
        shell(cmdline)
    return path

clearDir("out")
clearDir("out/apps")
clearDir("out/tests")
clearDir("out/mnt")
clearDir("out/iso/boot/grub")

class _BuildC(object):
    def __init__(self, flags):
        self.flags = flags
    
    def __call__(self, x):
        return buildC(x, flags=self.flags)

class _BuildCpp(object):
    def __init__(self, flags):
        self.flags = flags
    
    def __call__(self, x):
        return buildCpp(x, flags=self.flags)

# do not move the definition of the THE_POOL above here; because of how multiprocessing works
# all the things that we want the pooled processes to see and use must be defined before the
# pool itself is defined..
THE_POOL = Pool(5)

class Project(object):
    def __init__(self, name, srcdir, cflags=None, cppflags=None, asmflags=None, ldflags=None, ipaths=None, assembler="nasm", linkerdeps=None, outwhere="out", announce=True):
        self.name = name
        self.srcdir = srcdir
        self.cflags = ' '.join(cflags if cflags else BASIC_CFLAGS)
        self.cppflags = ' '.join(cppflags if cppflags else (BASIC_CFLAGS + BASIC_CPPFLAGS))
        self.asmflags = ' '.join(asmflags if asmflags else BASIC_ASFLAGS)
        self.ldflags = ' '.join(ldflags if ldflags else BASIC_LDFLAGS)
        self.assembler = assembler
        self.ipaths = ipaths if ipaths else ["include"]
        self.linkerdeps = linkerdeps if linkerdeps else []
        self.outwhere = outwhere
        self.announce = announce
        self.cflags = self.cflags + " %s " % ' '.join([" -I%s " % x for x in self.ipaths])
        self.cppflags = self.cppflags + " %s " % ' '.join([" -I%s " % x for x in self.ipaths])

    def findCFiles(self):
        return findAll(self.srcdir, "c")

    def findCPPFiles(self):
        return findAll(self.srcdir, "cpp")

    def findSFiles(self):
        return findAll(self.srcdir, "s")

    def buildCFiles(self):
        return THE_POOL.map(_BuildC(self.cflags), self.findCFiles())

    def buildCPPFiles(self):
        return THE_POOL.map(_BuildCpp(self.cppflags), self.findCPPFiles())

    def buildSFiles(self):
        S_OUTPUT = []
        for inp in self.findSFiles():
            S_OUTPUT.append(buildAsm(inp, flags=self.asmflags, assembler=self.assembler))
        return S_OUTPUT

    def compile(self):
        return self.buildCFiles() + self.buildCPPFiles() + self.buildSFiles()

    def linkAr(self, out):
        destfile = "%s/lib%s.a" % (self.outwhere, self.name.lower())
        shell("i686-elf-ar rcs %s %s" % (destfile, ' '.join(out)))
        return destfile

    def linkGcc(self, out):
        destfile = "%s/%s" % (self.outwhere, self.name.lower())
        out = out + self.linkerdeps
        linkGcc(files=out, flags=self.ldflags, out=destfile)
        return destfile

    def link(self, out):
        pass
    
    def build(self):
        if self.announce: print("Building %s" % self.name)
        BEGIN = time.time()
        DEST = self.link(self.compile())
        END = time.time()
        DURATION = int(END - BEGIN)
        if DURATION > 0:
            if self.announce: print("Duration: %s seconds" % DURATION)
        if self.announce: print("Output size: %s bytes" % (os.stat(DEST).st_size))
        return DEST

class UserspaceTool(Project):
    def __init__(self, name, srcdir, cflags=None, cppflags=None, outwhere="out/apps", stdlib=None, linkerdeps=[], announce=False):
        if stdlib is None:
            if os.path.exists(os.path.join(srcdir, "newlib.use")):
                stdlib = 'newlib'
            else:
                stdlib = 'libuserspace'

        if stdlib == 'libuserspace':
            ipaths = None
            ldflags = BASIC_LDFLAGS + ["-T build/app.ld", "-e__app_entry"]
            ldeps = ["out/libuserspace.a", "out/libmuzzle.a"]
        elif stdlib == 'newlib':
            ipaths=["include", "include/newlib"]
            ldflags = BASIC_LDFLAGS + ["-T build/newlib.ld", "-Wl,-e__start"]
            ldeps=["newlib/lib/crt0.o", "newlib/lib/libc.a", "out/libnewlibinterface.a"]
        else:
            raise ValueError("stdlib should be either libuserspace or newlib")

        ldeps = ldeps + linkerdeps

        Project.__init__(self,
                         name=name,
                         srcdir=srcdir,
                         cflags=cflags,
                         cppflags=cppflags,
                         ldflags=ldflags,
                         ipaths=ipaths,
                         assembler="nasm",
                         linkerdeps=ldeps,
                         outwhere=outwhere,
                         announce=announce)
        self.link = self.linkGcc

FatFS = Project(name="FatFS",
    srcdir="third_party/fatfs",
    assembler="nasm")
FatFS.link = FatFS.linkAr

Muzzle = Project(name="Muzzle",
    srcdir="third_party/muzzle/src",
    asmflags=["-nostartfiles", "-nodefaultlibs", "-Wall", "-Wextra", "-fdiagnostics-color=always", "-nostdlib", "-c"],
    assembler="i686-elf-gcc")
Muzzle.link = Muzzle.linkAr

Kernel = Project(name="Kernel",
    srcdir="kernel/src",
    cflags=BASIC_CFLAGS + ["-mgeneral-regs-only"],
    cppflags=BASIC_CFLAGS + BASIC_CPPFLAGS + ["-mgeneral-regs-only"],
    ldflags=BASIC_LDFLAGS + ["-T build/kernel.ld"],
    assembler="nasm",
    linkerdeps=["out/libmuzzle.a", "out/libfatfs.a"])
Kernel.link = Kernel.linkGcc

Userspace = Project(name="Userspace",
    srcdir="libuserspace/src",
    assembler="nasm",
    linkerdeps=["out/libmuzzle.a"])
Userspace.link = Userspace.linkAr

NewlibInterface = Project(name="NewlibInterface",
    srcdir="newlib/syscalls",
    assembler="nasm",
    ipaths=["include", "include/newlib"])
NewlibInterface.link = NewlibInterface.linkAr

Checkup = Project(name="Checkup",
    srcdir="checkup/src",
    assembler="nasm",
    linkerdeps=["out/libuserspace.a"])
Checkup.link = Checkup.linkAr

FatFS.build()
Muzzle.build()
Kernel.build()
Userspace.build()
NewlibInterface.build()
Checkup.build()

# apps can end up in /initrd and/or /apps in the main filesystem
# this table allows one to configure which apps land where (the default
# being /apps in the main filesystem and not /initrd)
APPS_CONFIG = {
    "out/apps/init"  : {"initrd": True, "mainfs":False},
    "out/apps/mount" : {"initrd": True, "mainfs":False},
    "out/apps/ls"    : {"initrd": True, "mainfs":True},
    "out/apps/klog"  : {"initrd": True, "mainfs":True},
}

APPS = []
TESTS = []

INITRD_REFS = [] # apps for initrd
APP_REFS = ["out/apps/newlibdemo"] # apps for main filesystem
TEST_REFS = []
APP_PROJECTS = []
TEST_PROJECTS = []

USER_CONTENT_BEGIN = time.time()

APP_DIRS = findSubdirectories("apps", self=False)
for app in APP_DIRS:
    app_p = UserspaceTool(name = os.path.basename(app),
                          srcdir = app)
    APP_PROJECTS.append(app_p.name)
    app_o = app_p.build()
    APPS.append(app_o)
    config = APPS_CONFIG.get(app_o, {"initrd": False, "mainfs": True})
    if config["mainfs"]: APP_REFS.append(app_o)
    if config["initrd"]: INITRD_REFS.append(app_o)

TEST_PLAN = {}
KNOWN_FAIL_TESTS = ["tests_mutexkill"]

TEST_DIRS = findSubdirectories("tests", self=False)
for test in TEST_DIRS:
    test_name = test.replace('/','_')
    test_name_define = ' -DTEST_NAME=\\"%s\\" ' % test_name
    test_p = UserspaceTool(name = os.path.basename(test),
                           srcdir = test,
                           cflags = BASIC_CFLAGS + [test_name_define],
                           cppflags = BASIC_CFLAGS + BASIC_CPPFLAGS + [test_name_define],
                           outwhere="out/tests",
                           linkerdeps = ["out/libcheckup.a"])
    TEST_PROJECTS.append(test_p.name)
    test_o = test_p.build()
    TESTS.append(test_o)
    TEST_REFS.append(test_o)
    test_ref = "/system/%s" % (test_o.replace("out/", "")) # this is a bit hacky..
    if test_name not in KNOWN_FAIL_TESTS:
        TEST_PLAN[test_name] = {
            "path" : test_ref,
            "id" : test_name,
            "wait" : "10" # allow tests to wait for more or less than 10 seconds
        }

with open("out/testplan.json", "w") as f:
    json.dump(TEST_PLAN, f)

USER_CONTENT_END = time.time()
USER_CONTENT_DURATION = int(USER_CONTENT_END - USER_CONTENT_BEGIN)

print("Built %d apps (%s) and %d tests (%s) in %s seconds" %
    (len(APP_PROJECTS), ', '.join(APP_PROJECTS),
    len(TEST_PROJECTS), ', '.join(TEST_PROJECTS),
    USER_CONTENT_DURATION))

INITRD_ARGS = ["--file " + x for x in INITRD_REFS]
shell("initrd/gen.py --dest out/iso/boot/initrd.img %s" % ' '.join(INITRD_ARGS))
MENU_MODULE_REFS = ["module /boot/initrd.img /initrd"] # add kernel modules here, should any exist

print("Size of initrd image: %d bytes" % os.stat("out/iso/boot/initrd.img").st_size)

menulst = read('build/grub.cfg')
menulst = menulst.replace("${MODULES}", '\n'.join(MENU_MODULE_REFS))
write("out/iso/boot/grub/grub.cfg", menulst)

copy("out/kernel", "out/iso/boot/puppy")

print("Generating kernel symbol table")

CMDLINE = "nm out/kernel | grep -e ' [BbDdGgSsTtRr] ' | awk '{ print $1 \" \" $3 }' > out/kernel.sym"
shell(CMDLINE)

print("Generating os.img")

# the -D option does not seem to exist on Travis
# but that's OK because in that case a new VM gets spawned
# at every rebuild, so don't fail for that reason
CMDLINE="losetup -D"
shell(CMDLINE, onerrignore=True)

CMDLINE="dd if=/dev/zero of=out/os.img bs=1MB count=64"
shell(CMDLINE)
CMDLINE="fdisk out/os.img"
shell(CMDLINE, stdin=open('build/fdisk.in'))

CMDLINE="losetup --find --show out/os.img"
DISK_LO = shell(CMDLINE).splitlines()[0]

CMDLINE="losetup --offset $((2048*512)) --show --find out/os.img"
PART_LO = shell(CMDLINE).splitlines()[0]

CMDLINE="mkfs.fat -F32 %s" % (PART_LO)
shell(CMDLINE)

CMDLINE="mount -o loop %s out/mnt" % (PART_LO)
shell(CMDLINE)

makeDir("out/mnt/apps")
makeDir("out/mnt/tests")

for app in APP_REFS:
    copy(app, "out/mnt/apps/%s" % os.path.basename(app))

for test in TEST_REFS:
    copy(test, "out/mnt/tests/%s" % os.path.basename(test))

makeDir("out/mnt/config")
copy("build/config/init", "out/mnt/config/init")

anydiff = "0" != shell('git diff HEAD | wc -c | sed "s/ //g"').replace('\n', '')
sysinfo = read('build/config/sysinfo')
sysinfo = sysinfo.replace("${NOW}", datetime.now().__str__())
sysinfo = sysinfo.replace("${GIT-HASH}", shell("git rev-parse HEAD").replace('\n', ''))
sysinfo = sysinfo.replace("${ANY-DIFF}", "Local diff applied" if anydiff else "No diff applied")
sysinfo = sysinfo.replace("${GCC-VERSION}", shell("i686-elf-gcc --version").replace('\n', ''))
sysinfo = sysinfo.replace("${NASM-VERSION}", shell("nasm -v").replace('\n', ''))
write("out/mnt/config/sysinfo", sysinfo)

CMDLINE="grub-install -v --modules=\"part_msdos biosdisk fat multiboot configfile\" --target i386-pc --root-directory=\"%s/out/mnt\" %s" % (MYPATH, DISK_LO)
shell(CMDLINE)

copy("out/kernel", "out/mnt/boot/puppy")
copy("out/iso/boot/initrd.img", "out/mnt/boot/initrd.img")
copy("out/iso/boot/grub/grub.cfg", "out/mnt/boot/grub/grub.cfg")

CMDLINE="df %s/out/mnt -BK --output=used" % (MYPATH)
PART_USAGE = int(shell(CMDLINE).splitlines()[1][0:-1]) * 1024

CMDLINE="umount out/mnt"
shell(CMDLINE)

print("Size of OS disk image: %10d bytes\n                       %10d bytes used" % (os.stat("out/os.img").st_size, PART_USAGE))

BUILD_END = time.time()
print("Build took %s seconds" % int(BUILD_END - BUILD_START))
