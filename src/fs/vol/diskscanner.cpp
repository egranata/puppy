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

#include <fs/vol/diskscanner.h>
#include <log/log.h>
#include <fs/vol/ptable.h>

DiskScanner::DiskScanner(IDEController* ide) : mDiskController(ide) {
    if (mDiskController) {
        for (auto channel = 0; channel < 2; ++channel) {
            for (auto bus = 0; bus < 2; ++bus) {
                LOG_DEBUG("trying to find a disk on ch=%u bus=%u", channel, bus);
                IDEController::disk_t dsk((IDEController::channelid_t)channel, (IDEController::bus_t)bus);
                if (ide->fill(dsk)) {
                    unsigned char buffer[512] = {0};
                    if (ide->read(dsk, 0, 1, buffer)) {
                        parse(dsk, &buffer[0]);
                    }
                }
            }
        }
    }
}

void DiskScanner::parse(const IDEController::disk_t& dsk, unsigned char* sector0) {
    // TODO: extended partitions
    diskpart_t* p0 = (diskpart_t*)&sector0[diskpart_t::gPartitionOffset(0)];
    diskpart_t* p1 = (diskpart_t*)&sector0[diskpart_t::gPartitionOffset(1)];
    diskpart_t* p2 = (diskpart_t*)&sector0[diskpart_t::gPartitionOffset(2)];
    diskpart_t* p3 = (diskpart_t*)&sector0[diskpart_t::gPartitionOffset(3)];

    parse(dsk, p0);
    parse(dsk, p1);
    parse(dsk, p2);
    parse(dsk, p3);
}

IDEController* DiskScanner::controller() const {
    return mDiskController;
}

void DiskScanner::parse(const IDEController::disk_t& dsk, diskpart_t *dp) {
    if (dp->sysid == 0 || dp->size == 0) {
        LOG_DEBUG("empty partition found - ignoring");
        return;
    }
    const char* parttype = "<unknown>";
    for (auto i = 0; true; ++i) {
        auto&& fsinfo = gKnownFilesystemTypes[i];
        if (fsinfo.sysid == 0 && fsinfo.type == nullptr) break;
        if (fsinfo.sysid == dp->sysid) {
            parttype = fsinfo.type;
            break;
        }
    }

    Volume* vol = new Volume(mDiskController, dsk, *dp);

    LOG_DEBUG("found partition of type %s (%u) - starts at sector %u and spans %u sectors - storing as %p",
        parttype, dp->sysid, dp->sector, dp->size, vol);
    
    mVolumes.add(vol);
}

DiskScanner::VolumesIterator DiskScanner::begin() {
    return mVolumes.begin();
}
DiskScanner::VolumesIterator DiskScanner::end() {
    return mVolumes.end();
}
