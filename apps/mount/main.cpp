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
#include <printf.h>
#include <file.h>
#include <exit.h>
#include <memory.h>
#include <syscalls.h>

static void usage() {
    printf("mount: <device> <path>\n");
    exit(1);
}

int main(int argc, const char** argv) {
    if (argc != 2) {
        usage();
    }
    auto fd = open(argv[0], filemode_t::read);
    if (fd == gInvalidFd) {
        printf("Could not open %s\n", argv[0]);
        exit(1);
    }

    if (argv[1][0] == '/') ++argv[1];

    trymount_syscall(fd, argv[1]);
}
