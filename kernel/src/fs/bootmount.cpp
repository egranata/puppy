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

#include <kernel/boot/phase.h>
#include <kernel/log/log.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/vol/diskmgr.h>
#include <kernel/fs/vol/scanner.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/fs/memfs/memfs.h>
#include <kernel/panic/panic.h>
#include <kernel/sys/config.h>
#include <muzzle/string.h>

namespace boot::mount {
    uint32_t init() {
        auto path2mainfs = gKernelConfiguration()->mainfs.value;

    	auto& vfs(VFS::get());
        auto& dmgr(DiskManager::get());

        for(auto disk : dmgr.disks()) {
            DiskScanner scanner(disk);
            LOG_DEBUG("running disk scanning on disk %s", disk->id());
            scanner.scan([&dmgr] (Volume *vol) -> bool {
                LOG_DEBUG("found volume %s", vol->id());
                dmgr.onNewVolume(vol);
                return true;
            });
        }

        for(auto vol : dmgr.volumes()) {
            if (0 == strcmp(path2mainfs, vol->id())) {
                auto ok = vfs.mount(vol, "system").first;
                if (false == ok) {
                    LOG_ERROR("%s could not be mounted as mainfs - trying to find another candidate", vol->id());
                } else {
                    LOG_INFO("%s is mainfs (/system)", vol->id());
                    bootphase_t::printf("/system mounted from controller %s disk %s volume %s\n",
                        vol->disk()->controller()->id(), vol->disk()->id(), vol->id());
                }
            }
        }

        return 0;
    }
}
