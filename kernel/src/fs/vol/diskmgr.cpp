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

DiskManager& DiskManager::get() {
    static DiskManager gManager;

    return gManager;
}

DiskManager::DiskManager() = default;

void DiskManager::onNewDiskController(DiskController* ctrl) {
    if (ctrl) {
        mDiskControllers.push_back(ctrl);
        LOG_INFO("added new DiskController 0x%p", ctrl);
    }
}

void DiskManager::onNewDisk(Disk *dsk) {
    if (dsk) {
        mDisks.push_back(dsk);
        LOG_INFO("added new Disk 0x%p, controller is 0x%p", dsk, dsk->controller());
    }
}
