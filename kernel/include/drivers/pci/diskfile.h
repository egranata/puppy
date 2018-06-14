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
#include <kernel/fs/ramfs/fsobject.h>

class IDEDiskFile : public RAMFile {
    public:
        IDEDiskFile(IDEController*, const IDEController::disk_t&, uint32_t ctrlid);
        RAMFileData* buffer() override;
    private:
        IDEController *mController;
        IDEController::disk_t mDisk;

        friend class IDEDiskFileData;
};

#endif
