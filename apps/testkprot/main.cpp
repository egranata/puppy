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
#include <libmuzzle/getopt.h>

int main(int argc, char * const argv[]) {
    int ch = -1;
    bool read = true;
    uintptr_t address = 0;

    while((ch = getopt(argc, argv, "rwa:")) != -1) {
        switch (ch) {
            case 'r':
                read = true;
                break;
            case 'w':
                read = false;
                break;
            case 'a':
                address = atoi(optarg);
                break;
            case '?':
            default:
                exit(1);
        }
    }

    uint32_t *ptr = (uint32_t*)address;

    printf("Will attempt to access %p...", ptr);
    if (read) {
        printf("Address %p is expected to be%sreadable\n", ptr, readable(ptr) ? " " : " not ");
        address = *ptr;
        printf("At address %p, I read %llu\n", ptr, address);
    } else {
        printf("Address %p is expected to be%swritable\n", ptr, writable(ptr) ? " " : " not ");
        *ptr = address;
        printf("To address %p, I wrote %llu\n", ptr, address);
    }

    exit(0);
}
