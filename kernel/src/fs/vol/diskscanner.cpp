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

#include <kernel/fs/vol/diskscanner.h>
#include <kernel/log/log.h>
#include <kernel/fs/vol/ptable.h>
#include <kernel/fs/fsidents.h>

IDEDiskScanner::IDEDiskScanner(IDEController* ide) : mDiskController(ide) {}

uint32_t IDEDiskScanner::parseDisk(uint8_t channel, uint8_t bus) {
    uint32_t cnt = 0;

    LOG_DEBUG("trying to find a disk on ch=%u bus=%u", channel, bus);

    IDEController::disk_t dsk((IDEController::channelid_t)channel, (IDEController::bus_t)bus);
    if (mDiskController->fill(dsk)) {
        x86_mbr_t mbr;
        if (mDiskController->read(dsk, 0, 1, (uint8_t*)&mbr)) {
            cnt += parse(dsk, mbr);
        }
    }

    return cnt;
}

uint32_t IDEDiskScanner::parseAllDisks() {
    uint32_t cnt = 0;
    for (auto channel = 0; channel < 2; ++channel) {
        for (auto bus = 0; bus < 2; ++bus) {
            cnt += parseDisk(channel, bus);
        }
    }

    return cnt;
}

uint32_t IDEDiskScanner::parse(const IDEController::disk_t& dsk, const x86_mbr_t& mbr) {
    uint32_t cnt = 0;

    // TODO: extended partitions
    cnt += parse(dsk, mbr.partition0);
    cnt += parse(dsk, mbr.partition1);
    cnt += parse(dsk, mbr.partition2);
    cnt += parse(dsk, mbr.partition3);

    return cnt;
}

IDEController* IDEDiskScanner::controller() const {
    return mDiskController;
}

uint32_t IDEDiskScanner::parse(const IDEController::disk_t& dsk, const diskpart_t& dp) {
    if (dp.sysid == 0 || dp.size == 0) {
        LOG_DEBUG("empty partition found - ignoring");
        return 0;
    }
    const char* parttype = "<unknown>";
    for (auto i = 0; true; ++i) {
        auto&& fsinfo = gKnownFilesystemTypes[i];
        if (fsinfo.sysid == 0 && fsinfo.type == nullptr) break;
        if (fsinfo.sysid == dp.sysid) {
            parttype = fsinfo.type;
            break;
        }
    }

    IDEVolume* vol = new IDEVolume(mDiskController, nullptr, dsk, dp);

    LOG_DEBUG("found partition of type %s (%u) - starts at sector %u and spans %u sectors - storing as 0x%p",
        parttype, dp.sysid, dp.sector, dp.size, vol);
    
    return mVolumes.add(vol), 1;
}

IDEDiskScanner::VolumesIterator IDEDiskScanner::begin() {
    return mVolumes.begin();
}
IDEDiskScanner::VolumesIterator IDEDiskScanner::end() {
    return mVolumes.end();
}

void IDEDiskScanner::clear() {
    return mVolumes.clear();
}
