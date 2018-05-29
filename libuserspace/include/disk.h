/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBUSERSPACE_DISK
#define LIBUSERSPACE_DISK

#include <sys/stdint.h>

// must be binary identical to diskid_t in ide.h
struct disk {
    uint8_t channel;
    uint8_t bus;
    bool present;
    uint8_t model[41];
    uint32_t sectors;
    uint16_t iobase;
};

typedef uint32_t controller;

struct diskinfo {
    controller ctrl;
    disk d;
};

controller getcontroller(uint8_t n);
diskinfo getdisk(controller ctrl, uint8_t channel, uint8_t bus);

bool readsector(const diskinfo& di, uint32_t sec0, uint8_t count, unsigned char* buffer);
bool writesector(const diskinfo& di, uint32_t sec0, uint8_t count, unsigned char* buffer);

#endif
