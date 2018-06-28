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

import json, os, subprocess, time, sys

CMDLINE='qemu-system-i386 -drive format=raw,media=disk,file=out/os.img -display none -serial file:out/kernel.log -m 768 -d guest_errors ' + \
        '-rtc base=localtime -monitor stdio -smbios type=0,vendor="Puppy" -smbios type=1,manufacturer="Puppy",product="Puppy System",serial="P0PP1"'

def ensureGone(path):
    try:
        os.remove(path)
    except:
        pass

def spawn():
    return subprocess.Popen(CMDLINE, shell=True, stdin=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=os.getcwd())

def readLog():
    with open('out/kernel.log') as f:
        return f.read()

def checkAlive():
    log = readLog()
    return log.find("init is up and running") > 0

def checkTestPass(tid):
    log = readLog()
    return log.find("TEST[%s] PASS" % tid) > 0

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
        else:
            say(qemu, "sendkey %s" % c)
    say(qemu, "sendkey kp_enter")
    qemu.stdin.flush()

ensureGone("out/kernel.log")

qemu = spawn()

time.sleep(10)

if checkAlive():
    print('\n\nPuppy booted!')
else:
    print("\n\nerror: no OS alive")
    sys.exit(1)

TEST_PLAN = json.load(open(sys.argv[1], "r"))
print(TEST_PLAN)

for tid in TEST_PLAN:
    TEST = TEST_PLAN[tid]
    path = "&%s" % TEST["path"]
    wait = int(TEST["wait"])
    sendString(qemu, path)
    time.sleep(wait)
    if checkTestPass(tid):
        print("\n\%s test passed!" % tid)
    else:
        print("\n\n%s test not passed" % tid)
        sys.exit(1)


say(qemu, "q")
qemu.communicate()
