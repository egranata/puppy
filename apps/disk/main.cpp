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

#include <printf.h>
#include <disk.h>

bool read(const diskinfo& di, unsigned char *buf) {
#if 0
    return (readsector(di, 1, 1, buf) &&
            readsector(di, 2, 1, &buf[512]));
#endif
    return readsector(di, 1, 2, buf);
}

bool write(const diskinfo& di, unsigned char *buf) {
#if 0    
    return (writesector(di, 1, 1, buf) &&
            writesector(di, 2, 1, &buf[512]));
#endif
    return writesector(di, 1, 2, buf);
}

void dump(unsigned char* buf) {
    for (auto i = 0; i < 1024; i+=32) {
        printf("%03x  ", i);
        for (auto j = 0; j < 32; ++j, ++buf) {
            printf("%02x ", *buf);
        }
        printf("\n");
    }
}

int main(int, const char**) {
    unsigned char buf[1024] = {0};
    for (auto i = 0; i < 10; ++i) {
        auto ctrl = getcontroller(i);
        if (ctrl != 0) {
            printf("Found a controller! Controller: %u\n", ctrl);
            for (auto c = 0; c < 2; ++c) {
                for (auto b = 0; b < 2; ++b) {
                    auto di = getdisk(i, c, b);
                    if (di.d.present) {
                        printf("Found a disk! Controller: %u Channel: %u Bus: %u - Disk name '%s', sector count %u\n",
                            ctrl, c, b, di.d.model, di.d.sectors);
                        if (!read(di, buf)) {
                            printf("Failed to read from this disk!\n");
                        } else {
                            buf[0] = 'W';
                            buf[1] = 'e';
                            buf[2] = 'l';
                            buf[3] = 'c';
                            buf[4] = 'o';
                            buf[5] = 'm';
                            buf[6] = 'e';
                            buf[7] = ' ';
                            buf[8] = 't';
                            buf[9] = 'o';
                            buf[10] = ' ';
                            buf[11] = 'P';
                            buf[12] = 'u';
                            buf[13] = 'p';
                            buf[14] = 'p';
                            buf[15] = 'y';
                            ++buf[16];
                            --buf[17];
                            --buf[1020];
                            ++buf[1021];
                            buf[1022] -= 0x55;
                            buf[1023] += 0xAA;
                            write(di, buf);
                            read(di, buf);
                            dump(buf);
                        }
                    }
                }
            }
        }
    }
}
