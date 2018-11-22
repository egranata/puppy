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
from build.chronometer import Chronometer

VERBOSE = "-v" in sys.argv

CLEAN_SLATE = "-keep-out" not in sys.argv
BUILD_CORE = "-keep-kernel" not in sys.argv

APPS_TO_REBUILD = []
for arg in sys.argv:
    if arg.startswith("-rebuild-app="):
        APPS_TO_REBUILD.append(arg.replace("-rebuild-app=", ""))

if not BUILD_CORE: CLEAN_SLATE = False

MYPATH = os.path.abspath(os.getcwd())

if BUILD_CORE:
    print("Build type: Full; OS path: %s" % MYPATH)
else:
    print("Build type: Userland; OS path: %s" % MYPATH)

BUILD_START = time.time()

C_OPTIONS = [
    '-D__puppy__',
    '-fdiagnostics-color=always',
    '-ffreestanding',
    '-fno-builtin',
    '-fno-exceptions',
    '-fno-omit-frame-pointer',
    '-fno-stack-protector',
    '-funsigned-char',
    '-m32',
    '-march=i686',
    '-nostdlib',
    '-O2',
    '-Wall',
    '-Wextra']
BASIC_CFLAGS = [
    '-masm=intel',
    '-Werror',
    '-Wno-error=format',
    '-Wno-missing-field-initializers',
    '-nodefaultlibs',
    '-nostartfiles',
    '-c'] + C_OPTIONS
BASIC_CPPFLAGS = [
    '-fno-exceptions',
    '-fno-rtti',
    '-std=c++14']
BASIC_ASFLAGS = ["-f elf"]
BASIC_LDFLAGS = ["-ffreestanding", "-nostdlib"]

USERSPACE_CFLAGS = ["-c"]
USERSPACE_CPPFLAGS = [""]
USERSPACE_ASFLAGS = ["-f elf"]
USERSPACE_LDFLAGS = [""]

def forEachFile(path, f):
    for fld,x,lst in os.walk(path):
        for nm in lst:
            rp = os.path.join(fld,nm)
            f(rp)

def calculateSize(path):
    totalSize = 0
    def callback(flpt):
        nonlocal totalSize
        totalSize = totalSize + os.stat(flpt).st_size
    forEachFile(path, callback)
    return totalSize

def findSubdirectories(dir, self=True):
    if self:
        yield dir
    for subdir in os.listdir(dir):
        candidate = os.path.join(dir, subdir)
        if os.path.isdir(candidate):
            yield candidate

def error(msg):
    print("error: %s" % msg)
    raise SystemError # force the subprocesses to exit as brutally as possible

def shell(command, shell=True, stdin=None, printout=False, onerrignore=False, curdir=None):
    if printout or VERBOSE: print("$ %s" % command)
    try:
        stdout = subprocess.check_output(command, stdin=stdin, stderr=subprocess.STDOUT, shell=shell, cwd=curdir)
        o = stdout.decode('utf-8')
        if printout or VERBOSE: print(o)
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

def xcopy(src, dst):
    cmdline = 'cp %s "%s"' % (src, dst)
    shell(cmdline)
    return dst

def rcopy(src, dst):
    cmdline = 'cp -Lr "%s" "%s"' % (src, dst)
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

class _BuildC(object):
    def __init__(self, gcc, flags):
        self.gcc = gcc
        self.flags = flags
    
    def __call__(self, x):
        return buildC(x, compiler=self.gcc, flags=self.flags)

class _BuildCpp(object):
    def __init__(self, gcc, flags):
        self.gcc = gcc
        self.flags = flags
    
    def __call__(self, x):
        return buildCpp(x, compiler=self.gcc, flags=self.flags)

# do not move the definition of the THE_POOL above here; because of how multiprocessing works
# all the things that we want the pooled processes to see and use must be defined before the
# pool itself is defined..
THE_POOL = Pool(5)

