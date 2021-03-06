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

LOG_TAG(FILEIO, 2);

#define FILE_LIKE(x, y) case file_kind_t:: x: return (Filesystem::File*)object;
#define DIR_LIKE(x, y) case file_kind_t:: x: return nullptr;
static Filesystem::File* asFile(Filesystem::FilesystemObject* object) {
    switch (object->kind()) {
#include <kernel/fs/file_kinds.tbl>
    }

    return nullptr;
}
#undef FILE_LIKE
#undef DIR_LIKE

#define DIR_LIKE(x, y) case file_kind_t:: x: return (Filesystem::Directory*)object;
#define FILE_LIKE(x, y) case file_kind_t:: x: return nullptr;
static Filesystem::Directory* asDirectory(Filesystem::FilesystemObject* object) {
    switch (object->kind()) {
#include <kernel/fs/file_kinds.tbl>
    }

    return nullptr;
}
#undef FILE_LIKE
#undef DIR_LIKE

syscall_response_t fopen_syscall_handler(const char* path, uint32_t mode) {
    auto&& vfs(VFS::get());

    auto&& file = vfs.open(path, mode);
    if (file.filesystem == nullptr || file.object == nullptr) {
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
        if (file) {
            file.close();
        } else {
            LOG_DEBUG("file.filesystem = 0x%p, file.object = 0x%p, will not close", file.filesystem, file.object);
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
        if (file.object) {
            auto realFile = asFile(file.object);
            if (realFile == nullptr) return ERR(NOT_A_FILE);
            auto sz = realFile->read(len, buffer);
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
        if (file.object) {
            auto realFile = asFile(file.object);
            if (realFile == nullptr) return ERR(NOT_A_FILE);
            auto sz = realFile->write(len, buffer);
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
        if (file.object) {
            return file.object->stat(*stat) ? OK : ERR(NO_SUCH_FILE);
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
        if (file.object) {
            auto realFile = asFile(file.object);
            if (realFile == nullptr) return ERR(NOT_A_FILE);
            return realFile->seek(pos) ? OK : ERR(NO_SUCH_FILE);
        } else {
            return ERR(NO_SUCH_FILE);
        }
    }
}

syscall_response_t ftell_syscall_handler(uint16_t fid, size_t* pos) {
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.object) {
            auto realFile = asFile(file.object);
            if (realFile == nullptr) return ERR(NOT_A_FILE);
            return realFile->tell(pos) ? OK : ERR(NO_SUCH_FILE);
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
        if (file.object) {
            auto realFile = asFile(file.object);
            if (realFile == nullptr) return ERR(NOT_A_FILE);
            auto ret = realFile->ioctl(a1,a2);
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
    if (file.filesystem == nullptr || file.object == nullptr) {
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

syscall_response_t freaddir_syscall_handler(uint16_t fid, file_info_t* info) {
    Filesystem::Directory::fileinfo_t finfo;
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else {
        if (file.object) {
            auto realDirectory = asDirectory(file.object);
            if (realDirectory == nullptr) return ERR(NOT_A_FILE);
            auto ok = realDirectory->next(finfo);
            if (ok) {
                info->kind = finfo.kind;
                info->size = finfo.size;
                info->time = finfo.time;
                bzero(info->name, sizeof(info->name));
                strncpy(info->name, finfo.name, gMaxPathSize);
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

syscall_response_t fdup_syscall_handler(uint32_t fid, uint32_t minNewFid) {
    VFS::filehandle_t file = {nullptr, nullptr};
    if (!gCurrentProcess->fds.is(fid,&file)) {
        return ERR(NO_SUCH_FILE);
    } else if (file.object == nullptr) {
        return ERR(NO_SUCH_FILE);
    } else {
        file.object->incref();
        size_t newfid = 0;
        bool ok = gCurrentProcess->fds.set(file, newfid, minNewFid);
        if (ok) {
            LOG_DEBUG("file handle %u duplicated as handle %u in process %u", fid, newfid, gCurrentProcess->pid);
            return OK | (newfid << 1);
        }
        return ERR(NO_SUCH_FILE);
    }
}
