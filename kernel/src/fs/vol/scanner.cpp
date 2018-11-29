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

#include <kernel/fs/vol/scanner.h>
#include <kernel/fs/vol/disk.h>
#include <kernel/fs/fsidents.h>
#include <kernel/libc/move.h>

DiskScanner::DiskScanner(Disk *d) : mDisk(d) {}

void DiskScanner::scan(const callback_f& f) {
    x86_mbr_t mbr;
    if (mDisk->read(0, 1, (uint8_t*)&mbr)) {
        scan(mbr, f);
    }
}

void DiskScanner::scan(const x86_mbr_t& mbr, const callback_f& f) {
    scan(mbr.partition0, f);
    scan(mbr.partition1, f);
    scan(mbr.partition2, f);
    scan(mbr.partition3, f);
}

void DiskScanner::scan(const diskpart_t& dp, const callback_f& f) {
    if (dp.sysid == 0 || dp.size == 0) return;

    const char* parttype = nullptr;
    for (auto i = 0; true; ++i) {
        auto&& fsinfo = gKnownFilesystemTypes[i];
        if (fsinfo.sysid == 0 && fsinfo.type == nullptr) break;
        if (fsinfo.sysid == dp.sysid) {
            parttype = fsinfo.type;
            break;
        }
    }

    if (parttype == nullptr) return;
    Volume *vol = mDisk->volume(dp);
    f(move(vol));
}