class Project(object):
    def __init__(self, name, srcdir, cflags=None, cppflags=None, asmflags=None, ldflags=None, ipaths=None, assembler="nasm", linkerdeps=None, outwhere="out", gcc="i686-elf-gcc", announce=True):
        self.name = name
        self.srcdir = srcdir
        self.cflags = ' '.join(cflags if cflags else BASIC_CFLAGS)
        self.cppflags = ' '.join(cppflags if cppflags else (BASIC_CFLAGS + BASIC_CPPFLAGS))
        self.asmflags = ' '.join(asmflags if asmflags else BASIC_ASFLAGS)
        self.ldflags = ' '.join(ldflags if ldflags else BASIC_LDFLAGS)
        self.assembler = assembler
        self.ipaths = ipaths if ipaths is not None else ["include"]
        self.linkerdeps = linkerdeps if linkerdeps else []
        self.outwhere = outwhere
        self.announce = announce
        self.cflags = self.cflags + " %s " % ' '.join([" -I%s " % x for x in self.ipaths])
        self.cppflags = self.cppflags + " %s " % ' '.join([" -I%s " % x for x in self.ipaths])
        self.gcc = gcc if gcc else "i686-elf-gcc"

    def findCFiles(self):
        return findAll(self.srcdir, "c")

    def findCPPFiles(self):
        return findAll(self.srcdir, "cpp")

    def findSFiles(self):
        return findAll(self.srcdir, "s")

    def buildCFiles(self):
        return THE_POOL.map(_BuildC(self.gcc, self.cflags), self.findCFiles())

    def buildCPPFiles(self):
        return THE_POOL.map(_BuildCpp(self.gcc, self.cppflags), self.findCPPFiles())

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
        linkGcc(linker=self.gcc, files=out, flags=self.ldflags, out=destfile)
        return destfile

    def linkDylib(self, out):
        destfile = "%s/%s" % (self.outwhere, self.name.lower())
        shell("i686-elf-ld -shared %s -o %s" % (' '.join(out), destfile))
        return destfile

    def linkCopy(self, out):
        if len(out) != 1:
            raise ValueError("linkCopy can only handle a single file: %s" % out)
        out = out[0]
        destfile = "%s/%s" % (self.outwhere, self.name.lower())
        copy(out, destfile)
        return destfile

    def link(self, out):
        pass

    def build(self):
        chrono = Chronometer("Compiling %s" % self.name if self.announce else None)
        with chrono:
            return self.link(self.compile())

class UserspaceTool(Project):
    def __init__(self, name, srcdir, cflags=None, cppflags=None, outwhere="out/apps", linkerdeps=[], announce=False):
        cflags = USERSPACE_CFLAGS + (cflags if cflags else [])
        cppflags = USERSPACE_CFLAGS + USERSPACE_CPPFLAGS + (cppflags if cppflags else [])
        ipaths=[]
        ldflags = USERSPACE_LDFLAGS
        ldeps=[""]
        gcc="build/gcc.sh"

        ldeps = linkerdeps + ldeps

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
                         gcc=gcc,
                         announce=announce)
        self.link = self.linkGcc

    def hasMakefile(self):
        return os.path.exists(os.path.join(self.srcdir, "Makefile"))
    
    def build(self):
        if not self.hasMakefile(): return Project.build(self)
        guessname = os.path.basename(self.srcdir)
        shell("make V=1 PUPPY_ROOT=%s OUTWHERE=%s CC=%s CXX=%s -j" % (MYPATH, self.outwhere, MY_CC_PATH, MY_CXX_PATH), curdir=self.srcdir)
        return os.path.join(self.outwhere, guessname)

def writeSpecsFile(outfile):
    with open(outfile, "w") as f:
        print("*cpp_unique_options:", file=f)
        specs_include_paths = [os.path.abspath(x) for x in SPECS_INCLUDE_PATHS]
        specs_include_paths = ["-I%s" % x for x in specs_include_paths]
        specs_include_paths = ' '.join(specs_include_paths)
        print("+   -D__puppy__ %s" % specs_include_paths, file=f)
        print("", file=f)
        print("*cc1plus:", file=f)
        cc1plus_options = ' '.join(C_OPTIONS) + ' ' + ' '.join(BASIC_CPPFLAGS)
        print("    %s" % cc1plus_options, file=f)
        print("", file=f)
        print("*cc1_options:", file=f)
        print("    %s" % specs_include_paths, file=f)
        print("", file=f)
        print("*startfile:", file=f)
        print("    %s" % os.path.abspath(NEWLIB_CRT0), file=f)
        print("", file=f)
        print("*lib:", file=f)
        libs = [os.path.abspath(x) for x in NEWLIB_ARS]
        libs = ' '.join(libs)
        print("    %s" % libs, file=f)
        print("", file=f)
        print("*link:", file=f)
        print("    -T %s -e_start -L %s" % (os.path.abspath(USERSPACE_LD_SCRIPT),
                                            os.path.join(MYPATH, "out", "mnt", "libs")), file=f)

