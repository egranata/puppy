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

import json
import os.path
import sys

SYSCALLS_DIR = os.path.split(os.path.abspath(__file__))[0]
TABLE_FILE = os.path.join(SYSCALLS_DIR, "syscalls.tbl")

OS_ROOT_DIR = os.path.abspath(os.path.join(SYSCALLS_DIR, '..', '..', '..'))
LIBUSERSPACE_DIR = os.path.abspath(os.path.join(OS_ROOT_DIR, 'libuserspace'))
LIBUSERSPACE_SRC_DIR = os.path.join(LIBUSERSPACE_DIR, 'src')
LIBUSERSPACE_INCLUDE_DIR = os.path.join(LIBUSERSPACE_DIR, 'include')
NEWLIB_INCLUDE_DIR = os.path.join(OS_ROOT_DIR, "newlib", "include")
NEWLIB_SYSCALLS_DIR = os.path.join(OS_ROOT_DIR, "newlib", "syscalls")

print('SYSCALLS_DIR = %s' % SYSCALLS_DIR)
print('TABLE_FILE = %s' % TABLE_FILE)
print('OS_ROOT_DIR = %s' % OS_ROOT_DIR)
print('LIBUSERSPACE_DIR = %s' % LIBUSERSPACE_DIR)
print('LIBUSERSPACE_SRC_DIR = %s' % LIBUSERSPACE_SRC_DIR)
print('LIBUSERSPACE_INCLUDE_DIR = %s' % LIBUSERSPACE_INCLUDE_DIR)
print('NEWLIB_INCLUDE_DIR = %s' % NEWLIB_INCLUDE_DIR)
print('NEWLIB_SYSCALLS_DIR = %s' % NEWLIB_SYSCALLS_DIR)

def error(msg):
    print("error: %s" % msg)
    sys.exit(1)

class Tee(object):
    def __init__(self, f1, f2):
        self.f1 = f1
        self.f2 = f2
    def write(self, x):
        self.f1.write(x)
        self.f2.write(x)
    def flush(self):
        self.f1.flush()
        self.f2.flush()
    def close(self):
        self.f1.close()
        self.f2.close()
    def __enter__(self):
        return self
    def __exit__(self, type, value, traceback):
        self.close()

class SystemCall(object):
    def __init__(self, node):
        self.name = node['name']
        self.number = node['number']
        if 'argtypes' in node:
            self.argtypes = node['argtypes']
            self.argc = len(self.argtypes)
        else:
            self.argc = node['argc']
            self.argtypes = ['uint32_t'] * self.argc
        self.system = ('system' in node)
        self.autohelper = ('helper' not in node)
        if self.system:
            self.systemwhy = "/** %s */" % (node['system'])
        else:
            self.systemwhy = ""

    def __str__(self):
        return "%d is %s : syscall%d" % (self.number, self.name, self.argc)
    
    def __repr__(self):
        return self.__str__()

    def argNames(self):
        A = []
        for i in range(0, self.argc):
            A.append('arg%d' % (i+1))
        return ','.join(A)

    def argDecls(self):
        A = []
        for i in range(0, self.argc):
            A.append('%s arg%d' % (self.argtypes[i], (i+1)))
        return ','.join(A)

    def handlerMacro(self):
        return "#define %s_HANDLER(%s) syscall_response_t %s_syscall_handler(%s)" % (self.name, self.argNames(), self.name, self.argDecls())

    def argUsage(self):
        A = ['%s_syscall_id' % (self.name)]
        for i in range(0, self.argc):
            A.append('(uint32_t)arg%d' % (i+1))
        return ','.join(A)

    def helperArgDecl(self):
        if self.argc == 0:
            return "SyscallManager::Request&"
        else:
            return "SyscallManager::Request& req"

    def helperArgImpl(self):
        A = []
        for i in range(0, self.argc):
            A.append("(%s)req.arg%d" % (self.argtypes[i], (i+1)))
        return ','.join(A)

    def genTypeSafeChecks(self):
        A = []
        for at in self.argtypes:
            A.append('static_assert(sizeof(%s) <= sizeof(uint32_t), "type is not safe to pass in a register");' % at)
        return "\n".join(A)

    def commonHeader(self):
        return 'syscall_response_t %s_syscall(%s)' % (self.name, self.argDecls())

    def commonConstant(self):
        return 'constexpr uint8_t %s_syscall_id = 0x%x' % (self.name, self.number)

    def commonImpl(self):
        return 'syscall_response_t %s_syscall(%s) {\n\treturn syscall%d(%s);\n}' % (self.name, self.argDecls(), self.argc, self.argUsage())

    def syscallHandlerSet(self):
        return 'handle(%d, %s_syscall_helper, %s); %s' % (self.number, self.name, 'true' if self.system else 'false', self.systemwhy)

    def syscallHandlerDecl(self):
        return 'extern syscall_response_t %s_syscall_handler(%s)' % (self.name, self.argDecls())

    def syscallHelperDecl(self):
        return 'extern syscall_response_t %s_syscall_helper(%s)' % (self.name, self.helperArgDecl())

    def syscallHelperImpl(self):
        h = "syscall_response_t %s_syscall_helper(%s) {" % (self.name, self.helperArgDecl())
        b = "return %s_syscall_handler(%s)" % (self.name, self.helperArgImpl())
        return "%s\n\t%s;\n}" % (h,b)

