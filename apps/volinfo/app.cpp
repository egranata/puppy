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
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("%s: %s <path>\n", argv[0], argv[0]);
        printf("print IOCTL information about a given disk volume file.\n");
        exit(1);
    }

    FILE *f = fopen(argv[1], "r");
    if (f == nullptr) {
        printf("error: file name not valid.\n");
        exit(2);
    }

    int fd = fileno(f);

    uint32_t num_sectors = ioctl(fd, (uintptr_t)blockdevice_ioctl_t::IOCTL_GET_NUM_SECTORS, 0);
    uint32_t sector_size = ioctl(fd, (uintptr_t)blockdevice_ioctl_t::IOCTL_GET_SECTOR_SIZE, 0);
    blockdevice_usage_stats_t stats;
    uint32_t ok = ioctl(fd, (uintptr_t)blockdevice_ioctl_t::IOCTL_GET_USAGE_STATS, (uintptr_t)&stats);
    if (num_sectors != (uint32_t)-1 && sector_size != (uint32_t)-1 && ok != 0) {
        printf("Number of sectors:  %u\n", num_sectors);
        printf("Size of a sector:   %u bytes\n", sector_size);
        printf("Volume size:        %llu bytes\n", (uint64_t)num_sectors * sector_size);
        printf("Total sectors read:    %llu (of which %llu are a cache hit)\n", stats.sectors_read, stats.cache_hits);
        if (stats.sectors_read) {
            double ratio = 100.0 * (double)stats.cache_hits / (double)stats.sectors_read;
            printf("                       which makes for a cache hit ratio of: %.2f%%\n", ratio);
        }
        printf("Total sectors written: %llu\n", stats.sectors_written);
    } else {
        printf("error: file may not be a valid volume.\n");
        exit(3);
    }

    return 0;
}
