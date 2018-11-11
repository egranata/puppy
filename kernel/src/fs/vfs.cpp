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

#include <kernel/fs/vfs.h>
#include <kernel/log/log.h>
#include <kernel/libc/string.h>
#include <kernel/libc/memory.h>
#include <kernel/fs/vol/ptable.h>
#include <kernel/boot/phase.h>
#include <kernel/fs/initrd/fs.h>
#include <kernel/boot/bootinfo.h>
#include <kernel/fs/devfs/devfs.h>
#include <kernel/time/manager.h>

namespace boot::vfs {
    uint32_t init() {
    	auto bootmodules(bootmodinfo());
        auto& vfs(VFS::get());

        vfs.mount("devices", DevFS::get().getMemFS());

        bool anyfs = false;

        for (auto mod_id = 0u; mod_id < bootmodules->count; ++mod_id) {
            auto&& module = bootmodules->info[mod_id];
            auto initrd = Initrd::tryget(module.vmstart);
            if (initrd) {
                LOG_INFO("module %u at 0x%p is a valid initrd filesystem 0x%p - mounting at %s", mod_id, module.vmstart, initrd, module.args);
                if (vfs.mount(module.args, initrd)) anyfs = true;
            }
        }

        return anyfs ? 0 : -1;
    }

    bool fail(uint32_t) {
        return bootphase_t::gPanic;
    }
}

VFS& VFS::get() {
    static VFS gVFS;

    return gVFS;
}

VFS::VFS() : mMounts() {
    LOG_DEBUG("initializing VFS");
}

bool VFS::mount(const char* path, Filesystem* fs) {
    if (path[0] == '/') ++path;
    LOG_DEBUG("mounting /%s as 0x%p", path, fs);
    mMounts.add(mount_t{
        strdup(path),
        TimeManager::get().UNIXtime(),
        fs});
    return true;
}

Filesystem* VFS::findfs(const char* mnt) {
    auto b = mMounts.begin(), e = mMounts.end();
    for(; b != e; ++b) {
        auto&& m = *b;
        if (0 == strcmp(m.path, mnt)) {
            return m.fs;
        }
    }

    return nullptr;
}

bool VFS::unmount(const char* path) {
    LOG_DEBUG("asked to unmount at %s", path);
    auto b = mMounts.begin(), e = mMounts.end();
    for(; b != e; ++b) {
        auto&& m = *b;
        if (0 == strcmp(m.path, path)) {
            LOG_DEBUG("filesystem found - 0x%p", m.fs);
            mMounts.remove(b);
            free((void*)m.path);
            return true;
        }
    }

    LOG_DEBUG("no matching filesystem to unmount");
    return false;
}

fs_ident_t::mount_result_t VFS::mount(Volume* vol, const char* where) {
    auto sysid = vol->sysid();
    
    for (auto i = 0u; true; ++i) {
        auto&& fsinfo = gKnownFilesystemTypes[i];
        if (fsinfo.sysid == 0 && fsinfo.type == nullptr && fsinfo.fmount == nullptr) break;
        if (fsinfo.sysid == sysid) {
            if (fsinfo.fmount == nullptr) {
                LOG_DEBUG("found a match for volume 0x%p but no mounting helper", vol);
            } else {
                LOG_DEBUG("volume 0x%p matched mounter 0x%p for type %u", vol, fsinfo.fmount, sysid);
                auto mountinfo = fsinfo.fmount(vol, where);
                return mountinfo;
            }
        }
    }

    return {false, nullptr};
}

pair<Filesystem*, const char*> VFS::getfs(const char* root) {
    if (root == nullptr || root[0] == 0) return {nullptr, nullptr};
    if (root[0] == '/') ++root;
    auto b = mMounts.begin(), e = mMounts.end();
    for(; b != e; ++b) {
        auto&& m = *b;
        auto next = strprefix(m.path, root);
        if (next != nullptr) {
            LOG_DEBUG("found matching root fs %s (at 0x%p) - next = %s", m.path, m.fs, next);
            return {m.fs, next};
        }
    }

    return {nullptr, nullptr};
}

VFS::filehandle_t VFS::open(const char* path, uint32_t mode) {
    if (!isAbsolutePath(path)) return {nullptr, nullptr};

    auto rest = getfs(path);
    if (rest.first == nullptr) {
        LOG_DEBUG("could not find filesystem to open %s", path);
        return {nullptr, nullptr};
    }

    return {rest.first, rest.first->open(rest.second, mode)};
}

bool VFS::del(const char* path) {
    if (!isAbsolutePath(path)) return false;

    auto rest = getfs(path);
    if (rest.first == nullptr) {
        LOG_DEBUG("could not find filesystem to delete %s", path);
        return false;
    }

    return rest.first->del(rest.second);
}

bool VFS::mkdir(const char* path) {
    if (!isAbsolutePath(path)) return false;

    auto rest = getfs(path);
    if (rest.first == nullptr) {
        LOG_DEBUG("could not find filesystem to create %s", path);
        return false;
    }

    return rest.first->mkdir(rest.second);
}

// TODO: VFS should be itself a filesystem; for now this is just used
// to cleanup instances of RootDirectory
class VFSFilesystem : public Filesystem {
public:
        File* open(const char*, uint32_t) {
            return nullptr;
        }

        Directory* opendir(const char*) {
            return nullptr;
        }

        void doClose(FilesystemObject* obj) {
            delete obj;
        }
};

static VFSFilesystem gVFSFilesystem;

class RootDirectory : public Filesystem::Directory {
    public:
        RootDirectory() : mIterator(VFS::get().mMounts.begin()), mEnd(VFS::get().mMounts.end()) {
            mTime = TimeManager::get().UNIXtime();
        }

        bool doStat(stat_t& stat) {
            stat.kind = file_kind_t::directory;
            stat.size = 0;
            stat.time = mTime;
            return true;
        }

        bool next(fileinfo_t& fi) {
            if (mIterator == mEnd) return false;

            auto&& mi = *mIterator;

            bzero(fi.name, sizeof(fi.name));
            strncpy(fi.name, mi.path, gMaxPathSize);
            fi.kind = Filesystem::FilesystemObject::kind_t::directory;
            fi.size = 0;
            fi.time = mi.when;

            ++mIterator;

            return true;
        }
    private:
        decltype(VFS::mMounts)::iterator mIterator;
        decltype(VFS::mMounts)::iterator mEnd;
        uint64_t mTime;
};

VFS::filehandle_t VFS::opendir(const char* path) {
    if (!isAbsolutePath(path)) return {nullptr, nullptr};

    LOG_DEBUG("asked to opendir '%s'", path);
    if (path[0] == '/') {
        // asked to open the root directory
        if (path[1] == 0) {
            return {&gVFSFilesystem, new RootDirectory()};
        } else {
            // just look for the mountpoint and delegate
            ++path;
        }
    }
    auto b = mMounts.begin(), e = mMounts.end();
    for(; b != e; ++b) {
        auto&& m = *b;
        auto next = strprefix(m.path, path);
        if (next != nullptr) {
            LOG_DEBUG("found matching root fs %s (at 0x%p) - forwarding open request of %s", m.path, m.fs, next);
            return {m.fs, m.fs->opendir(next)};
        }
    }

    LOG_DEBUG("no matching filesystem found - failure");
    return {nullptr, nullptr};
}

bool VFS::isAbsolutePath(const char* path) {
    if (path == nullptr) return false;
    if (path[0] != '/') return false;
    if (nullptr != strstr(path, "/.")) return false;

    return true;
}
