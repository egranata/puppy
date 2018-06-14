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

#include <kernel/syscalls/handlers.h>
#include <kernel/drivers/pci/bus.h>
#include <kernel/drivers/pci/ide.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/drivers/pci/diskfile.h>
#include <kernel/process/current.h>
#include <kernel/fs/vol/diskscanner.h>
#include <kernel/log/log.h>

syscall_response_t trymount_syscall_handler(uint32_t fileid, const char* path) {
    auto& vfs(VFS::get());

    VFS::filehandle_t fh = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fileid, &fh)) {
        LOG_ERROR("no file found with id %u", fileid);
        return ERR(NO_SUCH_DEVICE);
    }

    Filesystem* devfs = VFS::get().findfs("devices");
    if (devfs == nullptr || devfs != fh.first || fh.second == nullptr) {
        LOG_ERROR("no devfs found %p, or invalid filesystem found %p or no file object found %p", devfs, fh.first, fh.second);
        return ERR(NO_SUCH_DEVICE);
    }
    if (fh.second->kind() != Filesystem::FilesystemObject::kind_t::blockdevice) {
        LOG_ERROR("no block file found %u", fh.second->kind());
        return ERR(NO_SUCH_DEVICE);
    }

    Filesystem::File* file = (Filesystem::File*)fh.second;

    Volume* volume = (Volume*)file->ioctl((uintptr_t)blockdevice_ioctl_t::IOCTL_GET_VOLUME, 0);
    if (volume == nullptr) {
        LOG_ERROR("no volume found %p", volume);
        return ERR(NO_SUCH_DEVICE);
    }

    auto mounted = vfs.mount(volume, path);
    if (mounted.first) {
        LOG_DEBUG("disk mounted at %s ", mounted.second);
        return OK;
    } else {
        LOG_DEBUG("disk could not be mounted");
        return ERR(NO_SUCH_DEVICE);
    }
}