def createDiskImage(file, megsOfSize=64):
    rootFile = file
    tempFile = "%s.tmp" % file
    headerFile = "%s.boot" % file
    fatFile = "%s.fat" % file

    megsOfFATVolume = megsOfSize - 1

    # in order for fdisk to be able to fill in the entire disk with our mainfs, it needs to know the actual
    # size of the disk; so create a full size disk only to then truncate it to its first megabyte
    CMDLINE="dd if=/dev/zero of=%s bs=%s count=%s" % (tempFile, 1024*1024, megsOfSize)
    shell(CMDLINE)

    CMDLINE="dd if=/dev/zero of=%s bs=%s count=%s" % (fatFile, 1024*1024, megsOfFATVolume)
    shell(CMDLINE)

    CMDLINE="fdisk %s" % tempFile
    shell(CMDLINE, stdin=open('build/fdisk.in'))

    CMDLINE="mkfs.fat -F32 %s" % (fatFile)
    shell(CMDLINE)

    CMDLINE="dd if=%s of=%s bs=%s count=%s" % (tempFile, headerFile, 1024, 1024)
    shell(CMDLINE)

    CMDLINE="dd if=build/bootsect.0 conv=notrunc bs=1 count=446 of=%s" % (headerFile)
    shell(CMDLINE)

    CMDLINE="dd if=build/bootsect.0 conv=notrunc ibs=1 obs=1 seek=510 skip=510 count=2 of=%s" % (headerFile)
    shell(CMDLINE)

    CMDLINE="grub-mkimage -c build/earlygrub.cfg -O i386-pc -o out/boot.ldr -p /boot/grub part_msdos biosdisk fat multiboot configfile"
    shell(CMDLINE)

    CMDLINE="dd if=out/boot.ldr bs=512 seek=1 of=%s conv=notrunc" % (headerFile)
    shell(CMDLINE)

    os.unlink(tempFile)

    return (rootFile, headerFile, fatFile)

def buildUserlandComponent(name, sourceDir, outWhere, prefixString, beforeBuild=None, afterBuild=None, cflags=None, cppflags=None, linkerdeps=[]):
    component = UserspaceTool(name = name,
                              srcdir = sourceDir,
                              outwhere = outWhere,
                              cflags=cflags,
                              cppflags=cppflags,
                              linkerdeps=linkerdeps)
    if beforeBuild: beforeBuild(component)
    print(' ' * len(prefixString), end='', flush=True)
    print(component.name, end='', flush=True)
    cout = component.build()
    if afterBuild: afterBuild(component, cout)
    print('')

def expandNewlibDeps(deps):
    out = []
    for dep in deps:
        if dep == "${NEWLIB}":
            out += NEWLIB_DEPS
        else:
            out += [dep]
    return out
def expandNewlibIncludes(ipaths):
    out = []
    for ipath in ipaths:
        if ipath == "${NEWLIB}":
            out += ["include", "include/newlib", "include/stl"]
        else:
            out += [ipath]
    return out

NEWLIB_CRT0 = "out/mnt/libs/crt0.o"
NEWLIB_ARS = ["out/mnt/libs/libshellsupport.a",
              "out/mnt/libs/libeastl.a",
              "out/mnt/libs/libcxxsupport.a",
              "out/mnt/libs/liblinenoise.a",
              "out/mnt/libs/libparson.a",
              "out/mnt/libs/libpcre2-posix.a",
              "out/mnt/libs/libpcre2-8.a",
              "out/mnt/libs/libm.a",
              "out/mnt/libs/libc.a",
              "out/mnt/libs/libnewlibinterface.a",
              "out/mnt/libs/libc.a"]
NEWLIB_DEPS = [NEWLIB_CRT0] + NEWLIB_ARS

SPECS_INCLUDE_PATHS = ["out/mnt/include", "out/mnt/include/newlib", "out/mnt/include/stl"]

USERSPACE_LD_SCRIPT = "out/mnt/libs/app.ld"

GCC_SPECS_PATH = "out/mnt/libs/gcc.specs"

MY_CC_PATH = os.path.join(MYPATH, "build", "gcc.sh")
MY_CXX_PATH = os.path.join(MYPATH, "build", "g++.sh")

LIBGCC_FILE = shell("i686-elf-gcc -print-libgcc-file-name").rstrip()

IMG_FILE = "out/os.img"

if CLEAN_SLATE:
    clearDir("out")
    clearDir("out/mnt")

