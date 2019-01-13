// Copyright 2019 Google LLC
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

#include "automount.h"
#include <stdlib.h>
#include <stdio.h>

#include <syscalls.h>
#include <string>

void automountVolumeHandler(const char* volid) {
    std::string fpath;
    fpath.append_sprintf("/devices/disks/%s", volid);
    FILE* f = fopen(fpath.c_str(), "r");
    if (f == nullptr) {
        printf("[vold] error: volume '%s' does not match any device file\n", volid);
        return;
    }
    int fid = fileno(f);
    int ok = mount_syscall(fid, volid);
    fclose(f);
    if (ok) {
        printf("[vold] error: volume '%s' cannot be mounted\n", volid);
    }
}
