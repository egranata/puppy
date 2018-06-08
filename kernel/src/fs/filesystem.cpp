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

#include <fs/filesystem.h>

Filesystem::FilesystemObject::FilesystemObject(kind_t kind) : mKind(kind) {}

Filesystem::FilesystemObject::kind_t Filesystem::FilesystemObject::kind() const {
    return mKind;
}

void Filesystem::FilesystemObject::kind(kind_t k) {
    mKind = k;
}

Filesystem::File::File() : FilesystemObject(Filesystem::FilesystemObject::kind_t::file) {}
Filesystem::Directory::Directory() : FilesystemObject(Filesystem::FilesystemObject::kind_t::directory) {}

uintptr_t Filesystem::File::ioctl(uintptr_t, uintptr_t) {
    return 0;
}
