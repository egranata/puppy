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
#include <fs/vfs.h>
#include <process/process.h>
#include <process/manager.h>
#include <log/log.h>

HANDLER2(fopen,pth,mod) {
    auto&& vfs(VFS::get());
    const char* path = (const char*)pth;
    auto mode = (Filesystem::mode_t)mod;

    auto&& file = vfs.open(path, mode);
    if (file.first == nullptr || file.second == nullptr) {
        return ERR(NO_SUCH_FILE);
    } else {
        auto&& pmm(ProcessManager::get());
        auto self = pmm.getcurprocess();
        size_t idx = 0;
        if (self->fds.set(file, idx)) {
            LOG_DEBUG("file %s opened as handle %u in process %u", path, idx, self->pid);
            return OK | (idx << 1);
        }
        return ERR(UNIMPLEMENTED);
    }
}

HANDLER1(fclose,fid) {
    auto&& pmm(ProcessManager::get());
    auto self = pmm.getcurprocess();
    LOG_DEBUG("closing file handle %u for process %u", fid, self->pid);
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!self->fds.is(fid,&file)) {
        if (file.first && file.second) {
            file.first->close(file.second);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }
    self->fds.clear(fid);
    return OK;
}

HANDLER3(fread,fid,len,buf) {
    char* buffer = (char*)buf;
    auto&& pmm(ProcessManager::get());
    auto self = pmm.getcurprocess();
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!self->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.second) {
            return ((Filesystem::File*)file.second)->read(len, buffer) ? OK : ERR(NO_SUCH_FILE);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }
}

HANDLER3(fwrite,fid,len,buf) {
    char* buffer = (char*)buf;
    auto&& pmm(ProcessManager::get());
    auto self = pmm.getcurprocess();
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!self->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.second) {
            return ((Filesystem::File*)file.second)->write(len, buffer) ? OK : ERR(NO_SUCH_FILE);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }
}

HANDLER2(fstat,fid,dst) {
    auto stat = (Filesystem::File::stat_t*)dst;
    auto&& pmm(ProcessManager::get());
    auto self = pmm.getcurprocess();
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!self->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.second) {
            return ((Filesystem::File*)file.second)->stat(*stat) ? OK : ERR(NO_SUCH_FILE);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }
}

HANDLER2(fseek,fid,pos) {
    auto&& pmm(ProcessManager::get());
    auto self = pmm.getcurprocess();
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!self->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.second) {
            return ((Filesystem::File*)file.second)->seek(pos) ? OK : ERR(NO_SUCH_FILE);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }    
}

HANDLER3(fioctl,fid,a1,a2) {
    auto&& pmm(ProcessManager::get());
    auto self = pmm.getcurprocess();
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!self->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.second) {
            auto ret = ((Filesystem::File*)file.second)->ioctl(a1,a2);
            return OK | (ret << 1);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }    
}

HANDLER1(fopendir,pth) {
    auto&& vfs(VFS::get());
    const char* path = (const char*)pth;

    auto&& file = vfs.opendir(path);
    if (file.first == nullptr || file.second == nullptr) {
        return ERR(NO_SUCH_FILE);
    } else {
        auto&& pmm(ProcessManager::get());
        auto self = pmm.getcurprocess();
        size_t idx = 0;
        if (self->fds.set(file, idx)) {
            LOG_DEBUG("directory %s opened as handle %u in process %u", path, idx, self->pid);
            return OK | (idx << 1);
        }
        return ERR(UNIMPLEMENTED);
    }
}

HANDLER2(freaddir,fid,fip) {
    auto&& pmm(ProcessManager::get());
    auto self = pmm.getcurprocess();
    Filesystem::Directory::fileinfo_t finfo;
    auto fi = (Filesystem::FilesystemObject::info_t*)fip;
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!self->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.second) {
            auto ok = ((Filesystem::Directory*)file.second)->next(finfo);
            if (ok) {
                fi->kind = finfo.kind;
                fi->size = finfo.size;
                bzero(&fi->name[0], sizeof(fi->name));
                memcpy(&fi->name[0], finfo.name.c_str(), finfo.name.size());
                return OK;
            } else {
                return ERR(NO_SUCH_FILE);
            }
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }
}
