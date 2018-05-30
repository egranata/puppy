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

#include <libmuzzle/string.h>
#include <libmuzzle/stdlib.h>
#include <exit.h>
#include <memory.h>
#include <printf.h>
#include <stdint.h>

int main(int argc, const char** argv) {
    if (argc < 2) {
        printf("testkprot <r|w> address");
        exit(1);
    }

    auto addr = atoi(argv[1]);
    uint32_t *ptr = (uint32_t*)addr;

    printf("Will attempt to access %p...", ptr);

    if (0 == strcmp(argv[0], "r")) {
        // read
        printf("Address %p is expected to be%sreadable\n", ptr, readable(ptr) ? " " : " not ");
        addr = *ptr;
        printf("At address %p, I read %llu\n", ptr, addr);
    } else {
        // write
        printf("Address %p is expected to be%swritable\n", ptr, writable(ptr) ? " " : " not ");
        *ptr = addr;
        printf("To address %p, I wrote %llu\n", ptr, addr);
    }

    exit(0);
}
