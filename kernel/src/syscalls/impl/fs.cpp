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
#include <kernel/process/current.h>
#include <kernel/log/log.h>
#include <kernel/syscalls/types.h>
#include <kernel/log/log.h>

LOG_TAG(FILEIO, 2);

syscall_response_t fopen_syscall_handler(const char* path, uint32_t mode) {
    auto&& vfs(VFS::get());

    auto&& file = vfs.open(path, mode);
    if (file.first == nullptr || file.second == nullptr) {
        return ERR(NO_SUCH_FILE);
    } else {
        size_t idx = 0;
        if (gCurrentProcess->fds.set(file, idx)) {
            LOG_DEBUG("file %s opened as handle %u in process %u", path, idx, gCurrentProcess->pid);
            return OK | (idx << 1);
        }
        return ERR(UNIMPLEMENTED);
    }
}

syscall_response_t fdel_syscall_handler(const char* path) {
    auto&& vfs(VFS::get());

    return vfs.del(path) ? OK : ERR(NO_SUCH_FILE);
}

HANDLER1(fclose,fid) {
    LOG_DEBUG("closing file handle %u for process %u", fid, gCurrentProcess->pid);
    VFS::filehandle_t file = {nullptr, nullptr};
    if (gCurrentProcess->fds.is(fid,&file)) {
        if (file.first && file.second) {
            file.first->close(file.second);
        } else {
            LOG_DEBUG("file.first = %p, file.second = %p, will not close", file.first, file.second);
            return ERR(NO_SUCH_FILE);
        }
    }
    gCurrentProcess->fds.clear(fid);
    return OK;
}

HANDLER3(fread,fid,len,buf) {
    char* buffer = (char*)buf;
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.second) {
            auto sz = ((Filesystem::File*)file.second)->read(len, buffer);
            TAG_DEBUG(FILEIO, "read %u bytes to handle %u", sz, fid);
            return OK | (sz << 1);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }
}

HANDLER3(fwrite,fid,len,buf) {
    char* buffer = (char*)buf;
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.second) {
            auto sz = ((Filesystem::File*)file.second)->write(len, buffer);
            TAG_DEBUG(FILEIO, "written %u bytes to handle %u", sz, fid);
            return OK | (sz << 1);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }
}

HANDLER2(fstat,fid,dst) {
    auto stat = (Filesystem::File::stat_t*)dst;
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
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
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
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
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
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
        size_t idx = 0;
        if (gCurrentProcess->fds.set(file, idx)) {
            LOG_DEBUG("directory %s opened as handle %u in process %u", path, idx, gCurrentProcess->pid);
            return OK | (idx << 1);
        }
        return ERR(UNIMPLEMENTED);
    }
}

HANDLER2(freaddir,fid,fip) {
    Filesystem::Directory::fileinfo_t finfo;
    auto fi = (Filesystem::FilesystemObject::info_t*)fip;
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
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

syscall_response_t mkdir_syscall_handler(const char* path) {
    auto&& vfs(VFS::get());

    return vfs.mkdir(path) ? OK : ERR(NO_SUCH_FILE);
}
