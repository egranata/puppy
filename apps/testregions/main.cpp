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

#include <libuserspace/printf.h>
#include <libuserspace/memory.h>
#include <libuserspace/exit.h>
#include <libuserspace/sysinfo.h>

void test(uint32_t* ptr, uint32_t value) {
    *ptr = value;
    printf("At address %p, I wrote %x\n", ptr, *ptr);    
}

void stats() {
    auto si = sysinfo(false, true);
    printf("This process has allocated %u bytes (%u of which are real RAM) and caused %u pagefaults so far\n", si.local.allocated, si.local.committed, si.local.pagefaults);
}

void testCustomRegion() {
    auto rgn = mapregion(1024 * 1024, true);
    if (rgn) {
        printf("Found and mapped a region of 1MB in size starting at %p\n", rgn);
    } else {
        printf("Failed to get 1MB of RAM.. that can't be good\n");
        exit(1);
    }

    uint32_t *mem = (uint32_t*)rgn;

    test(mem + 0, 0xBEEFF00D);
    test(mem + 1025, 0xF00DDEAD);

    stats();

    test(mem + 1025 * 1024, 0xB00BB00B); // this will fail, so the following call will *not* happen
    stats();
}

void testDealloc() {
    uint8_t* rgn = (uint8_t*)mapregion(128 * 1024, true);
    if (rgn) {
        printf("Found 128KB of memory at %p\n", rgn);
        unmapregion((uint32_t)(rgn + 1));
        if (readable(rgn + 1)) {
            printf("After unmapping, memory is expected readable - try...");
            printf("%p = %d\n", rgn, *rgn);
        } else {
            printf("After unmapping, memory is not readable!\n");
        }
    }
}

void testMalloc() {
    malloc(3 * 1024 * 1024);
    uint32_t *p1 = (uint32_t*)malloc(4);
    malloc(4 * 1024 * 1024);
    uint32_t *p2 = (uint32_t*)malloc(4);
    malloc(10 * 1024 * 1024);
    uint32_t *p3 = (uint32_t*)malloc(4);

    test(p1, 0xBEEFF00D);
    test(p2, 0xF00DDEAD);
    test(p3, 0xB00BB00B);

    stats();
}

int main(int, const char**) {
    return testMalloc(), testDealloc(), testCustomRegion(), 0;
}

