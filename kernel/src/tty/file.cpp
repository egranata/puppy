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

#include <kernel/tty/file.h>
#include <kernel/process/manager.h>
#include <kernel/log/log.h>

TTYFile::TTYFile(TTY* tty) : mTTY(tty) {}

bool TTYFile::seek(size_t) {
    return false;
}

bool TTYFile::read(size_t n, char* b) {
    for(;n > 0;++b,--n) {
ch:     auto c = mTTY->read();
        if (c == -1) {
            ProcessManager::get().yield();
            goto ch;
        }
        *b = c;
    }

    return true;
}

bool TTYFile::write(size_t, char* buffer) {
    mTTY->write(buffer);
    return true;
}

bool TTYFile::stat(stat_t&) {
    return false;
}

uintptr_t TTYFile::ioctl(uintptr_t a1, uintptr_t a2) {
    LOG_DEBUG("ttyfile %p ioctl(%u,%u)", this, a1, a2);
    switch (a1) {
        case IOCTL_FOREGROUND:
            // TODO: should processes be allowed to push themselves to the foreground freely?
            mTTY->pushfg((uint16_t)a2);
            return 1;
        case IOCTL_BACKGROUND:
            mTTY->popfg();
            return 1;
        case IOCTL_MOVECURSOR: {
            uint16_t row = a2 & 0xFFFF;
            uint16_t col = a2 >> 16;
            mTTY->setPosition(row, col);
            return 1;
        }
        default:
            return 0;
    }
}

