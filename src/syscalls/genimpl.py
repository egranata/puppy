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

import json
import os.path
import sys

SYSCALLS_DIR = os.path.split(os.path.abspath(__file__))[0]
TABLE_FILE = os.path.join(SYSCALLS_DIR, "syscalls.tbl")

LIBUSERSPACE_DIR = os.path.abspath(os.path.join(SYSCALLS_DIR, '..', '..', 'libuserspace'))
LIBUSERSPACE_SRC_DIR = os.path.join(LIBUSERSPACE_DIR, 'src')
LIBUSERSPACE_INCLUDE_DIR = os.path.join(LIBUSERSPACE_DIR, 'include')

print('SYSCALLS_DIR = %s' % SYSCALLS_DIR)
print('TABLE_FILE = %s' % TABLE_FILE)
print('LIBUSERSPACE_DIR = %s' % LIBUSERSPACE_DIR)
print('LIBUSERSPACE_SRC_DIR = %s' % LIBUSERSPACE_SRC_DIR)
print('LIBUSERSPACE_INCLUDE_DIR = %s' % LIBUSERSPACE_INCLUDE_DIR)

def error(msg):
    print("error: %s" % msg)
    sys.exit(1)

class SystemCall(object):
    def __init__(self, node):
        self.name = node['name']
        self.argc = node['argc']
        self.number = node['number']
    
    def __str__(self):
        return "%d is %s : syscall%d" % (self.number, self.name, self.argc)
    
    def __repr__(self):
        return self.__str__()

    def argDecls(self):
        A = []
        for i in range(0, self.argc):
            A.append('uint32_t arg%d' % (i+1))
        return ','.join(A)

    def argUsage(self):
        A = ['%s_syscall_id' % (self.name)]
        for i in range(0, self.argc):
            A.append('arg%d' % (i+1))
        return ','.join(A)

    def helperArgDecl(self):
        if self.argc == 0:
            return "SyscallManager::Request&"
        else:
            return "SyscallManager::Request& req"

    def helperArgImpl(self):
        A = []
        for i in range(0, self.argc):
            A.append("req.arg%d" % (i+1))
        return ','.join(A)

    def commonHeader(self):
        return 'syscall_response_t %s_syscall(%s)' % (self.name, self.argDecls())

    def commonConstant(self):
        return 'constexpr uint8_t %s_syscall_id = 0x%x' % (self.name, self.number)

    def commonImpl(self):
        return 'syscall_response_t %s_syscall(%s) {\n\treturn syscall%d(%s);\n}' % (self.name, self.argDecls(), self.argc, self.argUsage())

    def syscallHandlerSet(self):
        return 'handle(%d, %s_syscall_helper)' % (self.number, self.name)

    def syscallHandlerDecl(self):
        return 'extern syscall_response_t %s_syscall_handler(%s)' % (self.name, self.argDecls())

    def syscallHelperDecl(self):
        return 'extern syscall_response_t %s_syscall_helper(%s)' % (self.name, self.helperArgDecl())

    def syscallHelperImpl(self):
        h = "syscall_response_t %s_syscall_helper(%s) {" % (self.name, self.helperArgDecl())
        b = "return %s_syscall_handler(%s)" % (self.name, self.helperArgImpl())
        return "%s\n\t%s;\n}" % (h,b)

def getTable():
    def ascii_encode_dict(data):
        D = {}
        for k,v in data.viewitems():
            if type(k) is unicode: k = k.encode('ascii')
            if type(v) is unicode: v = v.encode('ascii')
            D[k] = v
        return D

    T = []
    ids = set()
    with file(TABLE_FILE, "r") as f:
        table = json.load(f, object_hook=ascii_encode_dict)
        table = table['syscalls']
        for call in table:
            call = SystemCall(call)
            T.append(call)
            if call.number in ids:
                error("duplicate system call id %d" % call.number)
            else:
                ids.add(call.number)
    return T

def genCommonHeader(table):
    with file(os.path.join(LIBUSERSPACE_INCLUDE_DIR, "syscalls.h"), "w") as f:
        f.write("#ifndef LIBUSERSPACE_SYSCALLS\n")
        f.write("#define LIBUSERSPACE_SYSCALLS\n")
        f.write("\n")

        f.write("#include <sys/stdint.h>\n")
        f.write("typedef uint32_t syscall_response_t;\n");
        f.write("\n")

        for syscall in table:
            f.write(syscall.commonHeader()); f.write(";\n")
            f.write(syscall.commonConstant()); f.write(";\n")

        f.write("\n")
        f.write("#endif")

def genCommonImpl(table):
    with file(os.path.join(LIBUSERSPACE_SRC_DIR, "syscalls.cpp"), "w") as f:
        f.write("#include <syscalls.h>\n")
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
    with file(os.path.join(SYSCALLS_DIR, "list.cpp"), "w") as f:
        f.write("#include <syscalls/manager.h>\n")
        f.write("#include <libc/string.h>\n")
        f.write("\n")

        for syscall in table:
            f.write(syscall.syscallHandlerDecl()); f.write(";\n")
            f.write(syscall.syscallHelperDecl()); f.write(";\n")
        f.write("\n")

        f.write("void SyscallManager::sethandlers() {\n")

        for syscall in table:
            f.write('\t%s;\n' % (syscall.syscallHandlerSet()))

        f.write("}\n")
        f.write("\n")

        for syscall in table:
            f.write(syscall.syscallHelperImpl()); f.write("\n")

SYSTEM_CALLS = getTable()
genCommonHeader(SYSTEM_CALLS)
genCommonImpl(SYSTEM_CALLS)
genManagerInit(SYSTEM_CALLS)

print("%d system calls generated" % len(SYSTEM_CALLS))
