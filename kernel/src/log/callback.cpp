// Copyright 2019 Google LLC
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

#include <kernel/log/log.h>
#include <kernel/synch/msgqueue.h>
#include <kernel/fs/filesystem.h>

namespace boot::logd {
    Filesystem::File *gMessageFile;

    void log_callback(kernel_log_msg_t, void*) {
    }

    uint32_t init() {
        gMessageFile = MessageQueueFS::get()->open("/kernel_log", FILE_OPEN_WRITE);
        set_log_callback(boot::logd::log_callback, nullptr);

        return 0;
    }
}