def getTable():
    T = []
    ids = set()
    with open(TABLE_FILE, "r") as f:
        table = json.load(f)
        table = table['syscalls']
        for call in table:
            call = SystemCall(call)
            T.append(call)
            if call.number in ids:
                error("duplicate system call id %d" % call.number)
            else:
                ids.add(call.number)
    return T

def printLicense(f, autogen=True):
    f.write("// Copyright 2018 Google LLC\n")
    f.write("//\n")
    f.write('// Licensed under the Apache License, Version 2.0 (the "License");\n')
    f.write("// you may not use this file except in compliance with the License.\n")
    f.write("// You may obtain a copy of the License at\n")
    f.write("//\n")
    f.write("//      http://www.apache.org/licenses/LICENSE-2.0\n")
    f.write("//\n")
    f.write("// Unless required by applicable law or agreed to in writing, software\n")
    f.write('// distributed under the License is distributed on an "AS IS" BASIS,\n')
    f.write("// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n")
    f.write("// See the License for the specific language governing permissions and\n")
    f.write("// limitations under the License.\n")
    f.write("\n")
    if autogen:
        f.write("// autogenerated by syscalls/genimply.do - DO NOT EDIT manually\n\n")

def genCommonHeader(table):
    f1 = os.path.join(LIBUSERSPACE_INCLUDE_DIR, "syscalls.h")
    f2 = os.path.join(NEWLIB_INCLUDE_DIR, "syscalls.h")
    with Tee(open(f1, "w"), open(f2, "w")) as f:
        printLicense(f)
        f.write("#ifndef LIBUSERSPACE_SYSCALLS\n")
        f.write("#define LIBUSERSPACE_SYSCALLS\n")
        f.write("\n")

        f.write("#include <kernel/syscalls/types.h>\n")
        f.write("\n")

        for syscall in table:
            f.write(syscall.commonHeader()); f.write(";\n")
            f.write(syscall.commonConstant()); f.write(";\n")

        f.write("\n")
        f.write("#endif")

def genCommonImpl(table):
    f1 = os.path.join(LIBUSERSPACE_SRC_DIR, "syscalls.cpp")
    f2 = os.path.join(NEWLIB_SYSCALLS_DIR, "syscalls.cpp")
    with Tee(open(f1, "w"), open(f2, "w")) as f:
        printLicense(f)        
        f.write("#include <libuserspace/syscalls.h>\n")
        f.write("\n")

        f.write('extern "C"\n')
        f.write("syscall_response_t syscall0(uint8_t);\n\n")

        f.write('extern "C"\n')
        f.write("syscall_response_t syscall1(uint8_t, uint32_t);\n\n")

        f.write('extern "C"\n')
        f.write("syscall_response_t syscall2(uint8_t, uint32_t, uint32_t);\n\n")

        f.write('extern "C"\n')
        f.write("syscall_response_t syscall3(uint8_t, uint32_t, uint32_t, uint32_t);\n\n")

        f.write('extern "C"\n')
        f.write("syscall_response_t syscall4(uint8_t, uint32_t, uint32_t, uint32_t, uint32_t);\n\n")

        for syscall in table:
            f.write(syscall.commonImpl()); f.write("\n")

def genManagerInit(table):
    with open(os.path.join(SYSCALLS_DIR, "list.cpp"), "w") as f:
        printLicense(f)
        f.write("#include <kernel/syscalls/manager.h>\n")
        f.write("#include <kernel/syscalls/types.h>\n")
        f.write("#include <kernel/libc/string.h>\n")
        f.write("\n")

        for syscall in table:
            if syscall.autohelper: f.write(syscall.syscallHandlerDecl()); f.write(";\n")
            f.write(syscall.syscallHelperDecl()); f.write(";\n")
        f.write("\n")

        f.write("void SyscallManager::sethandlers() {\n")

        for syscall in table:
            f.write('\t%s\n' % (syscall.syscallHandlerSet()))

        f.write("}\n")
        f.write("\n")

        for syscall in table:
            if syscall.autohelper:
                f.write(syscall.syscallHelperImpl()); f.write("\n")
                f.write(syscall.genTypeSafeChecks()); f.write("\n")
            f.write("\n")

SYSTEM_CALLS = getTable()
genCommonHeader(SYSTEM_CALLS)
genCommonImpl(SYSTEM_CALLS)
genManagerInit(SYSTEM_CALLS)

print("%d system calls generated" % len(SYSTEM_CALLS))
