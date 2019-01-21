// Copyright 2019 Google LLC
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <kernel/syscalls/types.h>

static void print(const irq_info_t& entry, size_t len) {
    if (entry.count == 0) return;

    printf("%-3.3u  %-*.*s  %14llu\n",
        entry.id, len + 2, len + 2, entry.description, entry.count);
}

static int print(irq_info_t* table, size_t N = 256) {
    size_t max_len = 0;
    for (size_t i = 0; i < N; ++i) {
        size_t len = strlen(table[i].description);
        if (len > max_len) max_len = len;
    }

    for (size_t i = 0; i < N; ++i) {
        print(table[i], max_len);
    }
}

int main(int argc, char** argv) {
    FILE* f = fopen("/devices/irqs", "r");
    if (f == nullptr) {
        printf("%s: can't open IRQ info file\n", argv[0]);
        exit(1);
    }

    irq_info_t *table = (irq_info_t*)mmap(nullptr, 256 * sizeof(irq_info_t), PROT_READ, MAP_PRIVATE, fileno(f), 0);
    if (table == nullptr) {
        printf("%s: can't map IRQ table in memory\n", argv[0]);
        exit(1);
    }

    return print(table);
}