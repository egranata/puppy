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

#ifndef DRIVERS_PCI_DISKFILE
#define DRIVERS_PCI_DISKFILE

#include <kernel/drivers/pci/ide.h>
#include <kernel/fs/memfs/memfs.h>

class IDEDiskFile : public MemFS::File {
    public:
        IDEDiskFile(IDEController*, const IDEController::disk_t&, uint32_t ctrlid);
        string content() override;
        uintptr_t ioctl(uintptr_t, uintptr_t) override;

    private:
        IDEController *mController;
        IDEController::disk_t mDisk;
};

#endif
