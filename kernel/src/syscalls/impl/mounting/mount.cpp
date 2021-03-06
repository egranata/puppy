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
#include <kernel/fs/vfs.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/process/current.h>
#include <kernel/log/log.h>
#include <kernel/fs/vol/volume.h>

syscall_response_t mount_syscall_handler(uint32_t fileid, const char* path) {
    if (path[0] == '/') ++path;
    if (path[0] == 0) {
        LOG_ERROR("empty mountpoint path not valid");
        return ERR(NOT_ALLOWED);
    }
    if (nullptr != strchr(path, '/')) {
        LOG_ERROR("mountpoint path '%s' not valid", path);
        return ERR(NOT_ALLOWED);
    }

    auto& vfs(VFS::get());

    VFS::filehandle_t fh = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fileid, &fh)) {
        LOG_ERROR("no file found with id %u", fileid);
        return ERR(NO_SUCH_DEVICE);
    }

    Filesystem* devfs = VFS::get().findfs("devices");
    if (devfs == nullptr || devfs != fh.filesystem || fh.object == nullptr) {
        LOG_ERROR("no devfs found 0x%p, or invalid filesystem found 0x%p or no file object found 0x%p", devfs, fh.filesystem, fh.object);
        return ERR(NO_SUCH_DEVICE);
    }
    if (fh.object->kind() != Filesystem::FilesystemObject::kind_t::blockdevice) {
        LOG_ERROR("no block file found %u", fh.object->kind());
        return ERR(NO_SUCH_DEVICE);
    }

    Filesystem::File* file = (Filesystem::File*)fh.object;

    Volume* volume = (Volume*)file->ioctl((uintptr_t)blockdevice_ioctl_t::IOCTL_GET_VOLUME, 0);
    if (volume == nullptr) {
        LOG_ERROR("no volume found 0x%p", volume);
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
