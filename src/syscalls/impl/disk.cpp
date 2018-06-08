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

#include <syscalls/handlers.h>
#include <drivers/pci/bus.h>
#include <drivers/pci/ide.h>
#include <fs/vfs.h>
#include <fs/devfs/devfs.h>
#include <drivers/pci/diskfile.h>
#include <process/current.h>
#include <fs/vol/diskscanner.h>
#include <log/log.h>

HANDLER2(getcontroller,n,resp) {
    uint32_t *result = (uint32_t*)resp;
    auto&& bus(PCIBus::get());
    auto b = bus.begin();
    auto e = bus.end();
    for(; n != 0; --n) {
        if (++b == e) break;
    }
    if (n != 0) {
        return ERR(NO_SUCH_DEVICE);
    } else {
        *result = (uint32_t)*b;
        return OK;
    }
}

HANDLER2(discoverdisk,ctrl,did) {
    IDEController::disk_t *disk = (IDEController::disk_t*)did;

    auto&& pci(PCIBus::get());
    auto b = pci.begin();
    auto e = pci.end();
    for(; ctrl != 0; --ctrl) {
        if (++b == e) break;
    }
    if (ctrl != 0) {
        return ERR(NO_SUCH_DEVICE);
    }
    IDEController *ide = (IDEController*)*b;
    if (ide->fill(*disk)) {
        return OK;
    }

    return ERR(NO_SUCH_DEVICE);
}

struct disksyscalls_disk_descriptor {
    IDEController *ide;
    IDEController::disk_t* disk;
};

HANDLER4(writesector,dif,sec0,count,buffer) {
    auto disk = (disksyscalls_disk_descriptor*)dif;
    return disk->ide->write(*disk->disk, sec0, (uint8_t)(count & 0xFF), (unsigned char*)buffer) ? OK : ERR(DISK_IO_ERROR);
}

HANDLER4(readsector,dif,sec0,count,buffer) {
    auto disk = (disksyscalls_disk_descriptor*)dif;
    return disk->ide->read(*disk->disk, sec0, (uint8_t)(count & 0xFF), (unsigned char*)buffer) ? OK : ERR(DISK_IO_ERROR);
}

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