if BUILD_CORE:
    ROOT_DISK, BOOT_DISK, FAT_DISK = createDiskImage(IMG_FILE, megsOfSize=72)
    print("OS disk image parts: %s and %s, which will be combined to produce %s" % (BOOT_DISK, FAT_DISK, ROOT_DISK))
else:
    ROOT_DISK = IMG_FILE
    BOOT_DISK = "%s.boot" % IMG_FILE
    FAT_DISK = "%s.fat" % IMG_FILE

if BUILD_CORE:
    makeDir("out/mnt/apps")
    makeDir("out/mnt/libs")
    makeDir("out/mnt/tests")
    makeDir("out/mnt/include")
    makeDir("out/mnt/libs/python")
    makeDir("out/mnt/boot")
    makeDir("out/mnt/config")

    CORE_PROJECTS = []
    for core_project in json.loads(open("build/core.json", "r").read()):
        assembler = core_project.get('assembler', 'nasm')

        cflags = core_project.get('cflags', BASIC_CFLAGS)
        cflags = cflags + core_project.get('cflagsAgument', [])

        cppflags = core_project.get('cppflags', BASIC_CFLAGS + BASIC_CPPFLAGS)
        cppflags = cppflags + core_project.get('cppflagsAgument', [])

        asmflags = core_project.get('asmflags', BASIC_ASFLAGS)
        asmflags = asmflags + core_project.get('asmflagsAugment', [])

        ldflags = core_project.get('ldflags', BASIC_LDFLAGS)
        ldflags = ldflags + core_project.get('ldflagsAugment', [])

        ipaths = core_project.get('includePaths', ["include"])
        ipaths = expandNewlibIncludes(ipaths)

        linkerdeps = core_project.get('linkerDependencies', [])
        linkerdeps = expandNewlibDeps(linkerdeps)

        project = Project(name = core_project.get('name'),
            srcdir = core_project.get('src'),
            assembler = assembler,
            cflags = cflags,
            cppflags = cppflags,
            asmflags = asmflags,
            ldflags = ldflags,
            ipaths = ipaths,
            linkerdeps = linkerdeps)

        linklogic = core_project.get('linkLogic', 'ar')
        if linklogic == 'ar':   project.link = project.linkAr
        if linklogic == 'copy': project.link = project.linkCopy
        if linklogic == 'gcc':  project.link = project.linkGcc

        CORE_PROJECTS.append(project)

    for project in CORE_PROJECTS:
        project.build()

with Chronometer("Copyings headers and core libraries"):
    rcopy("include", "out/mnt")
    xcopy("third_party/pcre2-10.32/libs/lib*.a", "out/mnt/libs")
    xcopy("out/lib*.a", "out/mnt/libs")
    xcopy("newlib/lib/lib*.a", "out/mnt/libs")
    xcopy("python/*.py", "out/mnt/libs/python")
    copy(LIBGCC_FILE, "out/mnt/libs")
    copy("out/newlibcrt0", NEWLIB_CRT0)
    copy("build/app.ld", "out/mnt/libs")

print("newlib dependency list: %s" % ', '.join(NEWLIB_DEPS))

with Chronometer("Generating GCC specs"):
    writeSpecsFile(GCC_SPECS_PATH)

with Chronometer("Generating kernel symbol table"):
    CMDLINE = "nm out/kernel | grep -e ' [BbDdGgSsTtRr] ' | awk '{ print $1 \" \" $3 }' > out/kernel.sym"
    shell(CMDLINE)

with Chronometer("Copying configuration data"):
    rcopy("config", "out/mnt")
    copy("LICENSE", "out/mnt/config/LICENSE")

    anydiff = "0" != shell('git diff HEAD | wc -c | sed "s/ //g"').replace('\n', '')
    sysinfo = read('config/sysinfo')
    sysinfo = sysinfo.replace("${NOW}", datetime.now().__str__())
    sysinfo = sysinfo.replace("${GIT-HASH}", shell("git rev-parse HEAD").replace('\n', ''))
    sysinfo = sysinfo.replace("${ANY-DIFF}", "Local diff applied" if anydiff else "No diff applied")
    sysinfo = sysinfo.replace("${GCC-VERSION}", shell("i686-elf-gcc --version").replace('\n', ''))
    sysinfo = sysinfo.replace("${NASM-VERSION}", shell("nasm -v").replace('\n', ''))
    write("out/mnt/config/sysinfo", sysinfo)

# apps can end up in /initrd and/or /apps in the main filesystem
# this table allows one to configure which apps land where (the default
# being /apps in the main filesystem and not /initrd)
APPS_CONFIG = {
    "init"  : {"initrd": True},
    "mount" : {"initrd": True},
    "ls"    : {"initrd": True},
}

