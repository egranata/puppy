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
#include <kernel/libc/time.h>
#include <kernel/libc/string.h>
#include <kernel/libc/deleteptr.h>
#include <kernel/syscalls/types.h>

LOG_TAG(FATCALENDAR, 0);

static uint8_t gNextId() {
    static uint8_t gId = 0;
    auto id = gId++;
    if (id == 10) {
        PANIC("unable to mount more than 10 FAT devices");
    }
    return id;
}

static uint64_t fatDateToUnix(uint16_t fdate) {
    auto day = fdate & 31;
    auto month = (fdate >> 5) & 15;
    auto year = (fdate >> 9) & 127;
    LOG_DEBUG("out of fdate=%x, day=%u month=%u year=%u", fdate, day, month, year);
    return date_components_to_epoch({
        (uint8_t)day,
        (uint8_t)month,
        (uint16_t)(year + 1980)
    });
}

static uint64_t fatTimeToUnix(uint16_t ftime) {
    auto second = 2 * (ftime & 31); // FAT uses an "every other second" model
    auto minute = (ftime >> 5) & 63;
    auto hour = (ftime >> 11) & 31;
    LOG_DEBUG("out of ftime=%x, hour=%u minute=%u second=%u", ftime, hour, minute, second);
    return time_components_to_epoch({
        (uint8_t)hour,
        (uint8_t)minute,
        (uint8_t)second
    });
}

FATFileSystem::FATFileSystem(Volume* vol) {
    char buf[3] = {0};
    auto nextid = gNextId();
    sprint(&buf[0], 2, "%d:", nextid);

    LOG_DEBUG("mounting volume %p as id %s", vol, &buf[0]);

    mFatFS.pdrv = nextid;
    mFatFS.vol = vol;
    f_mount(&mFatFS, &buf[0], 1);

    LOG_DEBUG("mount completed as drive %u, mFatFS = %p", mFatFS.pdrv, &mFatFS);
}

class FATFileSystemFile : public Filesystem::File {
    public:
        FATFileSystemFile(FIL *file, FILINFO fi) : mFile(file), mFileInfo(fi) {}

        bool seek(size_t pos) override {
            switch (f_lseek(mFile, pos)) {
                case FR_OK: return true;
                default: return false;
            }
        }

        size_t read(size_t size, char* dest) override {
            UINT br = 0;
            switch (f_read(mFile, dest, size, &br)) {
                case FR_OK: return br;
                default: return br;
            }
        }

        size_t write(size_t size, char* src) override {
            UINT bw = 0;
            FRESULT result;
            switch (result = f_write(mFile, src, size, &bw)) {
                case FR_OK: return bw;
                default:
                LOG_ERROR("FatFS error (%u) writing to file %p (mode=%x)", result, mFile, mFile->flag);
                return bw;
            }
        }

        bool stat(stat_t& stat) override {
            stat.kind = file_kind_t::file;
            stat.size = mFileInfo.fsize ? mFileInfo.fsize : f_size(mFile);
            stat.time = fatDateToUnix(mFileInfo.fdate) + fatTimeToUnix(mFileInfo.ftime);
            return true;
        }

        ~FATFileSystemFile() override {
            LOG_DEBUG("closing file ptr %p", mFile);

            if (mFile) {
                f_close(mFile);
                free(mFile);
                mFile = nullptr;
            }
        }

    private:
        FIL *mFile;
        FILINFO mFileInfo;
};

class FATFileSystemDirectory : public Filesystem::Directory {
    public:
        FATFileSystemDirectory(DIR* dir, FILINFO fi) : mDir(dir), mFileInfo(fi) {}

        bool next(fileinfo_t& fi) override {
            FILINFO fil;
            switch (f_readdir(mDir, &fil)) {
                default: return false;
                case FR_OK:
                    if (fil.fname[0] == 0) return false;
                    bzero(fi.name, sizeof(fi.name));
                    strncpy(fi.name, fil.fname, sizeof(fi.name));
                    fi.size = fil.fsize;
                    fi.time = fatDateToUnix(fil.fdate) + fatTimeToUnix(fil.ftime);
                    fi.kind = (fil.fattrib & AM_DIR) ? file_kind_t::directory : file_kind_t::file;
                    return true;
            }
        }

