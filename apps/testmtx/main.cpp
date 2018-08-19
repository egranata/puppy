// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdint.h>
#include <libuserspace/printf.h>
#include <libuserspace/exit.h>
#include <libuserspace/mutex.h>
#include <libuserspace/sleep.h>
#include <libuserspace/getpid.h>
#include <muzzle/string.h>

void error() {
    printf("syntax: testmutex <path>\n");
    exit(1);
}

int main(int argc, const char** argv) {
    if (argc != 2) error();

    auto p = getpid();

    Mutex mtx(argv[1]);
    printf("pid %u acquired mutex %s, handle is %x\n", p, argv[1], mtx.handle());
    mtx.lock();
    printf("pid %u locked mutex!\n", p);
    sleep(12000);
    mtx.unlock();
    printf("pid %u unlocked mutex!\n", p);

    return 0;
}
