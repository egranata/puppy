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

#include <kernel/fs/devfs/devfs.h>
#include <kernel/libc/str.h>

DevFS& DevFS::get() {
    static DevFS gDevFS;
    return gDevFS;
}

MemFS::Directory* DevFS::getRootDirectory() {
    return mFilesystem.root();
}

MemFS::Directory* DevFS::getDeviceDirectory(const char* device) {
    string _devices(device);
    auto found = mFilesystem.root()->get(_devices.buf());
    if (found != nullptr && found->kind() != Filesystem::FilesystemObject::kind_t::directory) {
        return (MemFS::Directory*)found;
    }
    auto newdir = new MemFS::Directory(device);
    return mFilesystem.root()->add(newdir), newdir;
}

MemFS* DevFS::getMemFS() {
    return &mFilesystem;
}
