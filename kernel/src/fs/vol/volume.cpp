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

#include <kernel/fs/vol/volume.h>
#include <kernel/process/current.h>
#include <kernel/log/log.h>

Volume::Volume() = default;

Volume::~Volume() = default;

bool Volume::read(uint32_t sector, uint16_t count, unsigned char* buffer) {
    bool ok = doRead(sector, count, buffer);
    if (ok) readAccounting(count);
    return ok;
}
bool Volume::write(uint32_t sector, uint16_t count, unsigned char* buffer) {
    bool ok = doWrite(sector, count, buffer);
    if (ok) writeAccounting(count);
    return ok;
}

void Volume::readAccounting(uint16_t sectors) {
    if (gCurrentProcess) {
        uint64_t totalCount = (uint64_t)sectors * (uint64_t)sectorsize();
        gCurrentProcess->iostats.read += totalCount;
    }
}
void Volume::writeAccounting(uint16_t sectors) {
    if (gCurrentProcess) {
        uint64_t totalCount = (uint64_t)sectors * (uint64_t)sectorsize();
        gCurrentProcess->iostats.written += totalCount;
    }
}
