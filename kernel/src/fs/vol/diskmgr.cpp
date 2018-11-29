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

#include <kernel/log/log.h>

#include <kernel/fs/vol/diskmgr.h>
#include <kernel/fs/vol/diskctrl.h>
#include <kernel/fs/vol/disk.h>
#include <kernel/fs/vol/volume.h>
#include <kernel/fs/devfs/devfs.h>

DiskManager& DiskManager::get() {
    static DiskManager gManager;

    return gManager;
}

DiskManager::DiskManager() {
    mDevFSDirectory = DevFS::get().getDeviceDirectory("disks");
}

void DiskManager::onNewDiskController(DiskController* ctrl) {
    if (ctrl) {
        mDiskControllers.push_back(ctrl);
        LOG_INFO("added new DiskController 0x%p %s", ctrl, ctrl->id());
    }
}

void DiskManager::onNewDisk(Disk *dsk) {
    if (dsk) {
        mDisks.push_back(dsk);
        LOG_INFO("added new Disk 0x%p %s, controller is 0x%p %s", dsk, dsk->id(), dsk->controller(), dsk->controller()->id());
        mDevFSDirectory->add(dsk->file());
    }
}

void DiskManager::onNewVolume(Volume *vol) {
    if (vol) {
        mVolumes.push_back(vol);
        LOG_INFO("added new Volume 0x%p %s disk 0x%p %s controller is 0x%p %s",
            vol, vol->id(), vol->disk(), vol->disk()->id(), vol->disk()->controller(), vol->disk()->controller()->id());
        mDevFSDirectory->add(vol->file());
    }
}

iterable_vector_view<DiskController*> DiskManager::controllers() {
    return iterable_vector_view<DiskController*>(mDiskControllers);
}
iterable_vector_view<Disk*> DiskManager::disks() {
    return iterable_vector_view<Disk*>(mDisks);
}
iterable_vector_view<Volume*> DiskManager::volumes() {
    return iterable_vector_view<Volume*>(mVolumes);
}
