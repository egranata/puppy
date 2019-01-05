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

import json, os, subprocess, time, sys, tempfile

CMDLINE='qemu-system-i386 -drive format=raw,media=disk,file=out/os.img -display none -serial file:out/kernel.log -m 768 -d guest_errors ' + \
        '-rtc base=utc -monitor stdio -smbios type=0,vendor="Puppy" -smbios type=1,manufacturer="Puppy",product="Puppy System",serial="P0PP1" ' + \
        '-k en-us -cpu n270'

MAX_TEST_LEN = 0
MAX_TEST_WAIT = 0

def ensureGone(path):
    try:
        os.remove(path)
    except:
        pass

def spawn():
    fout = tempfile.TemporaryFile()
    ferr = tempfile.TemporaryFile()
    return subprocess.Popen(CMDLINE, shell=True, stdin=subprocess.PIPE, stdout=fout, stderr=ferr, cwd=os.getcwd())

def readLog():
    with open('out/kernel.log') as f:
        return f.read()

def checkAlive():
    log = readLog()
    return log.find("init is up and running") > 0

def checkTestPass(tid):
    log = readLog()
    return log.find("TEST[%s] PASS" % tid) > 0

def checkTestFail(tid):
    log = readLog()
    return log.find("TEST[%s] FAIL" % tid) > 0

def say(qemu, msg):
    qemu.stdin.write(bytes("%s\n" % msg, 'ascii'))

def sendString(qemu, s):
    for c in s:
        if c == '/':
            say(qemu, "sendkey slash")
        elif c == '&':
            say(qemu, "sendkey shift-7")
        elif c == '\n':
            say(qemu, "sendkey kp_enter")
        elif c == '.':
            say(qemu, "sendkey dot")
        elif c == ' ':
            say(qemu, "sendkey spc")
        else:
            say(qemu, "sendkey %s" % c)
    say(qemu, "sendkey kp_enter")
    qemu.stdin.flush()

def quit(qemu, ec):
    say(qemu, "q")
    qemu.communicate()
    sys.exit(ec)

def waitFor(testid, deadline, passed, failed):
    print("%-*s   [" % (MAX_TEST_LEN+1, testid), end='', flush=True)
    expired = 0
    result = None
    while expired < deadline:
        time.sleep(1)
        print('.', end='', flush=True)
        expired = expired + 1
        residual = (' ' * (deadline-expired))
        if passed():
            print("%s] PASS (in %3d of %3d seconds)" % (residual, expired, deadline))
            result = True
            break
        if failed and failed():
            print("%s] FAIL (in %3d of %3d seconds)" % (residual, expired, deadline))
            result = False
            break
    if result: return result
    if result is None: print("] TIMEOUT")
    print(readLog())
    quit(qemu, 1)

ensureGone("out/kernel.log")
qemu = spawn()

TEST_PLAN = json.load(open(sys.argv[1], "r"))

for TEST in TEST_PLAN:
    tid = TEST["id"]
    wait = int(TEST["wait"])
    MAX_TEST_LEN = max(MAX_TEST_LEN, len(tid))
    MAX_TEST_WAIT = max(MAX_TEST_WAIT, wait)

waitFor("Boot", MAX_TEST_WAIT, checkAlive, None)

sendString(qemu, "/system/tests/runall.sh")

for TEST in TEST_PLAN:
    wait = int(TEST["wait"])
    tid = TEST["id"]
    waitFor(tid, wait, lambda: checkTestPass(tid), lambda: checkTestFail(tid))

quit(qemu, 0)
