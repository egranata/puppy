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
#include <kernel/log/log.h>
#include <kernel/process/current.h>
#include <kernel/libc/sprint.h>
#include <kernel/libc/ringbuffer.h>

LOG_TAG(USERSPACE, 0);

syscall_response_t klog_syscall_handler(const char* msg) {
    static constexpr size_t gBufferSize = 300;
    char buffer[gBufferSize] = {0};

    sprint(&buffer[0], gBufferSize, "(pid=%u) %s", gCurrentProcess->pid, msg);
    TAG_ERROR(USERSPACE, "%s", &buffer[0]);    
    return OK;
}


log_stats_t read_log_stats();

syscall_response_t klogread_syscall_handler(char *dest, size_t size, klog_stats_t* stats) {
    if (stats) {
        log_stats_t ls = read_log_stats();
        stats->numentries = ls.num_log_entries;
        stats->totalwritten = ls.total_log_size;
        stats->largestentry = ls.max_log_entry_size;
    }
    LogBuffer *logbuf = get_log_buffer();
    if (logbuf) {
        if (dest && size) {
            logbuf->read(dest, size);
        }
        return OK;
    } else {
        return ERR(NO_SUCH_OBJECT);
    }
}
