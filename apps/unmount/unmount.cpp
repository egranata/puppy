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

#include <syscalls.h>
#include <stdio.h>
#include <stdlib.h>

static void usage(const char* name) {
    printf("%s: <path>\n", name);
    exit(1);
}

int main(int argc, const char** argv) {
    if (argc != 2) {
        usage(argv[0]);
    }

    const char* mountpoint = argv[1];

    return unmount_syscall(mountpoint); // returns 0 on success
}