INITRD_REFS = [] # apps for initrd

with Chronometer("Building apps and tests"):
    DYLIBS_PRINT_PREFIX="Building dynamic libraries: "
    print(DYLIBS_PRINT_PREFIX)

    DYLIB_DIRS = findSubdirectories("dylibs", self=False)
    def markAsDynamic(lib):
        lib.link = lib.linkDylib
    for lib in DYLIB_DIRS:
        buildUserlandComponent(os.path.basename(lib),
                               lib,
                               "out/mnt/libs",
                               DYLIBS_PRINT_PREFIX,
                               beforeBuild = markAsDynamic)
    print('')

    APPS_PRINT_PREFIX="Building apps: "
    print(APPS_PRINT_PREFIX)

    APP_DIRS = findSubdirectories("apps", self=False)
    def needsInitrd(app, app_out):
        config = APPS_CONFIG.get(app.name, {"initrd": False})
        if config["initrd"]: INITRD_REFS.append(app_out)
    for app in APP_DIRS:
        bn = os.path.basename(app)
        if len(APPS_TO_REBUILD) == 0 or bn in APPS_TO_REBUILD or bn in APPS_CONFIG:
            buildUserlandComponent(bn,
                                   app,
                                   "out/mnt/apps",
                                   APPS_PRINT_PREFIX,
                                   afterBuild=needsInitrd)
    print('')

    TEST_PLAN = []

    TEST_PRINT_PREFIX="Building tests: "
    print(TEST_PRINT_PREFIX)

    TEST_DIRS = findSubdirectories("tests", self=False)
    def pushToTestPlan(test, test_out):
        test_ref = "/system/%s" % (test_out.replace("out/mnt/", ""))
        TEST_PLAN.append({
            "path" : test_ref,
            "id" : test.name,
            "wait" : "15" # allow tests to wait for more or less than 15 seconds
        })
    for test in TEST_DIRS:
        test_name = os.path.basename(test)
        test_name_define = ' -DTEST_NAME=\\"%s\\" ' % test_name
        if len(APPS_TO_REBUILD) == 0 or test_name in APPS_TO_REBUILD:
            buildUserlandComponent(test_name,
                                test,
                                "out/mnt/tests",
                                TEST_PRINT_PREFIX,
                                afterBuild=pushToTestPlan,
                                cflags = [test_name_define],
                                cppflags = [test_name_define],
                                linkerdeps = ["out/mnt/libs/libcheckup.a"])

    # provide a consistent sort order for test execution regardless of underlying FS
    TEST_PLAN.sort(key=lambda test: test["id"])

    with open("out/testplan.json", "w") as f:
        json.dump(TEST_PLAN, f)

    with open("out/mnt/tests/runall.sh", "w") as testScript:
        print("#!/system/apps/shell", file=testScript)
        for test in TEST_PLAN:
                print("%s" % test['path'], file=testScript)

    print('')

with Chronometer("Configuring bootloader"):
    INITRD_ARGS = ["--file " + x for x in INITRD_REFS]
    shell("initrd/gen.py --dest out/mnt/boot/initrd.img %s" % ' '.join(INITRD_ARGS))
    MENU_MODULE_REFS = ["module /boot/initrd.img /initrd"] # add kernel modules here, should any exist

    print("Size of initrd image: %d bytes" % os.stat("out/mnt/boot/initrd.img").st_size)

    rcopy("build/grub", "out/mnt/boot")
    copy("out/kernel", "out/mnt/boot/puppy")

    menulst = read('build/grub.cfg')
    menulst = menulst.replace("${MODULES}", '\n'.join(MENU_MODULE_REFS))
    write("out/mnt/boot/grub/grub.cfg", menulst)

with Chronometer("Building final disk image"):
    CMDLINE="mcopy -D overwrite -s -i %s out/mnt/* ::/" % (FAT_DISK)
    shell(CMDLINE)

    PART_USAGE = calculateSize("out/mnt")

    CMDLINE="build/concatimg.sh %s %s %s" % (ROOT_DISK, BOOT_DISK, FAT_DISK)
    shell(CMDLINE)

    print("Size of OS disk image: %10d bytes\n                       %10d bytes used" % (os.stat(ROOT_DISK).st_size, PART_USAGE))

BUILD_END = time.time()
print("Build took %s seconds" % int(BUILD_END - BUILD_START))
