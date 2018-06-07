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

#include <drivers/pci/diskfile.h>
#include <libc/sprint.h>

IDEDiskFile::IDEDiskFile(IDEController* ctrl, const IDEController::disk_t& d) : RAMFile(), mController(ctrl), mDisk(d) {
    char buf[64] = {0};
    sprint(buf, 63, "idedisk%u%u", (uint32_t)mDisk.chan, (uint32_t)mDisk.bus);
    name(&buf[0]);
}

RAMFileBuffer* IDEDiskFile::buffer() {
    return nullptr;
}