        bool stat(stat_t& stat) override {
            stat.kind = file_kind_t::directory;
            stat.size = 0;
            stat.time = fatDateToUnix(mFileInfo.fdate) + fatTimeToUnix(mFileInfo.ftime);
            return true;
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
        FILINFO mFileInfo;
};

Filesystem::File* FATFileSystem::open(const char* path, uint32_t mode) {
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
        FILINFO fileInfo;

        auto realmode = 0;
        if (mode & FILE_OPEN_READ) realmode |= FA_READ;
        if (mode & FILE_OPEN_WRITE) realmode |= FA_WRITE;
        if (mode & FILE_OPEN_NEW) realmode |= FA_CREATE_ALWAYS;
        if (mode & FILE_NO_CREATE) realmode |= FA_OPEN_EXISTING;
        if (mode & FILE_OPEN_APPEND) realmode |= FA_OPEN_APPEND;

        LOG_DEBUG("VFS mode: %x, FatFS mode: %x", mode, realmode);

        switch(f_stat(fullpath, &fileInfo)) {
            case FR_OK: break;
            default:
                bzero(&fileInfo, sizeof(fileInfo));
                break;
        }

        switch (f_open(fil, fullpath, realmode)) {
            case FR_OK:
                LOG_DEBUG("returning file handle %p for %s", fil, fullpath);
                return new FATFileSystemFile(fil_delptr.reset(), fileInfo);
            default:
                return nullptr;
        }
    }
    
}

bool FATFileSystem::del(const char* path) {
    LOG_DEBUG("FatFs on drive %d is trying to delete file %s", mFatFS.pdrv, path);
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
            case FR_OK: {
                switch (f_unlink(fullpath)) {
                    case FR_OK: return true;
                    default:
                        LOG_WARNING("unable to erase file at path %s", fullpath);
                        return false;
                }
            }
            default: {
                LOG_WARNING("no file found at path %s", fullpath);
                return true;
            }
        }
    }
}

void FATFileSystem::doClose(Filesystem::FilesystemObject* f) {
    LOG_DEBUG("closing filesystem object %p", f);
    delete f;
}

Filesystem::Directory* FATFileSystem::opendir(const char* path) {
    if (path == nullptr || path[0] == 0) path = "/";
    LOG_DEBUG("FatFs on drive %d is trying to open directory %s", mFatFS.pdrv, path);
    auto len = 4 + strlen(path);
    delete_ptr<char> fullpath((char*)calloc(len, 1));
    sprint(fullpath.get(), len, "%d:%s", mFatFS.pdrv, path);
    delete_ptr<DIR> dir((DIR*)calloc(sizeof(DIR), 1));
    
    FILINFO fileInfo;
    switch(f_stat(fullpath.get(), &fileInfo)) {
        case FR_OK: break;
        default:
            bzero(&fileInfo, sizeof(fileInfo));
            break;
    }

    switch(f_opendir(dir.get(), fullpath.get())) {
        case FR_OK:
            LOG_DEBUG("returning handle %p for directory %s", dir.get(), fullpath.get());
            return new FATFileSystemDirectory(dir.reset(), fileInfo);
        default:
            return nullptr;
    }
}

bool FATFileSystem::mkdir(const char* path) {
    if (path == nullptr || path[0] == 0) path = "/";
    LOG_DEBUG("FatFs on drive %d is trying to create directory %s", mFatFS.pdrv, path);
    auto len = 4 + strlen(path);
    delete_ptr<char> fullpath((char*)calloc(len, 1));
    sprint(fullpath.get(), len, "%d:%s", mFatFS.pdrv, path);

    switch (f_mkdir(fullpath.get())) {
        case FR_OK: return true;
        default:
            LOG_WARNING("failed to create directory by full path %s", fullpath.get());
    }

    return false;
}
