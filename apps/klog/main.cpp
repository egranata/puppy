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

#include <libuserspace/memory.h>
#include <libuserspace/printf.h>
#include <libuserspace/stdio.h>
#include <libuserspace/syscalls.h>
#include <muzzle/stdlib.h>

int main(int, const char**) {
    // TODO: this should be factored out somewhere
    static constexpr size_t gLogBufferSize = 64 * 1024;

    klog_stats_t log_stats;

    char *buffer = (char*)calloc(gLogBufferSize, 1);

    cwrite("Writing log entries to stdout.. Press a key to keep scrolling\n");

    size_t n_written = 0;

    if (0 == klogread_syscall(buffer, gLogBufferSize, &log_stats)) {
        printf("Total entries to log: %llu\nTotal written to log: %llu bytes (of which the largest individual entry was %u bytes)\n", log_stats.numentries,
               log_stats.totalwritten, log_stats.largestentry);
        for (auto i = 0u; i < gLogBufferSize; ++i) {
            auto c = buffer[i];
            if (c == '\n') ++n_written;
            putchar(c);
            if (n_written == 5) {
                while (-1 == getchar());
                n_written = 0;
            }
        }
    }

    return 0;
}
