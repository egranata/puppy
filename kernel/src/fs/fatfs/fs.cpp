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

#include <kernel/fs/fatfs/fs.h>
#include <kernel/panic/panic.h>
#include <kernel/libc/sprint.h>
#include <kernel/log/log.h>
#include <kernel/libc/memory.h>
#include <kernel/libc/string.h>
#include <kernel/libc/deleteptr.h>

static uint8_t gNextId() {
    static uint8_t gId = 0;
    auto id = gId++;
    if (id == 10) {
        PANIC("unable to mount more than 10 FAT devices");
    }
    return id;
}

FATFileSystem::FATFileSystem(Volume* vol) {
    char buf[3] = {0};
    sprint(&buf[0], 2, "%d:", gNextId());

    LOG_DEBUG("mounting volume %p as id %s", vol, &buf[0]);

    mFatFS.vol = vol;
    f_mount(&mFatFS, &buf[0], 1);

    LOG_DEBUG("mount completed as drive %u", mFatFS.pdrv);
}

class FATFileSystemFile : public Filesystem::File {
    public:
        FATFileSystemFile(FIL* file, const FILINFO& info) : mFileInfo(info), mFile(file) {}

        bool seek(size_t pos) override {
            switch (f_lseek(mFile, pos)) {
                case FR_OK: return true;
                default: return false;
            }
        }

        bool read(size_t size, char* dest) override {
            UINT br = 0;
            switch (f_read(mFile, dest, size, &br)) {
                case FR_OK: return true;
                default: return false;
            }
        }

        bool write(size_t size, char* src) override {
            UINT bw = 0;
            switch (f_write(mFile, src, size, &bw)) {
                case FR_OK: return true;
                default: return false;
            }
        }

        bool stat(stat_t& stat) override {
            stat.size = mFileInfo.fsize;
            return true;
        }

        ~FATFileSystemFile() override {
            if (mFile != nullptr) {
                f_close(mFile);
            }
            free(mFile);
            mFile = nullptr;
        }

    private:
        FILINFO mFileInfo;
        FIL* mFile;
};

class FATFileSystemDirectory : public Filesystem::Directory {
    public:
        FATFileSystemDirectory(DIR* dir) : mDir(dir) {}

        bool next(fileinfo_t& fi) override {
            FILINFO fil;
            switch (f_readdir(mDir, &fil)) {
                default: return false;
                case FR_OK:
                    if (fil.fname[0] == 0) return false;
                    fi.name = string((const char*)&fil.fname[0]);
                    fi.size = fil.fsize;
                    fi.kind = (fil.fattrib & AM_DIR) ? fileinfo_t::kind_t::directory : fileinfo_t::kind_t::file;
                    return true;
            }
        }

        ~FATFileSystemDirectory() {
            if (mDir != nullptr) {
                f_closedir(mDir);
            }
            free(mDir);
            mDir = nullptr;
        }

    private:
        DIR* mDir;
};

Filesystem::File* FATFileSystem::open(const char* path, mode_t mode) {
    LOG_DEBUG("FatFs on drive %d is trying to open file %s", mFatFS.pdrv, path);
    auto len = 4 + strlen(path);
    char* fullpath = allocate<char>(len);
    bzero((uint8_t*)&fullpath[0], len);
    sprint(&fullpath[0], len, "%d:%s", mFatFS.pdrv, path);
    auto fil = allocate<FIL>();
    bzero((uint8_t*)fil, sizeof(fil));

    {
        delete_ptr<char> fullpath_delptr(fullpath);
        delete_ptr<FIL> fil_delptr(fil);

        FILINFO fi;
        switch (f_stat(fullpath, &fi)) {
            case FR_OK: break;
            default: {
                LOG_WARNING("no file found at path %s", fullpath);
                return nullptr;
            }
        }

        LOG_DEBUG("found a valid FAT file at path %s - size was %u", fullpath, fi.fsize);

        auto realmode = (mode == mode_t::read ? FA_READ : FA_CREATE_ALWAYS);
        switch (f_open(fil, fullpath, realmode)) {
            case FR_OK:
                LOG_DEBUG("returning file handle %p for %s", fil, fullpath);
                return new FATFileSystemFile(fil_delptr.reset(), fi);
            default:
                return nullptr;
        }
    }
    
}

void FATFileSystem::close(Filesystem::FilesystemObject* f) {
    delete f;
}

Filesystem::Directory* FATFileSystem::opendir(const char* path) {
    if (path == nullptr || path[0] == 0) path = "/";
    LOG_DEBUG("FatFs on drive %d is trying to open directory %s", mFatFS.pdrv, path);
    auto len = 4 + strlen(path);
    delete_ptr<char> fullpath((char*)calloc(len, 1));
    sprint(fullpath.get(), len, "%d:%s", mFatFS.pdrv, path);
    delete_ptr<DIR> dir((DIR*)calloc(sizeof(DIR), 1));

    switch(f_opendir(dir.get(), fullpath.get())) {
        case FR_OK:
            LOG_DEBUG("returning handle %p for directory %s", dir.get(), fullpath.get());
            return new FATFileSystemDirectory(dir.reset());
        default:
            return nullptr;
    }
}
