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

#ifndef DRIVERS_PCI_VOLUMEFILE
#define DRIVERS_PCI_VOLUMEFILE

#include <drivers/pci/ide.h>
#include <fs/ramfs/fsobject.h>
#include <fs/vol/volume.h>

class IDEVolumeFile : public RAMFile {
    public:
        IDEVolumeFile(Volume* vol, uint32_t ctrlid);
        RAMFileData* buffer() override;
    private:
        Volume* mVolume;

        friend class IDEVolumeFileData;
};

#endif